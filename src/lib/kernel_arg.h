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
 * @brief Kernel argument wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_KERNEL_ARG_H_
#define _CCL_KERNEL_ARG_H_

#include "common.h"
#include "abstract_wrapper.h"

/**
 * @defgroup KERNEL_ARG Kernel argument wrappers
 * @ingroup KERNEL_WRAPPER
 *
 * @brief Wrapper objects for OpenCL kernel arguments.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

typedef CCLWrapper CCLArg;

#define ccl_arg_private(value, type) \
	ccl_arg_new(&value, sizeof(type))

#define ccl_arg_local(count, type) \
	ccl_arg_new(NULL, count * sizeof(type))
	
CCLArg* ccl_arg_new(void* value, size_t size);

void ccl_arg_destroy(CCLArg* arg);

size_t ccl_arg_size(CCLArg* arg);

void* ccl_arg_value(CCLArg* arg);

/** @} */

#endif
