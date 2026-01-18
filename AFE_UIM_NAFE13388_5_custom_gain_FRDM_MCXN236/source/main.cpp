/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	"PrintOutput.h"
#include	"utils.h"
#include	<math.h>
#include	<array>

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );

PrintOutput		out( "test" );
//PrintOutput	out;			//use this line to disable file output

constexpr int	INPUT_GND			= 0x0010;
constexpr int	INPUT_A1P_SINGLE	= 0x1710;

enum CoeffIndex {
	CAL_FOR_PGA_0_2	= 0,
	CAL_NONE		= 8,
	CAL__5V_NONE,
	CAL_10V_NONE,
	CAL__5V_CUSTOM,
	CAL_10V_CUSTOM,
	CAL_1V5V_NONE,
	CAL_1V5V_CUSTOM,
};

constexpr NAFE13388_UIM::ref_points	r[]	= {
	{ CAL__5V_NONE,    {  5.0, 2000 }, {  0.0, 0 }, CAL_NONE        },
	{ CAL_10V_NONE,    { 10.0, 2000 }, {  0.0, 0 }, CAL_NONE        },
	{ CAL__5V_CUSTOM,  {  5.0, 2000 }, {  0.0, 0 }, CAL_FOR_PGA_0_2 },
	{ CAL_10V_CUSTOM,  { 10.0, 2000 }, {  0.0, 0 }, CAL_FOR_PGA_0_2 },
	{ CAL_1V5V_NONE,   {  5.0, 2015 }, { 1.0, 16 }, CAL_NONE        },
	{ CAL_1V5V_CUSTOM, {  5.0, 2015 }, { 1.0, 16 }, CAL_FOR_PGA_0_2 },
};

constexpr ch_setting_t	chs[]	= {
	{ INPUT_A1P_SINGLE, (CAL_NONE        << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL_FOR_PGA_0_2 << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL__5V_NONE    << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL__5V_CUSTOM  << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL_10V_NONE    << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL_10V_CUSTOM  << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL_1V5V_NONE   << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_A1P_SINGLE, (CAL_1V5V_CUSTOM << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL_NONE        << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL_FOR_PGA_0_2 << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL__5V_NONE    << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL__5V_CUSTOM  << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL_10V_NONE    << 12) | 0x0084, 0x2900, 0x0000 },
	{ INPUT_GND       , (CAL_10V_CUSTOM  << 12) | 0x0084, 0x2900, 0x0000 },
};

int main( void )
{
	out.printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );
	out.printf( "---   custom gain & offset sample   ---\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	afe.begin();
	afe.blink_leds();
	
	uint64_t	sn	= afe.serial_number();

	out.printf( "part number   = %04lX (revision: %01X)\r\n", afe.part_number(), afe.revision_number() );
	out.printf( "serial number = %06lX%06lX\r\n", (uint32_t)(sn >> 24), (uint32_t)sn & 0xFFFFFF );	//	to use NewlibNano
	out.printf( "die temperature = %f℃\r\n", afe.temperature() );
	
	//
	//	logical channels setting
	//

	for ( auto i = 0U; i < sizeof( chs ) / sizeof( ch_setting_t ); i++ )
		afe.logical_channel[ i ].configure( chs[ i ] );

	out.printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
	logical_ch_config_view();

	//
	//	gain/offset coefficient settings
	//

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers default values ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );

	//	on-board re-calibration for "PGA_gain = 0.2" coefficients

	afe.self_calibrate( 0 );

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers after on-board calibration ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );

#if 0
	afe.recalibrate( 0, 2, 2.5 );

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers after on-board calibration ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );


#endif

	//	gain/offset customization
	
	for ( auto i = 0U; i < sizeof( r ) / sizeof( ref_points ); i++ )
		afe.gain_offset_coeff( r[ i ] );

	out.printf( "\r\n=== GAIN_COEFF and OFFSET_COEFF registers after overwrite ===\r\n" );
	reg_dump( GAIN_COEFF0, 32 );

	//
	//	operation with customized gain/offset
	//

	out.printf( "\r\n" );
	out.printf( "     count" );

	out.printf( "      NONE" );
	out.printf( "  Cal_dflt" );
	out.printf( "   5V_NONE" );
	out.printf( "    5V_Cal" );
	out.printf( "  10V_NONE" );
	out.printf( "   10V_Cal" );
	out.printf( " 1V5V_NONE" );
	out.printf( "  1V5V_Cal" );

	out.printf( "      NONE" );
	out.printf( "  Cal_dflt" );
	out.printf( "   5V_NONE" );
	out.printf( "    5V_Cal" );
	out.printf( "  10V_NONE" );
	out.printf( "   10V_Cal" );
	out.printf( "\r\n" );

	raw_t			data;
	long			count		= 0;

	while ( true )
	{
		out.printf( " %8ld, ", count++ );
		
		for ( auto ch = 0; ch < afe.enabled_logical_channels(); ch++ )
		{
			data	= afe.logical_channel[ ch ];
			out.color( ch % 2 ? PrintOutput::Color::bg_none : PrintOutput::Color::bg_gray );
			out.printf( " %8ld,", data );
		}
		out.printf( "\r\n" );

		wait( 0.05 );
	}
}
