/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	<math.h>

#include	"coeffs.h"
#include	"PrintOutput.h"

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

constexpr int	INPUT_GND			= 0x0010;
constexpr int	INPUT_A1P_SINGLE	= 0x1010;

constexpr int	CALIB_FOR_PGA_0_2	= 0;
constexpr int	CALIB_NONE			= 8;
constexpr int	CALIB_NONE_5V		= 9;
constexpr int	CALIB_NONE_10V		= 10;
constexpr int	CALIB_CUSTOM_5V		= 11;
constexpr int	CALIB_CUSTOM_10V	= 12;
constexpr int	CALIB_NONE_1V_5V	= 13;
constexpr int	CALIB_CUSTOM_1V_5V	= 14;

void	logical_ch_config_view( void );
void	table_view( const std::vector<uint16_t> &reg_list, std::function<void(int)> func, int cols = 4 );

using 	raw_t		= NAFE13388_UIM::raw_t;

PrintOutput	out( "test" );

int main( void )
{
	out.printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );
	out.printf( "---   custom gain & offset sample   ---\r\n" );

	spi.frequency( 1000 * 1000 );
	spi.mode( 1 );

	afe.begin();

	afe.logical_ch_config(  0, INPUT_GND       , (CALIB_NONE         << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  1, INPUT_GND       , (CALIB_NONE_5V      << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  2, INPUT_GND       , (CALIB_NONE_10V     << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  3, INPUT_GND       , (CALIB_FOR_PGA_0_2  << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  4, INPUT_GND       , (CALIB_CUSTOM_5V    << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  5, INPUT_GND       , (CALIB_CUSTOM_10V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  6, INPUT_A1P_SINGLE, (CALIB_NONE         << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  7, INPUT_A1P_SINGLE, (CALIB_NONE_5V      << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  8, INPUT_A1P_SINGLE, (CALIB_NONE_10V     << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config(  9, INPUT_A1P_SINGLE, (CALIB_FOR_PGA_0_2  << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 10, INPUT_A1P_SINGLE, (CALIB_CUSTOM_5V    << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 11, INPUT_A1P_SINGLE, (CALIB_CUSTOM_10V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 12, INPUT_A1P_SINGLE, (CALIB_NONE_1V_5V   << 12) | 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 13, INPUT_A1P_SINGLE, (CALIB_CUSTOM_1V_5V << 12) | 0x0084, 0x2900, 0x0000 );

	out.printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_channels );
	logical_ch_config_view();

	//
	//	showing gain/offset coefficients
	//

	std::vector<uint16_t>	registers_list( 32 );
	
	for ( auto i = 0; i < 32; i++ )
		registers_list[ i ]	= 0x80 + i;

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers before overwrite ===\r\n" );
	table_view( registers_list, []( int v ){ out.printf( "  %8ld @ 0x%04X", afe.read_r24( v ), v ); } );

	//
	//	gain/offset customization
	//
	
	ref_points	r[]	= {
		{ CALIB_NONE_5V,      {  5.0, 2000 }, {  0.0, 0 }, CALIB_NONE        },
		{ CALIB_NONE_10V,     { 10.0, 2000 }, {  0.0, 0 }, CALIB_NONE        },
		{ CALIB_CUSTOM_5V,    {  5.0, 2000 }, {  0.0, 0 }, CALIB_FOR_PGA_0_2 },
		{ CALIB_CUSTOM_10V,   { 10.0, 2000 }, {  0.0, 0 }, CALIB_FOR_PGA_0_2 },
		{ CALIB_NONE_1V_5V,   {  5.0, 2015 }, { 1.0, 16 }, CALIB_NONE        },
		{ CALIB_CUSTOM_1V_5V, {  5.0, 2015 }, { 1.0, 16 }, CALIB_FOR_PGA_0_2 },
	};
	
	for ( unsigned int i = 0; i < sizeof( r ) / sizeof( ref_points ); i++ )
		gain_offset_coeff( afe, r[ i ] );

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers after overwrite ===\r\n" );
	table_view( registers_list, []( int v ){ out.printf( "  %8ld @ 0x%04X", afe.read_r24( v ), v ); } );

	//
	//	operation with customized gain/offset
	//

	out.printf( "\r\n" );
	out.printf( "     count" );

	out.printf( "      NONE" );
	out.printf( "   NONE_5V" );
	out.printf( "  NONE_10V" );
	out.printf( "  Cal_dflt" );
	out.printf( "    Cal_5V" );
	out.printf( "   Cal_10V" );

	out.printf( "      NONE" );
	out.printf( "   NONE_5V" );
	out.printf( "  NONE_10V" );
	out.printf( "  Cal_dflt" );
	out.printf( "    Cal_5V" );
	out.printf( "   Cal_10V" );

	out.printf( " NONE_1V5V" );
	out.printf( "  Cal_1V5V" );

	out.printf( "\r\n" );

	raw_t			data;
	long			count		= 0;
	constexpr float read_delay	= 0.01;

	while ( true )
	{
		out.printf( " %8ld, ", count++ );
		
		for ( auto ch = 0; ch < 14; ch++ )
		{
			data	= afe.read<raw_t>( ch, read_delay );
			out.printf( " %8ld,", data );
		}
		out.printf( "\r\n" );

		wait( 0.05 );
	}
}

void logical_ch_config_view( void )
{
	uint16_t	en_ch_bitmap= afe.read_r16( 0x0024 );
	
	std::vector<uint16_t>	reg_list = { 0x0020, 0x0021, 0x0022, 0x0023 };

	for ( auto channel = 0; channel < 16; channel++ )
	{	
		out.printf( "  logical channel %2d : ", channel );

		if ( en_ch_bitmap & (0x1 << channel) )
		{
			afe.write_r16( channel );
			table_view( reg_list, []( int v ){ out.printf(  "  0x%04X: 0x%04X", v, afe.read_r16( v ) ); }, 4 );
		}
		else
		{
			out.printf(  "  (disabled)\r\n" );
		}
	}
}

void table_view( const std::vector<uint16_t> &reg_list, std::function<void(int)> func, int cols )
{
	const auto	length	= (int)reg_list.size();
	const auto	raws	= (int)(length + cols - 1) / cols;
	
	for ( auto i = 0; i < raws; i++  )
	{
		if ( i )
			out.printf( "\r\n" );
			
		for ( auto j = 0; j < cols; j++  )
		{
			auto	index	= i + j * raws;
			
			if ( index < length  )
				func( reg_list[ index ] );
		}
	}
	
	out.printf( "\r\n" );
}

