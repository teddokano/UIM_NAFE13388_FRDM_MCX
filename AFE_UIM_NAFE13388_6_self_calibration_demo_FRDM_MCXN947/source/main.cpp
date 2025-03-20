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
	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.begin();

	uint64_t	sn	= afe.serial_number();

	printf( "part number   = %04lX (revision: %01X)\r\n", afe.part_number(), afe.revision_number() );
	printf( "serial number = %06lX%06lX\r\n", (uint32_t)(sn >> 24), (uint32_t)sn & 0xFFFFFF );	//	to use NewlibNano
	printf( "die temperature = %f℃\r\n", afe.temperature() );

	//
	//	gain/offset coefficient settings
	//

	printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers default values ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );

	//	on-board re-calibration for "PGA_gain = 0.2" coefficients

	for ( auto i = 0; i < 8; i++ )
	{
		if ( afe.self_calibrate( i ) )
			printf( "Error\r\n" );;
	}

	printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers after on-board calibration ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );

	printf( "\r\ndemo completed\r\n" );
}
