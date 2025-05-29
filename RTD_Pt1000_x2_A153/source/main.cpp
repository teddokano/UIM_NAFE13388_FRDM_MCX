/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"utils.h"

enum gain {
	GAIN_0_2	= 0,
	GAIN_0_4,
	GAIN_0_8,
	GAIN_1_0,
	GAIN_2_0,
	GAIN_4_0,
	GAIN_8_0,
	GAIN_16_0,
};

enum iexc_idx {
	iexc250	=  0,
	iexc500,
	iexc1000,
};

enum iexc {
	uA250	=  9 << 10,
	uA500	= 11 << 10,
	uA1000	= 13 << 10,
};

uint16_t	ie[]		= { uA250, uA500, uA1000 };
double 		gain_val[]	= { 0.2, 0.4, 0.8, 1, 2, 4, 8, 16 };
double	 	iexc_val[]	= { 250e-6, 500e-6, 1000e-6 };

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );


constexpr	int	length	= 64;

double	nc[ length ];
double	uc[ length ];

constexpr uint16_t	CC_0	= 0x0011;
constexpr uint16_t	CC_1	= 0x0064;
constexpr uint16_t	CC_2	= 0x8400;
constexpr uint16_t	CC_3	= 0x8200;

constexpr int	I_EXC	= uA250;
//constexpr int	GAIN	= GAIN_2_0;
constexpr int	GAIN	= GAIN_8_0;

constexpr int	EXC0	= 1;
constexpr int	DIFF0	= 2;
constexpr int	EXC1	= 3;
constexpr int	DIFF1	= 4;
constexpr int	ACOM	= 7;


uint16_t	meas[ 4 ][ 4 ]	= {
	{ DIFF0 << 12 | DIFF0 << 8 | GAIN << 5 | CC_0, 0x8000 | CC_1, CC_2, I_EXC | EXC0 << 3 | 0x0 << 0 | CC_3 },
	{  ACOM << 12 | DIFF0 << 8 | GAIN << 5 | CC_0, 0x8000 | CC_1, CC_2, I_EXC | EXC0 << 3 | 0x0 << 0 | CC_3 },
	{ DIFF1 << 12 | DIFF1 << 8 | GAIN << 5 | CC_0, 0x8000 | CC_1, CC_2, I_EXC | EXC1 << 3 | 0x0 << 0 | CC_3 },
	{  ACOM << 12 | DIFF1 << 8 | GAIN << 5 | CC_0, 0x8000 | CC_1, CC_2, I_EXC | EXC1 << 3 | 0x0 << 0 | CC_3 },
};

uint32_t	fc_gain_coeff[ 16 ];
uint32_t	gain_cef[ 3 ][ 16 ];
uint32_t	ofst_cef[ 3 ][ 16 ];

void set_gain_and_iexc( int g_idx, int i_idx )
{
	for ( auto i = 0; i < 4; i++ )
	{
		meas[ i ][ 0 ]	&= ~0x00E0;
		meas[ i ][ 0 ]	|= g_idx << 5;

		meas[ i ][ 3 ]	&= ~0x3C00;
		meas[ i ][ 3 ]	|= ie[ i_idx ];
	}

	for ( auto i = 0; i < 4; i++ )
		afe.open_logical_channel( i, meas[ i ] );

#if 0
	afe.reg( OFFSET_COEFF8, ofst_cef[ 0 ][ GAIN ] );
	afe.reg( GAIN_COEFF8,   gain_cef[ 0 ][ GAIN ] );
#else
	afe.reg( OFFSET_COEFF8, ofst_cef[ i_idx ][ g_idx ] );
	afe.reg( GAIN_COEFF8,   gain_cef[ i_idx ][ g_idx ] );
#endif
}

#define	MEAS_Pt1000
//#define	RF1_is_100ohm
#if 1
double	voltage2resistance( double v, int i_idx )
{
#ifdef	MEAS_Pt1000
#ifdef	RF1_is_100ohm
	static constexpr double	rf1	= 100.0;
#else
	static constexpr double	rf1	= 2400.0;
#endif
	static constexpr double	k	= 530000.0 / (1000.0 + rf1 + 530000);
	static constexpr double	rzo	= 530e3;

	return k * -((rzo + rf1) * v) / (v - iexc_val[ i_idx ] * rzo);
#else
	return v / iexc_val[ i_idx ];
#endif
}
#else
double	voltage2resistance( double v, int i_idx )
{
	return v;
}
#endif

int main( void )
{
	/*
	 *		starting NAFE
	 */
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.begin();

	/*
	 *		showing register info
	 */

	uint64_t	sn	= afe.serial_number();

	printf( "part number   = %04lX (revision: %01X)\r\n", afe.part_number(), afe.revision_number() );
	printf( "serial number = %06lX%06lX\r\n", (uint32_t)(sn >> 24), (uint32_t)sn & 0xFFFFFF );	//	to use NewlibNano
	printf( "die temperature = %f℃\r\n", afe.temperature() );


	RegVct	registers	= { OPT_COEF3, OPT_COEF4, OPT_COEF5, OPT_COEF6, OPT_COEF7, OPT_COEF8, OPT_COEF9, OPT_COEF10 };
	reg_dump( GAIN_COEFF0, 32 );
	reg_dump( registers );

	/*
	 *		calculating new coefficients for RTD settings
	 */

	for ( auto g = 0; g < 8; g++ )
		fc_gain_coeff[ g ]	= afe.reg( GAIN_COEFF0 + g );

	uint32_t	ofst_high_gain	= afe.reg( OPT_COEF3 );
	uint32_t	ofst_low_gain	= afe.reg( OPT_COEF9 );
	
	int	gain_ref[]	= { GAIN_16_0, GAIN_8_0, GAIN_0_8 };
	NAFE13388_Base::Register24	gain_iexc[]	= { OPT_COEF4, OPT_COEF6, OPT_COEF10 };

	for ( auto i = 0; i < 3; i++ )
	{
		double	gain_ratio	= (double)afe.reg( GAIN_COEFF0 + gain_ref[ i ] ) / (double)afe.reg( gain_iexc[ i ] );
		
		printf( "%d, %lf, %ld, %ld\r\n", i, gain_ratio, afe.reg( GAIN_COEFF0 + gain_ref[ i ] ), afe.reg( gain_iexc[ i ] ) );

		for ( auto g = 0; g < 8; g++ )
		{
			gain_cef[ i ][ g ]	= round( fc_gain_coeff[ g ] / gain_ratio );
			ofst_cef[ i ][ g ]	= (g < GAIN_1_0) ? ofst_low_gain : ofst_high_gain;
		}
	}

	for ( auto g = 0; g < 8; g++ )
	{
		for ( auto i = 0; i < 3; i++ )
		{
			printf( "   %8ld", gain_cef[ i ][ g ] );
		}
		printf( "\r\n" );
	}

	afe.use_DRDY_trigger( false );	//	default = true

	int32_t	data0;
	int32_t	data1;
	int32_t	data2;
	int32_t	data3;
	
	double	uca;
	double	nca;
	double	result[ 2 ][ 3 ][ 8 ];

	for ( auto c = 0; c < 3; c++ )
	{
		for ( auto g = 0; g < 8; g++ )
		{
			set_gain_and_iexc( g, c );
			printf( "   gain = %4.1lf, iexc = %lg: ", gain_val[ g ], iexc_val[ c ] );

			for ( auto i = 0; i < length; i++ )
			{
				data0	= afe.start_and_read( 0 );
				data1	= afe.start_and_read( 1 );
				data2	= afe.start_and_read( 2 );
				data3	= afe.start_and_read( 3 );

				data0	-= data1;
				data2	-= data3;

				uc[ i ]	= afe.raw2uv( 0, data0 ) / 1e6;
				nc[ i ]	= afe.raw2uv( 2, data2 ) / 1e6;
			}

			uca	= 0;
			nca	= 0;
			
			for ( auto i = 0; i < length; i++ )
			{
				uca	+= uc[ i ];
				nca	+= nc[ i ];
			}

			uca	/= (double)length;
			nca	/= (double)length;

			result[ 0 ][ c ][ g ]	= voltage2resistance( uca, c );
			result[ 1 ][ c ][ g ]	= voltage2resistance( nca, c );

			printf( "   @A2P/A2N: %12.8lf Ω, ", result[ 0 ][ c ][ g ] );
			printf( "   @A4P/A4N: %12.8lf Ω, ", result[ 1 ][ c ][ g ] );
			printf( "\r\n" );
		}
	}

	for ( auto i = 0; i < 2; i++ )
	{
		for ( auto c = 0; c < 3; c++ )
		{
//			printf( "\r\n ======== ch = %d, iexc = %lg: ========\r\n", i, iexc_val[ c ] );
			for ( auto g = 0; g < 8; g++ )
			{
				printf( "   %12.8lf\r\n", result[ i ][ c ][ g ] );
			}
		}

	}

	afe.begin();
	printf( "done.\r\n" );

}
