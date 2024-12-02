/*table.c*/
/** @file
 *
 * @brief Table related functions.
 */
#include "internal.h"

/**
 * @brief Create table object.
 * @param[in] context (btl_context *) pointer to the context object.
 * @param[in] table_ptr (btl_table **) pointer to variable receiving pointer to the table.
 * @param[in] l2_size (unsigned) log2 of number of elements.
 * @param[in] parent_table (btl_table *) pointer to the parent table.
 * @param[in] parent_index (unsigned) index of the entry in the parent table.
 * @return (btl_result_t) status code.
 */
btl_result_t btl_table_create(
	btl_context *	context,
	btl_table **	table_ptr,
	unsigned		l2_size,
	btl_table *		parent_table,
	unsigned		parent_index
) {
	btl_result_t			result;
	btl_table_allocator *	table_allocator;
	btl_table *				table;

	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;

	debugbreak_if( NULL == table_ptr )
		return BTL_ERROR_INVALID_PARAMETER;
	*table_ptr = NULL;

	// Acquire allocator
	if( NULL != context->table_allocator ) {
		if( NULL == context->table_allocator->alloc_table || NULL == context->table_allocator->release_table )
			return BTL_ERROR_INVALID_ALLOCATOR;
		table_allocator = context->table_allocator;
	} else
		table_allocator = &default_table_allocator;

	// Create table object
	table = NULL;
	result = table_allocator->alloc_table(
		table_allocator,
		context,
		&table,
		l2_size
	);
	if( BTL_SUCCESS != result )
		return result;

	// If entries haven't been allocated, allocate them now
	if( NULL == table->entry_data ) {
		result = btl_table_set_size(
			table,
			l2_size
		);
		if( BTL_SUCCESS != result ) {
			table_allocator->release_table(
				table_allocator,
				context,
				table
			);
			return result;
		}
	}

	// Initialize the object
	table->parent_table = parent_table;
	table->parent_index = parent_index;

	// Done
	*table_ptr = table;

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Destroy table object.
 * @param[in] table (btl_table *) pointer to the table.
 * @return (btl_result_t) status code.
 *
 *	This function deinitialized the object (by destroying nested tables and
 * freeing all memory) and then calls the table allocator to release the memory
 * used by the table object.
 */
btl_result_t btl_table_destroy(
	btl_table *		table
) {
	btl_result_t	result;
	btl_context *	context;
	btl_table_allocator *	table_allocator;

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;
	context = table->context;

	debugbreak_if( &context->root_table == table )
		return BTL_ERROR_INVALID_PARAMETER;

	if( NULL != context->table_allocator ) {
		if( NULL == context->table_allocator->alloc_table || NULL == context->table_allocator->release_table )
			return BTL_ERROR_INVALID_ALLOCATOR;
		table_allocator = context->table_allocator;
	} else
		table_allocator = &default_table_allocator;

	// Deinitialize the object
	result = btl_table_deinitialize(
		table
	);
	if( BTL_SUCCESS != result )
		return result;

	// Release the memory
	result = table_allocator->release_table(
		table_allocator,
		context,
		table
	);
	if( BTL_SUCCESS != result )
		return result;

	// Exit
	return BTL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize table object.
 * @param[in] table (btl_table *) pointer to the table.
 * @param[in] context (btl_context *) pointer to the context object.
 * @param[in] parent_table (btl_table *) pointer to the parent table.
 * @param[in] parent_index (unsigned) index of the entry in the parent table.
 * @return (btl_result_t) status code.
 *
 *	This function deinitialized the object (by destroying nested tables and
 * freeing all memory) and then calls the table allocator to release the memory
 * used by the table object.
 */
#undef btl_table_initialize
btl_result_t btl_table_initialize(
	btl_table *		table,
	btl_context *	context,
	btl_table *		parent_table,
	unsigned		parent_index
) {
	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;

	// Initialize the table object
	table->entry_type = NULL;
	table->entry_data = NULL;
	table->context = context;
	table->parent_table = parent_table;
	table->parent_index = parent_index;
	table->l2_table_size = 0;
	table->flags = 0;

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Deinitialize table object.
 * @param[in] table (btl_table *) pointer to the table.
 * @return (btl_result_t) status code.
 *
 *	This function destroys nested tables and frees all memory.
 */
btl_result_t btl_table_deinitialize(
	btl_table *		table
) {

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;

	// Clean up the table object
	if( 0 != table->l2_table_size ) {
		size_t i;
		size_t count = 1UL << table->l2_table_size;

		// Delete all nested tables
		for( i = 0; i < count; ++ i ) {
			if( btl_et_subtable == table->entry_type[i] ) {
				btl_table_destroy( table->entry_data[i].table );
			}
		}

		// Delete buffers
		if( 0 == (table->flags & BTL_TABLE_F_EXT_ARRAYS) ) {
			_btl_table_release_arrays( table );
		}
	}

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Destroy table object.
 * @param[in] table (btl_table *) pointer to the table.
 * @return (btl_result_t) status code.
 *
 *	This function deinitialized the object (by destroying nested tables and
 * freeing all memory) and then calls the table allocator to release the memory
 * used by the table object.
 */
btl_result_t btl_table_valid(
	btl_table * table
) {

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;

	// Exit
	return BTL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Release all arrays in the table object.
 * @param[in] table (btl_table *) pointer to the table object.
 * @return (btl_result_t) status code.
 */
btl_result_t _btl_table_release_arrays(
	btl_table *	table
	) {

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;
	if( 0 == table->l2_table_size )
		return BTL_SUCCESS;

	// Release buffers
	if( table->flags & BTL_TABLE_F_EXT_ARRAYS ) {

		table->context->heap_allocator->alloc(
			table->context->heap_allocator,
			&table->entry_type,
			0
			);

		table->context->heap_allocator->alloc(
			table->context->heap_allocator,
			&table->entry_data,
			0
			);
	}

	table->l2_table_size = 0;

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Set size of all arrays in the table object.
 * @param[in] table (btl_table *) pointer to the table object.
 * @param[in] l2_entry_count (size_t ) new number of entries. If 0, just release all arrays.
 * @return (btl_result_t) status code.
 */
btl_result_t btl_table_set_size(
	btl_table *	table,
	size_t		l2_entry_count
) {
	btl_result_t result;

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;
	if( table->l2_table_size == l2_entry_count )
		return BTL_SUCCESS;

	// Release previous data
	result = _btl_table_release_arrays( table );
	if( BTL_SUCCESS != result )
		return result;

	// Set new data
	if( 0 != l2_entry_count ) {
		result = table->context->heap_allocator->alloc(
			table->context->heap_allocator,
			&table->entry_type,
			1 << l2_entry_count
			);
		if( BTL_SUCCESS != result )
			return result;

		result = table->context->heap_allocator->alloc(
			table->context->heap_allocator,
			&table->entry_data,
			1 << l2_entry_count
			);
		if( BTL_SUCCESS != result ) {
			table->context->heap_allocator->alloc(
				table->context->heap_allocator,
				&table->entry_type,
				0
				);
			return result;
		}

		table->l2_table_size = (uint8_t) l2_entry_count;
	}

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Set user-created arrays.
 * @param[in] table (btl_table *) pointer to the table object.
 * @param[in] table (unsigned char *) pointer to the array of entry types.
 * @param[in] table (btl_entry_data *) pointer to the array of entry data.
 * @param[in] entry_count (size_t ) number of entries, should equal to 1 << table->l2_table_size.
 * @return (btl_result_t) status code.
 */
btl_result_t btl_table_set_arrays(
	btl_table *		table,
	unsigned char *	entry_type,
	btl_entry_data *entry_data,
	size_t			entry_count
) {
	btl_result_t result;

	// Check current state
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( (NULL == entry_type) != (NULL == entry_data) )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == entry_type && 0 != entry_count )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL != entry_type && 0 == entry_count )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( 0 != entry_count && entry_count != (1UL << table->l2_table_size) )
		return BTL_ERROR_INVALID_PARAMETER;

	// Release previous data
	result = _btl_table_release_arrays( table );
	if( BTL_SUCCESS != result )
		return result;

	// Set new data
	if( 0 != entry_count ) {
		table->entry_type = entry_type;
		table->entry_data = entry_data;
		table->flags |= BTL_TABLE_F_EXT_ARRAYS;
	}

	// Exit
	return BTL_SUCCESS;
}

/*END OF table.c*/
