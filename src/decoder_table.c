/*decoder_table.c*/
/** @file
 * @brief Huffman decoder table management functions.
 *
 */
#include "pch.h"
#include "main.h"

/**
 * @brief Resize huffman table.
 * @internal
 * @param[in] table (libhuffman_decoder_table *) pointer to the Huffman decoder table.
 * @param[in] number of entries required.
 * @return (f2_status_t) operation status code.
 */
static f2_status_t
	_resize_table
	(
		libhuffman_decoder_table *	table,
		unsigned					entry_count
	)
{
	f2_status_t	status;
	unsigned		bit_count;

	size_t	old_count = 1UL << table->l2_count;
	if( entry_count <= old_count )
		return F2_STATUS_SUCCESS;
	entry_count = next_power_of_two( entry_count );	// TODO: check for already power of two

	bit_count = log2_uint64( entry_count );
	debugbreak_if( 0 != table->l2_max_count && table->l2_max_count < bit_count )
		return F2_STATUS_ERROR_INVALID_STATE;

	status = table->context->allocator->realloc_decoder_table_entries(
		table->context->allocator,
		&table,
		(libhuffman_decoder_table_entry **) &table->entry,
		entry_count*sizeof(libhuffman_decoder_table_entry)
		);
	if( f2_failed(status) )
		return status;
	table->l2_count = (uint8_t) bit_count;

	return F2_STATUS_SUCCESS;
}

/**
 * @brief Insert element into the table, creating intermediate layers if needed.
 * @internal
 * @param[in] root_table (libhuffman_decoder_table *) pointer to the table where the element must be inserted to.
 * @param[in] bit_run (const libhuffman_bit_run *) insertion position.
 * @param[out] en (libhuffman_decoder_table_entry **) pointer to variable receiving pointer to the new entry.
 * @return (f2_status_t) operation status code.
*/
static
f2_status_t
	_insert_element
	(
		libhuffman_decoder_table *	root_table,
		const libhuffman_bit_run *	bit_run,
		libhuffman_decoder_table_entry ** en
	)
{
	f2_status_t status;
	libhuffman_decoder_table *	table = root_table;
	unsigned bit_run_length = bit_run->length;
	unsigned bit_count_limit;

	debugbreak_if( 0 == bit_run_length )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( nullptr == en )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	*en = nullptr;

	for(;;)
	{
		// Get current table size limit log2
		if( 0 != table->l2_max_count && bit_run->length > table->l2_max_count )
			bit_count_limit = table->l2_max_count;			// size limit is specified inside the table
		else if( 0 != table->context->l2_max_count && bit_run->length > table->context->l2_max_count )
			bit_count_limit = table->context->l2_max_count;	// size limit is specified globally inside the context
		else
			bit_count_limit = bit_run->length;				// no size limit, use all bits

		// Alocate table entries first
		status = _resize_table( table, 1UL << bit_count_limit );
		if( f2_failed( status ) )
			return status;

		// Use or insert subtables if bit run doesn't fit this table
		if( bit_run->length > bit_count_limit )
		{
			libhuffman_decoder_table_entry * sub_table_entry = (libhuffman_decoder_table_entry *) &table->entry[1UL << bit_count_limit];	// element where the sub-table should exist
			if( 0 == (sub_table_entry->flags & LIBHUFFMAN_DTE_SUBTABLE) )
			{
				libhuffman_decoder_table * sub_table;

				if( 0 != (sub_table_entry->flags & LIBHUFFMAN_DTE_TYPE_MASK) )
					return F2_STATUS_ERROR_INVALID_DATA;	// location is already occupied by a non-table element

				status = libhuffman_decoder_table_create(
					&sub_table,
					table->context,
					0,
					nullptr,
					0
				);
				if( f2_failed( status ) )
					return status;
				sub_table_entry->sub_table = sub_table;
			}

			bit_run_length -= bit_count_limit;
			table = (libhuffman_decoder_table *) sub_table_entry->sub_table;
		}
		else
		{
			*en = (libhuffman_decoder_table_entry *) &table->entry[1UL << bit_run->length];		// element where the Huffman should be stored
			break;
		}
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Get element from the table, navigating intermediate layers if needed.
 * @param[in] root_table (libhuffman_decoder_table *) pointer to the table where the element must be inserted to.
 * @param[in] bit_run (const libhuffman_bit_run *) insertion position.
 * @param[out] en (libhuffman_decoder_table_entry **) pointer to variable receiving pointer to the existing entry.
 * @return (f2_status_t) operation status code.
*/
static
f2_status_t
	_get_element
	(
		libhuffman_decoder_table *	root_table,
		const libhuffman_bit_run *	bit_run,
		libhuffman_decoder_table_entry ** en
	)
{
	const libhuffman_decoder_table *	table = root_table;
	unsigned bit_run_length = bit_run->length;
	unsigned bit_count;
	size_t bit_mask, index;
	const libhuffman_decoder_table_entry * entry;
	libhuffman_code_t	bits;

	debugbreak_if( 0 == bit_run_length )
		return F2_STATUS_ERROR_INVALID_STATE;
	debugbreak_if( nullptr == en )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	*en = nullptr;
	bits = bit_run->bits;

	for(;;)
	{
		// Get pointer to element
		if( bit_run_length < table->l2_count )
			bit_count = bit_run_length;
		else
			bit_count = table->l2_count;
		bit_run_length -= bit_count;
		bit_mask = (1UL << bit_count) - 1;
		index = bits & bit_mask;
		entry = &table->entry[index];

		// If all bits are passed, return the entry found
		if( 0 == bit_run_length )
			break;

		// Navigate through the table
		if( 0 == (entry->flags & LIBHUFFMAN_DTE_SUBTABLE) )
			return F2_STATUS_ERROR_NOT_FOUND;
		table = entry->sub_table;
		bits >>= bit_count;
	}

	*en = (libhuffman_decoder_table_entry *) entry;

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create decoder table using user-supplied allocator.
 *
 * @param[out] this_p (libhuffman_decoder_table **) pointer to variable receiving pointer to the decoder table.
 * @param[ in] context (libhuffman_context *) pointer to context structure.
 * @param[ in] bit_run_width (unsigned) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @param[ in] code_array (libhuffman_decoder_table_entry *) optional pointer to array of code descriptors. If nullptr, they're taken from the context.
 * @param[ in] code_count (size_t) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @return (f2_status_t) operation status code.
 *
 *	Note that since all tables are only destroyed all at once in the end,
 * table structures can be allocated from a simple dynamic array such as
 * std::vector and all get erased via `clear()' call.
 */
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_create
	(
		libhuffman_decoder_table **		this_p_p,		///< pointer to variable receiving pointer to the decoder table
		libhuffman_context *			context,		///< pointer to context structure
		unsigned						bit_run_width,	///< table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
		libhuffman_code_desc *			desc_array,		///< optional pointer to array of code descriptors. If nullptr, they're taken from the context.
		size_t							desc_count		///< number of elements in the code array.
	)
{
	f2_status_t				status;
	libhuffman_decoder_table *	table;

	// Check current state
	debugbreak_if( nullptr == this_p_p )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	*this_p_p = nullptr;

	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context->allocator )
		return F2_STATUS_ERROR_INVALID_STATE;

	// Allocate object
	if( nullptr == context->last_failed_decoder_table )
	{
		status = context->allocator->alloc_decoder_table(
			context->allocator,
			&table
			);
		if( f2_failed( status ) )
			return status;
	}
	else
	{
		table = context->last_failed_decoder_table;
		context->last_failed_decoder_table = nullptr;
	}

	// Initialize object
	status = libhuffman_decoder_table_initialize(
		table,
		context,
		bit_run_width,
		desc_array,
		desc_count
		);
	if( f2_failed( status ) )
	{
		context->last_failed_decoder_table = table;
		return status;
	}

	// Exit
	*this_p_p = table;
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Initialize decoder table.
 * @param[in] this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param[in] context (libhuffman_context *) pointer to context structure.
 * @param[in] bit_run_width (unsigned) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @param[in] code_array (const libhuffman_decoder_table_entry *) optional pointer to array of code descriptors. If nullptr, they're taken from the context.
 * @param[in] code_count (size_t) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @return (f2_status_t) operation status code.
*/
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_initialize
	(
		libhuffman_decoder_table *				table,			///< pointer to the decoder table
		libhuffman_context *					context,		///< pointer to context structure
		unsigned								bit_run_width,	///< table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
		const libhuffman_code_desc *			desc_array,		///< optional pointer to array of code descriptors. If nullptr, they're taken from the context.
		size_t									desc_count		///< number of elements in the code array.
	)
{
	f2_status_t	status = F2_STATUS_SUCCESS;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize table
	table->context = context;
	table->callback = nullptr;
	table->entry = nullptr;
	table->l2_count = 0;
	table->l2_max_count = 0;

	// Allocate space for the table elements
	if( 0 != bit_run_width )
	{
		status = _resize_table(
			table,
			1UL << bit_run_width
			);
		if( f2_failed(status) )
			return status;
	}

	// Initialize contents
	if( nullptr != desc_array && 0 != desc_count )
	{
		status = libhuffman_decoder_table_append_codes(
			table,
			desc_array,
			desc_count
			);
	}
	else if( 0 != context->code_count )
	{
		status = libhuffman_decoder_table_append_codes(
			table,
			context->code_list,
			context->code_count
			);
	}
	if( f2_failed(status) )
		return status;

	// Exit
	return F2_STATUS_SUCCESS;
}
/*f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_deinitialize( libhuffman_decoder_table * table )
{
	size_t	i;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize table entries
	if( nullptr != table->entry )
	{
		libhuffman_decoder_table_entry * entry = (libhuffman_decoder_table_entry *) table->entry;
		size_t n = 1UL << table->l2_count;

		// Destroy all subtables
		for( i = 0; i < n; ++ i, ++ entry )
		{
			if( entry->flags & LIBHUFFMAN_DTE_SUBTABLE )
			{
				libhuffman_decoder_table * sub_table = (libhuffman_decoder_table *) entry->sub_table;
				libhuffman_decoder_table_deinitialize( sub_table );
				table->context->allocator->free( table->context->allocator, sub_table, sizeof(libhuffman_decoder_table), 0 );
			}
		}

		// Free memory
		table->context->allocator->free( table->context->allocator, (void *) table->entry, n*sizeof(*table->entry), 0 );
	}

	if( nullptr != table->callback )
		table->callback( table, LIBHUFFMAN_TM_DESTROY, 0 );

	// Exit
	return F2_STATUS_SUCCESS;
}*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Append several Huffman codes to the decoder table.
 * @param table (libhuffman_decoder_table *) pointer to the decoder table.
 * @param desc (libhuffman_decoder_table_entry *) pointer to he array of code descriptors.
 * @param count (size_t) number of elements in the array of code descriptors.
 * @return (f2_status_t) operation status code.
 */
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_append_codes
	(
		libhuffman_decoder_table *		table,
		const libhuffman_code_desc *	desc,
		size_t							count
	)
{
	f2_status_t	status;
	size_t i;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Append all codes
	for( i = 0; i < count; ++ i )
	{
		if( nullptr == desc->escape )
			status = libhuffman_decoder_table_append_code_as_escape( table, &desc[i].code, desc[i].escape );//, desc[i].callback_param );
		else
			status = libhuffman_decoder_table_append_code_as_value ( table, &desc[i].code, &desc[i].value );//, desc[i].callback_param );
		if( f2_failed( status ) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Append a Huffman code to the decoder table as a numeric value.
 * @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param huffman_code_bit_run (const libhuffman_bit_run *) pointer to the Huffman code bits.
 * @param value_bit_run (const libhuffman_bit_run *) pointer to the target code bits.
 * @return (f2_status_t) operation status code.
 */
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_append_code_as_value
	(
		libhuffman_decoder_table *	table,
		const libhuffman_bit_run *	huffman_code_bit_run,
		const libhuffman_bit_run *	value_bit_run
	)
{
#if 1
	f2_status_t	status;
	libhuffman_decoder_table_entry * en = nullptr;

	status = _insert_element( table, huffman_code_bit_run, &en );
	if( f2_failed(status) )
		return status;
	debugbreak_if( nullptr == en )
		return F2_STATUS_ERROR_INVALID_STATE;

	en->bit_run = *value_bit_run;
	en->flags	= LIBHUFFMAN_DTE_VALUE;

#else
	f2_status_t	status;
	libhuffman_decoder_table * current_table = table;
	libhuffman_decoder_table * next_table = nullptr;
	unsigned	bits_left = bit_length;
	uint8_t		bits;
	libhuffman_code_t	code_bits = code;
	unsigned	first_index;
	unsigned	index_count;
	libhuffman_decoder_table_entry * entry;
	libhuffman_decoder_table_entry * prev_entry = nullptr;
	libhuffman_decoder_table_entry * end_entry;
	libhuffman_decoder_table_entry * prev_end_entry = nullptr;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// For all components of the code, generate chain of tables
	while( bits_left > 0 ) {

		// Fetch another group of bits
		bits = (uint8_t) min( 8, bits_left );
		bits_left -= bits;

		// Calculate range of the entries that the group of bits should occupy
		index_count = 1 << (8 - bits);
		first_index = (unsigned) ( ( code_bits & ((1U << bits) - 1) ) << (8 - bits) );
		code_bits >>= bits;

		// Resize table to fit required number of entries
		if( current_table->size < first_index + index_count ) {
			status = _resize_table( table, first_index + index_count );
			if( f2_failed(status) )
				return status;
		}

		// Link table with the previous entry
		if( nullptr != prev_entry ) {
			for( ; prev_entry < prev_end_entry; ++ prev_entry ) {
				prev_entry->next_table = table;
				++ table->ref_count;
			}
		}

		// Initialize the entries
		next_table = (libhuffman_decoder_table *) table->entry->next_table;
		entry = prev_entry = (libhuffman_decoder_table_entry *) table->entry + first_index;
		end_entry = prev_end_entry = entry + index_count;
		for( ; entry < end_entry; ++ entry ) {

			// Check whether entry is already occupied as a end point
			debugbreak_if( 0 != entry->bit_length && 0 == bits_left )
				return F2_STATUS_ERROR_INVALID_ALPHABET;
			debugbreak_if( 0 != entry->bit_length && bits != entry->bit_length )
				return F2_STATUS_ERROR_INVALID_ALPHABET;
			debugbreak_if( next_table != entry->next_table )
				return F2_STATUS_ERROR_INVALID_ALPHABET;

			// Initialize the entry
			entry->bit_length = bits;
			if( 0 == bits_left ) {
				entry->callback_param = param;
			}
		}

		// Create new table if there's no next table
		if( nullptr == next_table ) {
			status = table->context->allocator->alloc( table->context->allocator, (void **) &next_table, sizeof(*next_table), 0 );
			if( f2_failed(status) )
				return status;

			status = libhuffman_initialize_decoder_table( next_table, table->context, table );
			if( f2_failed(status) ) {
				(void) table->context->allocator->free( table->context->allocator, next_table, sizeof(*next_table), 0 );
				return status;
			}
		}
		table = next_table;
	}
#endif
	// Exit
	return F2_STATUS_SUCCESS;
}
/**
 * @brief Append a Huffman code to the decoder table as a escape.
 * @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param huffman_code_bit_run (const libhuffman_bit_run *) pointer to the Huffman code bits.
 * @param escape (libhuffman_escape *) pointer to the escape descriptor.
 * @return (f2_status_t) operation status code.
 */
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_append_code_as_escape
	(
		libhuffman_decoder_table *	table,
		const libhuffman_bit_run *	huffman_code_bit_run,
		libhuffman_escape *			escape
	)
{
	f2_status_t	status;
	libhuffman_decoder_table_entry * en = nullptr;

	status = _insert_element( table, huffman_code_bit_run, &en );
	if( f2_failed(status) )
		return status;
	debugbreak_if( nullptr == en )
		return F2_STATUS_ERROR_INVALID_STATE;

	en->escape	= escape;
	en->flags	= LIBHUFFMAN_DTE_ESCAPE;

	return F2_STATUS_SUCCESS;
}

/**
* @brief Sets a cllback parameter for the specified entry.
* @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
* @param callback_param (ptrdiff_t) callback parameter.
* @return (f2_status_t) operation status code.
*/
f2_status_t
LIBHUFFMAN_CALLCONV
	libhuffman_decoder_table_set_callback
	(
		libhuffman_decoder_table *	table,
		const libhuffman_bit_run *	huffman_code_bit_run,
		ptrdiff_t					callback_param
	)
{
	f2_status_t	status;
	libhuffman_decoder_table_entry * en = nullptr;

	status = _get_element( table, huffman_code_bit_run, &en );
	if( f2_failed(status) )
		return status;
	debugbreak_if( nullptr == en )
		return F2_STATUS_ERROR_INVALID_STATE;

	en->callback_param = callback_param;
	en->flags |= LIBHUFFMAN_DTE_CALLBACK;

	return F2_STATUS_SUCCESS;
}

/*END OF decoder_table.c*/
