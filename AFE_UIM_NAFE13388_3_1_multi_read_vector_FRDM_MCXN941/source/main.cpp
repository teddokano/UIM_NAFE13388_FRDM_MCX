/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"utils.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.begin();
	afe.blink_leds();

	uint64_t	sn	= afe.serial_number();

	printf( "part number   = %04lX (revision: %01X)\r\n", afe.part_number(), afe.revision_number() );
	printf( "serial number = %06lX%06lX\r\n", (uint32_t)(sn >> 24), (uint32_t)sn & 0xFFFFFF );	//	to use NewlibNano
	printf( "die temperature = %f℃\r\n", afe.temperature() );

	RegVct	registers	= { PN2, PN1, PN0, SERIAL1, SERIAL0, DIE_TEMP, SYS_CONFIG0, SYS_STATUS0 };
	reg_dump( registers );

	constexpr uint16_t	cc0	= 0x0010;
	constexpr uint16_t	cc1	= 0x00A4;
	constexpr uint16_t	cc2	= 0x4C00;
	constexpr uint16_t	cc3	= 0x0000;

	afe.open_logical_channel(  0, cc0 | 0 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  1, cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  2, cc0 | 2 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  3, cc0 | 3 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  4, cc0 | 4 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  5, cc0 | 5 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  6, cc0 | 6 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  7, cc0 | 7 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  8, cc0 | 1 << 12 | 0 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel(  9, cc0 | 1 << 12 | 1 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 10, cc0 | 1 << 12 | 2 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 11, cc0 | 1 << 12 | 3 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 12, cc0 | 1 << 12 | 4 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 13, cc0 | 1 << 12 | 5 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 14, cc0 | 1 << 12 | 6 << 8, cc1, cc2, cc3 );
	afe.open_logical_channel( 15, cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 );

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
	logical_ch_config_view();

	afe.use_DRDY_trigger( true );	//	default = true

	std::vector<raw_t>	dv( 16 );

	while ( true )
	{
		afe.start_and_read( dv );

		for ( auto& v: dv )
			printf( " %8ld,", v );

		printf( "\r\n" );
	}
}
