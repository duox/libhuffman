/*binary.c*/
#include "pch.h"
#include "main.h"

f2_status_t f2_callconv libhuffman_binary_initialize(
	libhuffman_binary *			thisp,
	libhuffman_context *		context,
	libhuffman_decoder_table *	default_table,
	size_t						stream_count
) {
	f2_status_t status;

	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize object
	thisp->context = context;
	thisp->default_table = default_table;

	if( 0 != stream_count ) {
		status = context->allocator->alloc(
			context->allocator,
			&thisp->streams,
			stream_count * sizeof(libhuffman_stream),
			F2_AF_CLEAR_MEM
		);
		if( f2_failed( status ) )
			return status;
		thisp->stream_count = stream_count;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}
f2_status_t f2_callconv libhuffman_binary_deinitialize(
	libhuffman_binary *		thisp
) {
	f2_status_t status;

	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize object
	thisp->context = NULL;
	thisp->default_table = NULL;

	if( 0 != thisp->stream_count ) {
		status = thisp->context->allocator->free(
			thisp->context->allocator,
			&thisp->streams,
			thisp->stream_count * sizeof(libhuffman_stream),
			0
		);
		if( f2_failed( status ) )
			return status;
		thisp->stream_count = 0;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t f2_callconv libhuffman_set_stream(
	libhuffman_binary *			thisp,
	size_t						index,
	const void *				data,
	size_t						data_bit_offset,
	size_t						data_bit_count,
	libhuffman_decoder_table *	table
) {
	f2_status_t				status;
	libhuffman_stream *		stream;

	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == data || 0 == data_bit_count )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( index >= thisp->stream_count )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize stream descriptor
	stream = &thisp->streams[index];

	stream->binary = thisp;
	stream->table = nullptr != table ? table : thisp->default_table;
	stream->data = PB(data) + (data_bit_offset >> 3);
	stream->data_bit_offset = data_bit_offset & 0x7;
	stream->data_bit_count = data_bit_count;

	// Exit
	return F2_STATUS_SUCCESS;
}

/*END OF binary.c*/
