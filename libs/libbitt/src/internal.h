/*internal.h*/

#include "../include/libbitt/libbitt.h"

#define debugbreak_if( expr )	if( expr )
#define unreferenced_parameter( x )		(x)

extern btl_heap_allocator	default_heap_allocator;
extern btl_table_allocator	default_table_allocator;

#define ASSERT(expr)

// memory.c
#define PREFETCH_DATA( ptr )
void * small_memcpy( void * dst, const void * src, size_t count );
#define small_memcpy( dst, src, count )	small_memmove( dst, src, count )
void * small_memmove( void * dst, const void * src, size_t count );


// bitfield.c
typedef struct btl_bitfield_iterator	btl_bitfield_iterator;
struct btl_bitfield_iterator {
	const uint8_t *	data;			//< pointer to the bit buffer
	const uint8_t *	data_end;		//< pointer to the end of bit buffer (a first byte after the last byte)
	size_t			data_bits_left;	//< total bits left in acc and in the data array
	unsigned		acc_bits_left;	//< number of bits left in the accumulator
	uint32_t		acc;			//< accumulator (work area)
};
btl_result_t btl_bitfield_iterator_initialize(
	btl_bitfield_iterator *	iterator,
	const void *			data,
	unsigned				bit_offset,
	size_t					bit_count
	);
btl_result_t btl_bitfield_iterator_fetch(
	btl_bitfield_iterator *	iterator,
	uint32_t *				value,
	unsigned				required_size,
	unsigned *				acquired_size
	);
#define btl_bitfield_iterator_finished( iterator )		(NULL == (iterator) || 0 == (iterator)->data_bits_left)

// table.c
btl_result_t btl_table_create(
	btl_context *	context,
	btl_table **	table_ptr,
	unsigned		l2_size,
	btl_table *		parent_table,
	unsigned		parent_index
	);
btl_result_t btl_table_destroy(
	btl_table *		table
	);
btl_result_t _btl_table_release_arrays(
	btl_table *		table
	);

/*END OF internal.h*/
