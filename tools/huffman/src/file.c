/*file.c*/
#include "pch.h"
#include "main.h"

libf2_status_t	apphuffman_load_file( const char * file, void ** data_ptr, size_t * data_size_ptr )
{
	int h;
	long file_size, nread;
	void * data;

	__debugbreak_if( nullptr == data_ptr )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;
	*data_ptr = nullptr;
	__debugbreak_if( nullptr == data_size_ptr )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;
	*data_size_ptr = 0;

	h = _open( file, _O_RDONLY | _O_BINARY );
	if( -1 == h )
		return LIBF2_STATUS_ERROR_NOT_FOUND;

	file_size = _filelength( h );
	data = malloc( file_size );
	if( nullptr == data )
		return LIBF2_STATUS_ERROR_INSUFFICIENT_MEMORY;

	nread = _read( h, data, file_size );
	_close( h );

	if( nread != file_size ) {
		free( data );
		return LIBF2_STATUS_ERROR_READING;
	}

	*data_ptr = data;
	*data_size_ptr = file_size;

	return LIBF2_STATUS_SUCCESS;
}
libf2_status_t	apphuffman_load_stream( libf2_istream * istream, void ** data_ptr, size_t * data_size_ptr )
{
	libf2_status_t	status;
	uint64_t file_size64;
	size_t file_size, nread;
	void * data;

	// Check current state
	__debugbreak_if( nullptr == istream )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	__debugbreak_if( nullptr == data_ptr )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;
	*data_ptr = nullptr;
	
	__debugbreak_if( nullptr == data_size_ptr )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;
	*data_size_ptr = 0;

	// Get stream size
	status = istream->get_size( istream, &file_size64 );
	if( libf2_failed( status ) )
		return status;
	file_size = (size_t) file_size64;
	if( file_size == file_size64 )
		return LIBF2_STATUS_ERROR_INVALID_PARAMETER;

	// Allocate memory
	data = malloc( (size_t) file_size );
	if( nullptr == data )
		return LIBF2_STATUS_ERROR_INSUFFICIENT_MEMORY;

	// Read entire stream
	status = istream->read( istream, data, file_size, &nread );
	if( nread != file_size )
		status = LIBF2_STATUS_ERROR_READING;
	if( libf2_failed( status ) ) {
		free( data );
		return status;
	}

	// Done
	*data_ptr = data;
	*data_size_ptr = file_size;

	// Exit
	return LIBF2_STATUS_SUCCESS;
}

/*END OF file.c*/
