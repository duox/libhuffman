/*alloc.c*/
/** @file
 * @brief Default allocator implementation.
 */
#include <malloc.h>
#include "./internal.h"

/**
 * @brief Callback for accessing the default heap allocator.
 * @param[in] thisp (btl_heap_allocator *) pointer to the allocator object.
 * @param[in] block_ptr (void **) pointer to variable containing and/or receiving pointer to the block.
 * @param[in] size (size_t) pointer to the allocator object.
 * @return (btl_result) status code.
 *
 *	This function can allocate, reallocate or free memory, depending on its arguments:
 * - NULL == *block_ptr and 0 != size: allocate new memory block.
 * - NULL != *block_ptr and 0 != size: reallocate existing memory block.
 * - NULL != *block_ptr and 0 == size: free memory block.
 * - NULL == *block_ptr and 0 == size: no-op, same as free( NULL ).
 * - all other combinations are invalid.
 */
static btl_result_t BTL_CALLBACK btl_default_heap_allocator_callback(
	btl_heap_allocator *	thisp,
	void **					block_ptr,
	size_t					size
)
{
	debugbreak_if( NULL == block_ptr )
		return BTL_ERROR_INVALID_PARAMETER;

	if( NULL == *block_ptr )
	{
		debugbreak_if( 0 == size )
			return BTL_ERROR_INVALID_PARAMETER;

		*block_ptr = malloc( size );
		if( NULL == *block_ptr )
			return BTL_ERROR_INSUFFICIENT_MEMORY;
	}
	else	// if( NULL != *block_ptr )
	{
		if( 0 == size )
		{
			free( *block_ptr );
			*block_ptr = NULL;
		}
		else
		{
			void * ptr = malloc( size );
			if( NULL == ptr )
				return BTL_ERROR_INSUFFICIENT_MEMORY;
			*block_ptr = ptr;
		}
	}

	unreferenced_parameter( thisp );
	return BTL_SUCCESS;
}

//! Default heap allocator descriptor
btl_heap_allocator default_heap_allocator = {
	&btl_default_heap_allocator_callback
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Table allocator

/**
 * @brief Callback for accessing the default heap table allocator.
 * @param[in] thisp (btl_heap_allocator *) pointer to the allocator object.
 * @param[in] context (btl_context *) pointer to uninitialized context object.
 * @param[out] table_ptr (btl_table **) pointer to variable receiving pointer to the new table object.
 * @param[in] l2_count (unsigned) number of elements in the table. if 0, no entries are required to be allocated.
 * @return (btl_result) status code.
 *
 *	The allocator function can completely ignore the l2_count parameter. However, the allocator
 * can allocate a single block for the table object and all entry arrays, thus avoiding heap
 * fragmentation.
 */
static btl_result_t BTL_CALLBACK btl_table_allocator_alloc_table(
	btl_table_allocator *	thisp,		//< pointer to the btl_table_allocator structure
	btl_context *			context,	//< pointer to the context
	btl_table **			table_ptr,	//< pointer to variable receiving pointer to the allocated table
	unsigned				l2_count	//< log2 of the initial number of elements in the table (0 = default)
) {
	btl_result_t result;
	btl_heap_allocator * allocator;
	btl_table *			 table;

	// Check current state
	debugbreak_if( NULL == thisp )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == table_ptr )
		return BTL_ERROR_INVALID_PARAMETER;

	*table_ptr = NULL;

	if( 0 == l2_count )
		l2_count = 8;

	// Allocate memory using context's allocator
	allocator = context->heap_allocator;
	if( NULL == allocator )
		allocator = &default_heap_allocator;

	table = NULL;
	result = allocator->alloc(
		allocator,		// thisp
		&table,			// pointer to object
		sizeof(*table)	// allocate specified number of bytes
		);
	if( BTL_SUCCESS != result )
		return result;

	// Initialize the table
	result = btl_table_initialize(
		table,			// table object
		context,		// context object
		NULL,			// parent table
		0				// index in the parent table
		);
	if( BTL_SUCCESS == result ) {
		result = btl_table_set_size(
			table,		// table object
			l2_count	// log2 of entries
			);
	}

	// Check operation result
	if( BTL_SUCCESS != result )
	{
		allocator->alloc(
			allocator,	// thisp
			&table,		// pointer to object
			0			// release the memory
			);
		return result;
	}

	// Done
	*table_ptr = table;

	// Exit
	return BTL_SUCCESS;
}
/**
 * @brief Callback for accessing the default heap table allocator.
 * @param[in] thisp (btl_heap_allocator *) pointer to the allocator object.
 * @param[in] context (btl_context *) pointer to uninitialized context object.
 * @param[in] table (btl_table *) pointer to the new table object.
 * @return (btl_result) status code.
 *
 *	The allocator function can completely ignore the l2_count parameter. However, the allocator
 * can allocate a single block for the table object and all entry arrays, thus avoiding heap
 * fragmentation.
 */
static btl_result_t BTL_CALLBACK btl_table_allocator_release_table(
	btl_table_allocator *	thisp,		//< pointer to the btl_table_allocator structure
	btl_context *			context,	//< pointer to the context
	btl_table *				table		//< pointer to the allocated table
) {
	btl_result_t result;
	btl_heap_allocator * allocator;

	// Check current state
	debugbreak_if( NULL == thisp )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;

	// Free memory using context's allocator
	allocator = context->heap_allocator;
	if( NULL == allocator )
		allocator = &default_heap_allocator;

	result = allocator->alloc(
		allocator,		// thisp
		&table,			// pointer to object
		0				// free memory
		);
	if( BTL_SUCCESS != result )
		return result;

	// Exit
	return BTL_SUCCESS;
}

//! Default table allocator descriptor
btl_table_allocator	default_table_allocator = {
	btl_table_allocator_alloc_table,
	btl_table_allocator_release_table
};

/*END OF alloc.c*/
