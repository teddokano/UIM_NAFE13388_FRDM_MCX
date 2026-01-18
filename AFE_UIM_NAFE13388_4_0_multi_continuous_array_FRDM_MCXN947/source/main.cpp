/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"utils.h"

using	microvolt_t	= NAFE13388_UIM::microvolt_t;

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

microvolt_t		dp[ 16 ];
volatile bool	conversion_done	= false;

void drdy_callback( void )
{
	conversion_done	= true;
}

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

	afe.logical_channel[  0 ].configure( cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  1 ].configure( cc0 | 7 << 12 | 1 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  2 ].configure( cc0 | 2 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  3 ].configure( cc0 | 7 << 12 | 2 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  4 ].configure( cc0 | 3 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  5 ].configure( cc0 | 7 << 12 | 3 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  6 ].configure( cc0 | 4 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  7 ].configure( cc0 | 7 << 12 | 4 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  8 ].configure( cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[  9 ].configure( cc0 | 7 << 12 | 1 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 10 ].configure( cc0 | 2 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 11 ].configure( cc0 | 7 << 12 | 2 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 12 ].configure( cc0 | 3 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 13 ].configure( cc0 | 7 << 12 | 3 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 14 ].configure( cc0 | 4 << 12 | 7 << 8, cc1, cc2, cc3 );
	afe.logical_channel[ 15 ].configure( cc0 | 7 << 12 | 4 << 8, cc1, cc2, cc3 );

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
	logical_ch_config_view();

	afe.set_DRDY_callback( drdy_callback );	//	set callback function for when DRDY detected
	afe.DRDY_by_sequencer_done( true );		//	generate DRDY at all logical channel conversions are done

	afe.start_continuous_conversion();

	while ( true )
	{
		if ( conversion_done )
		{
			conversion_done	= false;

			afe.read( dp );

			for ( auto i = 0; i < 14; i++ )
				printf( "  %12.9lfV,", dp[ i ] * 1e-6 );

			printf( "\r\n" );
		}
	}
}
