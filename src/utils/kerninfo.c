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
 * Prints information about an OpenCL kernel. 
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * 
 */

#include "program_wrapper.h"
#include "kernel_wrapper.h"

/** 
 * Kernel info main function.
 * 
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return ::CCL_SUCCESS if program returns with no error, or another 
 * ::CCLErrorCode value otherwise.
 * */
int main(int argc, char *argv[]) {

	/* ***************** */	
	/* Program variables */
	/* ***************** */	
	
	/* Function and program return status. */
	int status;
	/* Error management. */
	GError *err = NULL;
	/* Context wrapper. */
	CCLContext* ctx = NULL;
	/* Program wrapper. */
	CCLProgram* prg = NULL;
	/* Kernel wrapper. */
	CCLKernel* krnl = NULL;
	/* Device wrapper. */
	CCLDevice* dev = NULL;
	/* Kernel information. */
	CCLWrapperInfo* info = NULL;
	/* Device filters. */
	CCLDevSelFilters filters = NULL;
	/* Default device index. */
	cl_int dev_idx = -1;
	/* OpenCL version. */
	double ocl_ver;
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	ccl_if_err_create_goto(err, CCL_ERROR, (argc < 3) || (argc > 4), 
		CCL_ERROR_ARGS, error_handler, 
		"Usage: %s <program_file> <kernel_name> [device_index]\n", 
		argv[0]);
	if (argc == 4) dev_idx = atoi(argv[3]);
	
	/* ********************************************* */
	/* Initialize OpenCL variables and build program */
	/* ********************************************* */
	
	/* Select a context/device. */
	ccl_devsel_add_dep_filter(
		&filters, ccl_devsel_dep_menu, 
		(dev_idx == -1) ? NULL : (void*) &dev_idx);
	ctx = ccl_context_new_from_filters(&filters, &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Get program which contains kernel. */
	prg = ccl_program_new_from_source_file(ctx, argv[1], &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Build program. */
	ccl_program_build(prg, NULL, &err);
	ccl_if_err_goto(err, error_handler);

	/* Get kernel */
	krnl = ccl_program_get_kernel(prg, argv[2], &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Get the device. */
	dev = ccl_context_get_device(ctx, 0, &err);
	ccl_if_err_goto(err, error_handler);

	/* Check platform  OpenCL version. */
	ocl_ver = ccl_kernel_get_opencl_version(krnl, &err);
	ccl_if_err_goto(err, error_handler);

	/* *************************** */
	/*  Get and print kernel info  */
	/* *************************** */
	
	g_printf("\n   ======================== Static Kernel Information =======================\n\n");
	
	info = ccl_kernel_get_workgroup_info(
		krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Maximum workgroup size                  : %lu\n", 
		(unsigned long) ccl_info_scalar(info, size_t));
	
	/* Only show info about CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
	 * if OpenCL version of the underlying platform is >= 1.1. */
	if (ocl_ver >= 1.1) { 
		info = ccl_kernel_get_workgroup_info(
			krnl, dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &err);
		ccl_if_err_goto(err, error_handler);
		g_printf("     Preferred multiple of workgroup size    : %lu\n", 
			(unsigned long) ccl_info_scalar(info, size_t));
	}
	
	info = ccl_kernel_get_workgroup_info(
		krnl, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     WG size in __attribute__ qualifier      : (%lu, %lu, %lu)\n", 
		(unsigned long) ccl_info_array(info, size_t*)[0], 
		(unsigned long) ccl_info_array(info, size_t*)[1], 
		(unsigned long) ccl_info_array(info, size_t*)[2]);
		
	info = ccl_kernel_get_workgroup_info(
		krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Local memory used by kernel             : %lu bytes\n", 
		(unsigned long) ccl_info_scalar(info, cl_ulong));
		
	info = ccl_kernel_get_workgroup_info(
		krnl, dev, CL_KERNEL_PRIVATE_MEM_SIZE, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Min. private mem. used by each workitem : %lu bytes\n", 
		(unsigned long) ccl_info_scalar(info, cl_ulong));
	
	g_printf("\n");
	
	/* ************** */
	/* Error handling */
	/* ************** */
	
	/* If we get here, no need for error checking, jump to cleanup. */
	g_assert(err == NULL);
	status = CCL_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);
	g_fprintf(stderr, "%s\n", err->message);
	status = (err->domain == CCL_ERROR) ? err->code : CCL_ERROR_OTHER;
	g_error_free(err);

cleanup:
		
	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	if (prg != NULL) ccl_program_destroy(prg);
	if (ctx != NULL) ccl_context_destroy(ctx);
	
	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

	/* Return status. */
	return status;

}

