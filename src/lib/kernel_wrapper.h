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
 * @brief OpenCL kernel wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_KERNEL_WRAPPER_H
#define CL4_KERNEL_WRAPPER_H 

#include <glib.h>
#include "oclversions.h"
#include "abstract_wrapper.h"
#include "kernel_arg.h"
#include "event_wrapper.h"
#include "queue_wrapper.h"

/* Forward declaration of CL4Program. */
typedef struct cl4_program CL4Program;

/** @brief Kernel wrapper object. */
typedef struct cl4_kernel CL4Kernel;

/** @brief Get the kernel wrapper for the given OpenCL kernel. */
CL4Kernel* cl4_kernel_new_wrap(cl_kernel kernel);

/** @brief Decrements the reference count of the kernel wrapper object. 
 * If it reaches 0, the kernel wrapper object is destroyed. */
void cl4_kernel_destroy(CL4Kernel* krnl);

CL4Kernel* cl4_kernel_new(
	CL4Program* prg, const char* kernel_name, GError** err);

void cl4_kernel_set_arg(CL4Kernel* krnl, cl_uint arg_index, 
	CL4Arg* arg);

void cl4_kernel_set_args(CL4Kernel* krnl, ...) G_GNUC_NULL_TERMINATED;

void cl4_kernel_set_args_v(CL4Kernel* krnl, va_list args);

CL4Event* cl4_kernel_run(CL4Kernel* krnl, CL4CQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CL4EventWaitList evt_wait_lst, GError** err);

/** @brief Set kernel arguments and run it. */
CL4Event* cl4_kernel_set_args_and_run(CL4Kernel* krnl, CL4CQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CL4EventWaitList evt_wait_lst, GError** err, ...)
	G_GNUC_NULL_TERMINATED;

CL4Event* cl4_kernel_set_args_and_run_v(CL4Kernel* krnl, CL4CQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CL4EventWaitList evt_wait_lst, GError** err, va_list args);

/**
 * @brief Get a ::CL4WrapperInfo kernel information object.
 * 
 * @param krnl The kernel wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel information object. This object will
 * be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_kernel_get_info(krnl, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) krnl, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetKernelInfo, CL_TRUE, err)

/** 
 * @brief Macro which returns a scalar kernel information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param krnl The kernel wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel information value. This value will be 
 * automatically freed when the kernel wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define cl4_kernel_get_scalar_info(krnl, param_name, param_type, err) \
	*((param_type*) cl4_wrapper_get_info_value((CL4Wrapper*) krnl, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetKernelInfo, \
		CL_TRUE, err))

/** 
 * @brief Macro which returns an array kernel information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param krnl The kernel wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel information value. This value will be 
 * automatically freed when the kernel wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define cl4_kernel_get_array_info(krnl, param_name, param_type, err) \
	(param_type) cl4_wrapper_get_info_value((CL4Wrapper*) krnl, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetKernelInfo, \
		CL_TRUE, err)

/**
 * @brief Get a ::CL4WrapperInfo kernel workgroup information object.
 * 
 * @param krnl The kernel wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel workgroup information object. This 
 * object will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_kernel_get_workgroup_info(krnl, dev, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) krnl, (CL4Wrapper*) dev, \
		param_name, (cl4_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar kernel workgroup information 
 * value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param krnl The kernel wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel workgroup information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, zero is returned.
 * */
#define cl4_kernel_get_scalar_workgroup_info(krnl, dev, param_name, \
	param_type, err) \
	*((param_type*) cl4_wrapper_get_info_value((CL4Wrapper*) krnl, \
		(CL4Wrapper*) dev, param_name, \
		(cl4_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, err))

/** 
 * @brief Macro which returns an array kernel workgroup information 
 * value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param krnl The kernel wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested kernel workgroup information value. This value 
 * will be automatically freed when the kernel wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_kernel_get_array_workgroup_info(krnl, dev, param_name, \
	param_type, err) \
	(param_type) cl4_wrapper_get_info_value((CL4Wrapper*) krnl, \
		(CL4Wrapper*) dev, param_name, \
		(cl4_wrapper_info_fp) clGetKernelWorkGroupInfo, \
		CL_FALSE, err)

/** 
 * @brief Increase the reference count of the kernel object.
 * 
 * @param krnl The kernel wrapper object. 
 * */
#define cl4_kernel_ref(krnl) \
	cl4_wrapper_ref((CL4Wrapper*) krnl)

/**
 * @brief Alias to cl4_kernel_destroy().
 * 
 * @param krnl Kernel wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define cl4_kernel_unref(krnl) cl4_kernel_destroy(krnl)

/**
 * @brief Get the OpenCL kernel object.
 * 
 * @param krnl The kernel wrapper object.
 * @return The OpenCL kernel object.
 * */
#define cl4_kernel_unwrap(krnl) \
	((cl_kernel) cl4_wrapper_unwrap((CL4Wrapper*) krnl))		

#endif

