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
 * Definition of a wrapper class and its methods for OpenCL kernel
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_KERNEL_WRAPPER_H_
#define _CCL_KERNEL_WRAPPER_H_

#include <glib.h>
#include "oclversions.h"
#include "abstract_wrapper.h"
#include "kernel_arg.h"
#include "event_wrapper.h"
#include "queue_wrapper.h"

/* Forward declaration of CCLProgram. */
typedef struct ccl_program CCLProgram;

/**
 * @defgroup KERNEL_WRAPPER Kernel wrapper
 *
 * The kernel wrapper module provides functionality for simple
 * handling of OpenCL kernel objects.
 *
 * Kernel wrappers can be obtained using two approaches:
 *
 * 1. Using the ::ccl_program_get_kernel() function. This function
 * always returns the same kernel wrapper object (with the same
 * underlying OpenCL kernel object) associated with a program. The
 * returned object is automatically freed when the program wrapper
 * object is destroyed; as such, client code should not call
 * ::ccl_kernel_destroy().
 * 2. Using the ::ccl_kernel_new() constructor. The created kernel
 * wrapper should be released with the ::ccl_kernel_destroy() function,
 * in accordance with the _cf4ocl_ @ref ug_new_destroy "new/destroy"
 * rule.
 *
 * While the first approach might be more convenient in many situations,
 * it will not work properly if the same kernel function is to be
 * handled and executed by different threads. In these cases, use the
 * second approach to create distinct kernel wrapper instances (wrapping
 * distinct OpenCL kernel objects) for the same kernel function, one for
 * each thread.
 *
 * This module offers several functions which simplify kernel execution.
 * For example, the ccl_kernel_set_args_and_enqueue_ndrange() function
 * can set all kernel arguments and execute the kernel in one call.
 *
 * Information about kernel objects can be fetched using the kernel
 * @ref ug_getinfo "info macros":
 *
 * * ::ccl_kernel_get_info_scalar()
 * * ::ccl_kernel_get_info_array()
 * * ::ccl_kernel_get_info()
 *
 * Six additional macros are provided for getting kernel workgroup info
 * and kernel argument info (the later is only available from OpenCL
 * 1.2 onwards). These work in the same way as the regular info macros:
 *
 * * ::ccl_kernel_get_workgroup_info_scalar()
 * * ::ccl_kernel_get_workgroup_info_array()
 * * ::ccl_kernel_get_workgroup_info()
 * * ::ccl_kernel_get_arg_info_scalar()
 * * ::ccl_kernel_get_arg_info_array()
 * * ::ccl_kernel_get_arg_info()
 *
 * _Example: getting a kernel wrapper from a program wrapper_
 *
 * @code{.c}
 * CCLProgram* prg;
 * CCLKernel* krnl;
 * @endcode
 * @code{.c}
 * krnl = ccl_program_get_kernel(prg, "some_kernel", NULL);
 * @endcode
 *
 * _Example: creating a kernel wrapper_
 *
 * @code{.c}
 * CCLProgram* prg;
 * CCLKernel* krnl;
 * @endcode
 * @code{.c}
 * krnl = ccl_kernel_new(prg, "some_kernel", NULL);
 * @endcode
 * @code{.c}
 * ccl_kernel_destroy(krnl);
 * @endcode
 *
 * @{
 */

/**
 * Kernel wrapper class.
 *
 * @extends ccl_wrapper
 */
typedef struct ccl_kernel CCLKernel;

/* Get the kernel wrapper for the given OpenCL kernel. */
CCLKernel* ccl_kernel_new_wrap(cl_kernel kernel);

/* Create a new kernel wrapper object. */
CCLKernel* ccl_kernel_new(
	CCLProgram* prg, const char* kernel_name, GError** err);

/* Decrements the reference count of the kernel wrapper object.
 * If it reaches 0, the kernel wrapper object is destroyed. */
void ccl_kernel_destroy(CCLKernel* krnl);

/* Set one kernel argument. */
void ccl_kernel_set_arg(CCLKernel* krnl, cl_uint arg_index,
	CCLArg* arg);

/* Set all kernel arguments. This function accepts a variable list of
 * arguments which must end with `NULL`. */
void ccl_kernel_set_args(CCLKernel* krnl, ...) G_GNUC_NULL_TERMINATED;

/* Set all kernel arguments. This function accepts a `NULL`-terminated
 * array of kernel arguments. */
void ccl_kernel_set_args_v(CCLKernel* krnl, CCLArg** args);

/* Enqueues a kernel for execution on a device. */
CCLEvent* ccl_kernel_enqueue_ndrange(CCLKernel* krnl, CCLQueue* cq,
	cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Set kernel arguments and enqueue it for execution. */
CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange(CCLKernel* krnl,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, GError** err, ...)
	G_GNUC_NULL_TERMINATED;

/* Set kernel arguments and enqueue it for execution. */
CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange_v(CCLKernel* krnl,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, CCLArg** args, GError** err);

/* Get the OpenCL version of the platform associated with this
 * kernel. */
cl_uint ccl_kernel_get_opencl_version(CCLKernel* krnl, GError** err);

/**
 * @class ccl_kernel
 * @todo Wrapper function for clEnqueueNativeKernel(). A possible header
 * for such wrapper function could be:
 * @code CCLEvent* ccl_kernel_enqueue_native_full(CCLQueue* cq,
 *     void (CL_CALLBACK *user_func)(void *), CCLKernelNativeArgs args,
 *     CCLEventWaitList* evt_wait_lst, GError** err); @endcode
 * Some links regarding the use of clEnqueueNativeKernel():
 * * http://steckdenis.wordpress.com/2011/06/18/use-opencl-to-execute-native-kernels/
 * * http://stackoverflow.com/questions/10140494/using-clenqueuenativekernel-in-opencl
 * * https://github.com/pcpratts/gcc_opencl/blob/master/example.c
 * */

/**
 * Get a ::CCLWrapperInfo kernel information object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel information object. This object will
 * be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_info(krnl, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) (krnl), NULL, (param_name), \
		(ccl_wrapper_info_fp) clGetKernelInfo, CL_TRUE, (err))

/**
 * Macro which returns a scalar kernel information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel information value. This value will be
 * automatically freed when the kernel wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_info_scalar(krnl, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		NULL, (param_name), (ccl_wrapper_info_fp) clGetKernelInfo, \
		CL_TRUE, (err)))

/**
 * Macro which returns an array kernel information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel information value. This value will be
 * automatically freed when the kernel wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_info_array(krnl, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		NULL, (param_name), (ccl_wrapper_info_fp) clGetKernelInfo, \
		CL_TRUE, (err))

/**
 * Get a ::CCLWrapperInfo kernel workgroup information object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel workgroup information object. This
 * object will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_workgroup_info(krnl, dev, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) (krnl), (CCLWrapper*) (dev), \
		(param_name), (ccl_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, (err))

/**
 * Macro which returns a scalar kernel workgroup information
 * value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel workgroup information value. This value
 * will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_workgroup_info_scalar(krnl, dev, param_name, \
	param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		(CCLWrapper*) (dev), (param_name), \
		(ccl_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, (err)))

/**
 * Macro which returns an array kernel workgroup information
 * value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel workgroup information value. This value
 * will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_workgroup_info_array(krnl, dev, param_name, \
	param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		(CCLWrapper*) (dev), (param_name), \
		(ccl_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, (err))

#ifdef CL_VERSION_1_2

/* Get a ::CCLWrapperInfo kernel argument information object. */
CCLWrapperInfo* ccl_kernel_get_arg_info(CCLKernel* krnl, cl_uint idx,
	cl_kernel_arg_info param_name, GError** err);

/**
 * Macro which returns a scalar kernel argument information
 * value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] idx Argument index.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel argument information value. This value
 * will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_arg_info_scalar(krnl, idx, param_name, \
	param_type, err) \
	(param_type) \
	((ccl_kernel_get_arg_info((krnl), (idx), (param_name), (err)) != NULL) \
	? **((param_type**) ccl_kernel_get_arg_info( \
		(krnl), (idx), (param_name), (err))) \
	: 0)

/**
 * Macro which returns an array kernel argument information
 * value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] idx Argument index.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel argument information value. This value
 * will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_arg_info_array(krnl, idx, param_name, \
	param_type, err) \
	(ccl_kernel_get_arg_info((krnl), (idx), (param_name), (err)) != NULL) \
	? *((param_type*) ccl_kernel_get_arg_info( \
		(krnl), (idx), (param_name), (err))) \
	: NULL

#endif /* OpenCL >=1.2 */

/**
 * Increase the reference count of the kernel object.
 *
 * @param[in] krnl The kernel wrapper object.
 * */
#define ccl_kernel_ref(krnl) \
	ccl_wrapper_ref((CCLWrapper*) (krnl))

/**
 * Alias to ccl_kernel_destroy().
 *
 * @param[in] krnl Kernel wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_kernel_unref(krnl) ccl_kernel_destroy(krnl)

/**
 * Get the OpenCL kernel object.
 *
 * @param[in] krnl The kernel wrapper object.
 * @return The OpenCL kernel object.
 * */
#define ccl_kernel_unwrap(krnl) \
	((cl_kernel) ccl_wrapper_unwrap((CCLWrapper*) (krnl)))


/** @} */

#endif

