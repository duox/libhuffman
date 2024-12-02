/*apphuffman_context.h*/

typedef enum apphuffman_mode_t {
	APPHUFFMAN_M_UNDEFINED,
	APPHUFFMAN_M_DECODE,
	APPHUFFMAN_M_ENCODE,
} apphuffman_mode_t;

typedef enum apphuffman_table_format_t
{
	APPHUFFMAN_TF_NONE,
	APPHUFFMAN_TF_BINARY,
	APPHUFFMAN_TF_TEXT,
	APPHUFFMAN_TF_INVALID
} apphuffman_table_format_t;

typedef struct apphuffman_context {
	apphuffman_mode_t			mode;
	apphuffman_table_format_t	format;
	char *	table_file;
	char *	input_file;
	char *	output_file;
} apphuffman_context;

libf2_status_t	apphuffman_initialize_context( apphuffman_context * thisp );
libf2_status_t	apphuffman_deinitialize_context( apphuffman_context * thisp );

libf2_status_t	apphuffman_set_context_table_file( apphuffman_context * thisp, const char * table_file );
libf2_status_t	apphuffman_set_context_input_file( apphuffman_context * thisp, const char * input_file );
libf2_status_t	apphuffman_set_context_output_file( apphuffman_context * thisp, const char * output_file );
libf2_status_t	apphuffman_set_context_output_table_mode( apphuffman_context * thisp, format );

libf2_status_t	apphuffman_process_context( apphuffman_context * thisp );

// Helper functions
libf2_status_t	libhuffman_callconv apphuffman_deserialize_encoder_table_file( libhuffman_encoder_table * table,
	const char * file );
libf2_status_t	libhuffman_callconv apphuffman_deserialize_encoder_table_stream( libhuffman_encoder_table * table,
	libf2_istream * istream );
//libf2_status_t	libhuffman_callconv apphuffman_serialize_encoder_table_file( const libhuffman_encoder_table * table,
//	const char * file );
//libf2_status_t	libhuffman_callconv apphuffman_serialize_encoder_table_stream( const libhuffman_encoder_table * table,
//	libf2_ostream * ostream );

libf2_status_t	libhuffman_callconv apphuffman_deserialize_decoder_table_file( libhuffman_decoder_table * table,
	const char * file );
libf2_status_t	libhuffman_callconv apphuffman_deserialize_decoder_table_stream( libhuffman_decoder_table * table,
	libf2_istream * istream );
//libf2_status_t	libhuffman_callconv apphuffman_serialize_decoder_table_file( const libhuffman_decoder_table * table,
//	const char * file );
//libf2_status_t	libhuffman_callconv apphuffman_serialize_decoder_table_stream( const libhuffman_decoder_table * table,
//	libf2_ostream * ostream );

/*END OF apphuffman_context.h*/
