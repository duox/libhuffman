/*table.c*/
#include "pch.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
libf2_status_t	libhuffman_callconv apphuffman_serialize_decoder_table( libhuffman_decoder_table * table,
	libf2_ostream * ostream )
{
	libf2_unreferenced_parameter( table );
	libf2_unreferenced_parameter( ostream );
	return LIBF2_STATUS_ERROR_NOT_IMPLEMENTED;
}

libf2_status_t	libhuffman_callconv apphuffman_deserialize_decoder_table( libhuffman_decoder_table * table,
	libf2_istream * istream )
{
	//s
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

libf2_status_t	libhuffman_callconv apphuffman_deserialize_encoder_table_file( libhuffman_encoder_table * table, const char * file )
{
	libf2_status_t	status;
	libhuffman_code_desc * desc_array;
	size_t desc_count;

	// Load table file
	status = apphuffman_load_table_from_file( &desc_array, &desc_count, file );
	if( libf2_failed(status) )
		return status;

	// Add all codes
	status = libhuffman_encoder_table_append_codes( table, desc_array, desc_count );
	free( desc_array );

	// Exit
	return status;
}
libf2_status_t	libhuffman_callconv apphuffman_deserialize_encoder_table_stream( libhuffman_encoder_table * table,
	libf2_istream * istream )
{
	libf2_status_t	status;
	libhuffman_code_desc * desc_array;
	size_t desc_count;

	// Load table file
	status = apphuffman_load_table_from_stream( &desc_array, &desc_count, istream, nullptr );
	if( libf2_failed(status) )
		return status;

	// Add all codes
	status = libhuffman_encoder_table_append_codes( table, desc_array, desc_count );
	free( desc_array );

	// Exit
	return status;
}


libf2_status_t	libhuffman_callconv apphuffman_deserialize_decoder_table_file( libhuffman_decoder_table * table, const char * file )
{
	libf2_status_t	status;
	libhuffman_code_desc * desc_array;
	size_t desc_count;

	// Load table file
	status = apphuffman_load_table_from_file( &desc_array, &desc_count, file );
	if( libf2_failed(status) )
		return status;

	// Add all codes
	status = libhuffman_decoder_table_append_codes( table, desc_array, desc_count );
	free( desc_array );

	// Exit
	return status;
}
libf2_status_t	libhuffman_callconv apphuffman_deserialize_decoder_table_stream( libhuffman_decoder_table * table,
	libf2_istream * istream )
{
	libf2_status_t	status;
	libhuffman_code_desc * desc_array;
	size_t desc_count;

	// Load table file
	status = apphuffman_load_table_from_stream( &desc_array, &desc_count, istream, nullptr );
	if( libf2_failed(status) )
		return status;

	// Add all codes
	status = libhuffman_decoder_table_append_codes( table, desc_array, desc_count );
	free( desc_array );

	// Exit
	return status;
}

/*libf2_status_t	apphuffman_load_file_table( libhuffman_decoder_table * table, const char * file )
{
	libf2_status_t	status;
	libf2_static_memory_istream	memory_istream;
	void * data;
	size_t data_size;

	// Open file
	status = apphuffman_load_file( file, &data, &data_size );
	if( libf2_failed(status) )
		return status;

	// Initialize stream I/O
	status = libhuffman_static_buffer_istream_initialize( &memory_istream, data, data_size );
	if( libf2_succeeded(status) )
	{
		// Load file
		const char * ext = strrchr( file, '.' );
		if( nullptr != ext && !stricmp( ext, ".txt" ) )
			status = apphuffman_deserialize_decoder_table( table, &memory_istream.istream );
		else
			status = LIBF2_STATUS_ERROR_FORMAT_NOT_SUPPORTED;
	}

	// Done, clean up
	libhuffman_static_buffer_istream_deinitialize( &memory_istream );
	free( data );

	// Exit
	return status;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char_to_digit( int c )
{
	if( '0' <= c && c <= '9' )
		return c - '0';
	if( 'A' <= c && c <= 'Z' )
		return c - 'A' + 10;
	if( 'a' <= c && c <= 'z' )
		return c - 'a' + 10;
	return (unsigned) -1;
}

int fetch_uint64( const char ** in_s, const char * end_s, uint64_t * out_value )
{
	const char * s = *in_s;
	unsigned radix = 10;
	uint64_t value = 0;
	int res;

	// Fetch prefix
	if( '0' == s[0] ) {
		switch( s[1] ) {
		case 'b':case 'B':	radix =  2; s += 2; *in_s += 2; break;
		case 'o':case 'O':	radix =  8; s += 2; *in_s += 2; break;
		case 'd':case 'D':	radix = 10; s += 2; *in_s += 2; break;
		case 'x':case 'X':	radix = 16; s += 2; *in_s += 2; break;
		}
	}

	// Fetch number
	for( ; s < end_s; ++ s ) {
		unsigned digit = char_to_digit( *s );
		if( (unsigned) -1 == digit )
			break;
		value = value * radix + digit;
	}

	// Done
	* out_value = value;
	res = *in_s != s;

	// Exit
	*in_s = s;
	return res;
}
int fetch_uint32( const char ** s, const char * end_s, uint32_t * value )
{
	uint64_t value64;
	if( !fetch_uint64( s, end_s, &value64 ) )
		return 0;
	*value = (uint32_t) value64;
	return *s < end_s;
}
int skip_spaces( const char ** in_s, const char * end_s )
{
	const char * s = *in_s;
	for( ; s < end_s; ++ s ) {
		if( '\x20' == *s || '\t' == *s || '\f' == *s )
			continue;
		if( '\n' == *s || '\r' == *s )
			continue;
		if( '/' == *s ) {
			if( '/' == s[1] ) {
				for( ; s < end_s; ++ s ) {
					if( '\n' == *s || '\r' == *s )
						break;
				}
			} else if( '*' == s[1] ) {
				for( ; ; ) {
					s += 2;
					if( s >= end_s - 1 )
						break;
					if( '*' == s[0] || '/' == s[1] )
						break;
				}
			} else
				break;
		}
		break;
	}
	*in_s = s;
	return s < end_s;
}
int skip_new_line( const char ** s, const char * end_s )
{
	if( !skip_spaces( s, end_s ) )
		return 0;
	while( *s < end_s && ('\n' == **s || '\r' == **s) )
		++ *s;
	return *s < end_s;
}

libf2_status_t	apphuffman_load_table_from_txt(
	libhuffman_code_desc ** desc_array_out, size_t * desc_count_out,
	const void * data, size_t data_size )
{
	libf2_status_t	status;
	const char * s, * end_s;
	size_t desc_array_size = 0;
	//libhuffman_code_desc code_desc;
	libhuffman_code_desc * desc_array = nullptr;
	libhuffman_code_desc * desc;// = &code_desc;

	// Prepare
	s = (const char *) data;
	end_s = (const char *) data + data_size;

	// Process file data
	status = LIBF2_STATUS_ERROR_FORMAT_NOT_SUPPORTED;
	for(;;) {
		if( !skip_spaces( &s, end_s ) )
			break;

		// Resize buffer
		desc = 0 == desc_array_size ?
			(libhuffman_code_desc *) malloc( sizeof(libhuffman_code_desc) ):
			(libhuffman_code_desc *) realloc( desc_array, (desc_array_size + 1) * sizeof(libhuffman_code_desc) );
		__debugbreak_if( nullptr == desc ) {
			status = LIBF2_STATUS_ERROR_INSUFFICIENT_MEMORY;
			break;
		}
		desc_array = desc;
		desc = desc + desc_array_size;
		desc_array_size ++;
		memset( desc, 0, sizeof(*desc) );

		// Read bit run
		desc->code = desc->code_bit_len = 0;
		while( s < end_s && ('0' == *s || '1' == *s) ) {
			++ desc->code_bit_len;
			desc->code = (desc->code << 1) | (*s - '0');
		}
		if( !skip_spaces( &s, end_s ) )
			break;

		// Read event id or value count
		if( ':' != *s )
			break;
		if( 'E' == *s || 'e' == *s ){
			++ s;
			if( !skip_spaces( &s, end_s ) )
				break;
			desc->count = (unsigned) -1;
		} else {
			if( !fetch_uint32( &s, end_s, &desc->count ) )
				break;
			if( !skip_spaces( &s, end_s ) )
				break;

			// Read value length
			if( ':' != *s )
				break;
			if( !fetch_uint32( &s, end_s, &desc->value_bit_len ) )
				break;
			if( !skip_spaces( &s, end_s ) )
				break;
		}

		// Read value
		if( ':' != *s )
			break;
		if( '\"' == *s || '\'' == *s || '`' == *s ) {
			status = LIBF2_STATUS_ERROR_NOT_SUPPORTED;
			//unsigned char end_ch = *s ++;
		} else if( isdigit( *s ) ) {
			uint64_t value;
			if( !fetch_uint64( &s, end_s, &value ) )
				break;

			desc->value = (libhuffman_value_t) value;
			if( desc->value != value )
				return LIBF2_STATUS_ERROR_INVALID_STATE;
		}
		if( !skip_spaces( &s, end_s ) )
			break;

		// Set table entry
		//status = libhuffman_decoder_table_append_codes( table, desc, 1 );
		//if( libf2_failed( status ) )
		//	break;

		// Skip new line
		if( !skip_new_line( &s, end_s ) )
			break;
	}

	// Done
	if( libf2_failed( status ) ) {
		free( desc_array );
		*desc_array_out = nullptr;
		*desc_count_out = 0;
	} else {
		*desc_array_out = desc_array;
		*desc_count_out = desc_array_size;
	}

	// Exit
	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Different stream loaders

libf2_status_t	apphuffman_load_table_from_memory(
	libhuffman_code_desc ** desc_array_out, size_t * desc_count_out,
	void * data, size_t data_size, const char * file )
{
	libf2_status_t	status;
	libhuffman_code_desc * desc_array = nullptr;
	size_t desc_array_size = 0;
	const char * ext;

	// Load file
	ext = strrchr( file, '.' );
	if( nullptr != ext && !stricmp( ext, ".txt" ) )
		status = apphuffman_load_table_from_txt( &desc_array, &desc_array_size, data, data_size );
	else
		status = LIBF2_STATUS_ERROR_FORMAT_NOT_SUPPORTED;

	// Done, clean up and store results
	*desc_array_out = desc_array;
	*desc_count_out = desc_array_size;

	// Exit
	return status;
}

libf2_status_t	apphuffman_load_table_from_file(
	libhuffman_code_desc ** desc_array_out, size_t * desc_count_out,
	const char * file )
{
	libf2_status_t	status;
	void * data;
	size_t data_size;

	// Open file
	status = apphuffman_load_file( file, &data, &data_size );
	if( libf2_failed(status) )
		return status;

	// Load file and exit
	status = apphuffman_load_table_from_memory( desc_array_out, desc_count_out, data, data_size, file );
	free( data );

	// Exit
	return status;
}

libf2_status_t	apphuffman_load_table_from_stream(
	libhuffman_code_desc ** desc_array_out, size_t * desc_count_out,
	libf2_istream * istream, const char * file )
{
	libf2_status_t	status;
	void * data;
	size_t data_size;

	// Open file
	status = apphuffman_load_stream( istream, &data, &data_size );
	if( libf2_failed(status) )
		return status;

	// Load file
	status = apphuffman_load_table_from_memory( desc_array_out, desc_count_out, data, data_size, file );
	free( data );

	// Exit
	return status;
}

/*END OF file.c*/
