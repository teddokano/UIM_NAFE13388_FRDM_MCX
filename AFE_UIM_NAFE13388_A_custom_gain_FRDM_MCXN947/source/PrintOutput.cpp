/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"PrintOutput.h"

#include	<time.h>
#include	<stdarg.h>


PrintOutput::PrintOutput( const char *file_name, const char *file_ext, bool time_info )
{
	constexpr int	filename_length	= 256;
	char	filename[ filename_length ];

	if ( time_info )
	{
		time_t	current_time;
		current_time	= time( NULL );

		sprintf( filename, "%s_%s.%s", file_name, ctime( &current_time ), file_ext );
	}
	else
	{
		sprintf( filename, "%s.%s", file_name, file_ext );
	}
	
	if ( NULL == (fp	= fopen( filename, "w" )) )
		::printf( "file open error\r\n" );
}

void PrintOutput::printf( const char *format, ... )
{
	constexpr int	char_length	= 256;
	char 			s[ char_length ];

	va_list args;
	va_start( args, format );
	vsnprintf( s, char_length, format, args );
	va_end( args );

	::printf( s );
	
	if ( fp )
		fputs( s, fp );
}
