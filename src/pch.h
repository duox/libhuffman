/*pch.h*/

#include <memory.h>
#include <stdarg.h>
#include <stdint.h>

#pragma warning(disable:4201)	//: nonstandard extension used : nameless struct/union

typedef int	int_t;

void bitcpy( void * dst, size_t dst_bit_offset, const void * src, unsigned src_bit_count );
#define libhuffman_bitcopy( dst, dst_bit_offset, src, src_bit_count )	bitcpy( dst, dst_bit_offset, src, src_bit_count )

/*END OF pch.h*/
