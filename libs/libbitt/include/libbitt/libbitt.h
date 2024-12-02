/*libbitt.h*/
/** @file
 *
 * @brief Bit table library.
 *
 * Copyright (c) duox.
 * Licensed under the MIT License.
 */
#include <stdint.h>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4201)	//: nonstandard extension used : nameless struct/union
#endif // def _MSC_VER


// non-standard callback attributes (e.g. __stdcall)
#ifndef BTL_CALLBACK
# define BTL_CALLBACK
#endif // ndef BTL_CALLBACK

typedef struct btl_heap_allocator	btl_heap_allocator;
typedef struct btl_table_allocator	btl_table_allocator;
typedef enum btl_result_t			btl_result_t;
typedef enum btl_entry_type			btl_entry_type;
typedef struct btl_table			btl_table;
typedef struct btl_entry_data		btl_entry_data;
typedef struct btl_entry_ref		btl_entry_ref;
typedef struct btl_context			btl_context;

//! Operation result codes
enum btl_result_t {
	BTL_SUCCESS,
	BTL_STOP,
	BTL_ERROR_INVALID_PARAMETER = -2000,
	BTL_ERROR_INVALID_ALLOCATOR,
	BTL_ERROR_INSUFFICIENT_MEMORY,
	BTL_ERROR_ENTRY_ALREADY_OCCUPIED,
	BTL_ERROR_NO_MORE_DATA,
	BTL_ERROR_INVALID_DATA,
	BTL_ERROR_INVALID_SIZE,
	BTL_ERROR_NULL_CALLBACK,
	BTL_ERROR_UNRELATED,
};

//! Generic allocator
struct btl_heap_allocator {
	/**
	 * @brief Allocator callback.
	 * @param[in] thisp (btl_heap_allocator *) pointer to the allocator object.
	 * @param[in] block_ptr (void **) pointer to variable, containing and/or receiving pointer to the memory block.
	 * @param[in] size (size_t) block size, in bytes.
	 *
	 *	This function can allocate, reallocate or free memory, depending on its arguments:
	 * - NULL == *block_ptr and 0 != size: allocate new memory block.
	 * - NULL != *block_ptr and 0 != size: reallocate existing memory block.
	 * - NULL != *block_ptr and 0 == size: free memory block.
	 * - NULL == *block_ptr and 0 == size: no-op, same as free( NULL )/operator delete( nullptr ).
	 * - all other combinations are invalid.
	 */
	btl_result_t (BTL_CALLBACK *alloc)(
		btl_heap_allocator *thisp,
		void **				block_ptr,
		size_t				size
	);
};

//! Table allocator
struct btl_table_allocator {
	/**
	 * @brief Allocates and initialize the table object
	 * @param[in] thisp (btl_table_allocator *) pointer to the allocator object.
	 * @param[in] context (btl_context *) pointer to context object.
	 * @param[in] table (btl_table **) pointer to variable receiving pointer to the allocated table.
	 * @param[in] l2_count (unsigned) log2 entry count.
	 *
	 *	The allocator function can allocate the table object and entries in a single block
	 * so btl_table_set_size will not be called during the table creation process.
	 */
	btl_result_t (BTL_CALLBACK *alloc_table)(
		btl_table_allocator *	thisp,		//< pointer to the btl_table_allocator structure
		btl_context *			context,	//< pointer to the context
		btl_table **			table,		//< pointer to variable receiving pointer to the allocated table
		unsigned				l2_count	//< log2 of the initial number of elements in the table (0 = default)
	);
	//< Release table. All subtables have already been deleted.
	btl_result_t (BTL_CALLBACK *release_table)(
		btl_table_allocator *	thisp,		//< pointer to the btl_table_allocator structure
		btl_context *			context,	//< pointer to the context
		btl_table *				table		//< pointer to the allocated table
	);
};

typedef btl_result_t (BTL_CALLBACK *btl_entry_callback)( void * param, btl_table * table, unsigned index );
typedef btl_result_t (BTL_CALLBACK *btl_decode_callback)( void * param, const void * entry_ptr_param, size_t entry_int_param );

//! Table entry types; types for all entries are located in the btl_table::entry_type array in form of uint8_t's
enum btl_entry_type {
	btl_et_unused,		//< table entry is not used
	btl_et_subtable,	//< btl_entry_data::table is valid
	btl_et_callback,	//< btl_entry_data::callback and btl_entry_data::callback_param are valid
	btl_et_data,		//< btl_entry_data::entry_ptr_param and btl_entry_data::entry_int_param are valid
};
//! Table entry data; data for all entries are located in the btl_table::entry_data array.
struct btl_entry_data {
	union {
		btl_table *			table;			//< pointer to sub-table
		btl_entry_callback	callback;		//< pointer to callback function
		const void *		entry_ptr_param;//< data pointer
	};
	union {
		size_t	entry_int_param;			//< data integer
		void *	callback_param;				//< callback parameter (first parameter of the btl_entry_callback)
	};
};
//! Entry address (where the entry is located)
struct btl_entry_ref {
	btl_table *		table;				//< table that contains the entry
	unsigned		index;				//< zero-based position of the entry in the table
};
//! Table object
struct btl_table {
	unsigned char *	entry_type;			//< pointer to 1<<l2_table_size elements of `btl_entry_type'
	btl_entry_data*	entry_data;			//< pointer to 1<<l2_table_size elements of `btl_entry_data'
	btl_context *	context;			//< pointer to context object
	btl_table *		parent_table;		//< pointer to the parent table
	unsigned		parent_index;		//< first index in the parent table
	uint8_t			l2_table_size;		//< log2 of number of entries in the `entry_type' and `entry_data' arrays (0 = not set)

	#define BTL_TABLE_F_EXT_ARRAYS	0x01	//< arrays were set by the btl_table_set_arrays function
	uint8_t			flags;				//< state flags
};
#define BTL_TABLE_INITIALZIE()	{ (unsigned char *)NULL, (btl_entry_data *)NULL, (btl_context *) NULL, (btl_table *)NULL, 0, 0, 0 }
btl_result_t	btl_table_initialize( btl_table * table, btl_context * context, btl_table * parent_table, unsigned parent_index );
#define btl_table_initialize( _table, _context, _parent_table, _parent_index )	(\
	NULL == (_table) ?\
		BTL_ERROR_INVALID_PARAMETER :\
		(\
			(_table)->entry_type = NULL,\
			(_table)->entry_data = NULL,\
			(_table)->context = _context,\
			(_table)->parent_table = _parent_table,\
			(_table)->parent_index = _parent_index,\
			(_table)->l2_table_size = 0,\
			(_table)->flags = 0,\
			BTL_SUCCESS\
		)\
	)
btl_result_t	btl_table_deinitialize( btl_table * table );
btl_result_t	btl_table_valid( btl_table * table );

btl_result_t	btl_table_set_size( btl_table * table, size_t l2_entry_count );
btl_result_t	btl_table_set_arrays( btl_table * table, unsigned char * entry_type, btl_entry_data * entry_data, size_t entry_count );

struct btl_context {
	btl_table				root_table;
	btl_heap_allocator *	heap_allocator;		//< raw memory allocator
	btl_table_allocator *	table_allocator;	//< table object allocator
};
#define BTL_CONTEXT_INITIALZIE()	{ BTL_TABLE_INITIALIZE(), NULL }
btl_result_t	btl_context_initialize( btl_context * context );
#define btl_context_initialize( context )	(\
	NULL == (context) ?\
		BTL_ERROR_INVALID_PARAMETER :\
		(\
			(context)->allocator = NULL,\
			btl_table_initialize( &(context)->root_table )\
		)\
	)
btl_result_t	btl_context_deinitialize( btl_context * context );
#define btl_context_deinitialize( context )	(\
	NULL == (context) ?\
		BTL_ERROR_INVALID_PARAMETER :\
		(\
			btl_table_deinitialize( &(context)->root_table )\
		)\
	)

btl_result_t	btl_append_imm_entry( btl_context * context, uint64_t bit_value, unsigned bit_count, btl_entry_ref * ref );
btl_result_t	btl_append_ptr_entry( btl_context * context, const void * value, size_t bit_count, btl_entry_ref * ref );
//btl_result_t	btl_remove_imm_entry( btl_context * context, uint64_t bit_value, unsigned bit_count );
//btl_result_t	btl_remove_ptr_entry( btl_context * context, const void * value, size_t bit_count );
//btl_result_t	btl_remove_ref_entry( btl_context * context, btl_entry_ref * ref );
btl_result_t	btl_remove_all_entries( btl_context * context );

btl_result_t	btl_find_imm_entry64( btl_context * context, uint64_t bit_value, unsigned bit_count, btl_entry_ref * ref );
btl_result_t	btl_find_imm_entry32( btl_context * context, uint32_t bit_value, unsigned bit_count, btl_entry_ref * ref );
btl_result_t	btl_find_ptr_entry( btl_context * context, const void * value, size_t bit_count, btl_entry_ref * ref );

btl_result_t	btl_is_entry_ref_valid( btl_context * context, btl_entry_ref * ref );
btl_result_t	btl_is_entry_tbl_valid( btl_context * context, btl_table * table, unsigned index );

btl_result_t	btl_decode( btl_context * context,
	btl_decode_callback decode_callback, void * callback_param,
	const void * data, unsigned data_bit_offset, size_t data_bit_count );


#ifdef _MSC_VER
# pragma warning(pop)
#endif // def _MSC_VER

/*END OF libbitt.h*/
