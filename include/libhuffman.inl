/*libhuffman.inl*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline
libhuffman::context::context( libhuffman::allocator * allocator )
{
	::libhuffman_context_initialize( this, allocator );
}
inline
libhuffman::context::~context()
{
	::libhuffman_context_deinitialize( this );
}

inline
libf2_status_t libhuffman::context::set_client( client * client )
{
	return ::libhuffman_context_set_client( this, client );
}
inline
const libhuffman::client * libhuffman::context::get_client() const
{
	return super::client;
}
inline
libhuffman::client * libhuffman::context::get_client()
{
	return super::client;
}
inline
libf2_status_t libhuffman::context::append_codes( code_desc * code_list, size_t count )
{
	return ::libhuffman_context_append_codes( this, code_list, count );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*END OF libhuffman.inl*/
