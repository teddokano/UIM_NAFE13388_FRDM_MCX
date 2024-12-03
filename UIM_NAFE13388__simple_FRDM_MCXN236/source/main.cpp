/** NXP Analog Front End class library for MCX
 *
 *  @author  Tedd OKANO
 *
 *  Copyright: 2023 - 2024 Tedd OKANO
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"NAFE13388_UIM.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );
InterruptIn		ADC_nDRDY( D3 );			//	ADC_nDRDY is re-routed to D3 by pin-adapter

void	logical_ch_config_view( int channel );
double	single_read( int channel );

enum	output_type	{ RAW, MICRO_VOLT };

using 	microvolt_t	= NAFE13388_UIM::microvolt_t;
using 	raw_t		= NAFE13388_UIM::raw_t;

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1000 * 1000 );
	spi.mode( 1 );

	afe.begin();

	afe.logical_ch_config( 0, 0x1070, 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 1, 0x2070, 0x0084, 0x2900, 0x0000 );

	logical_ch_config_view( 0 );
	logical_ch_config_view( 1 );

//	constexpr bool output_type_selection	= MICRO_VOLT;
	constexpr bool output_type_selection	= RAW;

	if ( output_type_selection == MICRO_VOLT )
		printf( "values in micro-volt\r\n" );
	else
		printf( "values in raw\r\n" );

	while ( true )
	{		
		if ( output_type_selection == MICRO_VOLT )
			printf( "%11.2f, %11.2f\r\n", afe.read<microvolt_t>( 0, 0.01 ), afe.read<microvolt_t>( 1, 0.01 ) );
		else
			printf( "%8ld, %8ld\r\n", afe.read<raw_t>( 0, 0.01 ), afe.read<raw_t>( 1, 0.01 ) );

		wait( 0.05 );
	}
}

void logical_ch_config_view( int channel )
{
	printf( "logical channel: %02d\r\n", channel );

	afe.write_r16( channel );

	printf( "  0x%04X\r\n", afe.read_r16( 0x0020 ) );
	printf( "  0x%04X\r\n", afe.read_r16( 0x0021 ) );
	printf( "  0x%04X\r\n", afe.read_r16( 0x0022 ) );
	printf( "  0x%04X\r\n", afe.read_r16( 0x0023 ) );
	printf( "\r\n" );
}
