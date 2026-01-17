/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"utils.h"

using	microvolt_t		= NAFE13388_Base::microvolt_t;
using	LogicalChannel	= NAFE13388_UIM::LogicalChannel;

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

volatile bool		conversion_done	= false;

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

	LogicalChannel	lc[]	= {
		LogicalChannel( afe,  0, cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  1, cc0 | 7 << 12 | 1 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  2, cc0 | 2 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  3, cc0 | 7 << 12 | 2 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  4, cc0 | 3 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  5, cc0 | 7 << 12 | 3 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  6, cc0 | 4 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  7, cc0 | 7 << 12 | 4 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  8, cc0 | 1 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe,  9, cc0 | 7 << 12 | 1 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 10, cc0 | 2 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 11, cc0 | 7 << 12 | 2 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 12, cc0 | 3 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 13, cc0 | 7 << 12 | 3 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 14, cc0 | 4 << 12 | 7 << 8, cc1, cc2, cc3 ),
		LogicalChannel( afe, 15, cc0 | 7 << 12 | 4 << 8, cc1, cc2, cc3 ),
	};

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
	logical_ch_config_view();

	afe.set_DRDY_callback( drdy_callback );	//	set callback function for when DRDY detected
	afe.DRDY_by_sequencer_done( true );		//	generate DRDY at all logical channel conversions are done

	std::vector<microvolt_t>	dv( afe.enabled_logical_channels() );

	afe.start_continuous_conversion();

	while ( true )
	{
		if ( conversion_done )
		{
			conversion_done	= false;

			afe.read( dv );

			for ( auto&& v: dv )
				printf( "  %12.9lfV,", v * 1e-6 );

			printf( "\r\n" );
		}
	}
}
