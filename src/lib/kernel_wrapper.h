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
 * A wrapper object for OpenCL kernels and functions to manage 
 * them.
 * 
 * @todo detailed description of module with code examples.
 * 
 * @{
 */

/**
 * Kernel wrapper class.
 * 
 * @extends ccl_wrapper
 */
typedef struct ccl_kernel CCLKernel;

//~ typedef void (CL_CALLBACK *ccl_kernel_callback)(void* args)
//~ 
//~ typedef CCLDevSelDevices (*ccl_devsel_dep)(
	//~ CCLDevSelDevices devices, void *data, GError **err);

/** Get the kernel wrapper for the given OpenCL kernel. */
CCLKernel* ccl_kernel_new_wrap(cl_kernel kernel);

CCLKernel* ccl_kernel_new(
	CCLProgram* prg, const char* kernel_name, GError** err);

/** Decrements the reference count of the kernel wrapper object. 
 * If it reaches 0, the kernel wrapper object is destroyed. */
void ccl_kernel_destroy(CCLKernel* krnl);

void ccl_kernel_set_arg(CCLKernel* krnl, cl_uint arg_index, 
	CCLArg* arg);

void ccl_kernel_set_args(CCLKernel* krnl, ...) G_GNUC_NULL_TERMINATED;

void ccl_kernel_set_args_v(CCLKernel* krnl, va_list args);

CCLEvent* ccl_kernel_enqueue_ndrange(CCLKernel* krnl, CCLQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList* evt_wait_lst, GError** err);

/** Set kernel arguments and enqueue it for execution. */
CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange(CCLKernel* krnl, 
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList* evt_wait_lst, GError** err, ...)
	G_GNUC_NULL_TERMINATED;

CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange_v(CCLKernel* krnl, CCLQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList* evt_wait_lst, GError** err, va_list args);
	
/** Get the OpenCL version of the platform associated with this 
 * kernel. */
double ccl_kernel_get_opencl_version(CCLKernel* krnl, GError** err);

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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel information value. This value will be 
 * automatically freed when the kernel wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_scalar_info(krnl, param_name, param_type, err) \
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel information value. This value will be 
 * automatically freed when the kernel wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_array_info(krnl, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		NULL, (param_name), (ccl_wrapper_info_fp) clGetKernelInfo, \
		CL_TRUE, (err))

/**
 * Get a ::CCLWrapperInfo kernel workgroup information object.
 * 
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel workgroup information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_scalar_workgroup_info(krnl, dev, param_name, \
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel workgroup information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_array_workgroup_info(krnl, dev, param_name, \
	param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) (krnl), \
		(CCLWrapper*) (dev), (param_name), \
		(ccl_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, (err))

#ifdef CL_VERSION_1_2

/** Get a ::CCLWrapperInfo kernel argument information object. */
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] idx Argument index.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel argument information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_kernel_get_scalar_arg_info(krnl, idx, param_name, \
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
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @param[in] idx Argument index.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested kernel argument information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_kernel_get_array_arg_info(krnl, idx, param_name, \
	param_type, err) \
	(ccl_kernel_get_arg_info((krnl), (idx), (param_name), (err)) != NULL) \
	? *((param_type*) ccl_kernel_get_arg_info( \
		(krnl), (idx), (param_name), (err))) \
	: NULL

#endif /* OpenCL >=1.2 */

/** 
 * Increase the reference count of the kernel object.
 * 
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object. 
 * */
#define ccl_kernel_ref(krnl) \
	ccl_wrapper_ref((CCLWrapper*) (krnl))

/**
 * Alias to ccl_kernel_destroy().
 * 
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl Kernel wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_kernel_unref(krnl) ccl_kernel_destroy(krnl)

/**
 * Get the OpenCL kernel object.
 * 
 * @public @memberof ccl_kernel
 * 
 * @param[in] krnl The kernel wrapper object.
 * @return The OpenCL kernel object.
 * */
#define ccl_kernel_unwrap(krnl) \
	((cl_kernel) ccl_wrapper_unwrap((CCLWrapper*) (krnl)))


/** @} */

#endif

