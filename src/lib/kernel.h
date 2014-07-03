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
 
#ifndef CL4_KERNEL_H
#define CL4_KERNEL_H 

#include <glib.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include "wrapper.h"

/** @brief Kernel wrapper object. */
typedef struct cl4_kernel CL4Kernel;

/** @brief Create a ::CL4Kernel wrapper object by wrapping a given
 * OpenCL kernel. */
CL4Kernel* cl4_kernel_new(cl_kernel kernel);

/** @brief Decrements the reference count of the kernel wrapper 
 * object. If it reaches 0, the kernel wrapper object is 
 * destroyed. */
void cl4_kernel_destroy(CL4Kernel* krnl);

/**
 * @brief Get kernel information object.
 * 
 * @param krnl The kernel wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested information object. This object will be 
 * automatically freed when the wrapper object is destroyed. If an error 
 * occurs, NULL is returned.
 * */
#define cl4_kernel_info(krnl, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) krnl, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetKernelInfo, err)

/**
 * @brief Get kernel workgroup information object.
 * 
 * @param krnl The kernel wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested information object. This object will be 
 * automatically freed when the wrapper object is destroyed. If an error 
 * occurs, NULL is returned.
 * */
#define cl4_kernel_workgroup_info(krnl, dev, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) krnl, (CL4Wrapper*) dev, NULL, \
		param_name, (cl4_wrapper_info_fp) clGetKernelWorkGroupInfo, err)

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

