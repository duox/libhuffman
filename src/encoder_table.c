/*encoder_table.c*/
#include "pch.h"
#include "main.h"

/**
 * @brief Initialize encoder table object.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the uninitialized object.
 * @param[in] context (libhuffman_context *) pointer to the general context object.
 * @param[in] src_bit_width (unsigned) log2 of number of elements in the table.
 * @param[in] desc_array (const libhuffman_code_desc *) pointer to array of code descriptors.
 * @param[in] desc_count (size_t) number of code descriptors.
 * @return (f2_status_t) operation status code.
 */
f2_status_t LIBHUFFMAN_CALLCONV libhuffman_initialize_encoder_table(
	libhuffman_encoder_table *	this_p,			///< pointer to the uninitialized object
	libhuffman_context *		context,		///< pointer to the general context object
	unsigned					src_bit_width,	///< maximum bit width of the source data
	const libhuffman_code_desc *desc_array,		///< optional pointer to array of code descriptors. If nullptr, they're taken from the context.
	size_t						desc_count		///< number of elements in the code array.
	)
{
	f2_status_t status;

	// Check current state
	debugbreak_if( nullptr == this_p )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( src_bit_width >= sizeof(unsigned)*8 )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize object
	this_p->context = context;
	this_p->entry = nullptr;
	//this_p->pool = nullptr;

	if( 0 != src_bit_width )
	{
		status = context->allocator->realloc_encoder_table_entries(
			context->allocator,
			&this_p,
			&this_p->entry,
			1UL << src_bit_width
			);
		if( f2_failed( status ) )
			return status;

		this_p->l2_entry_count	= (uint8_t) src_bit_width;
		this_p->l2_max_count	= (uint8_t) src_bit_width;
	}

	if( nullptr != desc_array )
	{
		status = libhuffman_encoder_table_set_codes(
			this_p,
			desc_array,
			desc_count
			);
		if( f2_failed( status ) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}
/**
 * @brief Deinitialize encoder table object.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the initialized object.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_deinitialize_encoder_table(
	libhuffman_encoder_table * this_p
	)
{
	// Check current state
	debugbreak_if( nullptr == this_p )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == this_p->context )
		return F2_STATUS_ERROR_NOT_INITIALIZED;

	// Release memory
	if( nullptr != this_p->entry )
	{
		debugbreak_if( nullptr == this_p->context->allocator )
			return F2_STATUS_ERROR_INVALID_STATE;
		debugbreak_if( nullptr == this_p->context->allocator->realloc_encoder_table_entries )
			return F2_STATUS_ERROR_INVALID_INTERFACE;

		this_p->context->allocator->realloc_encoder_table_entries(
			this_p->context->allocator,
			&this_p,
			&this_p->entry,
			0
			);
	}
	/*if( nullptr != this_p->pool )
	{
		debugbreak_if( nullptr == this_p->context->allocator )
			return F2_STATUS_ERROR_INVALID_STATE;

		this_p->context->allocator->realloc_encoder_list_entries(
			this_p->context->allocator,
			&this_p->pool,
			0
			);
	}*/

	// Deinitialize object
	this_p->context = nullptr;

	// Exit
	return F2_STATUS_SUCCESS;
}

/**
 * @brief Set codes.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the initialized object.
 * @param[in] codes (const libhuffman_code_desc *) pointer to the initialized object.
 * @param[in] count (size_t) pointer to the initialized object.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encoder_table_set_codes(
	libhuffman_encoder_table *		this_p,
	const libhuffman_code_desc *	codes,
	size_t							count
	)
{
	f2_status_t status;
	size_t max_size;
	size_t cur_size;
	size_t new_size;
	unsigned cur_bit_len, new_bit_len;
	size_t i;

	// Check current state
	debugbreak_if( nullptr == this_p )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == codes )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	if( 0 == count )
		return F2_STATUS_SUCCESS;

	max_size = 0 == this_p->l2_max_count ?	// maximum number of entries allowed
		(size_t) -1 :
		1UL << this_p->l2_max_count;

	cur_size = 0 == this_p->l2_entry_count ?// current number of entries
		0 :
		1UL << this_p->l2_entry_count;
	new_size = cur_size + count;			// number of entries after new entries are added

	debugbreak_if( new_size > max_size )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Determine required size of the table
	cur_bit_len = 0 == this_p->l2_max_count ?
		this_p->l2_entry_count : this_p->l2_max_count;
	new_bit_len = cur_bit_len;
	for( i = 0; i < count; ++ i )
	{
		if( codes[i].code.length > new_bit_len )
			new_bit_len = codes[i].code.length;
	}

	// Resize table if the new bit length is greater than existing
	if( new_bit_len > cur_bit_len )
	{
		// Allocate more elements
		libhuffman_encoder_table_entry * entry = nullptr;
		status = this_p->context->allocator->realloc_encoder_table_entries(
			this_p->context->allocator,
			&this_p,
			&entry,
			1UL << new_bit_len
			);
		if( f2_failed( status ) )
			return status;

		// Copy data from the old table
		{
		const unsigned sub_count = 1UL << (new_bit_len - cur_bit_len);
		libhuffman_encoder_table_entry * new_entry = entry;
		libhuffman_encoder_table_entry * old_entry = this_p->entry;
		libhuffman_encoder_table_entry * old_entry_end = this_p->entry + cur_size;
		for( ; old_entry < old_entry_end; ++ old_entry )
		{
			libhuffman_encoder_table_entry * new_entry_end = entry + sub_count;
			for( ; new_entry < new_entry_end; ++ new_entry )
			{
				new_entry->list = nullptr;
				new_entry->count = old_entry->count;
				status = this_p->context->allocator->realloc_encoder_list_entries(
					this_p->context->allocator,
					&new_entry->list,
					new_entry->count
					);
				if( f2_failed( status ) )
					return status;

				f2_memcpy(
					new_entry->list,
					old_entry->list,
					new_entry->count * sizeof(libhuffman_encoder_list_entry)
					);
			}

			status = this_p->context->allocator->realloc_encoder_list_entries(
				this_p->context->allocator,
				&old_entry->list,
				0
				);
		}

		// Store new elements
		for( i = 0; i < count; ++ i, ++ new_entry )
		{
		}
		}
	}
	else if( new_bit_len < cur_bit_len )
	{
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

#if 0
static f2_status_t
	_resize_table(
		libhuffman_encoder_table *	table,
		unsigned					entry_count
	)
{
	f2_status_t	status;

	if( entry_count < table->size )
		return F2_STATUS_SUCCESS;
	debugbreak_if( LIBHUFFMAN_MAX_DICT_SIZE < entry_count )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	status = nullptr == table->codes ?
		table->context->allocator->alloc( table->context->allocator, (void **)&table->codes, entry_count*sizeof(*table->codes), F2_AF_CLEAR_MEM ):
		table->context->allocator->realloc( table->context->allocator, (void **)&table->codes, (void *) table->codes, entry_count*sizeof(*table->codes), F2_AF_CLEAR_MEM );
	if( f2_failed( status ) )
		return status;

	status = nullptr == table->code_lengths ?
		table->context->allocator->alloc( table->context->allocator, (void **)&table->code_lengths, entry_count*sizeof(*table->codes), F2_AF_CLEAR_MEM ):
		table->context->allocator->realloc( table->context->allocator, (void **)&table->code_lengths, (void *) table->code_lengths, entry_count*sizeof(*table->code_lengths), F2_AF_CLEAR_MEM );
	if( f2_failed( status ) )
		return status;

	table->size = (uint8_t) entry_count;

	return F2_STATUS_SUCCESS;
}


f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_initialize_encoder_table( libhuffman_encoder_table * table,
	libhuffman_context * context, unsigned initial_count )
{
	f2_status_t	status;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Initialize table
	table->context = context;
	table->codes = nullptr;
	table->code_lengths = nullptr;
	table->size = 0;

	if( 0 != initial_count ) {
		status = _resize_table( table, initial_count );
		if( f2_failed( status ) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_deinitialize_encoder_table( libhuffman_encoder_table * table )
{
	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	// Deinitialize table
	table->context->allocator->free( table->context->allocator, (void *) table->codes, table->size*sizeof(*table->codes), 0 );
	table->context->allocator->free( table->context->allocator, (void *) table->code_lengths, table->size*sizeof(*table->code_lengths), 0 );
	table->codes = nullptr;
	table->code_lengths = nullptr;
	table->size = 0;

	// Exit
	return F2_STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encoder_table_append_codes( libhuffman_encoder_table * table,
	const libhuffman_code_desc * desc, size_t count )
{
	f2_status_t	status;
	size_t	 i;
	libhuffman_code_t max_index;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == desc )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	if( 0 == count )
		return F2_STATUS_SUCCESS;

	// Prepare
	max_index = desc[0].value & ((1ULL << desc[0].value_bit_len) - 1);
	for( i = 1; i < count; ++ i ) {
		libhuffman_code_t value =  desc[i].value & ((1ULL << desc[0].value_bit_len) - 1);
		if( value > max_index )
			max_index = value;
	}

	if( max_index > table->size ) {
		status = _resize_table( table, max_index + 1 );
		if( f2_failed( status ) )
			return status;
	}

	// Append all codes
	for( i = 0; i < count; ++ i ) {
		status = libhuffman_encoder_table_append_code( table, desc[i].value, desc[i].value_bit_len, desc[i].code, desc[i].code_bit_len );
		if( f2_failed( status ) )
			return status;
	}

	// Exit
	return F2_STATUS_SUCCESS;
}

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encoder_table_append_code( libhuffman_encoder_table * table,
	libhuffman_value_t value, unsigned value_length, libhuffman_code_t run, unsigned run_length )
{
	f2_status_t	status;
	unsigned index;

	// Check current state
	debugbreak_if( nullptr == table )
		return F2_STATUS_ERROR_INVALID_PARAMETER;
	debugbreak_if( nullptr == table->context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	index = (unsigned) ( value & ((1 << value_length) - 1) );

	// Resize table if needed
	if( index >= table->size ) {
		status = _resize_table( table, index + 1 );
		if( f2_failed( status ) )
			return status;
	}

	table->codes[index] = run;
	table->code_lengths[index] = (uint8_t) run_length;

	// Exit
	return F2_STATUS_SUCCESS;
}
#endif
/*END OF encoder_table.c*/
