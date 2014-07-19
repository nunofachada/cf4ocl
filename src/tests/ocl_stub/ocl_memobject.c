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
 * @brief OpenCL memory object stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
	
	g_atomic_int_inc(&memobj->ref_count);
	return CL_SUCCESS;	

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&memobj->ref_count)) {

		/* Release memory object depending ontype.*/
		if (memobj->type == CL_MEM_OBJECT_BUFFER) {
			/* It's a buffer. */
			g_slice_free(struct _cl_mem, memobj);
		} else {
			/* It's an image. */
			g_slice_free(struct _cl_image, (struct _cl_image*) memobj);
		}
	
	}
	
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem memobj, cl_mem_info param_name, 
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {

	cl_int status = CL_SUCCESS;

	if (memobj == NULL) {
		status = CL_INVALID_MEM_OBJECT;
	} else {
		switch (param_name) {
			case CL_MEM_TYPE:
				cl4_test_basic_info(cl_mem_object_type, memobj, type);
			case CL_MEM_FLAGS:
				cl4_test_basic_info(cl_mem_flags, memobj, flags);
			case CL_MEM_SIZE:
				cl4_test_basic_info(size_t, memobj, size);
			case CL_MEM_HOST_PTR:
				cl4_test_basic_info(void*, memobj, host_ptr);
			case CL_MEM_MAP_COUNT:
				cl4_test_basic_info(cl_uint, memobj, map_count);
			case CL_MEM_REFERENCE_COUNT:
				cl4_test_basic_info(cl_uint, memobj, ref_count);
			case CL_MEM_CONTEXT:
				cl4_test_basic_info(cl_context, memobj, context);
			case CL_MEM_ASSOCIATED_MEMOBJECT:
				cl4_test_basic_info(cl_mem, memobj, associated_object);
			case CL_MEM_OFFSET:
				cl4_test_basic_info(size_t, memobj, offset);
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;
}
