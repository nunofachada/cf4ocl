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
 *
 * Definition of a wrapper type and related functions for a OpenCL kernel
 * arguments.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_KERNEL_ARG_H_
#define _CCL_KERNEL_ARG_H_

#include "ccl_common.h"
#include "ccl_abstract_wrapper.h"

/**
 * The ::CCLArg type is an alias for ::CCLWrapper type. The value
 * and size of local/private arguments is kept inside ::CCLWrapper
 * instances and manipulated by the ccl_arg_*() functions. This
 * allows client code to pass real wrappers, e.g. ::CCLBuffer, or
 * local/private parameters as kernel arguments in a transparent
 * fashion.
 * */
typedef CCLWrapper CCLArg;

/* Create a new kernel argument. */
CCL_EXPORT
CCLArg* ccl_arg_new(void* value, size_t size);

/* Destroy a kernel argument. */
CCL_EXPORT
void ccl_arg_destroy(CCLArg* arg);

/* Get size in bytes of kernel argument. */
CCL_EXPORT
size_t ccl_arg_size(CCLArg* arg);

/* Get value of kernel argument. */
CCL_EXPORT
void* ccl_arg_value(CCLArg* arg);

/**
 * @defgroup CCL_KERNEL_ARG Kernel argument wrappers
 * @ingroup CCL_KERNEL_WRAPPER
 *
 * This module defines the ::CCLArg* class which wraps kernel arguments.
 *
 * Several functions in the @ref CCL_KERNEL_WRAPPER "kernel wrapper module",
 * such as ::ccl_kernel_set_args() or
 * ::ccl_kernel_set_args_and_enqueue_ndrange(), accept kernel arguments
 * as parameters. ::CCLBuffer*, ::CCLImage* and ::CCLSampler* objects
 * can be directly passed as global kernel arguments to these functions.
 * However, local and private kernel arguments need to be passed using
 * the macros provided in this module, namely ::ccl_arg_local() and
 * ::ccl_arg_priv(), respectively.
 *
 * The ::ccl_arg_skip constant can be passed to methods which accept a
 * variable list of ordered arguments in order to skip a specific
 * argument.
 *
 * _Example:_
 *
 * _Kernel code:_
 * @code{.c}
 * __kernel void my_kernel(
 *     __global int* g, __local int *l, __private float p) {
 * @endcode
 * @code{.c}
 * }
 * @endcode
 * _Host code:_
 * @code{.c}
 * #define LOC_SIZE 16
 * const cl_float pi=3.1415;
 * @endcode
 * @code{.c}
 * CCLProgram* prg;
 * CCLKernel* krnl;
 * CCLBuffer* buf;
 * @endcode
 * @code{.c}
 * krnl = ccl_program_get_kernel(prg, "my_kernel", NULL);
 * ccl_kernel_set_args(krnl, buf, ccl_arg_local(LOC_SIZE, cl_int),
 *     ccl_arg_priv(pi, cl_float), NULL);
 * @endcode
 * @{
 *
 * @note These macros wrap the ::ccl_arg_new() function, which returns
 * a new ::CCLArg* object. ::CCLArg* objects are destroyed when the
 * kernel to which they were passed is released. For further control
 * of argument instantiation, client code can use the ::ccl_arg_full()
 * macro instead of the ::ccl_arg_new() function in order to respect the
 * @ref ug_new_destroy "new/destroy" rule.
 *
 * @attention A ::CCLArg* object can only be passed once to a kernel.
 * One way to guarantee this is to use the macros directly when setting
 * the kernel arguments, as shown in the example above.
 */

/**
 * Use this constant to skip kernel arguments in the
 * ::ccl_kernel_set_args(), ::ccl_kernel_set_args_v(),
 * ::ccl_kernel_set_args_and_enqueue_ndrange() and
 * ::ccl_kernel_set_args_and_enqueue_ndrange_v() functions.
 * */
CCL_EXPORT
extern const CCLArg* ccl_arg_skip;

/**
 * Define a private kernel argument.
 *
 * The created object is automatically released when kernel is
 * enqueued.
 *
 * @param[in] value Argument value. Must be a variable, not a literal
 * value.
 * @param[in] type Argument scalar type, such as `cl_int`, `cl_float`,
 * etc.
 * @return A private ::CCLArg* kernel argument.
 * */
#define ccl_arg_priv(value, type) \
	ccl_arg_new(&value, sizeof(type))

/**
 * Defines a local kernel argument, which allocates local memory
 * within the kernel with the specified size.
 *
 * The created object is automatically released when kernel is
 * enqueued.
 *
 * @param[in] count Number of values of type given in next parameter.
 * @param[in] type Argument scalar type, such as `cl_int`, `cl_float`,
 * etc.
 * @return A local ::CCLArg* kernel argument.
 * */
#define ccl_arg_local(count, type) \
	ccl_arg_new(NULL, count * sizeof(type))

/**
 * Defines a kernel argument with more control.
 *
 * The created object is automatically released when kernel is
 * enqueued.
 *
 * @param[in] value Memory location of argument value. Can be NULL if
 * argument is local.
 * @param[in] size Size in bytes of argument.
 * @return A private or local ::CCLArg* kernel argument.
 * */
#define ccl_arg_full(value, size) \
	ccl_arg_new(value, size)

/** @} */

#endif
