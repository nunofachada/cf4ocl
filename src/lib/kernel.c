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

#include "kernel.h"

/**
 * @brief Kernel wrapper object.
 */
struct cl4_kernel {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
};

/** 
 * @brief Decrements the reference count of the kernel wrapper 
 * object. If it reaches 0, the kernel wrapper object is 
 * destroyed.
 *
 * @param krnl The kernel wrapper object.
 * */
void cl4_kernel_destroy(CL4Kernel* krnl) {
	
	/* Make sure kernel wrapper object is not NULL. */
	g_return_if_fail(krnl != NULL);
	
	/* Wrapped OpenCL object (a kernel in this case), returned by
	 * the parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_kernel kernel;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	kernel = (cl_kernel) cl4_wrapper_unref((CL4Wrapper*) krnl);
	
	/* If an OpenCL kernel was returned, the reference count of 
	 * the wrapper object reached 0, so we must destroy remaining 
	 * kernel wrapper properties and the OpenCL kernel
	 * itself. */
	if (kernel != NULL) {

		/* Release krnl. */
		g_slice_free(CL4Kernel, krnl);
		
		/* Release OpenCL kernel. */
		clReleaseKernel(kernel);
		
	}

}

