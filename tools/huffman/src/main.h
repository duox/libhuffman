/*main.h*/

#define LIBF2_CFG_STATIC_MEMORY_ISTREAM		1
#include "../../../include/libhuffman.h"
#include "../include/apphuffman.h"

#define libhuffman_memset( p, c, n )		memset( p, c, n )
#define libhuffman_free( p )				free( p )
#define libhuffman_strdup( s )				_strdup( s )

libf2_status_t	apphuffman_load_file( const char * file, void ** data, size_t * data_size );
libf2_status_t	apphuffman_load_stream( libf2_istream * istream, void ** data_ptr, size_t * data_size_ptr );

//libf2_status_t	apphuffman_load_encoder_table_file( libhuffman_encoder_table * table, const char * file );
//libf2_status_t	apphuffman_load_decoder_table_file( libhuffman_decoder_table * table, const char * file );

libf2_status_t	apphuffman_load_table_txt ( libhuffman_code_desc ** desc_array_out, size_t * desc_count_out, const void * data, size_t data_size );

libf2_status_t	apphuffman_load_table_from_file( libhuffman_code_desc ** desc_array_out, size_t * desc_count_out, const char * file );
libf2_status_t	apphuffman_load_table_from_memory( libhuffman_code_desc ** desc_array_out, size_t * desc_count_out, void * data, size_t data_size, const char * file );
libf2_status_t	apphuffman_load_table_from_stream( libhuffman_code_desc ** desc_array_out, size_t * desc_count_out, libf2_istream * istream, const char * file );

/*END OF main.h*/
