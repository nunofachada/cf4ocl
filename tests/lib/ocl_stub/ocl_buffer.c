/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */

 /**
 * @file
 * OpenCL buffer stub functions.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"


CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size,
	void* host_ptr, cl_int* errcode_ret) {

	cl_mem memobj = NULL;

	if (context == NULL) {
		seterrcode(errcode_ret, CL_INVALID_CONTEXT);
	} else if ((flags & CL_MEM_COPY_HOST_PTR & flags & CL_MEM_USE_HOST_PTR)
		|| (flags & CL_MEM_ALLOC_HOST_PTR & flags & CL_MEM_USE_HOST_PTR)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
	} else if (((host_ptr == NULL) && (flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR)))
		|| ((host_ptr != NULL) && !(flags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR)))) {
		seterrcode(errcode_ret, CL_INVALID_HOST_PTR);
	} /* Not checking for CL_INVALID_BUFFER_SIZE */
	else {
		seterrcode(errcode_ret, CL_SUCCESS);

		memobj = g_slice_new(struct _cl_mem);

		memobj->ref_count = 1;
		memobj->type = CL_MEM_OBJECT_BUFFER;
		memobj->flags = flags;
		memobj->size = size;
		memobj->host_ptr = host_ptr;
		memobj->map_count = 0;
		memobj->context = context;
		memobj->associated_object = NULL;
		memobj->offset = 0;
		memobj->callbacks = NULL;
		if (flags & CL_MEM_COPY_HOST_PTR) {
			memobj->mem = g_memdup(host_ptr, (guint) size);
		} else if (flags & CL_MEM_USE_HOST_PTR) {
			memobj->mem = host_ptr;
		} else {
			memobj->mem = g_malloc(size);
		}
	}
	return memobj;
}

#ifdef CL_VERSION_1_1

CL_API_ENTRY cl_mem CL_API_CALL
clCreateSubBuffer(cl_mem buffer, cl_mem_flags flags,
	cl_buffer_create_type buffer_create_type,
	const void* buffer_create_info, cl_int* errcode_ret) {

	seterrcode(errcode_ret, CL_SUCCESS);
	cl_mem memobj = g_slice_new(struct _cl_mem);

	buffer->ref_count++;

	memobj->ref_count = 1;
	memobj->type = CL_MEM_OBJECT_BUFFER;
	memobj->flags = flags;
	memobj->size = (buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
		? ((struct _cl_buffer_region*) buffer_create_info)->size
		: 0;
	memobj->host_ptr = buffer->host_ptr;
	memobj->map_count = 0;
	memobj->context = buffer->context;
	memobj->associated_object = buffer;
	memobj->offset = (buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
		? ((struct _cl_buffer_region*) buffer_create_info)->origin
		: 0;

	memobj->mem = ((cl_uchar*) buffer->mem) + memobj->offset;
	memobj->callbacks = NULL;

	return memobj;
}

#endif
