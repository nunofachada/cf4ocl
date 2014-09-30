/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with cf4ocl. If not, see
 * <http://www.gnu.org/licenses/>.
 * */

 /**
 * @file
 *
 * Definition of a wrapper class and its methods for OpenCL buffer
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_BUFFER_WRAPPER_H_
#define _CCL_BUFFER_WRAPPER_H_

#include "memobj_wrapper.h"

/**
 * @defgroup BUFFER_WRAPPER Buffer wrapper
 *
 * The buffer wrapper module provides functionality for simple
 * handling of OpenCL buffer objects.
 *
 * All the functions in this module are direct wrappers of the
 * respective OpenCL buffer functions, except for the
 * ::ccl_buffer_new_from_region() function. This function wraps
 * clCreateSubBuffer() but assumes that the sub-buffer will represent a
 * specific region in the original buffer (which is the only sub-buffer
 * type, up to OpenCL 2.0).
 *
 * Buffer wrapper objects can be directly passed as kernel arguments to
 * functions such as ::ccl_kernel_set_args_and_enqueue_ndrange() or
 * ::ccl_kernel_set_args_v().
 *
 * Information about buffer objects can be fetched using the
 * @ref ug_getinfo "info macros" from the
 * @ref MEMOBJ_WRAPPER "memory object module":
 *
 * * ::ccl_memobj_get_info_scalar()
 * * ::ccl_memobj_get_info_array()
 * * ::ccl_memobj_get_info()
 *
 * Instantiation and destruction of buffer wrappers follows the
 * _cf4ocl_ @ref ug_new_destroy "new/destroy" rule.
 *
 * _Example:_
 *
 * @code{.c}
 * CCLBuffer* buf;
 * cl_float host_data[BSIZE];
 * size_t buf_size = BSIZE * sizeof(cl_float);
 * @endcode
 *
 * @code{.c}
 * buf = ccl_buffer_new(
 *     context, CL_MEM_READ_WRITE, buf_size, NULL, NULL);
 * @endcode
 *
 * @code{.c}
 * ccl_buffer_enqueue_write(queue, buf, CL_TRUE, 0, buf_size,
 *     host_data, NULL, NULL);
 * @endcode
 *
 * @code{.c}
 * ccl_buffer_enqueue_read(queue, buf, CL_TRUE, 0, buf_size,
 *     host_data, NULL, NULL);
 * @endcode
 *
 * @code{.c}
 * ccl_buffer_destroy(buf);
 * @endcode
 *
 * @{
 */

/* Get the buffer wrapper for the given OpenCL buffer. */
CCLBuffer* ccl_buffer_new_wrap(cl_mem mem_object);

/* Create a ::CCLBuffer wrapper object. */
CCLBuffer* ccl_buffer_new(CCLContext* ctx, cl_mem_flags flags,
	size_t size, void *host_ptr, GError** err);

/* Decrements the reference count of the wrapper object. If it
 * reaches 0, the wrapper object is destroyed. */
void ccl_buffer_destroy(CCLBuffer* buf);

/* Read from a buffer object to host memory. */
CCLEvent* ccl_buffer_enqueue_read(CCLBuffer* buf, CCLQueue* cq,
	cl_bool blocking_read, size_t offset, size_t size, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Write to a buffer object from host memory. */
CCLEvent* ccl_buffer_enqueue_write(CCLBuffer* buf, CCLQueue* cq,
	cl_bool blocking_write, size_t offset, size_t size, void *ptr,
 	CCLEventWaitList* evt_wait_lst, GError** err);

/* Map a region of the buffer object given by buffer into the host
 * address space and returns a pointer to this mapped region. */
void* ccl_buffer_enqueue_map(CCLBuffer* buf, CCLQueue* cq,
	cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, CCLEventWaitList* evt_wait_lst, CCLEvent** evt,
	GError** err);

/* Copy from one buffer object to another. */
CCLEvent* ccl_buffer_enqueue_copy(CCLBuffer* src_buf,
	CCLBuffer* dst_buf, CCLQueue* cq, size_t src_offset,
	size_t dst_offset, size_t size, CCLEventWaitList* evt_wait_lst,
	GError** err);

/* Copy a buffer object to an image object. This function wraps the
 * clEnqueueCopyBufferToImage() OpenCL function. */
CCLEvent* ccl_buffer_enqueue_copy_to_image(CCLBuffer* src_buf,
	CCLImage* dst_img, CCLQueue* cq, size_t src_offset,
	const size_t *dst_origin, const size_t *region,
	CCLEventWaitList* evt_wait_lst, GError** err);

#ifdef CL_VERSION_1_1

/* Creates a sub-buffer that represents a specific region in the given
 * buffer. */
CCLBuffer* ccl_buffer_new_from_region(CCLBuffer* buf,
	cl_mem_flags flags, size_t origin, size_t size, GError** err);

/* Read from a 2D or 3D rectangular region from a buffer object to host
 * memory. */
CCLEvent* ccl_buffer_enqueue_read_rect(CCLBuffer* buf, CCLQueue* cq,
	cl_bool blocking_read, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region,
	size_t buffer_row_pitch, size_t buffer_slice_pitch,
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Write a 2D or 3D rectangular region to a buffer object from host
 * memory. */
CCLEvent* ccl_buffer_enqueue_write_rect(CCLBuffer* buf, CCLQueue* cq,
	cl_bool blocking_write, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region,
	size_t buffer_row_pitch, size_t buffer_slice_pitch,
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Copy a 2D or 3D rectangular region from a buffer object to another
 * buffer object. */
CCLEvent* ccl_buffer_enqueue_copy_rect(CCLBuffer* src_buf,
	CCLBuffer* dst_buf, CCLQueue* cq, const size_t *src_origin,
	const size_t *dst_origin, const size_t *region,
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
	size_t dst_slice_pitch, CCLEventWaitList* evt_wait_lst,
	GError** err);

#endif

#ifdef CL_VERSION_1_2

/* Fill a buffer object with a pattern of a given pattern size. */
CCLEvent* ccl_buffer_enqueue_fill(CCLBuffer* buf, CCLQueue* cq,
	const void *pattern, size_t pattern_size, size_t offset,
	size_t size, CCLEventWaitList* evt_wait_lst, GError** err);

#endif

/**
 * Increase the reference count of the buffer wrapper object.
 *
 * @param[in] buf The buffer wrapper object.
 * */
#define ccl_buffer_ref(buf) \
	ccl_wrapper_ref((CCLWrapper*) buf)

/**
 * Alias to ccl_buffer_destroy().
 *
 * @param[in] buf Buffer wrapper object to unreference.
 * */
#define ccl_buffer_unref(buf) ccl_buffer_destroy(buf)

/**
 * Get the OpenCL buffer object.
 *
 * @param[in] buf The buffer wrapper object.
 * @return The OpenCL buffer object.
 * */
#define ccl_buffer_unwrap(buf) \
	((cl_mem) ccl_wrapper_unwrap((CCLWrapper*) buf))

/** @} */

#endif



