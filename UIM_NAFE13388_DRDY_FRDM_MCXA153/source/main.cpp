/** NXP Analog Front End class library for MCX
 *
 *  @author  Tedd OKANO
 *
 *  Copyright: 2024 Tedd OKANO
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"NAFE13388_UIM.h"

using 	microvolt_t	= NAFE13388_UIM::microvolt_t;
using 	raw_t		= NAFE13388_UIM::raw_t;

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );
InterruptIn		ADC_nDRDY( D4 );			//	Uses interrupt by ADC_nDRDY pin
volatile bool	drdy_wait;

constexpr uint32_t	timeout_limit	= 100000000;

void	DRDY_int_handler( void );
void	logical_ch_config_view( int channel );
void	register16_dump( const std::vector<uint16_t> &reg_list );

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	volatile auto	timeout_count	= timeout_limit;

	spi.frequency( 1000 * 1000 );
	spi.mode( 1 );

	afe.begin();

	afe.logical_ch_config( 0, 0x1070, 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 1, 0x2070, 0x0084, 0x2900, 0x0000 );

	logical_ch_config_view( 0 );
	logical_ch_config_view( 1 );

	ADC_nDRDY.rise( DRDY_int_handler );

	while ( true )
	{
		for ( auto ch = 0; ch < 2; ch++ )
		{
			drdy_wait		= true;
			timeout_count	= timeout_limit;

			afe.start( ch );

			while ( drdy_wait && --timeout_count )
				;

			if ( !timeout_count )
			{
				printf( "DRDY signal wait timeout\r\n" );
				continue;
			}

			// Enable one of following lines
			//printf( " %11.2f,", afe.read<microvolt_t>( ch ) );
			printf( " %8ld,",   afe.read<raw_t>( ch ) );
		}
		printf( "\r\n" );
		wait( 0.05 );
	}
}

void DRDY_int_handler( void )
{
	drdy_wait	= false;
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
