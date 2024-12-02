/*libhuffman.h*/
#ifdef __cplusplus
extern "C" {
#endif // def __cplusplus

#include <stdint.h>

#define F2_CFG_STATIC_MEMORY_OSTREAM		1
#define F2_CFG_DYNAMIC_MEMORY_OSTREAM		1
#include <libf2.h>
#include <libbitt.h>

#ifdef _MSC_VER
# pragma warning(disable:4201)	//: nonstandard extension used : nameless struct/union
#endif // def _MSC_VER


#ifndef LIBHUFFMAN_CALLCONV
 #define LIBHUFFMAN_CALLCONV
#endif // ndef LIBHUFFMAN_CALLCONV


typedef struct libhuffman_binary		libhuffman_binary;
typedef struct libhuffman_block			libhuffman_block;
typedef struct libhuffman_client		libhuffman_client;
typedef struct libhuffman_context		libhuffman_context;
typedef struct libhuffman_decoder		libhuffman_decoder;
typedef struct libhuffman_decoder_table	libhuffman_decoder_table;
typedef struct libhuffman_encoder		libhuffman_encoder;
typedef struct libhuffman_encoder_table	libhuffman_encoder_table;
typedef struct libhuffman_stream		libhuffman_stream;


struct libhuffman_decoder_table {
	btl_context	bit_context;
};
f2_status_t f2_callconv libhuffman_binary_initialize( libhuffman_decoder_table * thisp );
f2_status_t f2_callconv libhuffman_binary_deinitialize( libhuffman_decoder_table * thisp );

struct libhuffman_block {
	size_t		bit_offset;
	unsigned	bit_length;
};

struct libhuffman_stream {
	libhuffman_binary *	binary;

	libhuffman_decoder_table *	table;

	libhuffman_block *	blocks;
	size_t				block_count;

	void *				data;
	size_t				data_bit_offset;
	size_t				data_bit_count;
};
f2_status_t f2_callconv libhuffman_stream_set_block_count( libhuffman_stream * thisp, size_t block_count );
f2_status_t f2_callconv libhuffman_stream_set_block( libhuffman_stream * thisp, size_t block_index, size_t bit_offset, size_t bit_count );
f2_status_t	f2_callconv libhuffman_stream_decode( libhuffman_stream * stream, f2_ostream * outp );

struct libhuffman_binary {
	libhuffman_context *context;
	libhuffman_decoder_table *	default_table;

	libhuffman_stream *	streams;
	size_t				stream_count;

//	uint8_t				l2_max_count;	//< log2 of maximum size of tables set by default to all tables, if not 0; otherwise size is not limited
};
f2_status_t f2_callconv libhuffman_binary_initialize( libhuffman_binary * thisp, libhuffman_context * context, libhuffman_decoder_table * default_table, size_t stream_count );
f2_status_t f2_callconv libhuffman_binary_deinitialize( libhuffman_binary * thisp );
f2_status_t f2_callconv libhuffman_set_stream( libhuffman_binary * thisp, size_t index,
	const void * data, size_t data_bit_offset, size_t data_bit_count, libhuffman_decoder_table * table );

struct libhuffman_decoder {
	libhuffman_context *	context;		//< common context
	libhuffman_binary *		binary;			//< input binary object
};
f2_status_t	f2_callconv libhuffman_decoder_initialize( libhuffman_decoder * thisp, libhuffman_context * context );
f2_status_t	f2_callconv libhuffman_decoder_deinitialize( libhuffman_decoder * thisp );
f2_status_t	f2_callconv libhuffman_decoder_set_binary( libhuffman_decoder * thisp, libhuffman_binary * binary );
f2_status_t	f2_callconv libhuffman_decoder_decode( libhuffman_decoder * thisp, libhuffman_binary * binary, f2_ostream * outp );

struct libhuffman_client {
	f2_status_t	(f2_callconv * launch_decoder)( libhuffman_client * thisp, libhuffman_stream * stream, f2_ostream * outp );
	f2_status_t	(f2_callconv * notify_status) ( libhuffman_client * thisp, f2_status_t status, void * ptr_param, size_t int_param );
};

struct libhuffman_context {
	f2_allocator *			allocator;		//< allocator used for dynamic memory management
	libhuffman_client *		client;			//< client object
};
f2_status_t	f2_callconv libhuffman_context_initialize( libhuffman_context * thisp, f2_allocator * allocator, libhuffman_client * client );
f2_status_t	f2_callconv libhuffman_context_deinitialize( libhuffman_context * thisp );
f2_status_t f2_callconv libhuffman_context_set_client( libhuffman_context * thisp, libhuffman_client * client );



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0

typedef struct libhuffman_allocator				libhuffman_allocator;
typedef struct libhuffman_decoder_table			libhuffman_decoder_table;
typedef struct libhuffman_decoder_table_entry	libhuffman_decoder_table_entry;

typedef struct libhuffman_encoder_list_entry	libhuffman_encoder_list_entry;
typedef struct libhuffman_encoder_table			libhuffman_encoder_table;
typedef struct libhuffman_encoder_table_entry	libhuffman_encoder_table_entry;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Common definitions

#ifndef LIBHUFFMAN_CALLCONV
 #define LIBHUFFMAN_CALLCONV
#endif // ndef LIBHUFFMAN_CALLCONV

#ifndef libhuffman_code_t
 typedef uint32_t	libhuffman_code_t;
 #define libhuffman_code_t	libhuffman_code_t
#endif	// libhuffman_code_t
#ifndef libhuffman_value_t
 typedef uint16_t	libhuffman_value_t;
 #define libhuffman_value_t	libhuffman_value_t
#endif // libhuffman_value_t

///! Bit run definition
typedef struct libhuffman_bit_run
{
	libhuffman_code_t	bits;			//< bits value
	uint8_t				length;			//< length of the run in bits, 1..16 (table size is 2..64K elements)
} libhuffman_bit_run;

///! Escape definition.
typedef struct libhuffman_escape
{
	const char *		name;			//< escape name as specified in the table description file
	libhuffman_bit_run	code;			//< encoding bits
	uint16_t			name_length;	//< escape name length (lets avoid extra calls to string compare functions)
} libhuffman_escape;

///! Element of code table
typedef struct libhuffman_code_desc
{
	libhuffman_bit_run	 code;			//< encoding bits (Huffman code)
	libhuffman_bit_run	value;			//< value encoded with the bit run
	unsigned			count;			//< number of value elements
	libhuffman_escape *	escape;			//< optional pointer to escape code, value is given as a parameter
	ptrdiff_t	callback_param;
} libhuffman_code_desc;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocator interfaces

/**
 * @brief Allocate new decoder table.
 * @param[ in] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[out] table (libhuffman_decoder_table **) pointer to variable receiving a pointer to the allocated structure.
 * @returns (f2_status_t) operation status code.
 *
 * No structure initialization is required; just an allocator.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_alloc_decoder_table)(
	libhuffman_allocator *		this_p,	//< pointer to the allocator structure
	libhuffman_decoder_table **	table	//< pointer to variable receiving pointer to the table
	);

/**
 * @brief Release all decoder tables.
 * @param[ in] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @returns (f2_status_t) operation status code.
 *
 *	This function emplies that the caller must keep track of all allocated decoder tables. Most convenient option is
 * to keep all these tables in a single buffer object and when this function is called, release entire buffer at once.
 *	Note that all entries belonging to the table must also be released.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_free_all_decoder_tables)(
	libhuffman_allocator *		this_p	//< pointer to the allocator structure
	);
/**
 * @brief Allocate new encoder table.
 * @param[ in] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[out] table (libhuffman_encoder_table **) pointer to variable receiving a pointer to the allocated structure.
 * @returns (f2_status_t) operation status code.
 *
 * No structure initialization is required; just an allocator.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_alloc_encoder_table)(
	libhuffman_allocator *		this_p,	//< pointer to the allocator structure
	libhuffman_encoder_table **	table	//< pointer to variable receiving pointer to the table
	);
/**
 * @brief Release all encoder tables.
 * @param[ in] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @returns (f2_status_t) operation status code.
 *
 *	This function emplies that the caller must keep track of all allocated decoder tables. Most convenient option is
 * to keep all these tables in a single buffer object and when this function is called, release entire buffer at once.
 *	Note that all entries belonging to the table must also be released.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_free_all_encoder_tables)(
	libhuffman_allocator *		this_p	//< pointer to the allocator structure
	);

/**
 * @brief Allocate code descriptors.
 * @param[in	] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[in,out] entry_ptr (libhuffman_code_desc **) pointer to variable receiving a pointer to the allocated structures.
 * @param[in	] new_count (size_t) required number of elements (0 == free elements).
 * @returns (f2_status_t) operation status code.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_realloc_code_descs)(
	libhuffman_allocator *		this_p,		//< pointer to the allocator structure
	libhuffman_code_desc **		desc_ptr,	//< pointer to variable, containing and receiving pointer to descs
	size_t						new_count	//< required number of entries
	);

/**
 * @brief Allocate decoder table entries.
 * @param[in	] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[in,out] table (libhuffman_decoder_table **) pointer to variable containing and receiving a pointer to the table.
 * @param[in,out] entry_ptr (libhuffman_decoder_table_entry **) pointer to variable receiving a pointer to the allocated structures.
 * @param[in	] new_count (size_t) required number of elements (0 == free elements).
 * @returns (f2_status_t) operation status code.
 *
 *	In the first place, library sets pointer to entries to nullptr and calls this function with non-0 new_count. Allocator
 * allocates required number of entries and returns the pointer. Next, library calls this function with non-nullptr pointer
 * and with 0-null new_count. The allocator implementation resizes the entry array and returns new pointer to it. In the end,
 * library calls this function with non-nullptr pointer and new_count == 0. Allocator releases memory occupied by the entries
 * and sets the pointer to nullptr.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_realloc_decoder_table_entries)(
	libhuffman_allocator *				this_p,		//< pointer to the allocator structure
	libhuffman_decoder_table **			table,		//< pointer to variable, containing and receiving pointer to table
	libhuffman_decoder_table_entry **	entry_ptr,	//< pointer to variable, containing and receiving pointer to entries
	size_t								new_count	//< required number of entries
	);
/**
 * @brief Allocate encoder table entries.
 * @param[in	] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[in,out] table (libhuffman_encoder_table **) pointer to variable containing and receiving a pointer to the table.
 * @param[in,out] entry_ptr (libhuffman_encoder_table_entry **) pointer to variable receiving a pointer to the allocated structures.
 * @param[in	] new_count (size_t) required number of elements (0 == free elements).
 * @returns (f2_status_t) operation status code.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_realloc_encoder_table_entries)(
	libhuffman_allocator *				this_p,		//< pointer to the allocator structure
	libhuffman_encoder_table **			table,		//< pointer to variable, containing and receiving pointer to table
	libhuffman_encoder_table_entry **	entry_ptr,	//< pointer to variable, containing and receiving pointer to entries
	size_t								new_count	//< required number of entries
	);

/**
 * @brief Allocate encoder lists entries.
 * @param[in	] this_p (libhuffman_allocator *) pointer to the allocator object.
 * @param[in,out] entry_ptr (libhuffman_encoder_list_entry **) pointer to variable receiving a pointer to the allocated structures.
 * @param[in	] new_count (size_t) required number of elements (0 == free elements).
 * @returns (f2_status_t) operation status code.
 */
typedef f2_status_t	(LIBHUFFMAN_CALLCONV * libhuffman_allocator_realloc_encoder_list_entries)(
	libhuffman_allocator *				this_p,		//< pointer to the allocator structure
	libhuffman_encoder_list_entry **	entry_ptr,	//< pointer to variable, containing and receiving pointer to entries
	size_t								new_count	//< required number of entries
	);

struct libhuffman_allocator
{
	libhuffman_allocator_alloc_decoder_table			alloc_decoder_table;
//	libhuffman_allocator_free_last_decoder_table		free_last_decoder_table;
	libhuffman_allocator_free_all_decoder_tables		free_all_decoder_tables;

	libhuffman_allocator_alloc_encoder_table			alloc_encoder_table;
//	libhuffman_allocator_free_last_decoder_table		free_last_decoder_table;
	libhuffman_allocator_free_all_encoder_tables		free_all_encoder_tables;

	libhuffman_allocator_realloc_code_descs				realloc_code_descs;
	libhuffman_allocator_realloc_decoder_table_entries	realloc_decoder_table_entries;
	libhuffman_allocator_realloc_encoder_table_entries	realloc_encoder_table_entries;
	libhuffman_allocator_realloc_encoder_list_entries	realloc_encoder_list_entries;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client

typedef struct libhuffman_client	libhuffman_client;

typedef enum libhuffman_client_message
{
	LIBHUFFMAN_CM_START_DECODE,		//< decode is about to start (param is not used and set to nullptr)
	LIBHUFFMAN_CM_END_DECODE,		//< decode is just finished (param is not used and set to nullptr)
	LIBHUFFMAN_CM_MESSAGE,			//< decoder detected a special code in the input stream (param = pointer to libhuffman_escape)
	LIBHUFFMAN_CM_START_ENCODE,		//< encode is about to start (param is not used and set to nullptr)
	LIBHUFFMAN_CM_END_ENCODE,		//< encode is just finished (param is not used and set to nullptr)
} libhuffman_client_message;

typedef f2_status_t	(LIBHUFFMAN_CALLCONV *libhuffman_client_callback)(
	libhuffman_client *			this_p,	//< pointer to the client structure
	libhuffman_client_message	msg,	//< message identifier
	const void *				param	//< message parameter
	);
struct libhuffman_client
{
	libhuffman_client_callback	callback;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Context

typedef struct libhuffman_context {
	libhuffman_allocator *	allocator;		//< allocator used for dynamic memory management
	libhuffman_client *		client;			//< client object

	libhuffman_code_desc *	code_list;		//< list of all codes
	size_t					code_count;		//< number of elements in the list

	libhuffman_decoder_table *	last_failed_decoder_table;	//< last decoder table which initialization is failed
	libhuffman_encoder_table *	last_failed_encoder_table;	//< last encoder table which initialization is failed

	uint8_t					l2_max_count;	//< log2 of maximum size of tables set by default to all tables, if not 0; otherwise size is not limited
} libhuffman_context;

/**
 * @brief Initialize context structure.
 * @param[in] this_p (libhuffman_context *) pointer to an uninitialized context structure.
 * @param[in] allocator (libhuffman_allocator *) optional pointer to an allocator object.
 * @param[in] client (libhuffman_client *) optional pointer to a client object.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_context_initialize(
	libhuffman_context *	this_p,
	libhuffman_allocator *	allocator
);
/**
 * @brief Deinitialize context structure.
 * @param[in] this_p (libhuffman_context *) pointer to an initialized context structure.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV	libhuffman_context_deinitialize(
	libhuffman_context *	this_p
);

/**
 * @brief Set client to the context object.
 * @param[in] this_p (libhuffman_context *) pointer to an initialized context structure.
 * @param[in] client (libhuffman_client *) pointer to the client structure.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_context_set_client(
	libhuffman_context *	this_p,
	libhuffman_client *		client
);

/**
 * @brief Append huffman codes to the context object.
 * @param[in] this_p (libhuffman_context *) pointer to an initialized context structure.
 * @param[in] client (libhuffman_client *) pointer to the client structure.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_context_append_codes(
	libhuffman_context *	context,
	libhuffman_code_desc *	code_list,
	size_t					count
);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Huffman decoder

typedef enum libhuffman_table_message_t
{
	LIBHUFFMAN_TM_DESTROY,
} libhuffman_table_message_t;

///! callback that called each time a new code is fetched
typedef f2_status_t (LIBHUFFMAN_CALLCONV * libhuffman_table_callback)(
	libhuffman_decoder_table *	this_p,
	libhuffman_table_message_t	message,
	ptrdiff_t					param
	);

/**! Table containing value codes.
 */
struct libhuffman_decoder_table {
	libhuffman_table_callback	callback;
	libhuffman_context *		context;
	const libhuffman_decoder_table_entry *	entry;
	//ptrdiff_t *							callback_entry;	// TODO: use a separate optional array
	uint8_t		l2_count;		// log2 of number of elements in the `entry' array, since size should always be multiple of 2
	uint8_t		l2_max_count;	// log2 of maximum size of the table, if not 0; otherwise size is not limited
};

///! callback that called each time a new code is fetched
typedef f2_bool_t (LIBHUFFMAN_CALLCONV * libhuffman_code_callback)(
	ptrdiff_t param
	);

struct libhuffman_decoder_table_entry {
	union {
		const libhuffman_decoder_table *sub_table;	//< sub-table that decodes bits that didn't fit this table
		libhuffman_escape *				escape;		//< pointer to escape descriptor
		libhuffman_bit_run				bit_run;	//< numeric value that is transmitted to the output stream
	};
	ptrdiff_t	callback_param;
	#define LIBHUFFMAN_DTE_TYPE_MASK	0x0007	//< mask of element types (OR'ed all VALUE, SUBTABLE, ESCAPE)
	#define LIBHUFFMAN_DTE_VALUE		0x0001	//< use bit_run as a result
	#define LIBHUFFMAN_DTE_SUBTABLE		0x0002	//< use sub_table to decode following bits
	#define LIBHUFFMAN_DTE_ESCAPE		0x0003	//< use escape to notify client on a special condition
	#define LIBHUFFMAN_DTE_CALLBACK		0x1000	//< call callback when this entry is hit
	unsigned	flags;
};

/**
 * @brief Create decoder table.
 * @param[out] this_p (libhuffman_decoder_table **) pointer to variable receiving pointer to the decoder table.
 * @param[ in] context (libhuffman_context *) pointer to context structure.
 * @param[ in] bit_run_width (unsigned) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @param[ in] code_array (libhuffman_decoder_table_entry *) optional pointer to array of code descriptors. If nullptr, they're taken from the context.
 * @param[ in] code_count (size_t) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @return (f2_status_t) operation status code.
*/
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_create(
	libhuffman_decoder_table **		this_p_p,		//< pointer to variable receiving pointer to the decoder table
	libhuffman_context *			context,		//< pointer to context structure
	unsigned						bit_run_width,	//< table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
	libhuffman_code_desc *			desc_array,		//< optional pointer to array of code descriptors. If nullptr, they're taken from the context.
	size_t							desc_count		//< number of elements in the code array.
	//libhuffman_decoder_table *	parent
);

/**
 * @brief Initialize decoder table.
 * @param[in] this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param[in] context (libhuffman_context *) pointer to context structure.
 * @param[in] bit_run_width (unsigned) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @param[in] code_array (const libhuffman_decoder_table_entry *) optional pointer to array of code descriptors. If nullptr, they're taken from the context.
 * @param[in] code_count (size_t) table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
 * @return (f2_status_t) operation status code.
*/
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_initialize(
	libhuffman_decoder_table *		this_p,			//< pointer to the decoder table
	libhuffman_context *			context,		//< pointer to context structure
	unsigned						bit_run_width,	//< table element width, in bits (e.g. 8 means that the table will have 256 entries); 0 = auto.
	const libhuffman_code_desc *	desc_array,		//< optional pointer to array of code descriptors. If nullptr, they're taken from the context.
	size_t							desc_count		//< number of elements in the code array.
	//libhuffman_decoder_table *			parent
);
/**
 * @brief Deinitialize decoder table.
 * @param[in] this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @return (f2_status_t) operation status code.
*/
/*f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_deinitialize(
	libhuffman_decoder_table *	this_p			//< pointer to the decoder table
);*/

/**
 * @brief Append several Huffman codes to the decoder table.
 * @param table (libhuffman_decoder_table *) pointer to the decoder table.
 * @param desc (libhuffman_decoder_table_entry *) pointer to he array of code descriptors.
 * @param count (size_t) number of elements in the array of code descriptors.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_append_codes(
	libhuffman_decoder_table *		this_p,
	const libhuffman_code_desc *	desc,
	size_t							count
);
/**
 * @brief Append a Huffman code to the decoder table as a numeric value.
 * @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param huffman_code_bit_run (const libhuffman_bit_run *) pointer to the Huffman code bits.
 * @param value_bit_run (const libhuffman_bit_run *) pointer to the target code bits.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_append_code_as_value(
	libhuffman_decoder_table *	this_p,
	const libhuffman_bit_run *	huffman_code_bit_run,
	const libhuffman_bit_run *	value_bit_run
);
/**
 * @brief Append a Huffman code to the decoder table as a escape.
 * @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
 * @param huffman_code_bit_run (const libhuffman_bit_run *) pointer to the Huffman code bits.
 * @param escape (libhuffman_escape *) pointer to the escape descriptor.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_append_code_as_escape(
	libhuffman_decoder_table *	this_p,
	const libhuffman_bit_run *	huffman_code_bit_run,
	libhuffman_escape *			escape
);

/**
* @brief Sets a cllback parameter for the specified entry.
* @param this_p (libhuffman_decoder_table *) pointer to the decoder table.
* @param callback_param (ptrdiff_t) callback parameter.
* @return (f2_status_t) operation status code.
*/
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decoder_table_set_callback(
	libhuffman_decoder_table *	this_p,
	const libhuffman_bit_run *	huffman_code_bit_run,
	ptrdiff_t					callback_param
);

///! Decode context
typedef struct libhuffman_decode_context {
	libhuffman_context *		context;		//< libhuffman context
	libhuffman_decoder_table *	table;			//< root table
	f2_istream *				istream;		//< stream the encoded data are fetched from
	f2_ostream *				ostream;		//< stream the decoded data are stored to
	libhuffman_code_callback	callback;		//< optional callback that is called each time the encoded code is recognized
} libhuffman_decode_context;

/**
 * @brief Initialize decoding context structure.
 * @param this_p (libhuffman_decode_context *) pointer to uninitialized decode context.
 * @param context (libhuffman_context *) pointer to the library context.
 * @param istream (f2_istream *) pointer to the input stream object.
 * @param ostream (f2_ostream *) pointer to the output stream object.
 * @param root_table (libhuffman_decoder_table *) pointer to the initialized decoder table object.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decode_context_initialize(
	libhuffman_decode_context *	this_p,
	libhuffman_context *		context,
	f2_istream *				istream,
	f2_ostream *				ostream,
	libhuffman_decoder_table *	root_table
);
/**
 * @brief Deinitialize decoding context structure.
 * @param this_p (libhuffman_decode_context *) pointer to initialized decode context.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decode_context_deinitialize(
	libhuffman_decode_context * this_p
);

/**
 * @brief Perform decoding.
 * @param this_p (libhuffman_decode_context *) pointer to initialized decode context.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_decode(
	libhuffman_decode_context * context
);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Huffman encoder

#define LIBHUFFMAN_MAX_DICT_SIZE		(sizeof(libhuffman_code_t)*8)

///! entry of the list contaning Huffman codes for sequences of source bits of different lengths
struct libhuffman_encoder_list_entry {
	libhuffman_code_t	code;				//< Huffman code bit run
	unsigned			length;				//< length of source bit sequence which is replaced with `code'
};
///! table entry corresponding to the source bit sequence
struct libhuffman_encoder_table_entry {
	libhuffman_encoder_list_entry *	list;	//< list of lengths and corresponding codes
	unsigned			count;				//< number of elements in the list
};
///! table containg array of bit sequence entries each of which corresponding to the group of Huffman codes
struct libhuffman_encoder_table {
	libhuffman_context *	context;		//< owner context
	libhuffman_encoder_table_entry *entry;	//< array of code bit runs
//	libhuffman_encoder_list_entry *	pool;	//< pool of list entries
	uint8_t					l2_entry_count;	//< log2 of a number of elements in the entry array
	uint8_t					l2_max_count;	//< log2 of a table size limit
};

/**
 * @brief Initialize encoder table object.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the uninitialized object.
 * @param[in] context (libhuffman_context *) pointer to the general context object.
 * @param[in] src_bit_width (unsigned) log2 of number of elements in the table.
 * @param[in] desc_array (const libhuffman_code_desc *) pointer to array of code descriptors.
 * @param[in] desc_count (size_t) number of code descriptors.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_initialize_encoder_table(
	libhuffman_encoder_table *	this_p,			//< pointer to the uninitialized object
	libhuffman_context *		context,		//< pointer to the general context object
	unsigned					src_bit_width,	//< maximum bit width of the source data
	const libhuffman_code_desc *desc_array,		//< optional pointer to array of code descriptors. If nullptr, they're taken from the context
	size_t						desc_count		//< number of elements in the code array
	);
/**
 * @brief Deinitialize encoder table object.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the initialized object.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_deinitialize_encoder_table(
	libhuffman_encoder_table * this_p
	);

/**
 * @brief Set codes.
 * @param[in] this_p (libhuffman_encoder_table *) pointer to the initialized object.
 * @param[in] codes (const libhuffman_code_desc *) pointer to the initialized object.
 * @param[in] count (size_t) pointer to the initialized object.
 * @return (f2_status_t) operation status code.
 */
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encoder_table_set_codes(
	libhuffman_encoder_table *		thisp,
	const libhuffman_code_desc *	codes,
	size_t							count
	);

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encoder_table_append_code(
	libhuffman_encoder_table *	thisp,
	libhuffman_value_t			value,
	unsigned					value_length,
	libhuffman_code_t			run,
	unsigned					run_length
	);



typedef struct libhuffman_encode_context {
	libhuffman_context *		context;
	libhuffman_encoder_table *	table;
	f2_istream *		istream;
	f2_ostream *		ostream;
	libhuffman_code_callback	callback;
	unsigned					value_bit_size;
} libhuffman_encode_context;

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_initialize_encode_context( libhuffman_encode_context * encode_context,
	libhuffman_context * context, libhuffman_encoder_table * table, f2_istream * istream, f2_ostream * ostream );
f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_deinitialize_encode_context( libhuffman_encode_context * encode_context );

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_build_encoder_table( libhuffman_encode_context * context );

f2_status_t	LIBHUFFMAN_CALLCONV libhuffman_encode( libhuffman_encode_context * context );

#endif // 0

#ifdef __cplusplus
}
#endif // def __cplusplus
/*END OF libhuffman.h*/
