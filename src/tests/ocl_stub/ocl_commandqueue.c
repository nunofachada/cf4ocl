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
 * @brief OpenCL command queue stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context context, cl_device_id device, 
	cl_command_queue_properties properties, cl_int* errcode_ret) 
	CL_API_SUFFIX__VERSION_1_0 {
		
	seterrcode(errcode_ret, CL_SUCCESS);
		
	cl_command_queue queue = g_slice_new(struct _cl_command_queue);
		
	queue->context = context;
	queue->device = device; 
	queue->properties = properties;
	queue->ref_count = 1;
	
	return queue;
		
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) 
	CL_API_SUFFIX__VERSION_1_0 {

	g_atomic_int_inc(&command_queue->ref_count);
	return CL_SUCCESS;


}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) 
	CL_API_SUFFIX__VERSION_1_0 {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&command_queue->ref_count)) {

		g_slice_free(struct _cl_command_queue, command_queue);
		
	}

	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue command_queue,
	cl_command_queue_info param_name, size_t param_value_size,
	void* param_value, size_t* param_value_size_ret) 
	CL_API_SUFFIX__VERSION_1_0 {
		
	cl_int status = CL_SUCCESS;

	if (command_queue == NULL) {
		status = CL_INVALID_COMMAND_QUEUE;
	} else {
		switch (param_name) {
			
			case CL_QUEUE_CONTEXT:
				ccl_test_basic_info(cl_context, command_queue, context);
			case CL_QUEUE_DEVICE:	
				ccl_test_basic_info(cl_device_id, command_queue, device);
			case CL_QUEUE_REFERENCE_COUNT:
				ccl_test_basic_info(cl_uint, command_queue, ref_count);
			case CL_CONTEXT_PROPERTIES:
				ccl_test_basic_info(cl_command_queue_properties, command_queue, properties);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;
		
}

CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
	command_queue = command_queue;
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
	command_queue = command_queue;	
	return CL_SUCCESS;
}
