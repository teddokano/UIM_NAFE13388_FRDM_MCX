/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE13388_UIM.h"
#include	<math.h>

constexpr	int	GAIN_COEFF			= 0x80;
constexpr	int	OFFSET_COEFF		= 0x90;



typedef struct	_point	{
	int32_t	data;
	double	voltage;
} point;

typedef struct	_ref_points	{
	int		coeff_index;
	point	high;
	point	low;
	int		cal_index;
} ref_points;

void	gain_offset_coeff( NAFE13388_UIM &afe, ref_points ref );
