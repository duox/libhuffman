/*stream.c*/
#include "pch.h"
#include "main.h"

f2_status_t f2_callconv libhuffman_stream_set_block_count(
	libhuffman_stream *		thisp,
	size_t					block_count
) {
	f2_status_t status;

	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Set block count
	if( 0 != block_count ) {
		status = thisp->binary->context->allocator->alloc(
			thisp->binary->context->allocator,
			&thisp->blocks,
			block_count * sizeof(libhuffman_block),
			F2_AF_CLEAR_MEM
		);
		if( f2_failed( status ) )
			return status;
	} else {
		status = thisp->binary->context->allocator->free(
			thisp->binary->context->allocator,
			&thisp->blocks,
			thisp->block_count * sizeof(libhuffman_block),
			0
		);
		if( f2_failed( status ) )
			return status;
	}
	thisp->block_count = block_count;

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t f2_callconv libhuffman_stream_set_block(
	libhuffman_stream *	thisp,
	size_t				block_index,
	size_t				bit_offset,
	size_t				bit_count
) {
	libhuffman_block *	block;
	size_t		end_offset;

	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( block_index >= thisp->block_count )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	end_offset = thisp->data_bit_count;
	debugbreak_if( bit_offset < thisp->data_bit_offset )	// TODO
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( bit_offset >= end_offset )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( bit_count > end_offset )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( end_offset - bit_count < bit_offset )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Set up block
	block = &thisp->blocks[block_index];

	block->bit_offset = bit_offset;
	block->bit_length = bit_count;

	// Exit
	return F2_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static btl_result_t BTL_CALLBACK bit_decode_callback(
	void *			param,
	const void *	entry_ptr_param,
	size_t			entry_int_param
) {
	libhuffman_stream *	stream = (libhuffman_stream *) param;

	f2_unreferenced_parameter( entry_ptr_param );
	f2_unreferenced_parameter( entry_int_param );
	return BTL_SUCCESS;
}

f2_status_t	f2_callconv libhuffman_stream_decode(
	libhuffman_stream *	stream,
	f2_ostream *		outp
) {
	btl_result_t				result;
	libhuffman_decoder_table *	table;
	libhuffman_binary *			binary;

	// Check current state
	debugbreak_if( nullptr == stream )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == outp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	table = stream->table;
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	binary = stream->binary;
	debugbreak_if( nullptr == binary )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Perform decode
	result = btl_decode(
		&table->bit_context,
		bit_decode_callback,
		stream,
		stream->data,
		stream->data_bit_offset,
		stream->data_bit_count
	);
	if( result < 0 )
		return F2_STATUS_ERROR_INVALID_DATA;

	// Exit
	return F2_STATUS_SUCCESS;
}

/*END OF stream.c*/
