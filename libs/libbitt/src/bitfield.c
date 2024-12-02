/*bitfield.c*/
/** @file
 * @brief Bitfield iteration interface.
 *
 *	These functions allow client to iterate over bits in a array of bytes.
 */
#include "./internal.h"

/**
 * @brief Initialize bitfield iterator.
 *
 * @param[in] iterator (btl_bitfield_iterator *) iterator object.
 * @param[in] data (const void *) pointer to the data array.
 * @param[in] bit_offset (unsigned) initial offset of the first bitfield.
 * @param[in] bit_count (unsigned) total number of valid bits in the data array, starting with bit_offset.
 *
 * @return (btl_result_t) operation status code.
 */
btl_result_t btl_bitfield_iterator_initialize(
	btl_bitfield_iterator *	iterator,
	const void *			data,
	unsigned				bit_offset,
	size_t					bit_count
	)
{
	size_t extra_bytes;

	// Check current state
	debugbreak_if( NULL == iterator )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == data )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( 0 == bit_count )
		return BTL_ERROR_INVALID_PARAMETER;

	extra_bytes = bit_offset / 8;	// skip full bytes
	if( 0 != extra_bytes ) {
		data = (uint8_t *) data + extra_bytes;
		bit_offset %= 8;			// leave partial bits only
	}

	// Initialize the object
	iterator->data			= (const uint8_t *) data;
	iterator->data_end		= iterator->data + (bit_count + 7) / 8;
	iterator->acc			= 0;
	iterator->acc_bits_left	= 0;
	iterator->data_bits_left= bit_count;

	// Load first bits in the accumulator
	if( 0 != bit_offset ) {
		iterator->acc = (uint32_t) *iterator->data >> bit_offset;

		++ iterator->data;
		iterator->acc_bits_left = 8 - bit_offset; 
	}

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Fetch a bitfield from the data array.
 *
 * @param[in] iterator (btl_bitfield_iterator *) iterator object.
 * @param[in] value_ptr (uint32_t *) pointer to variable receiving bit field value.
 * @param[in] required_size (unsigned) required number of bis in the bitfield.
 * @param[in] acquired_size_ptr (unsigned *) pointer to receiving number of bits copied (can be less than required_size).
 *
 * @return (btl_result_t) operation status code.
 */
btl_result_t btl_bitfield_iterator_fetch(
	btl_bitfield_iterator *	iterator,
	uint32_t *				value_ptr,
	unsigned				required_size,
	unsigned *				acquired_size_ptr
	)
{
	unsigned fetch_bit_count;

	// Check current state
	debugbreak_if( NULL == iterator )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( 0 == required_size )
		return BTL_ERROR_INVALID_PARAMETER;

	if( 0 == iterator->data_bits_left )
		return BTL_ERROR_NO_MORE_DATA;

	// Calculate number of bits to fetch
	fetch_bit_count = required_size <= iterator->data_bits_left ?
		required_size:							// there enough bits
		(unsigned) iterator->data_bits_left;	// not enough bits, fetch what's left

	// Load accumulator with full bytes if more bits are needed
	if( iterator->acc_bits_left < fetch_bit_count )
	{
		uint32_t load = 0;

		size_t avail_bytes;
		size_t avail_bits = sizeof(iterator->acc) - iterator->acc_bits_left;
		size_t full_space_left = (iterator->data_end - iterator->data)*8;
		if( avail_bits > full_space_left )
			avail_bits = full_space_left;

		avail_bytes = (avail_bits + 7) / 8;
		small_memcpy( &load, iterator->data, avail_bytes );
		iterator->data += avail_bytes;

		iterator->acc |= load << iterator->acc_bits_left;
	}

	// Store data
	if( NULL != value_ptr ) {
		*value_ptr = iterator->acc & ((1 << fetch_bit_count) - 1);
	}
	iterator->acc >>= fetch_bit_count;
	iterator->data_bits_left -= fetch_bit_count;

	// Done
	if( NULL != acquired_size_ptr )
		*acquired_size_ptr = fetch_bit_count;

	// Exit
	return BTL_SUCCESS;
}

/*END OF bitfield.c*/
