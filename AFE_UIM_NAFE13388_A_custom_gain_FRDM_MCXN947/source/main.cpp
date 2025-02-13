/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	<math.h>
#include	<time.h>

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

void	logical_ch_config_view( int channel );
void	register16_dump( const std::vector<uint16_t> &reg_list );
void	register24_dump( const std::vector<uint16_t> &reg_list );

enum	output_type	{ RAW, MICRO_VOLT };

constexpr	int	GAIN_COEFF			= 0x80;
constexpr	int	OFFSET_COEFF		= 0x90;

constexpr	int	INPUT_GND			= 0x0010;
constexpr	int	INPUT_A1P_SINGLE	= 0x1010;

constexpr	int	CALIB_FOR_PGA_0_2	= 0;
constexpr	int	CALIB_NONE			= 8;
constexpr	int	CALIB_NONE_5V		= 9;
constexpr	int	CALIB_NONE_10V		= 10;
constexpr	int	CALIB_CUSTOM_5V		= 11;
constexpr	int	CALIB_CUSTOM_10V	= 12;
constexpr	int	CALIB_NONE_1V_5V	= 13;
constexpr	int	CALIB_CUSTOM_1V_5V	= 14;

using 	microvolt_t	= NAFE13388_UIM::microvolt_t;
using 	raw_t		= NAFE13388_UIM::raw_t;

typedef struct	_point	{
	uint32_t	data;
	float		voltage;
} point;

typedef struct	_ref_points	{
	int		coeff_index;
	point	high;
	point	low;
	int		cal_index;
} ref_points;

void	gain_offset_coeff( int coeff_index, ref_points ref );

void gain_offset_coeff( ref_points ref )
{
	constexpr float		pga1x_voltage		= 5.0;
	constexpr int		adc_resolution		= 24;
	constexpr float		pga_gain_setting	= 0.2;

	constexpr float		fullscale_voltage	= pga1x_voltage / pga_gain_setting;

	float	fullscale_data		= pow( 2, (adc_resolution - 1) ) - 1.0;
	float	ref_data_span		= ref.high.data		- ref.low.data;
	float	ref_voltage_span	= ref.high.voltage	- ref.low.voltage;
	
	float	dv_slope			= ref_data_span / ref_voltage_span;
	float	custom_gain			= dv_slope * (fullscale_voltage / fullscale_data);
	float	custom_offset		= (dv_slope - ref.low.data * ref.low.voltage) / custom_gain;
	
	uint32_t	gain_coeff_cal		= afe.read_r24( GAIN_COEFF   + ref.cal_index );
	uint32_t	offsset_coeff_cal	= afe.read_r24( OFFSET_COEFF + ref.cal_index );
	uint32_t	gain_coeff_new		= gain_coeff_cal * custom_gain;
	uint32_t	offset_coeff_new	= offsset_coeff_cal - custom_offset;
	
	printf( "ref_point_high = %8ld @%6.3f\r\n", ref.high.data, ref.high.voltage );
	printf( "ref_point_low  = %8ld @%6.3f\r\n", ref.low.data,  ref.low.voltage  );
	printf( "gain_coeff_new   = %8ld\r\n", gain_coeff_new   );
	printf( "offset_coeff_new = %8ld\r\n", offset_coeff_new );
	
	afe.write_r24( GAIN_COEFF   + ref.coeff_index, gain_coeff_new   );
	afe.write_r24( OFFSET_COEFF + ref.coeff_index, offset_coeff_new );
}


int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1000 * 1000 );
	spi.mode( 1 );

	afe.begin();

	afe.logical_ch_config(  0, INPUT_GND       , (CALIB_NONE         << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  1, INPUT_GND       , (CALIB_NONE_5V      << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  2, INPUT_GND       , (CALIB_NONE_10V     << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  3, INPUT_GND       , (CALIB_FOR_PGA_0_2  << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  4, INPUT_GND       , (CALIB_CUSTOM_5V    << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  5, INPUT_GND       , (CALIB_CUSTOM_10V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  6, INPUT_A1P_SINGLE, (CALIB_NONE         << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  7, INPUT_A1P_SINGLE, (CALIB_NONE_5V      << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  8, INPUT_A1P_SINGLE, (CALIB_NONE_10V     << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  9, INPUT_A1P_SINGLE, (CALIB_FOR_PGA_0_2  << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 10, INPUT_A1P_SINGLE, (CALIB_CUSTOM_5V    << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 11, INPUT_A1P_SINGLE, (CALIB_CUSTOM_10V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 12, INPUT_A1P_SINGLE, (CALIB_NONE_1V_5V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 13, INPUT_A1P_SINGLE, (CALIB_CUSTOM_1V_5V << 12) | 0x0084, 0x2900, 0x0000 );

	constexpr float read_delay	= 0.01;

	logical_ch_config_view( 0 );

	//
	//	showing gain/offset coefficients
	//

	std::vector<uint16_t>	registers_list( 32 );
	
	for ( auto i = 0; i < 32; i++ )
		registers_list[ i ]	= 0x80 + i;

	printf( "=== before overwrite ===\r\n" );
	register24_dump( registers_list );

	//
	//	gain/offset customization
	//

	//	for output (23^-1) @ 25V, no calibration
	afe.write_r24( GAIN_COEFF   + CALIB_NONE,        afe.read_r24( GAIN_COEFF   + CALIB_NONE        ) );
	afe.write_r24( OFFSET_COEFF + CALIB_NONE,        afe.read_r24( OFFSET_COEFF + CALIB_NONE        ) );

	//	for output 2000 @ 5V, no calibration
	afe.write_r24( GAIN_COEFF   + CALIB_NONE_5V,     5000 );
	afe.write_r24( OFFSET_COEFF + CALIB_NONE_5V,        0 );

	//	for output 2000 @ 10V, no calibration
	afe.write_r24( GAIN_COEFF   + CALIB_NONE_10V,    2500 );
	afe.write_r24( OFFSET_COEFF + CALIB_NONE_10V,       0 );

	//	for output (23^-1) @ 25V, with calibration
	afe.write_r24( GAIN_COEFF   + CALIB_FOR_PGA_0_2, afe.read_r24( GAIN_COEFF   + CALIB_FOR_PGA_0_2 ) );
	afe.write_r24( OFFSET_COEFF + CALIB_FOR_PGA_0_2, afe.read_r24( OFFSET_COEFF + CALIB_FOR_PGA_0_2 ) );

	//	for output 2000 @ 5V, with calibration
	afe.write_r24( GAIN_COEFF   + CALIB_CUSTOM_5V,   round( afe.read_r24( GAIN_COEFF   + CALIB_FOR_PGA_0_2 ) * 2000.0 / 1677721.4 ) );
	afe.write_r24( OFFSET_COEFF + CALIB_CUSTOM_5V,          afe.read_r24( OFFSET_COEFF + CALIB_FOR_PGA_0_2 )                        );

	//	for output 2000 @ 10V, with calibration
	afe.write_r24( GAIN_COEFF   + CALIB_CUSTOM_10V,  round( afe.read_r24( GAIN_COEFF   + CALIB_FOR_PGA_0_2 ) * 1000.0 / 1677721.4 ) );
	afe.write_r24( OFFSET_COEFF + CALIB_CUSTOM_10V,         afe.read_r24( OFFSET_COEFF + CALIB_FOR_PGA_0_2 )                        );
	
	//	for output 1V - 5V, no calibration
	afe.write_r24( GAIN_COEFF   + CALIB_NONE_1V_5V, 6247 );
	afe.write_r24( OFFSET_COEFF + CALIB_NONE_1V_5V,   16.0 / 0.00148937 );

	//	for output 1V - 5V, no calibration
	afe.write_r24( GAIN_COEFF   + CALIB_CUSTOM_1V_5V, round( afe.read_r24( GAIN_COEFF   + CALIB_FOR_PGA_0_2 ) * 6247 ) );
	afe.write_r24( OFFSET_COEFF + CALIB_CUSTOM_1V_5V,   afe.read_r24( OFFSET_COEFF + CALIB_FOR_PGA_0_2 )   + 16 - ((2015.0 - 16) / (5.0 - 1.0)) );

	
	ref_points	r[]	= {
						{ CALIB_CUSTOM_5V,    { 2000,  5.0 }, {  0, 0 }, CALIB_FOR_PGA_0_2 },
						{ CALIB_CUSTOM_10V,   { 2000, 10.0 }, {  0, 0 }, CALIB_FOR_PGA_0_2 },
						{ CALIB_CUSTOM_1V_5V, { 2015,  5.0 }, { 16, 1 }, CALIB_FOR_PGA_0_2 },
					};
	
	gain_offset_coeff( r[ 0 ] );
	gain_offset_coeff( r[ 1 ] );
	gain_offset_coeff( r[ 2 ] );
	
while ( 1 );	
	
	printf( "=== after overwrite ===\r\n" );
	register24_dump( registers_list );

	//
	//	open file for output
	//

	time_t	current_time;
	current_time	= time( NULL );

	char	filename[ 100 ];
	sprintf( filename, "test %s.csv", ctime( &current_time ) );

	FILE	*fp;
	if ( NULL == (fp	= fopen( filename, "w" )) )
	{
		printf( "file open error\r\n" );
		return 0;
	}

	//
	//	operation with customized gain/offset
	//

	printf( "     count" );
	printf( "      NONE" );
	printf( "   NONE_5V" );
	printf( "  NONE_10V" );
	printf( "  Cal_dflt" );
	printf( "    Cal_5V" );
	printf( "   Cal_10V" );

	printf( "      NONE" );
	printf( "   NONE_5V" );
	printf( "  NONE_10V" );
	printf( "  Cal_dflt" );
	printf( "    Cal_5V" );
	printf( "   Cal_10V" );

	printf( "\r\n" );

	long	count	= 0;
	raw_t	data;
	
	while ( true )
	{
		 printf(     " %8ld, ", count );
		fprintf( fp, " %8ld, ", count );
		count++;
		
		for ( auto ch = 0; ch < 14; ch++ )
		{
			data	= afe.read<raw_t>( ch, read_delay );
			 printf(     " %8ld,", data );
			fprintf( fp, " %8ld,", data );
		}
		 printf(     "\r\n" );
		fprintf( fp, "\r\n" );

		wait( 0.05 );
	}
}

void logical_ch_config_view( int channel )
{
	printf( "logical channel %02d\r\n", channel );
	afe.write_r16( channel );

	std::vector<uint16_t>	reg_list = { 0x0020, 0x0021, 0x0022, 0x0023 };
	register16_dump( reg_list );

	printf( "\r\n" );
}

void register16_dump( const std::vector<uint16_t> &reg_list )
{
	for_each(
		reg_list.begin(),
		reg_list.end(),
		[]( auto reg ) { printf( "  0x%04X: 0x%04X\r\n", reg, afe.read_r16( reg ) ); }
	);
}

void register24_dump( const std::vector<uint16_t> &reg_list )
{
	for_each(
		reg_list.begin(),
		reg_list.end(),
		//[]( auto reg ) { printf( "  0x%04X: 0x%06lX\r\n", reg, afe.read_r24( reg ) ); }
		[]( auto reg ) { printf( "  0x%04X: %ld\r\n", reg, afe.read_r24( reg ) ); }
	);
}
