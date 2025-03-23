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

	afe.open_logical_channel( 0, 0x1710, 0x00A4, 0xBC00, 0x0000 );
	afe.open_logical_channel( 1, 0x2710, 0x00A4, 0xBC00, 0x0000 );

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
	logical_ch_config_view();

	afe.use_DRDY_trigger( true );	//	default = true

	while ( true )
	{
		for ( auto ch = 0; ch < 2; ch++ )
		{
			raw_t	data	= afe.start_and_read( ch );

			printf( "   channel %2d : %8ld (%lfuV),", ch, data, afe.raw2uv( ch, data ) );
		}
		printf( "\r\n" );
	}
}
