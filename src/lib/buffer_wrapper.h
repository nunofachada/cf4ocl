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
 * @brief OpenCL buffer wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_BUFFER_WRAPPER_H
#define CL4_BUFFER_WRAPPER_H 

#include "memobj_wrapper.h"

/**
 * @defgroup BUFFER_WRAPPER Buffer wrapper
 *
 * @brief A wrapper object for OpenCL buffers and functions to manage 
 * them.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/** @brief Buffer wrapper object. */
typedef CL4MemObj CL4Buffer;

/** @brief Create a ::CL4Buffer wrapper object. */
CL4Buffer* cl4_buffer_new(CL4Context* ctx, cl_mem_flags flags,
	size_t size, void *host_ptr, GError** err);
	
#define cl4_buffer_destroy(buf) cl4_memobj_destroy((CL4MemObj*) buf)

CL4Event* cl4_buffer_read(CL4Queue* cq, CL4Buffer* buf,
	cl_bool blocking_read, size_t offset, size_t size, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

CL4Event* cl4_buffer_write(CL4Queue* cq, CL4Buffer* buf,
	cl_bool blocking_write, size_t offset, size_t size, void *ptr,
 	CL4EventWaitList evt_wait_lst, GError** err);

void* cl4_buffer_map(CL4Queue* cq, CL4Buffer* buf,
	cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, CL4EventWaitList evt_wait_lst, CL4Event** evt,
	GError** err);

CL4Event* cl4_buffer_copy(CL4Queue* cq, CL4Buffer* src_buf,
	CL4Buffer* dst_buf, size_t src_offset, size_t dst_offset, 
	size_t size, CL4EventWaitList evt_wait_lst, GError** err);
	
CL4Event* cl4_buffer_copy_to_image(CL4Queue* cq, CL4Buffer* src_buf,
	CL4Buffer* dst_img, size_t src_offset, const size_t *dst_origin,
	const size_t *region, CL4EventWaitList evt_wait_lst, GError** err);

#ifdef CL_VERSION_1_1

CL4Buffer* cl4_buffer_new_from_region(CL4Buffer* buf, 
	cl_mem_flags flags, size_t origin, size_t size, GError** err);

CL4Event* cl4_buffer_read_rect(CL4Queue* cq, CL4Buffer* buf,
	cl_bool blocking_read, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

CL4Event* cl4_buffer_write_rect(CL4Queue* cq, CL4Buffer* buf,
	cl_bool blocking_write, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

CL4Event* cl4_buffer_copy_rect(CL4Queue* cq, CL4Buffer* src_buf,
	CL4Buffer* dst_buf, const size_t *src_origin, 
	const size_t *dst_origin, const size_t *region, 
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
	size_t dst_slice_pitch, CL4EventWaitList evt_wait_lst, GError** err);

#endif

#ifdef CL_VERSION_1_2

CL4Event* cl4_buffer_fill(CL4Queue* cq, CL4Buffer* buf, 
	const void *pattern, size_t pattern_size, size_t offset, 
	size_t size, CL4EventWaitList evt_wait_lst, GError** err);

#endif

/// @todo We can emulate the OpenCL>1.0 functions if not available in
/// OpenCL

/** @} */

#endif



