/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"coeffs.h"

void gain_offset_coeff( NAFE13388_UIM &afe, ref_points ref )
{
	constexpr double	pga1x_voltage		= 5.0;
	constexpr int		adc_resolution		= 24;
	constexpr double	pga_gain_setting	= 0.2;

	constexpr double	fullscale_voltage	= pga1x_voltage / pga_gain_setting;

	double	fullscale_data		= pow( 2, (adc_resolution - 1) ) - 1.0;
	double	ref_data_span		= ref.high.data		- ref.low.data;
	double	ref_voltage_span	= ref.high.voltage	- ref.low.voltage;
	
	double	dv_slope			= ref_data_span / ref_voltage_span;
	double	custom_gain			= dv_slope * (fullscale_voltage / fullscale_data);
	double	custom_offset		= (dv_slope * ref.low.voltage - ref.low.data) / custom_gain;
	
	int32_t	gain_coeff_cal		= afe.read_r24( GAIN_COEFF   + ref.cal_index );
	int32_t	offsset_coeff_cal	= afe.read_r24( OFFSET_COEFF + ref.cal_index );
	int32_t	gain_coeff_new		= round( gain_coeff_cal * custom_gain );
	int32_t	offset_coeff_new	= custom_offset - offsset_coeff_cal;
	
	printf( "ref_point_high = %8ld @%6.3lf\r\n", ref.high.data, ref.high.voltage );
	printf( "ref_point_low  = %8ld @%6.3lf\r\n", ref.low.data,  ref.low.voltage  );
	printf( "gain_coeff_new   = %8ld\r\n", gain_coeff_new   );
	printf( "offset_coeff_new = %8ld\r\n", offset_coeff_new );
	
	afe.write_r24( GAIN_COEFF   + ref.coeff_index, gain_coeff_new   );
	afe.write_r24( OFFSET_COEFF + ref.coeff_index, offset_coeff_new );
}

