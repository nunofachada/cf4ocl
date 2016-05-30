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
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 *
 */

/**
 * @page ccl_kerninfo ccl_kerninfo
 *
 * Man page here.
 *
 * */

#include "ccl_utils.h"

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
	/* Default device index. */
	guint dev_idx = CCL_UTILS_NODEVICE;
	/* OpenCL version. */
	double ocl_ver;
	/* Kernel workgroup info variables. */
	size_t k_wg_size;
	size_t k_pref_wg_size_mult;
	size_t* k_compile_wg_size;
	cl_ulong k_loc_mem_size;
	cl_ulong k_priv_mem_size;

	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	/* If version was requested, output version and exit. */
	if ((argc == 2) && (g_strcmp0("--version", argv[1]) == 0)) {
		ccl_common_version_print("ccl_kerninfo");
		exit(0);
	}

	ccl_if_err_create_goto(err, CCL_ERROR, (argc < 4) || (argc > 5),
		CCL_ERROR_ARGS, error_handler,
		"Usage: %s (-s|-b) <program_file> <kernel_name> [device_index]\n",
		argv[0]);
	if (argc == 5) dev_idx = atoi(argv[4]);

	/* ********************************************* */
	/* Initialize OpenCL variables and build program */
	/* ********************************************* */

	/* Select a context/device. */
	if (dev_idx == CCL_UTILS_NODEVICE) {
		ctx = ccl_context_new_from_menu(&err);
	} else {
		ctx = ccl_context_new_from_device_index(&dev_idx, &err);
	}
	ccl_if_err_goto(err, error_handler);

	/* Get the device. */
	dev = ccl_context_get_device(ctx, 0, &err);
	ccl_if_err_goto(err, error_handler);

	/* Get program which contains kernel. */
	if (strcmp(argv[1], "-s") == 0) {
		prg = ccl_program_new_from_source_file(ctx, argv[2], &err);
	} else if (strcmp(argv[1], "-b") == 0) {
		prg = ccl_program_new_from_binary_file(ctx, dev, argv[2], NULL, &err);
	} else {
		err = g_error_new(CCL_ERROR, CCL_ERROR_ARGS,
			"Unknown option '%s'", argv[1]);
	}
	ccl_if_err_goto(err, error_handler);

	/* Build program. */
	ccl_program_build(prg, NULL, &err);
	ccl_if_err_goto(err, error_handler);

	/* Get kernel */
	krnl = ccl_program_get_kernel(prg, argv[3], &err);
	ccl_if_err_goto(err, error_handler);

	/* Check platform  OpenCL version. */
	ocl_ver = ccl_kernel_get_opencl_version(krnl, &err);
	ccl_if_err_goto(err, error_handler);

	/* *************************** */
	/*  Get and print kernel info  */
	/* *************************** */

	g_printf("\n   ======================== Static Kernel Information =======================\n\n");

	k_wg_size = ccl_kernel_get_workgroup_info_scalar(
		krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, size_t, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Maximum workgroup size                  : %lu\n",
		(unsigned long) k_wg_size);

	/* Only show info about CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
	 * if OpenCL version of the underlying platform is >= 1.1. */
	if (ocl_ver >= 110) {
		k_pref_wg_size_mult = ccl_kernel_get_workgroup_info_scalar(krnl,
			dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, size_t, &err);
		ccl_if_err_goto(err, error_handler);
		g_printf("     Preferred multiple of workgroup size    : %lu\n",
			(unsigned long) k_pref_wg_size_mult);
	}

	k_compile_wg_size = ccl_kernel_get_workgroup_info_array(krnl, dev,
		CL_KERNEL_COMPILE_WORK_GROUP_SIZE, size_t*, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     WG size in __attribute__ qualifier      : (%lu, %lu, %lu)\n",
		(unsigned long) k_compile_wg_size[0],
		(unsigned long) k_compile_wg_size[1],
		(unsigned long) k_compile_wg_size[2]);

	k_loc_mem_size = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_LOCAL_MEM_SIZE, cl_ulong, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Local memory used by kernel             : %lu bytes\n",
		(unsigned long) k_loc_mem_size);

	k_priv_mem_size = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_PRIVATE_MEM_SIZE, cl_ulong, &err);
	ccl_if_err_goto(err, error_handler);
	g_printf("     Min. private mem. used by each workitem : %lu bytes\n",
		(unsigned long) k_priv_mem_size);

	g_printf("\n");

	/* ************** */
	/* Error handling */
	/* ************** */

	/* If we get here, no need for error checking, jump to cleanup. */
	g_assert(err == NULL);
	status = EXIT_SUCCESS;
	goto cleanup;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);
	g_fprintf(stderr, "%s\n", err->message);
	status = EXIT_FAILURE;
	g_error_free(err);

cleanup:

	/* *********** */
	/* Free stuff! */
	/* *********** */

	if (prg != NULL) ccl_program_destroy(prg);
	if (ctx != NULL) ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

	/* Return status. */
	return status;

}

