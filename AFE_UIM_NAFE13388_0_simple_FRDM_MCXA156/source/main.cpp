/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license License
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

void	logical_ch_config_view( int channel );
void	register16_dump( const std::vector<uint16_t> &reg_list );

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

	logical_ch_config_view( 0 );
	logical_ch_config_view( 1 );

	//
	//	** ENABLE ONE OF NEXT TWO LINES **
	//

	constexpr bool output_type_selection	= MICRO_VOLT;
//	constexpr bool output_type_selection	= RAW;

	if ( output_type_selection == MICRO_VOLT )
		printf( "values in micro-volt\r\n" );
	else
		printf( "values in raw\r\n" );

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
