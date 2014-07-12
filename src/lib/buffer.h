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
 
#ifndef CL4_BUFFER_H
#define CL4_BUFFER_H 

#include "memobj.h"

/** @brief Buffer wrapper object. */
typedef CL4MemObj CL4Buffer;

/** @brief Create a ::CL4Buffer wrapper object. */
CL4Buffer* cl4_buffer_new(CL4Context* ctx, cl_mem_flags flags,
	size_t size, void *host_ptr, GError** err);

CL4Event* cl4_buffer_read(CL4CQueue* cq, CL4Buffer* buf,
	cl_bool blocking_read, size_t offset, size_t size, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

CL4Event* cl4_buffer_write(CL4CQueue* cq, CL4Buffer* buf,
	cl_bool blocking_write, size_t offset, size_t size, void *ptr,
 	CL4EventWaitList evt_wait_lst, GError** err);

#ifdef CL_VERSION_1_1

CL4Buffer* cl4_buffer_new_from_region(CL4Buffer* buf, 
	cl_mem_flags flags, size_t origin, size_t size, GError** err);

CL4Event* cl4_buffer_read_rect(CL4CQueue* cq, CL4Buffer* buf,
	cl_bool blocking_read, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

CL4Event* cl4_buffer_write_rect(CL4CQueue* cq, CL4Buffer* buf,
	cl_bool blocking_write, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	CL4EventWaitList evt_wait_lst, GError** err);

#endif

#ifdef CL_VERSION_1_2

CL4Event* cl4_buffer_fill(CL4CQueue* cq, CL4Buffer* buf, 
	const void *pattern, size_t pattern_size, size_t offset, 
	size_t size, CL4EventWaitList evt_wait_lst, GError** err);

#endif

/// @todo We can emulate the OpenCL>1.0 functions if not available in
/// OpenCL

#endif



