/*bits.c*/
#include "pch.h"
#include "main.h"

void bitcpy( void * dst_bits, size_t dst_bits_offset, const void * src_bits, unsigned src_bits_count )
{
		  uint8_t * dst = (		 uint8_t *) dst_bits;
	const uint8_t * src = (const uint8_t *) src_bits;
	unsigned	bit_count, byte_count;
	uint32_t	value;

	dst += dst_bits_offset / 8;
	dst_bits_offset %= 8;

	while( src_bits_count > 0) {

		// Load value
		value = 0;
		bit_count = min( src_bits_count, sizeof(value) * 8 );
		f2_small_memcpy( &value, src, (bit_count + 7) / 8 );
		src_bits_count -= bit_count;

		// Store value
		if( 0 != dst_bits_offset ) {
			uint8_t mask = (uint8_t) ((1U << dst_bits_offset) - 1);
			*dst = (uint8_t) ((*dst & mask) | (value << dst_bits_offset));
			dst_bits_offset = 0;
			value >>= dst_bits_offset;
			bit_count -= (unsigned) dst_bits_offset;
			++ dst;
		}

		byte_count = bit_count / 8;
		if( 0 != byte_count ) {
			f2_small_memcpy( dst, &value, byte_count );
			bit_count %= 8;
			dst += byte_count;
		}

		if( 0 != bit_count ) {
			uint8_t mask = (uint8_t) ((1U << bit_count) - 1);
			*dst = (uint8_t) ((*dst & ~mask) | (value & mask));
			dst_bits_offset = bit_count;	// next bit sequence starts here
			++ dst;
		}
	}

	return;
}

/*END OF bits.c*/
