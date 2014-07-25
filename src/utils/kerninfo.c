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

#include "program_wrapper.h"
#include "kernel_wrapper.h"

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
	
	/* Function and program return status. */
	int status;
	/* Error management. */
	GError *err = NULL;
	/* Context wrapper. */
	CL4Context* ctx = NULL;
	/* Program wrapper. */
	CL4Program* prg = NULL;
	/* Kernel wrapper. */
	CL4Kernel* krnl = NULL;
	/* Device wrapper. */
	CL4Device* dev = NULL;
	/* Kernel information. */
	CL4WrapperInfo* info = NULL;
	/* Device filters. */
	CL4DevSelFilters filters = NULL;
	/* Default device index. */
	cl_int dev_idx = -1;
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	gef_if_error_create_goto(err, CL4_ERROR, (argc < 3) || (argc > 4), 
		CL4_ERROR_ARGS, error_handler, 
		"Usage: %s <program_file> <kernel_name> [device_index]\n", 
		argv[0]);
	if (argc == 4) dev_idx = atoi(argv[3]);
	
	/* ********************************************* */
	/* Initialize OpenCL variables and build program */
	/* ********************************************* */
	
	/* Select a context/device. */
	cl4_devsel_add_dep_filter(
		&filters, cl4_devsel_dep_menu, 
		(dev_idx == -1) ? NULL : (void*) &dev_idx);
	ctx = cl4_context_new_from_filters(&filters, &err);
	gef_if_err_goto(err, error_handler);
	
	/* Get program which contains kernel. */
	prg = cl4_program_new_from_source_file(ctx, argv[1], &err);
	gef_if_err_goto(err, error_handler);
	
	/* Build program. */
	cl4_program_build(prg, NULL, &err);
	gef_if_err_goto(err, error_handler);

	/* Get kernel */
	krnl = cl4_program_get_kernel(prg, argv[2], &err);
	gef_if_err_goto(err, error_handler);
	
	/* Get the device. */
	dev = cl4_context_get_device(ctx, 0, &err);
	gef_if_err_goto(err, error_handler);

	/* *************************** */
	/*  Get and print kernel info  */
	/* *************************** */
	
	g_printf("\n   ======================== Static Kernel Information =======================\n\n");
	
	info = cl4_kernel_workgroup_info(
		krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, &err);
	gef_if_err_goto(err, error_handler);
	g_printf("     Maximum workgroup size                  : %lu\n", 
		(unsigned long) cl4_info_scalar(info, size_t));

	info = cl4_kernel_workgroup_info(
		krnl, dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, &err);
	gef_if_err_goto(err, error_handler);
	g_printf("     Preferred multiple of workgroup size    : %lu\n", 
		(unsigned long) cl4_info_scalar(info, size_t));
		
	info = cl4_kernel_workgroup_info(
		krnl, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, &err);
	gef_if_err_goto(err, error_handler);
	g_printf("     WG size in __attribute__ qualifier      : (%lu, %lu, %lu)\n", 
		(unsigned long) cl4_info_array(info, size_t*)[0], 
		(unsigned long) cl4_info_array(info, size_t*)[1], 
		(unsigned long) cl4_info_array(info, size_t*)[2]);
		
	info = cl4_kernel_workgroup_info(
		krnl, dev, CL_KERNEL_LOCAL_MEM_SIZE, &err);
	gef_if_err_goto(err, error_handler);
	g_printf("     Local memory used by kernel             : %lu bytes\n", 
		(unsigned long) cl4_info_scalar(info, cl_ulong));
		
	info = cl4_kernel_workgroup_info(
		krnl, dev, CL_KERNEL_PRIVATE_MEM_SIZE, &err);
	gef_if_err_goto(err, error_handler);
	g_printf("     Min. private mem. used by each workitem : %lu bytes\n", 
		(unsigned long) cl4_info_scalar(info, cl_ulong));
	
	g_printf("\n");
	
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
	g_fprintf(stderr, "%s\n", err->message);
	status = err->code;
	g_error_free(err);

cleanup:
		
	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	if (prg != NULL) cl4_program_destroy(prg);
	if (ctx != NULL) cl4_context_destroy(ctx);
	
	/* Return status. */
	return status;

}

