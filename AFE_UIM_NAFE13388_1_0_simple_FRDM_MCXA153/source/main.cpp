/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

using	microvolt_t		= NAFE13388_Base::microvolt_t;
using	LogicalChannel	= NAFE13388_UIM::LogicalChannel;

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.begin();

	LogicalChannel	lc0( afe, 0, 0x1710, 0x00A4, 0xBC00, 0x0000 );
	LogicalChannel	lc1( afe, 1, 0x2710, 0x00A4, 0xBC00, 0x0000 );

	afe.use_DRDY_trigger( false );	//	default = true

	printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );

	microvolt_t	data0;
	microvolt_t	data1;

	while ( true )
	{
		data0	= lc0;	//	read logical channel 0
		data1	= lc1;	//	read logical channel 1

		printf( "   channel 0 : %12.9lfV,   channel 1 : %12.9lfV\r\n", data0 * 1e-6, data1 * 1e-6 );
	}
}
