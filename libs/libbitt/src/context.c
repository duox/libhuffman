/*context.c*/
/** @file
 *
 * @brief Common context management functions.
 */
#include "./internal.h"

/**
 * @brief Initialize context object.
 * @param[in] context (btl_context *) pointer to uninitialized context object.
 * @return (btl_result) status code.
 */
#undef btl_context_initialize
btl_result_t btl_context_initialize( btl_context * context )
{
	btl_result_t result;

	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;

	// Initialize the object
	context->heap_allocator = NULL;

	result = btl_table_initialize( &context->root_table, context, NULL, 0 );
	if( 0 != result )
		return result;

	// Exit
	return BTL_SUCCESS;
}
/**
 * @brief Deinitialize context object.
 * @param[in] context (btl_context *) pointer to initialized context object.
 * @return (btl_result) status code.
 */
#undef btl_context_deinitialize
btl_result_t btl_context_deinitialize( btl_context * context )
{
	btl_result_t result;

	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;

	// Deinitialize the object
	result = btl_table_deinitialize( &context->root_table );
	if( 0 != result )
		return result;

	// Exit
	return BTL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Append immediate element.
 * @param[in] context (btl_context *) pointer to the context object.
 * @param[in] bit_value (uint64_t) bit sequence in the form of an immediate integer.
 * @param[in] bit_count (unsigned) number of bits in the sequence.
 * @param[out] ref (btl_entry_ref *) optional pointer to the new entry "address".
 * @return (btl_result) status code.
 */
btl_result_t btl_append_imm_entry(
	btl_context *	context,
	uint64_t		bit_value,
	unsigned		bit_count,
	btl_entry_ref *	ref
	)
{
	return btl_append_ptr_entry( context, &bit_value, bit_count, ref );
}
/**
 * @brief Append long element.
 * @param[in] context (btl_context *) pointer to the context object.
 * @param[in] value (const void *) bit sequence in the form of a byte array.
 * @param[in] bit_count (unsigned) number of bits in the sequence.
 * @param[out] ref (btl_entry_ref *) optional pointer to the new entry "address".
 * @return (btl_result) status code.
 */
btl_result_t btl_append_ptr_entry(
	btl_context *	context,	//< pointer to context object
	const void *	value,		//< pointer to bits
	size_t			bit_count,	//< number of bits pointed to
	btl_entry_ref *	ref			//< pointer to optional structure receiving entry parameters
	)
{
	btl_result_t result;
	btl_bitfield_iterator iterator;
	btl_table * table;
	btl_table * subtable;
	uint8_t * entry_type;
	btl_entry_data * entry_data;
	unsigned index_bit_count;
	uint32_t index = (uint32_t) -1;

	// Check current state and prepare
	if( NULL != ref ) {
		ref->table = NULL;
		ref->index = (unsigned) -1;
	}

	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == value )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( 0 == bit_count )
		return BTL_ERROR_INVALID_PARAMETER;

	table = &context->root_table;

	// Generate entry sequence
	btl_bitfield_iterator_initialize( &iterator, value, 0, bit_count );
	for(;;)
	{
		// Fetch index
		result = btl_bitfield_iterator_fetch(
			&iterator,
			&index,
			table->l2_table_size,
			&index_bit_count
		);
		if( BTL_SUCCESS != result )
			return result;
		debugbreak_if( index >= (1UL << table->l2_table_size) )
			return BTL_ERROR_INVALID_PARAMETER;

		// Check entry status
		entry_type = &table->entry_type[index];
		entry_data = &table->entry_data[index];

		if( *entry_type == (uint8_t) btl_et_subtable ) {// iterate immidiately if it's a sub-table
			table = entry_data->table;
			continue;
		}
		if( *entry_type != (uint8_t) btl_et_unused )	// otherwise, this entry should be unused
			return BTL_ERROR_ENTRY_ALREADY_OCCUPIED;

		if( btl_bitfield_iterator_finished( &iterator ) )// if no more bits left, leave the loop
			break;

		// If some bits left, create subtable
		result = btl_table_create(
			context,		// current context
			&subtable,		// pointer to the new table
			index_bit_count,// log2 size of the table
			table,			// parent table
			index			// parent index
			);
		if( 0 != result )
			return result;

		entry_data->table = subtable;
		*entry_type = btl_et_subtable;

		// Iterate with the new table
		table = subtable;
	}

	// If bits exactly fit the table, create a single normal (non-subtable) entry
	if( table->l2_table_size == index_bit_count ) {
		entry_data->entry_ptr_param = NULL;
		entry_data->entry_int_param = 0;
		*entry_type = btl_et_data;
	}
	// If bits are less than the the table size, create a group of normal (non-subtable) entries
	else if( table->l2_table_size > index_bit_count ) {
		unsigned diff = table->l2_table_size - index_bit_count;
		unsigned i, count = 1 << diff;
		for( i = 0; i < count; ++ i ) {
			entry_data[i].entry_ptr_param = NULL;
			entry_data[i].entry_int_param = 0;
			entry_type[i] = btl_et_data;
		}
	}

	// Done
	if( NULL != ref ) {
		ref->table = table;
		ref->index = index;
	}

	// Exit
	return BTL_SUCCESS;
}

/*btl_result_t btl_remove_imm_entry(
	btl_context *	context,
	uint64_t		bit_value,
	unsigned		bit_count
	)
{
	return btl_remove_ptr_entry( context, &bit_value, bit_count );
}
btl_result_t btl_remove_ptr_entry(
	btl_context *	context,	//< pointer to context object
	const void *	value,		//< pointer to bits
	size_t			bit_count	//< number of bits pointed to
	)
{
	btl_table * table;

	table = &context->root_table;


}
btl_result_t btl_remove_ref_entry(
	btl_context *	context,
	btl_entry_ref *	ref
)
{
}*/

/**
 * @brief Remove all entries of all tables in the context.
 * @param[in] context (btl_context *) context object.
 * @return (btl_result) status code.
 */
btl_result_t btl_remove_all_entries(
	btl_context *	context
) {
	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;

	// Exit
	return btl_table_deinitialize( &context->root_table );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Find a table entry by the specified 64-bit bit sequence.
 * @param[in] context (btl_context *) context object.
 * @param[in] bit_value (uint64_t) bit sequence in the form of 64-bit integer.
 * @param[in] bit_count (unsigned) number of valid bits in the bit sequence.
 * @param[out] ref (btl_entry_ref *) optional pointer to a structure receiving entry reference data.
 * @return (btl_result) status code.
 */
btl_result_t btl_find_imm_entry64(
	btl_context *	context,
	uint64_t		bit_value,
	unsigned		bit_count,
	btl_entry_ref *	ref
) {
	debugbreak_if( bit_count > sizeof(bit_value) )
		return BTL_ERROR_INVALID_PARAMETER;

	return btl_find_ptr_entry(
		context,
		&bit_value,
		bit_count,
		ref
	);
}
/**
 * @brief Find a table entry by the specified 32-bit bit sequence.
 * @param[in] context (btl_context *) context object.
 * @param[in] bit_value (uint64_t) bit sequence in the form of 32-bit integer.
 * @param[in] bit_count (unsigned) number of valid bits in the bit sequence.
 * @param[out] ref (btl_entry_ref *) optional pointer to a structure receiving entry reference data.
 * @return (btl_result) status code.
 */
btl_result_t btl_find_imm_entry32(
	btl_context *	context,
	uint32_t		bit_value,
	unsigned		bit_count,
	btl_entry_ref *	ref
) {
	debugbreak_if( bit_count > sizeof(bit_value) )
		return BTL_ERROR_INVALID_PARAMETER;

	return btl_find_ptr_entry(
		context,
		&bit_value,
		bit_count,
		ref
	);
}
/**
 * @brief Find a table entry by the specified array bit sequence.
 * @param[in] context (btl_context *) context object.
 * @param[in] value (const void *) bit sequence in the form of the byte array.
 * @param[in] bit_count (unsigned) number of valid bits in the bit sequence.
 * @param[out] ref (btl_entry_ref *) optional pointer to a structure receiving entry reference data.
 * @return (btl_result) status code.
 */
btl_result_t btl_find_ptr_entry(
	btl_context *	context,
	const void *	value,
	size_t			bit_count,
	btl_entry_ref *	ref
) {
	btl_result_t			result;
	btl_bitfield_iterator	iterator;
	btl_table *		table;
	uint8_t *		entry_type;
	btl_entry_data *entry_data;
	unsigned		index_bit_count;
	uint32_t		index = (uint32_t) -1;

	// Check current state
	if( NULL != ref ) {
		ref->table = NULL;
		ref->index = (unsigned) -1;
	}

	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == value )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( 0 == bit_count )
		return BTL_ERROR_INVALID_PARAMETER;

	table = &context->root_table;

	// Perform search
	btl_bitfield_iterator_initialize( &iterator, value, 0, bit_count );
	for(;;) {

		// Fetch index
		result = btl_bitfield_iterator_fetch(
			&iterator,
			&index,
			table->l2_table_size,
			&index_bit_count
		);
		if( BTL_SUCCESS != result )
			return result;
		if( index >= (1UL << table->l2_table_size) )
			return BTL_ERROR_INVALID_PARAMETER;

		// Check entry status
		entry_type = &table->entry_type[index];
		entry_data = &table->entry_data[index];

		if( *entry_type != (uint8_t) btl_et_subtable )	// iterate immidiately if it's a sub-table
			break;

		table = entry_data->table;
	}

	if( NULL != ref ) {
		ref->table = table;
		ref->index = index;
	}

	if( !btl_bitfield_iterator_finished( &iterator ) )// if no more bits left, leave the loop
		return BTL_ERROR_INVALID_PARAMETER;

	// Exit
	return BTL_SUCCESS;
}

/**
 * @brief Check if entry reference is valid.
 * @param[in] context (btl_context *) context object.
 * @param[in] ref (btl_entry_ref *) pointer to entry reference.
 * @return (btl_result) status code.
 */
btl_result_t btl_is_entry_ref_valid(
	btl_context *	context,
	btl_entry_ref *	ref
)
{
	// Check current state
	debugbreak_if( NULL == ref )
		return BTL_ERROR_INVALID_PARAMETER;

	// Exit
	return btl_is_entry_tbl_valid( context, ref->table, ref->index );
}
/**
 * @brief Check if the table and the index are valid.
 * @param[in] context (btl_context *) context object.
 * @param[in] table (btl_table *) pointer to table object.
 * @param[in] index (unsigned) index.
 * @return (btl_result) status code.
 */
btl_result_t btl_is_entry_tbl_valid(
	btl_context *	context,
	btl_table *		table,
	unsigned		index
)
{
	btl_result_t result;

	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == table )
		return BTL_ERROR_INVALID_PARAMETER;

	// Loop checking entire hierarchy of the tables
	do {

		// Check table validity
		result = btl_table_valid( table );
		if( BTL_SUCCESS != result )
			return result;

		// Check index validity
		if( index >= (1UL << table->l2_table_size) )
			return BTL_ERROR_INVALID_SIZE;

		// Move level up
		index = table->parent_index;
		table = table->parent_table;

	} while( NULL != table );

	if( &context->root_table != table )	// the last table must be the root table of the context
		return BTL_ERROR_UNRELATED;

	// Exit
	return BTL_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Pefform bufer decode.
 * @param[in] context (btl_context *) context object.
 * @param[in] decode_callback (btl_decode_callback) callback to be called each time an entry is decoded.
 * @param[in] callback_param (void *) decode callback parameter.
 * @param[in] data (const void *) data buffer.
 * @param[in] data_bit_offset (size_t) offset of first valid bit.
 * @param[in] data_bit_count (size_t) number of valid bits in the data buffer.
 * @return (btl_result) status code.
 */
btl_result_t btl_decode(
	btl_context *		context,
	btl_decode_callback	decode_callback,
	void *				callback_param,
	const void *		data,
	unsigned			data_bit_offset,
	size_t				data_bit_count
)
{
	btl_result_t result;
	btl_bitfield_iterator iterator;
	btl_table *		table;
	uint8_t			entry_type;
	btl_entry_data *entry_data;
	unsigned		index_bit_count;
	uint32_t		index = (uint32_t) -1;

	// Check current state
	debugbreak_if( NULL == context )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == decode_callback )
		return BTL_ERROR_INVALID_PARAMETER;
	debugbreak_if( NULL == data )
		return BTL_ERROR_INVALID_PARAMETER;

	// Generate entry sequence
	btl_bitfield_iterator_initialize( &iterator, data, data_bit_offset, data_bit_count );
	for(;;)
	{
		// Start new bit sequence from the root table
		table = &context->root_table;

		// If entire buffer is passed, exit
		if( btl_bitfield_iterator_finished( &iterator ) )
			break;

		// Get next data or callback entry
		for(;;)
		{
			// Fetch index
			result = btl_bitfield_iterator_fetch(
				&iterator,
				&index,
				table->l2_table_size,
				&index_bit_count
			);
			if( BTL_SUCCESS != result )
				return result;
			debugbreak_if( index >= (1UL << table->l2_table_size) )
				return BTL_ERROR_INVALID_DATA;

			// Check entry status
			entry_type =  table->entry_type[index];
			entry_data = &table->entry_data[index];

			if( (uint8_t) btl_et_subtable != entry_type )	// iterate immidiately if it's a sub-table
				break;

			table = entry_data->table;
		}

		// If it's a callback entry, call the entry callback
		if( (uint8_t) btl_et_callback == entry_type )
		{
			debugbreak_if( NULL == entry_data->callback )
				return BTL_ERROR_NULL_CALLBACK;

			result = (*entry_data->callback)(
				entry_data->m_callback_param,
				table,
				index
			);
		// If it's a data entry, call the decode callback
		} else if( (uint8_t) btl_et_data == entry_type ) {
			result = (*decode_callback)(
				callback_param,
				entry_data->entry_ptr_param,
				entry_data->entry_int_param
			);
		}

		// Process callback result
		if( BTL_STOP == result )
			break;
		if( BTL_SUCCESS != result )
			return result;
	}

	// Exit
	return BTL_SUCCESS;
}

/*END OF context.c*/
