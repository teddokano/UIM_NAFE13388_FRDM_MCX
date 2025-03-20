/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"utils.h"
#include	"PrintOutput.h"

extern	NAFE13388_UIM	afe;
extern	PrintOutput		out;

void reg_dump( RegVct reg_vctr )
{
	for ( auto r : reg_vctr )
	{
		if ( const NAFE13388_UIM::Register24 *ap	= std::get_if<NAFE13388_UIM::Register24>( &r ) )
		{
		    printf( "0x%04X: 0x%06lX\r\n", static_cast<int>( *ap ), afe.reg( *ap ) );
		}
		else if ( const NAFE13388_UIM::Register16 *ap	= std::get_if<NAFE13388_UIM::Register16>( &r ) )
		{
		    printf( "0x%04X: 0x%04X\r\n", static_cast<int>( *ap ), afe.reg( *ap ) );
		}
	}
}

void reg_dump( NAFE13388_UIM::Register24 addr, int length )
{
	table_view( length, 4, [ & ]( int v ){ printf( "  %8ld @ 0x%04X", afe.reg( v + addr ), v + (uint16_t)addr ); }, [](){ printf( "\r\n" ); });
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
			table_view( 4, 4, []( int v ){ printf( "  0x%04X @0x%04X", afe.reg( v + CH_CONFIG0 ), (uint16_t)(v + CH_CONFIG0) ); } );
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
