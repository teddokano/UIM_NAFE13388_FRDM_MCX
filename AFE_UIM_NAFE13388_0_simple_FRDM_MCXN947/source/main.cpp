/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license License
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

void	logical_ch_config_view( void );
void	table_view( const std::vector<uint16_t> &reg_list, std::function<void(int)> func, int cols = 4 );

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

	constexpr float read_delay	= 0.01;

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_channels );
	logical_ch_config_view();

	//
	//	** ENABLE ONE OF NEXT TWO LINES **
	//

	constexpr bool output_type_selection	= MICRO_VOLT;
//	constexpr bool output_type_selection	= RAW;

	if ( output_type_selection == MICRO_VOLT )
		printf( "\r\nvalues in micro-volt\r\n" );
	else
		printf( "\r\nvalues in raw\r\n" );

	while ( true )
	{
		for ( auto ch = 0; ch < 2; ch++ )
		{
			if ( output_type_selection == MICRO_VOLT )
				printf( " %11.2f,", afe.read<microvolt_t>( ch, read_delay ) );
			else
				printf( " %8ld,",   afe.read<raw_t>( ch, read_delay ) );
		}
		printf( "\r\n" );
		wait( 0.05 );
	}
}

void logical_ch_config_view( void )
{
	uint16_t	en_ch_bitmap= afe.read_r16( 0x0024 );
	
	std::vector<uint16_t>	reg_list = { 0x0020, 0x0021, 0x0022, 0x0023 };

	for ( auto channel = 0; channel < 16; channel++ )
	{	
		printf( "  logical channel %2d : ", channel );

		if ( en_ch_bitmap & (0x1 << channel) )
		{
			afe.write_r16( channel );
			table_view( reg_list, []( int v ){ printf(  "  0x%04X @ 0x%04X", afe.read_r16( v ), v ); }, 4 );
		}
		else
		{
			printf(  "  (disabled)\r\n" );
		}
	}
}

void table_view( const std::vector<uint16_t> &reg_list, std::function<void(int)> func, int cols )
{
	const auto	length	= (int)reg_list.size();
	const auto	raws	= (int)(length + cols - 1) / cols;
	
	for ( auto i = 0; i < raws; i++  )
	{
		if ( i )
			printf( "\r\n" );
			
		for ( auto j = 0; j < cols; j++  )
		{
			auto	index	= i + j * raws;
			
			if ( index < length  )
				func( reg_list[ index ] );
		}
	}
	
	printf( "\r\n" );
}
