/*app.c*/
#include "pch.h"
#include "main.h"

libf2_status_t	apphuffman_initialize_context( apphuffman_context * thisp )
{
	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize descriptor
	libhuffman_memset( thisp, 0, sizeof(*thisp) );

	// Exit
	return LIBF2_STATUS_SUCCESS;
}
libf2_status_t	apphuffman_deinitialize_context( apphuffman_context * thisp )
{
	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize descriptor
	if( nullptr != thisp->table_file ) {
		libhuffman_free( thisp->table_file );
		thisp->table_file = nullptr;
	}
	if( nullptr != thisp->input_file ) {
		libhuffman_free( thisp->input_file );
		thisp->input_file = nullptr;
	}
	if( nullptr != thisp->output_file ) {
		libhuffman_free( thisp->output_file );
		thisp->output_file = nullptr;
	}

	// Exit
	return LIBF2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

libf2_status_t	apphuffman_set_context_table_file( apphuffman_context * thisp, const char * table_file )
{
	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Set file name
	if( nullptr != thisp->table_file )
		libhuffman_free( thisp->table_file );

	thisp->table_file = nullptr == table_file ? nullptr : libhuffman_strdup( table_file );

	// Exit
	return LIBF2_STATUS_SUCCESS;
}
libf2_status_t	apphuffman_set_context_input_file( apphuffman_context * thisp, const char * input_file )
{
	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Set file name
	if( nullptr != thisp->input_file )
		libhuffman_free( thisp->input_file );

	thisp->input_file = nullptr == input_file ? nullptr : libhuffman_strdup( input_file );

	// Exit
	return LIBF2_STATUS_SUCCESS;
}
libf2_status_t	apphuffman_set_context_output_file( apphuffman_context * thisp, const char * output_file )
{
	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Set file name
	if( nullptr != thisp->output_file )
		libhuffman_free( thisp->output_file );

	thisp->output_file = nullptr == output_file ? nullptr : libhuffman_strdup( output_file );

	// Exit
	return LIBF2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

libf2_status_t	apphuffman_process_context( apphuffman_context * thisp )
{
	libf2_status_t				status;
	libhuffman_context			context;
	libf2_static_memory_istream	istream;
	libf2_static_memory_istream	table_istream;
	void *	data;
	size_t	data_size;
	void *	table_data;
	size_t	table_data_size;
	libf2_ostream ostream;

	// Check current state
	__debugbreak_if( nullptr == thisp )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Load files
	apphuffman_load_file( thisp->input_file, &data, &data_size );
	apphuffman_load_file( thisp->table_file, &table_data, &table_data_size );

	// Initialize libhuffman data
	libhuffman_initialize_context( &context, nullptr );

	status = libf2_static_buffer_istream_initialize( &istream, data, data_size );
	__debugbreak_if( libf2_failed( status ) )
		return status;
	status = libf2_static_buffer_istream_initialize( &table_istream, table_data, table_data_size );
	__debugbreak_if( libf2_failed( status ) ) {
		(void) libf2_static_buffer_istream_deinitialize( &istream );
		return status;
	}

	// Process data
	if( thisp->mode == APPHUFFMAN_M_DECODE ) {
		libhuffman_decoder_table	root_table;
		libhuffman_decode_context	decode_context;

		status = libhuffman_initialize_decoder_table( &root_table, &context, nullptr );
		__debugbreak_ifnot( libf2_succeeded( status ) ) {
			status = apphuffman_deserialize_decoder_table_stream( &root_table, &table_istream.istream );
			__debugbreak_ifnot( libf2_succeeded( status ) ) {
				status = libhuffman_initialize_decode_context( &decode_context, &context, &root_table, &istream.istream, &ostream );
				__debugbreak_ifnot( libf2_succeeded( status ) ) {
					status = libhuffman_decode( &decode_context );
					(void) libhuffman_deinitialize_decode_context( &decode_context );
				}
			}
			(void) libhuffman_deinitialize_decoder_table( &root_table );
		}
	} else if( thisp->mode == APPHUFFMAN_M_ENCODE ) {
		libhuffman_encoder_table	root_table;
		libhuffman_encode_context	encode_context;

		status = libhuffman_initialize_encoder_table( &root_table, &context, 0 );
		__debugbreak_ifnot( libf2_succeeded( status ) ) {
			status = apphuffman_deserialize_encoder_table_stream( &root_table, &table_istream.istream );
			__debugbreak_ifnot( libf2_succeeded( status ) ) {
				status = libhuffman_initialize_encode_context( &encode_context, &context, &root_table, &istream.istream, &ostream );
				__debugbreak_ifnot( libf2_succeeded( status ) ) {
					status = libhuffman_encode( &encode_context );
					(void) libhuffman_deinitialize_encode_context( &encode_context );
				}
			}
			(void) libhuffman_deinitialize_encoder_table( &root_table );
		}
	} else
		status = LIBF2_STATUS_ERROR_INVALID_STATE;

	// Done
	(void) libf2_static_buffer_istream_deinitialize( &table_istream );
	(void) libf2_static_buffer_istream_deinitialize( &istream );

	// Exit
	return status;
}

/*END OF app.c*/
