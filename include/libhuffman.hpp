/*libhuffman.hpp*/

#include "libhuffman.h"

namespace libhuffman {
	typedef ::libhuffman_bit_run			bit_run;
	typedef ::libhuffman_client_callback	client_callback;
	typedef ::libhuffman_code_t				code_t;
	typedef ::libhuffman_code_desc			code_desc;
	typedef ::libhuffman_escape				escape;

	class allocator;
	class client;
	class context;
}

class LIBHUFFMAN_CALLCONV libhuffman::allocator : protected ::libhuffman_allocator
{
public:
	typedef libhuffman_allocator	super;
	typedef allocator				self;

	// TODO
};

class LIBHUFFMAN_CALLCONV libhuffman::client : protected ::libhuffman_client
{
public:
	typedef libhuffman_client	super;
	typedef libhuffman::client	self;

	explicit client( client_callback callback );
};

class LIBHUFFMAN_CALLCONV libhuffman::context : protected ::libhuffman_context
{
public:
	typedef libhuffman_context	super;
	typedef context				self;

	explicit context( libhuffman::allocator * allocator = nullptr );
			~context();

	libf2_status_t	set_client( libhuffman_client * client );
	const libhuffman_client *	get_client() const;
		  libhuffman_client *	get_client();

	libf2_status_t	append_codes( code_desc * code_list, size_t count );
};

/*END OF libhuffman.hpp*/
