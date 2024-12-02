/*decoder.c*/
#include "pch.h"
#include "main.h"

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_initialize (
	libhuffman_decoder *	thisp,
	libhuffman_context *	context
) {
	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize structure
	thisp->context = context;
	thisp->binary = nullptr;

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_deinitialize(
	libhuffman_decoder *	thisp
) {
	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize object
	thisp->context = nullptr;
	thisp->binary = nullptr;

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t	f2_callconv libhuffman_decoder_set_binary(
	libhuffman_decoder *	thisp,
	libhuffman_binary *		binary
) {
	// Check current state
	debugbreak_if( nullptr == thisp )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize object
	thisp->binary = binary;

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_decode(
	libhuffman_decoder *	decoder,
	libhuffman_binary *		binary,
	f2_ostream *			outp
) {
	size_t	stream_index;
	libhuffman_stream * stream;
	f2_status_t	status;

	// Check current state
	debugbreak_if( nullptr == decoder )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == decoder->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == decoder->context->allocator )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == decoder->context->client )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	debugbreak_if( nullptr == binary )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == binary->streams || 0 == binary->stream_count )
		return F2_STATUS_ERROR_NOT_INITIALIZED;

	// Set up decoding process
	stream = binary->streams + 1;
	for( stream_index = 1; stream_index < binary->stream_count; ++ stream_index, ++ stream ) {
		status = decoder->context->client->launch_decoder(
			decoder->context->client,
			stream,
			outp
			);
		if( f2_failed(status) )
			return status;
	}
	status = decoder->context->client->launch_decoder(
		decoder->context->client,
		binary->streams,
		outp
		);

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*END OF decoder.c*/
