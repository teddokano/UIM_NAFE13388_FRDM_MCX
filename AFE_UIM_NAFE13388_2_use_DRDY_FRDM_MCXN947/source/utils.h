/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"

using enum	NAFE13388_UIM::Register16;
using enum	NAFE13388_UIM::Register24;
using enum	NAFE13388_UIM::Command;

using raw_t			= NAFE13388_UIM::raw_t;
using RegVct		= NAFE13388_UIM::RegVct;
using ch_setting_t	= NAFE13388_UIM::ch_setting_t;
using ref_points	= NAFE13388_UIM::ref_points;

void	reg_dump( RegVct reg_vctr );
void	reg_dump( NAFE13388_UIM::Register24 addr, int length );
void	logical_ch_config_view( void );
void	table_view( int size, int cols, std::function<void(int)> view, std::function<void(void)> linefeed = nullptr );
