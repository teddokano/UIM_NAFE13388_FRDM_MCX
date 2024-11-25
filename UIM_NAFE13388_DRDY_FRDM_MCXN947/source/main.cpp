/** NXP Analog Front End class library for MCX
 *
 *  @author  Tedd OKANO
 *
 *  Copyright: 2024 Tedd OKANO
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"NAFE13388_UIM.h"
#include 	<iostream>
#include	<iomanip>

using 	microvolt	= NAFE13388_UIM::microvolt;
using 	raw			= NAFE13388_UIM::raw;
using	std::cout;
using	std::endl;

SPI				spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS
NAFE13388_UIM	afe( spi );
InterruptIn		ADC_nDRDY( D4 );			//	Uses interrupt by ADC_nDRDY pin
volatile bool	drdy_wait;

constexpr uint32_t	timeout_limit	= 100000000;

void	DRDY_int_handler( void );
void	logical_ch_config_view( int channel );
void	register16_dump( std::vector<uint16_t> &reg_list );

int main( void )
{
	cout << "***** Hello, NAFE13388 UIM board! *****" << endl;
	cout << std::setw( 11 ) << std::right << std::fixed << std::setprecision(2);

	volatile auto	timeout_count	= timeout_limit;

	spi.frequency( 1000 * 1000 );
	spi.mode( 1 );

	afe.begin();

	afe.logical_ch_config( 0, 0x1070, 0x0084, 0x2900, 0x0000 );
	afe.logical_ch_config( 1, 0x2070, 0x0084, 0x2900, 0x0000 );

	logical_ch_config_view( 0 );
	logical_ch_config_view( 1 );

	ADC_nDRDY.rise( DRDY_int_handler );

	while ( true )
	{
		for ( auto ch = 0; ch < 2; ch++ )
		{
			drdy_wait		= true;
			timeout_count	= timeout_limit;

			afe.start( ch );

			while ( drdy_wait && --timeout_count )
				;

			if ( !timeout_count )
			{
				cout << "DRDY signal wait timeout" <<endl;
				continue;
			}

			// Enable one of following lines
			auto	data = afe.read<microvolt>( ch );	//	get data in mictovolt
//			auto	data = afe.read<raw>( ch );			//	get raw data in integer

			cout << std::setw( 11 ) << data << ", ";
		}
		cout << endl;
		wait( 0.05 );
	}
}

void DRDY_int_handler( void )
{
	drdy_wait	= false;
}

void logical_ch_config_view( int channel )
{
	cout << "logical channel " << std::setw( 2 ) << channel << endl;
	afe.write_r16( channel );

	std::vector<uint16_t>	reg_list = { 0x0020, 0x0021, 0x0022, 0x0023 };
	register16_dump( reg_list );

	cout << endl;
}

void register16_dump( std::vector<uint16_t> &reg_list )
{
	cout <<  std::showbase << std::hex << std::setfill( '0' ) << std::internal;

	for_each(
		reg_list.begin(),
		reg_list.end(),
		[]( auto reg )
		{
			cout
				<< "  "
				<< std::setw( 6 )
				<< reg
				<< ": "
				<< std::setw( 6 )
				<< afe.read_r16( reg )
				<< endl;
		}
	);

	cout << std::noshowbase << std::dec << std::setfill( ' ' ) << std::right;
}
