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
 * OpenCL event stub functions.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program program, const char* kernel_name,
	cl_int* errcode_ret) {

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
	const void* arg_value) {

	(void)(kernel);
	(void)(arg_index);
	(void)(arg_size);
	(void)(arg_value);

	return CL_SUCCESS;
}


CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(cl_kernel kernel) {

	g_atomic_int_inc(&kernel->ref_count);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&kernel->ref_count)) {

		g_slice_free(struct _cl_kernel, kernel);

	}

	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(cl_kernel kernel, cl_kernel_info param_name,
	size_t param_value_size, void* param_value,
	size_t* param_value_size_ret) {

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
#ifdef CL_VERSION_1_2
			case CL_KERNEL_ATTRIBUTES:
				ccl_test_char_info(kernel, attributes);
#endif
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;

}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(cl_kernel kernel, cl_device_id device,
	cl_kernel_work_group_info param_name, size_t param_value_size,
	void* param_value, size_t * param_value_size_ret) {

	cl_int status = CL_SUCCESS;

	if (kernel == NULL) {
		status = CL_INVALID_KERNEL;
	} else {
		switch (param_name) {
			case CL_KERNEL_WORK_GROUP_SIZE:
				ccl_test_basic_info(size_t, device, max_work_group_size);
			case CL_KERNEL_COMPILE_WORK_GROUP_SIZE: /* This is incorrect. */
				ccl_test_vector_info(size_t, device, max_work_item_sizes);
			case CL_KERNEL_LOCAL_MEM_SIZE: /* This is incorrect. */
				ccl_test_basic_info(cl_ulong, device, local_mem_size);
#ifdef CL_VERSION_1_2
			case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: /* This is incorrect. */
				ccl_test_basic_info(size_t, device, max_work_item_sizes[0]);
			case CL_KERNEL_PRIVATE_MEM_SIZE: /* This is incorrect. */
				ccl_test_basic_info(cl_ulong, device, local_mem_size);
#endif
#ifdef CL_VERSION_1_2
			case CL_KERNEL_GLOBAL_WORK_SIZE: /* This is incorrect. */
				ccl_test_basic_info(size_t, device, max_work_group_size);
#endif
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;
}

#ifdef CL_VERSION_1_2
CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(cl_kernel kernel, cl_uint arg_indx,
	cl_kernel_arg_info param_name, size_t param_value_size,
	void* param_value, size_t* param_value_size_ret) {

	(void)(kernel);
	(void)(arg_indx);
	(void)(param_name);
	(void)(param_value_size);
	(void)(param_value);
	(void)(param_value_size_ret);

	return CL_INVALID_VALUE;

}
#endif
