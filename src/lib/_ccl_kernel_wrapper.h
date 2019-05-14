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
 * @internal
 *
 * @file
 * This header provides the prototype of the ccl_kernel_get_arg_info_adapter()
 * function. This header is not part of the _cf4ocl_ public API.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef __CCL_KERNEL_WRAPPER_H_
#define __CCL_KERNEL_WRAPPER_H_

#include "ccl_oclversions.h"

#ifdef CL_VERSION_1_2

/* Kernel argument information adapter between a ccl_wrapper_info_fp() function
 * and the clGetKernelArgInfo() function. */
cl_int ccl_kernel_get_arg_info_adapter(cl_kernel kernel, void * ptr_arg_indx,
    cl_kernel_arg_info param_name, size_t param_value_size, void * param_value,
    size_t * param_value_size_ret);

#endif /* CL_VERSION_1_2 */

#endif /* __CCL_KERNEL_WRAPPER_H_ */
