/*encoder.c*/
#include "pch.h"
#include "main.h"

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_initialize_encode_context( libhuffman_encode_context * encode_context,
	libhuffman_context * context, libhuffman_encoder_table * root_table, f2_istream * istream, f2_ostream * ostream )
{
	// Check current state
	debugbreak_if( nullptr == encode_context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == root_table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == istream )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == ostream )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize structure
	encode_context->context = context;
	encode_context->table	= root_table;
	encode_context->istream = istream;
	encode_context->ostream = ostream;

	// Exit
	return F2_STATUS_SUCCESS;
}
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_deinitialize_encode_context( libhuffman_encode_context * encode_context )
{
	// Check current state
	debugbreak_if( nullptr == encode_context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize object
	encode_context->context = nullptr;

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static f2_status_t _store_bit_run( libhuffman_encode_context * encode_context, unsigned index )
{
	libhuffman_encoder_table * const	table = encode_context->table;
	uint8_t bit_length;
	libhuffman_code_t bit_runs;

	// Check current state
	debugbreak_if( index >= table->size )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	bit_runs = table->codes[index];
	bit_length = table->code_lengths[index];

	// Place bit field
	// TODO

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encode( libhuffman_encode_context * encode_context )
{
	f2_status_t	status;

	f2_istream * istream;
	f2_ostream * ostream;

	libhuffman_encoder_table *	table;

	// Check current state
	debugbreak_if( nullptr == encode_context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	istream = encode_context->istream;
	ostream = encode_context->ostream;
	table = encode_context->table;

	debugbreak_if( nullptr == encode_context->table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( encode_context->value_bit_size != next_power_of_two( encode_context->value_bit_size - 1 ) )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
//	debugbreak_if( encode_context->data_size % (encode_context->value_bit_size/8) )
//		return F2_STATUS_ERROR_INVALID_PARAMETER;

#if 1
	{
	unsigned	i;
	unsigned	mask = (1 << encode_context->value_bit_size) - 1;

		  uint64_t accum = 0;
		  unsigned count = (sizeof(accum) * 8) / encode_context->value_bit_size;
		  uint8_t src_buf[sizeof(accum) * 16];
	const uint8_t * src = nullptr;
	const uint8_t * src_end = nullptr;
		  size_t nread = 0;

	uint8_t		dst_buf[64];
	uint8_t *	dst = dst_buf;
	unsigned	dst_bit_offset = 0;
	uint8_t				bit_length;
	libhuffman_code_t	bit_runs;

	while( istream->eof( istream ) ) {

		// Download data to the input buffer
		if( src > src_end ) {
			status = istream->read( istream, src_buf, sizeof(src_buf), &nread );
			if( f2_failed( status ) )
				return status;
			src		= src_buf;
			src_end = src_buf + nread / sizeof(*src_buf);
		}

		// Encode buffer
		for( ; src < src_end; ++ src )
		{
			// Load data
			if( src_end - src < sizeof(accum) ) {
				accum = 0;
				f2_small_memcpy( &accum, src, src_end - src );
				count = (unsigned) ((src_end - src) * 8) / encode_context->value_bit_size;
			} else
				accum = * (uint64_t *) src;

			// Encode
			for( i = 0; i < count; ++ i ) {

				// Get bit field
				const unsigned index = (unsigned) (accum & mask);
				debugbreak_if( index >= table->size )
					return F2_STATUS_ERROR_INVALID_ALPHABET;
				bit_runs = table->codes[index];
				bit_length = table->code_lengths[index];

				// Flush dst buffer if there's no place for storing new run
				if( bit_length > sizeof(dst_buf)*8 - dst_bit_offset ) {
					const size_t whole_bytes_used = dst_bit_offset / 8;
					size_t nwritten;
					status = ostream->write( ostream, dst_buf, whole_bytes_used, &nwritten );
					debugbreak_if( f2_failed(status) )
						return status;

					f2_small_memcpy( dst_buf, dst_buf + whole_bytes_used, sizeof(dst_buf) - whole_bytes_used );
					dst_bit_offset = dst_bit_offset - whole_bytes_used*8;
				}

				// Copy bits
				libhuffman_bitcopy( dst, dst_bit_offset, &bit_runs, bit_length );
				dst_bit_offset += bit_length;

				// Iterate
				accum >>= encode_context->value_bit_size;
			}
		}
	}
#else
	// Prepare
	{
	unsigned	mask = (1 << encode_context->value_bit_size) - 1;
	unsigned	i;

	// Decode buffer
	if( 8 < encode_context->value_bit_size ) {
		uint16_t src_buf[32];
		const uint16_t * src = nullptr;
		const uint16_t * src_end = nullptr;
			  size_t nread = 0;
		const unsigned count = (sizeof(uint16_t) * 8) / encode_context->value_bit_size;

		while( istream->eof( istream ) ) {

			// Download data to the input buffer
			if( src > src_end ) {
				status = istream->read( istream, src_buf, sizeof(src_buf), &nread );
				if( f2_failed( status ) )
					return status;
				src		= src_buf;
				src_end = src_buf + nread / sizeof(*src_buf);

			}

			// Encode buffer
			for( ; src < src_end; ++ src )
			{
				uint16_t run = *src;
				for( i = 0; i < count; ++ i ) {
					_store_bit_run( encode_context, run & mask );
					run >>= encode_context->value_bit_size;
				}
			}
		}
	} else {
		uint8_t src_buf[32];
		const uint8_t * src = nullptr;
		const uint8_t * src_end = nullptr;
			  size_t nread = 0;
		const unsigned count = (sizeof( uint8_t) * 8) / encode_context->value_bit_size;

		while( istream->eof( istream ) ) {

			// Download data to the input buffer
			if( src > src_end ) {
				status = istream->read( istream, src_buf, sizeof(src_buf), &nread );
				if( f2_failed( status ) )
					return status;
				src		= src_buf;
				src_end = src_buf + nread / sizeof(*src_buf);

			}

			// Encode buffer
			for( ; src < src_end; ++ src )
			{
				uint8_t run = *src;
				for( i = 0; i < count; ++ i ) {
					_store_bit_run( encode_context, run & mask );
					run >>= encode_context->value_bit_size;
				}
			}
		}
	}
#endif
	// Exit
	return F2_STATUS_SUCCESS;
}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_build_encoder_table( libhuffman_encode_context * encode_context )
{
	f2_unreferenced_parameter( encode_context );
	return F2_STATUS_ERROR_NOT_IMPLEMENTED;	// TODO
}

/*END OF encoder.c*/
