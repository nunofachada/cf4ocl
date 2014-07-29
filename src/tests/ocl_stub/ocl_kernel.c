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
 * @brief OpenCL event stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program program, const char* kernel_name, 
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 {

	/* Allocate memory for kernel. */
	cl_kernel kernel = g_slice_new(struct _cl_kernel);
	
	kernel->program = program;
	kernel->function_name = kernel_name;
	kernel->context = program->context;
	kernel->ref_count = 1;
	kernel->num_args = 3; /* Bogus! */
	kernel->attributes = ""; /* Also bogus! */
	seterrcode(errcode_ret, CL_SUCCESS);
	return kernel;

}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
	const void* arg_value) CL_API_SUFFIX__VERSION_1_0 {

	kernel = kernel;
	arg_index = arg_index;
	arg_size = arg_size;
	arg_value = arg_value;
	
	return CL_SUCCESS;
}


CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {
	
	g_atomic_int_inc(&kernel->ref_count);
	return CL_SUCCESS;
	
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0 {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&kernel->ref_count)) {

		g_slice_free(struct _cl_kernel, kernel);
		
	}
	
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel kernel, cl_kernel_info param_name, 
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {

	cl_int status = CL_SUCCESS;

	if (kernel == NULL) {
		status = CL_INVALID_KERNEL;
	} else {
		switch (param_name) {
			case CL_KERNEL_FUNCTION_NAME:
				ccl_test_char_info(kernel, function_name);
			case CL_KERNEL_NUM_ARGS:
				ccl_test_basic_info(cl_uint, kernel, num_args);
			case CL_KERNEL_REFERENCE_COUNT:
				ccl_test_basic_info(cl_uint, kernel, ref_count);
			case CL_KERNEL_CONTEXT:
				ccl_test_basic_info(cl_context, kernel, context);
			case CL_KERNEL_PROGRAM:
				ccl_test_basic_info(cl_program, kernel, program);
			case CL_KERNEL_ATTRIBUTES:
				ccl_test_char_info(kernel, attributes);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;

}
