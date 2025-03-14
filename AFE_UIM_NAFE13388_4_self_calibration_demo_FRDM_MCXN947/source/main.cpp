/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

using enum	NAFE13388_UIM::Register16;
using enum	NAFE13388_UIM::Register24;
using enum	NAFE13388_UIM::Command;

void	reg_dump( NAFE13388_UIM::Register24 addr, int length );
void	logical_ch_config_view( void );
void	table_view( int size, int cols, std::function<void(int)> view, std::function<void(void)> linefeed = nullptr );

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

void reg_dump( NAFE13388_UIM::Register24 addr, int length )
{
	table_view( length, 4, [ & ]( int v ){ printf( "  %8ld @ 0x%04X", afe.reg( v + addr ), static_cast<int>( v + addr ) ); }, [](){ printf( "\r\n" ); });
}

void logical_ch_config_view( void )
{
	uint16_t en_ch_bitmap	= afe.reg( CH_CONFIG4 );

	for ( auto channel = 0; channel < 16; channel++ )
	{
		printf( "  logical channel %2d : ", channel );

		if ( en_ch_bitmap & (0x1 << channel) )
		{
			afe.command( channel );
			table_view( 4, 4, []( int v ){ printf( "  0x%04X: 0x%04X", static_cast<int>( v + CH_CONFIG0 ), afe.reg( v + CH_CONFIG0 ) ); }, [](){ printf( "\r\n" ); } );
		}
		else
		{
			printf(  "  (disabled)\r\n" );
		}
	}
}

void table_view( int length, int cols, std::function<void(int)> value, std::function<void(void)> linefeed )
{
	const auto	raws	= (int)(length + cols - 1) / cols;

	for ( auto i = 0; i < raws; i++  )
	{
		if ( i )
		{
			if ( linefeed )
				linefeed();
			else
				printf( "\r\n" );
		}

		for ( auto j = 0; j < cols; j++  )
		{
			auto	index	= i + j * raws;

			if ( index < length  )
				value( index );
		}
	}

	if ( linefeed )
		linefeed();
	else
		printf( "\r\n" );
}
