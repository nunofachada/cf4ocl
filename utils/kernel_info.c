/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */

/** 
 * @file
 * @brief Prints information about an OpenCL kernel. 
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * 
 */

#include "clutils.h"

/** 
 * @brief Kernel info main function.
 * 
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return CL_SUCCESS if program returns with no error, or another value otherwise.
 * */
int main(int argc, char *argv[])
{

	/* ***************** */	
	/* Program variables */
	/* ***************** */	
	
	int status;                  /* Function and program return status. */
	GError *err = NULL;          /* Error management. */
	cl_kernel kernel = NULL;     /* Kernel. */
	CLUZone* zone = NULL;        /* OpenCL zone. */
	CLUKernelWorkgroupInfo kwgi; /* Information about kernel. */
	int dev_idx = -1;
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	gef_if_error_create_goto(err, CLU_UTILS_ERROR, (argc < 3) || (argc > 4), CLU_ERROR_ARGS, error_handler, "Usage: %s <program_file> <kernel_name> [device_index]\n", argv[0]);
	if (argc == 4) dev_idx = atoi(argv[3]);
	
	/* ********************************************* */
	/* Initialize OpenCL variables and build program */
	/* ********************************************* */
	
	/* Get the required CL zone. */
	zone = clu_zone_new(CL_DEVICE_TYPE_ALL, 1, CL_QUEUE_PROFILING_ENABLE, clu_menu_device_selector, (dev_idx != -1 ? &dev_idx : NULL), &err);
	gef_if_error_goto(err, GEF_USE_GERROR, status, error_handler);
	
	/* Build program. */
	status = clu_program_create(zone, &argv[1], 1, NULL, &err);
	gef_if_error_goto(err, GEF_USE_GERROR, status, error_handler);

	/* Kernel */
	kernel = clCreateKernel(zone->program, argv[2], &status);
	gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to create '%s' kernel.", status, argv[2]);

	/* *************************** */
	/*  Get and print kernel info  */
	/* *************************** */
	
	status = clu_workgroup_info_get(kernel, zone->device_info.device_id, &kwgi, &err);
	gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get kernel information.", status);
	
	clu_workgroup_info_print(&kwgi);	
	
	/* ************** */
	/* Error handling */
	/* ************** */
	
	/* If we get here, no need for error checking, jump to cleanup. */
	g_assert (err == NULL);
	status = CL_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err != NULL);
	fprintf(stderr, "%s", err->message);
	status = err->code;
	g_error_free(err);

cleanup:
		
	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	/* Release OpenCL kernels */
	if (kernel) clReleaseKernel(kernel);

	/* Free OpenCL zone */
	if (zone) clu_zone_free(zone);
	
	/* Return status. */
	return status;

}

