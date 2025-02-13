/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	<stdio.h>

class	PrintOutput
{
public:
	PrintOutput( const char *file_name, const char *file_ext = "csv", bool time_info = true );
	void	printf( const char *format, ... );
private:
	FILE	*fp;
};

