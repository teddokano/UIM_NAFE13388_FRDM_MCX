/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS

#ifdef	TARGET_C444
NAFE13388_UIM	afe( spi, 0, false, D3, D5, D6, D7 );
#else
NAFE13388_UIM	afe( spi );
#endif

using enum	NAFE13388_UIM::Register16;
using enum	NAFE13388_UIM::Register24;
using enum	NAFE13388_UIM::Command;

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.command( CMD_RESET );

	while ( !(afe.reg( SYS_STATUS0 ) & 0x1 << 13) )	//	wait chip ready
		;

	afe.command( CMD_CH0 );
	afe.reg( CH_CONFIG0, 0x1710 );
	afe.reg( CH_CONFIG1, 0x00A4 );
	afe.reg( CH_CONFIG2, 0x4C00 );
	afe.reg( CH_CONFIG3, 0x0000 );

	afe.command( CMD_CH1 );
	afe.reg( CH_CONFIG0, 0x2710 );
	afe.reg( CH_CONFIG1, 0x00A4 );
	afe.reg( CH_CONFIG2, 0x4C00 );
	afe.reg( CH_CONFIG3, 0x0000 );

	afe.reg( CH_CONFIG4, 0x0003 );

	while ( true )
	{
		for ( auto ch = 0; ch < 2; ch++ )
		{
			afe.command( ch ? CMD_CH1 : CMD_CH0 );
			afe.command( CMD_SS );
			wait( 0.1 );

			uint32_t	data	= afe.reg( ch ? CH_DATA1 : CH_DATA0 );

			printf( "     channel %2d: %8ld", ch, data );
		}
		printf( "\r\n" );
		wait( 0.5 );
	}
}
