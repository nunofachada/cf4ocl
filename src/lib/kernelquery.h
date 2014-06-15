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
 * @brief Objects and functions for querying OpenCL objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_QUERY_H
#define CL4_QUERY_H 

#include "errors.h"
#include "gerrorf.h"
#include "common.h"
#include <glib.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif

/**
 * @brief Kernel workgroup information.
 */
typedef struct cl4_query_kernel_workgroup {
	size_t preferred_workgroup_size_multiple; /**< Preferred multiple of workgroup size for launch. */
	size_t compile_workgroup_size[3];         /**< Work-group size specified by the @code __attribute__((reqd_work_gr oup_size(X, Y, Z))) @endcode qualifier. If the work-group size is not specified using the above attribute qualifier (0, 0, 0) is returned. */
	size_t max_workgroup_size;                /**< Maximum work-group size that can be used to execute a kernel on a specific device. */
	cl_ulong local_mem_size;                  /**< Amount of local memory in bytes being used by a kernel. */
	cl_ulong private_mem_size;                /**< Minimum amount of private memory, in bytes, used by each workitem in the kernel.  */
} CL4QueryKernelWorkgroup;


/** @brief Get kernel workgroup info. */
int cl4_query_workgroup_get(cl_kernel kernel, cl_device_id device, CL4QueryKernelWorkgroup* kwgi, GError **err);

/** @brief Print kernel workgroup info. */
void cl4_query_workgroup_print(CL4QueryKernelWorkgroup* kwgi);


#endif
