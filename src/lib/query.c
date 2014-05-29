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
 
#include "query.h"


/** 
 * @brief Get kernel workgroup info. 
 * 
 * @param kernel OpenCL kernel to obtain info of.
 * @param device OpenCL device where kernel will run.
 * @param kwgi Kernel workgroup information structure, which will be populated by this function.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link cl4_man_error_codes::CL4_SUCCESS @endlink operation
 * successfully completed or another value of #cl4_man_error_codes if an 
 * error occurs.
 */
int cl4_query_workgroup_info_get(cl_kernel kernel, cl_device_id device, CL4QueryKernelWorkgroupInfo* kwgi, GError **err) {

	/* Status flag returned by OpenCL. */
	cl_int ocl_status;
	/* Status flag returned by this function. */
	int ret_status;

	/* Get preferred work-group size multiple. */
	ocl_status = clGetKernelWorkGroupInfo(
		kernel, 
		device, 
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, 
		sizeof(size_t), 
		&(kwgi->preferred_work_group_size_multiple), 
		NULL);
	gef_if_error_create_goto(
		*err, 
		CL4_ERROR, 
		CL_SUCCESS != ocl_status, 
		ret_status = CL4_OCL_ERROR, 
		error_handler, 
		"cl4_man_workgroup_info_get: Unable to get CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE (OpenCL error %d: %s).", 
		ocl_status,
		cl4_err(ocl_status));
	
	/* Get compile work-group size. */
	ocl_status = clGetKernelWorkGroupInfo(
		kernel, 
		device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 
		3 * sizeof(size_t), 
		kwgi->compile_work_group_size, 
		NULL);
	gef_if_error_create_goto(
		*err, CL4_ERROR, 
		CL_SUCCESS != ocl_status, 
		ret_status = CL4_OCL_ERROR, 
		error_handler, 
		"cl4_man_workgroup_info_get: Unable to get CL_KERNEL_COMPILE_WORK_GROUP_SIZE (OpenCL error %d: %s).", 
		ocl_status,
		cl4_err(ocl_status));

	/* Get work-group size. */
	ocl_status = clGetKernelWorkGroupInfo(
		kernel, 
		device, 
		CL_KERNEL_WORK_GROUP_SIZE, 
		sizeof(size_t), 
		&(kwgi->max_work_group_size), 
		NULL);
	gef_if_error_create_goto(
		*err, 
		CL4_ERROR, 
		CL_SUCCESS != ocl_status, 
		ret_status = CL4_OCL_ERROR, 
		error_handler, 
		"cl4_man_workgroup_info_get: Unable to get CL_KERNEL_WORK_GROUP_SIZE (OpenCL error %d: %s).", 
		ocl_status,
		cl4_err(ocl_status));

	/* Get local memory size. */
	ocl_status = clGetKernelWorkGroupInfo(
		kernel, 
		device, 
		CL_KERNEL_LOCAL_MEM_SIZE, 
		sizeof(cl_ulong), 
		&(kwgi->local_mem_size), 
		NULL);
	gef_if_error_create_goto(
		*err, 
		CL4_ERROR, 
		CL_SUCCESS != ocl_status, 
		ret_status = CL4_OCL_ERROR, 
		error_handler, 
		"cl4_man_workgroup_info_get: Unable to get CL_KERNEL_LOCAL_MEM_SIZE (OpenCL error %d: %s).", 
		ocl_status,
		cl4_err(ocl_status));

	/* Get private memory size. */
	ocl_status = clGetKernelWorkGroupInfo(
		kernel, 
		device, 
		CL_KERNEL_PRIVATE_MEM_SIZE, 
		sizeof(cl_ulong), 
		&(kwgi->private_mem_size), 
		NULL);
	gef_if_error_create_goto(
		*err, 
		CL4_ERROR, 
		CL_SUCCESS != ocl_status, 
		ret_status = CL4_OCL_ERROR, 
		error_handler, 
		"cl4_man_workgroup_info_get: Unable to get CL_KERNEL_PRIVATE_MEM_SIZE (OpenCL error %d: %s).", 
		ocl_status,
		cl4_err(ocl_status));

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	ret_status = CL4_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:	
	/* Return. */
	return ret_status;
}

/** 
 * @brief Print kernel workgroup info. 
 * 
 * @param kwgi Kernel workgroup information to print.
 */
void cl4_query_workgroup_info_print(CL4QueryKernelWorkgroupInfo* kwgi) {
	
	printf("\n   =========================== Kernel Information ==========================\n\n");
	printf("     Maximum workgroup size                  : %lu\n", (unsigned long) kwgi->max_work_group_size);
	printf("     Preferred multiple of workgroup size    : %lu\n", (unsigned long) kwgi->preferred_work_group_size_multiple);
	printf("     WG size in __attribute__ qualifier      : (%lu, %lu, %lu)\n", (unsigned long) kwgi->compile_work_group_size[0], (unsigned long) kwgi->compile_work_group_size[1], (unsigned long) kwgi->compile_work_group_size[2]);
	printf("     Local memory used by kernel             : %lu bytes\n", (unsigned long) kwgi->local_mem_size);
	printf("     Min. private mem. used by each workitem : %lu bytes\n\n", (unsigned long) kwgi->private_mem_size);

}

