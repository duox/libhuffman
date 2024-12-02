/*memory.c*/
/** @file
 * @brief Default allocator implementation.
 */
#include <memory.h>
#include "./internal.h"

#undef small_memcpy
void * small_memcpy( void * dst, const void * src, size_t count )
{
	return small_memmove( dst, src, count );
}
/**
 * @brief Memory copy routine for tiny memory blocks.
 * @param[in] dst (void *) pointer to destination memory.
 * @param[in] dst (const void *) pointer to source memory.
 * @param[in] count (size_t) number of bytes to copy.
 * @return (void *) pointer to destination memory (the value of dst argument).
 *
 * @todo This is a simple variant, convoluted optimizations could slow down small copy
 * operations. Probably, aligned word copy might help improve performance.
 */
void * small_memmove( void * dst, const void * src, size_t count )
{
	debugbreak_if( dst == src )	// copying to itself; break in debug builds to investigate
		return dst;
	if( count > 16 )			// big copy operations are handled in the standard way
		return memcpy( dst, src, count );

	PREFETCH_DATA( dst );		// preload destination cache line so it's not loaded during partial store
	PREFETCH_DATA( src );		// preload source cache line

	if( dst < src ) {
			  uint8_t * dst_bytes = (		uint8_t *) dst;
		const uint8_t * src_bytes = (const	uint8_t *) src;

		switch( count ) {
			default:	ASSERT(FALSE);	return NULL;	// unexpected error
			case 16:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 15:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 14:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 13:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 12:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 11:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case 10:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  9:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  8:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  7:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  6:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  5:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  4:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  3:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  2:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  1:	*dst_bytes++ = *src_bytes++;	// fallthrough
			case  0:	break;	// zero count does nothing
		}
	}
	else
	{
			  uint8_t * dst_bytes = (		uint8_t *) dst + count;
		const uint8_t * src_bytes = ( const uint8_t *) src + count;

		switch( count ) {
			default:	ASSERT(FALSE);	return NULL;	// unexpected error
			case 16:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 15:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 14:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 13:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 12:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 11:	*--dst_bytes = *--src_bytes;	// fallthrough
			case 10:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  9:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  8:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  7:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  6:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  5:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  4:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  3:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  2:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  1:	*--dst_bytes = *--src_bytes;	// fallthrough
			case  0:	break;	// zero count does nothing
		}
	}

	return dst;
}

/*END OF memory.c*/
