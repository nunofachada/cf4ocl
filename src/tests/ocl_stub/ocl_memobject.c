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
 * OpenCL memory object stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

struct cl_memobject_callbacks {
	void (CL_CALLBACK *pfn_notify)(cl_mem memobj, void* user_data);
	void* user_data;
};

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
	
	g_atomic_int_inc(&memobj->ref_count);
	return CL_SUCCESS;	

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&memobj->ref_count)) {
		
		/* Call callback functions. */
		GSList* curr = memobj->callbacks;
		while (curr != NULL) {
			struct cl_memobject_callbacks* cb = 
				(struct cl_memobject_callbacks*) curr->data;
			cb->pfn_notify(memobj, cb->user_data);
			curr = g_slist_next(curr);
		}
		
		/* Free callback list. */
		g_slist_free_full(memobj->callbacks, g_free);

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
				ccl_test_basic_info(cl_mem_object_type, memobj, type);
			case CL_MEM_FLAGS:
				ccl_test_basic_info(cl_mem_flags, memobj, flags);
			case CL_MEM_SIZE:
				ccl_test_basic_info(size_t, memobj, size);
			case CL_MEM_HOST_PTR:
				ccl_test_basic_info(void*, memobj, host_ptr);
			case CL_MEM_MAP_COUNT:
				ccl_test_basic_info(cl_uint, memobj, map_count);
			case CL_MEM_REFERENCE_COUNT:
				ccl_test_basic_info(cl_uint, memobj, ref_count);
			case CL_MEM_CONTEXT:
				ccl_test_basic_info(cl_context, memobj, context);
#ifdef CL_VERSION_1_1
			case CL_MEM_ASSOCIATED_MEMOBJECT:
				ccl_test_basic_info(cl_mem, memobj, associated_object);
			case CL_MEM_OFFSET:
				ccl_test_basic_info(size_t, memobj, offset);
#endif
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetMemObjectDestructorCallback(cl_mem memobj,
	void (CL_CALLBACK *pfn_notify)(cl_mem memobj, void* user_data),
	void* user_data) CL_API_SUFFIX__VERSION_1_1 {

	cl_int status;

	if (memobj == NULL) {
		status = CL_INVALID_MEM_OBJECT;
	} else if (pfn_notify == NULL) {
		status = CL_INVALID_VALUE;
	} else {
		
		struct cl_memobject_callbacks* cb = 
			g_new0(struct cl_memobject_callbacks, 1);
		
		cb->pfn_notify = pfn_notify;
		cb->user_data = user_data;
			
		memobj->callbacks = g_slist_prepend(
			memobj->callbacks, (gpointer) cb);
			
		status = CL_SUCCESS;
	}
	return status;

}
