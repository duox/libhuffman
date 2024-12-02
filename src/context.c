/*context.c*/
#include "pch.h"
#include "main.h"

/**
 * @brief Initialize context structure.
 * @param[in] context (libhuffman_context *) pointer to an uninitialized context structure.
 * @param[in] allocator (f2_allocator *) optional pointer to an allocator object.
 * @param[in] client (libhuffman_client *) optional pointer to a client object.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t LIBHUFFMAN_CALLCONV libhuffman_context_initialize (
	libhuffman_context *	context,
	f2_allocator *			allocator,
	libhuffman_client *		client
)
{
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	context->allocator = allocator;
	context->client = client;

	return F2_STATUS_SUCCESS;
}

/**
 * @brief Deinitialize context structure.
 * @param[in] context (libhuffman_context *) pointer to an initialized context structure.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t LIBHUFFMAN_CALLCONV libhuffman_context_deinitialize(
	libhuffman_context * context
)
{
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	context->allocator = nullptr;

	return F2_STATUS_SUCCESS;
}

/**
 * @brief Set client to the context object.
 * @param[in] context (libhuffman_context *) pointer to an initialized context structure.
 * @param[in] client (libhuffman_client *) pointer to the client structure.
 * @returns (f2_status_t) operation status code.
 */
f2_status_t LIBHUFFMAN_CALLCONV libhuffman_context_set_client(
	libhuffman_context *	context,
	libhuffman_client *		client
)
{
	debugbreak_if( nullptr == context )
		return F2_STATUS_ERROR_INVALID_PARAMETER;

	context->client = client;

	return F2_STATUS_SUCCESS;
}

/*END OF context.c*/
