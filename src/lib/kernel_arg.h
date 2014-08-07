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
 * Kernel argument wrapper.
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
 * The ::CCLArg type is an alias for ::CCLWrapper type. The value
 * and size of local/private arguments is kept inside ::CCLWrapper
 * instances and manipulated by the ccl_arg_*() functions. This
 * allows client code to pass real wrappers, e.g. ::CCLBuffer, or 
 * local/private parameters as kernel arguments in a transparent
 * fashion. 
 * */
typedef CCLWrapper CCLArg;

/** Create a new kernel argument. */
CCLArg* ccl_arg_new(void* value, size_t size);

/** Destroy a kernel argument. */
void ccl_arg_destroy(CCLArg* arg);

/** Get size in bytes of kernel argument. */
size_t ccl_arg_size(CCLArg* arg);

/** Get value of kernel argument. */
void* ccl_arg_value(CCLArg* arg);

/**
 * @defgroup KERNEL_ARG Kernel argument wrappers
 * @ingroup KERNEL_WRAPPER
 *
 * Wrapper objects for OpenCL kernel arguments.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/**
 * Define a private kernel argument.
 * 
 * The created object is automatically released when kernel is
 * enqueued.
 * 
 * @param[in] value Argument value.
 * @param[in] type Argument scalar type, such as cl_int, cl_float, etc.
 * @return A private kernel argument.
 * */
#define ccl_arg_priv(value, type) \
	ccl_arg_new(&value, sizeof(type))

/**
 * Define a local kernel argument, which allocates local memory
 * within the kernel with the specified size.
 * 
 * The created object is automatically released when kernel is
 * enqueued.
 * 
 * @param[in] count Number of values of type given in next parameter.
 * @param[in] type Argument scalar type, such as cl_int, cl_float, etc.
 * @return A local kernel argument.
 * */
#define ccl_arg_local(count, type) \
	ccl_arg_new(NULL, count * sizeof(type))

/**
 * Define a kernel argument which more control.
 * 
 * The created object is automatically released when kernel is
 * enqueued.
 * 
 * @param[in] value Argument value. Can be NULL if argument is local.
 * @param[in] size Size in bytes of argument.
 * @return A private or local kernel argument.
 * */
#define ccl_arg_full(value, size) \
	ccl_arg_new(value, size)

/** @} */

#endif
