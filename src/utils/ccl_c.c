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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 * Utility for offline compilation and linking of OpenCL kernels.
 *
 * @author Nuno Fachada
 * @date 2017
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/**
 * @page ccl_c
 *
 * @brief Utility for offline compilation and linking of OpenCL kernels.
 *
 * SYNOPSIS
 * ========
 *
 * **ccl_c** [_OPTION_]...
 *
 * DESCRIPTION
 * ===========
 *
 * The `ccl_c` tool performs offline compilation and analysis of OpenCL kernels.
 * Here is a list of the available options:
 *
 * <dl>
 * <dt>-l, --list</dt>
 * <dd>List available devices and exit.</dd>
 * <dt>-d, --device=DEV</dt>
 * <dd>Specify a device on which to perform the task.</dd>
 * <dt>-t, --task=TASK</dt>
 * <dd>0 (Build, default), 1 (Compile) or 2 (Link). Tasks 1 and 2 are only
 * available for platforms with support for OpenCL 1.2 or higher.</dd>
 * <dt>-0, --options=OPTIONS</dt>
 * <dd>Compiler/linker options.</dd>
 * <dt>-s, --src=FILE</dt>
 * <dd>Source input files. This option can be specified multiple times.</dd>
 * <dt>-i, --input-headers=FILE</dt>
 * <dd>Embedded header input files for the compile task. This option can be
 * specified multiple times.</dd>
 * <dt>-n, --header-include-names=STRING</dt>
 * <dd>Embedded header include names for the compile task. This option can be
 * specified multiple times and has a one to one correspondence with
 * --input-headers.</dd>
 * <dt>-b, --bin=FILE</dt>
 * <dd>Binary input file. This option can be specified multiple times.</dd>
 * <dt>-o, --output=FILE</dt>
 * <dd>Binary output file.</dd>
 * <dt>-k, --kernel-info=STRING</dt>
 * <dd>Show information about the specified kernel. This option can be
 * specified multiple times.</dd>
 * <dt>-u, --build-log=FILE</dt>
 * <dd>Save build log to the specified file. By default the build log is
 * printed to stderr.</dd>
 * <dt>--version</dt>
 * <dd>Output version information and exit.</dd>
 * <dt>-h, --help, -?</dt>
 * <dd>Show help options and exit.</dd>
 * </dl>
 *
 * AUTHOR
 * ======
 *
 * Written by Nuno Fachada.
 *
 * REPORTING BUGS
 * ==============
 *
 * * Report ccl_c bugs at https://github.com/fakenmc/cf4ocl/issues
 * * cf4ocl home page: http://fakenmc.github.io/cf4ocl/
 * * cf4ocl wiki: https://github.com/fakenmc/cf4ocl/wiki
 *
 * COPYRIGHT
 * =========
 *
 * Copyright (C) 2016 Nuno Fachada<br/>
 * License GPLv3+: GNU GPL version 3 or later
 * <http://gnu.org/licenses/gpl.html>.<br/>
 * This is free software: you are free to change and redistribute it.<br/>
 * There is NO WARRANTY, to the extent permitted by law.
 *
 * */

#include "ccl_utils.h"

#define CCL_C_DESCRIPTION "Static kernel compiler and analyzer"

#define ccl_c_get_build_status_str(build_status) \
	(build_status) == CL_BUILD_NONE ? "Program not built (unexpected)" : \
	((build_status) == CL_BUILD_ERROR ? "Error" : \
	(((build_status) == CL_BUILD_SUCCESS ? "Success" : \
	((((build_status) == CL_BUILD_IN_PROGRESS ? "In progress (unexpected)" : \
	"Unknown"))))))

#define ccl_c_is_build_error(err) \
	(((err != NULL) && (err->domain == CCL_OCL_ERROR) && \
	 ((err->code == CL_BUILD_PROGRAM_FAILURE) || \
	  (err->code == CL_COMPILE_PROGRAM_FAILURE) || \
	  (err->code == CL_LINK_PROGRAM_FAILURE))))

#define ccl_c_info_unavailable(err) \
	((err) != NULL) && ((err)->domain == CCL_ERROR) && \
	((err)->code == CCL_ERROR_INFO_UNAVAILABLE_OCL)

/* Available tasks. */
typedef enum ccl_c_tasks {
	CCL_C_BUILD = 0,
	CCL_C_COMPILE = 1,
	CCL_C_LINK = 2
} CCLCTasks;

/* Command line arguments and respective default values. */
static gboolean opt_list = FALSE;
static guint dev_idx = CCL_UTILS_NODEVICE;
static guint task = CCL_C_BUILD;
static gchar* options = NULL;
static gchar** src_files = NULL;
static gchar** bin_files = NULL;
static gchar** src_h_files = NULL;
static gchar** src_h_names = NULL;
static gchar** kernel_names = NULL;
static gchar* output = NULL;
static gchar* bld_log_out = NULL;
static gboolean version = FALSE;

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"list",                 'l', 0, G_OPTION_ARG_NONE,           &opt_list,
	 "List available devices and exit.",                          NULL},
	{"device",               'd', 0, G_OPTION_ARG_INT,            &dev_idx,
	 "Specify a device on which to perform the task.",            "DEV"},
	{"task",                 't', 0, G_OPTION_ARG_INT,            &task,
	 "0 (Build, default), 1 (Compile) or 2 (Link). Tasks 1 and 2 are only "
	 "available for platforms with support for OpenCL 1.2 or higher.",
	                                                              "TASK"},
	{"options",              '0', 0, G_OPTION_ARG_STRING,         &options,
	 "Compiler/linker options.",                                  "OPTIONS"},
	{"src",                  's', 0, G_OPTION_ARG_FILENAME_ARRAY, &src_files,
	 "Source input files. This option can be specified multiple times.",
	                                                              "FILE"},
	{"input-headers",        'i', 0, G_OPTION_ARG_FILENAME_ARRAY, &src_h_files,
	 "Embedded header input files for the compile task. This option can be "
	 "specified multiple times.",                                 "FILE"},
	{"header-include-names", 'n', 0, G_OPTION_ARG_STRING_ARRAY,   &src_h_names,
	 "Embedded header include names for the compile task. This option can be "
	 "specified multiple times and has a one to one correspondence with "
	 "--input-headers.",                                          "STRING"},
	{"bin",                  'b', 0, G_OPTION_ARG_FILENAME_ARRAY, &bin_files,
	 "Binary input file. This option can be specified multiple times.",
	                                                              "FILE"},
	{"output",               'o', 0, G_OPTION_ARG_FILENAME,       &output,
	 "Binary output file.",                                       "FILE"},
	{"kernel-info",          'k', 0, G_OPTION_ARG_STRING_ARRAY,   &kernel_names,
	 "Show information about the specified kernel. This option can be "
	 "specified multiple times.",                                "STRING"},
	{"build-log",            'u', 0, G_OPTION_ARG_FILENAME,       &bld_log_out,
	 "Save build log to the specified file. By default the build log is "
	 "printed to stderr.",                                       "FILE"},
	{"version",               0,  0, G_OPTION_ARG_NONE,           &version,
	 "Output version information and exit.",                      NULL},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }
};

/**
 * Parse and verify command line arguments.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @param[out] err Return location for a CCLErr object.
 * */
void ccl_c_args_parse(int argc, char* argv[], CCLErr** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail(err == NULL || *err == NULL);

	/* Command line options context. */
	GOptionContext* context = NULL;

	/* Create parsing context. */
	context = g_option_context_new(" - " CCL_C_DESCRIPTION);

	/* Add acceptable command line options to context. */
	g_option_context_add_main_entries(context, entries, NULL);

	/* Use context to parse command line options. */
	g_option_context_parse(context, &argc, &argv, err);
	g_if_err_goto(*err, error_handler);

	/* If we get here, no need for error treatment, jump to cleanup. */
	g_assert(*err == NULL);
	goto cleanup;

error_handler:

	/* If we got here, everything is OK. */
	g_assert(*err != NULL);

cleanup:

	/* Free context. */
	if (context) g_option_context_free(context);

	/* Return. */
	return;

}

/**
 * Show kernel information.
 *
 * @param[in] prg Program containing kernel.
 * @param[in] dev Device for which kernel was compiled.
 * @param[in] kernel Kernel name.
 * @param[out] err Return location for a CCLErr object.
 * */
void ccl_c_kernel_info_show(
	CCLProgram* prg, CCLDevice* dev, const char* kernel, CCLErr** err) {

	/* OpenCL version. */
	cl_uint ocl_ver;

	/* Kernel wrapper. */
	CCLKernel* krnl = NULL;

	/* Kernel workgroup info variables. */
	size_t k_wg_size;
	size_t k_pref_wg_size_mult;
	size_t* k_compile_wg_size;
	cl_ulong k_loc_mem_size;
	cl_ulong k_priv_mem_size;

	/* Internal error handling object. */
	CCLErr* err_internal = NULL;

	/* Get OpenCL version. */
	ocl_ver = ccl_program_get_opencl_version(prg, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);


	/* Get kernel. */
	krnl = ccl_program_get_kernel(prg, kernel, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	g_printf("\n");

	/* Show CL_KERNEL_WORK_GROUP_SIZE information. */
	k_wg_size = ccl_kernel_get_workgroup_info_scalar(
		krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, size_t, &err_internal);
	if (ccl_c_info_unavailable(err_internal)) {
		g_clear_error(&err_internal);
		g_printf("   - Maximum workgroup size                  : N/A\n");
	} else {
		g_if_err_propagate_goto(err, err_internal, error_handler);
		g_printf("   - Maximum workgroup size                  : %lu\n",
			(unsigned long) k_wg_size);
	}

	/* Only show info about CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE
	 * if OpenCL version of the underlying platform is >= 1.1. */
	if (ocl_ver >= 110) {
		k_pref_wg_size_mult = ccl_kernel_get_workgroup_info_scalar(krnl,
			dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
			size_t, &err_internal);
		if (ccl_c_info_unavailable(err_internal)) {
			g_clear_error(&err_internal);
			g_printf("   - Preferred multiple of workgroup size    : N/A\n");
		} else {
			g_if_err_propagate_goto(err, err_internal, error_handler);
			g_printf("   - Preferred multiple of workgroup size    : %lu\n",
				(unsigned long) k_pref_wg_size_mult);
		}
	}

	/* Show CL_KERNEL_COMPILE_WORK_GROUP_SIZE information. */
	k_compile_wg_size = ccl_kernel_get_workgroup_info_array(krnl, dev,
		CL_KERNEL_COMPILE_WORK_GROUP_SIZE, size_t*, &err_internal);
	if (ccl_c_info_unavailable(err_internal)) {
		g_clear_error(&err_internal);
		g_printf("   - WG size in __attribute__ qualifier      : N/A\n");
	} else {
		g_if_err_propagate_goto(err, err_internal, error_handler);
		g_printf("   - WG size in __attribute__ qualifier      : "
			"(%lu, %lu, %lu)\n",
			(unsigned long) k_compile_wg_size[0],
			(unsigned long) k_compile_wg_size[1],
			(unsigned long) k_compile_wg_size[2]);
	}

	/* Show CL_KERNEL_LOCAL_MEM_SIZE information. */
	k_loc_mem_size = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_LOCAL_MEM_SIZE, cl_ulong, &err_internal);
	if (ccl_c_info_unavailable(err_internal)) {
		g_clear_error(&err_internal);
		g_printf("   - Local memory used by kernel             : N/A\n");
	} else {
		g_if_err_propagate_goto(err, err_internal, error_handler);
		g_printf("   - Local memory used by kernel             : %lu bytes\n",
			(unsigned long) k_loc_mem_size);
	}

	/* Show CL_KERNEL_PRIVATE_MEM_SIZE information. */
	k_priv_mem_size = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_PRIVATE_MEM_SIZE, cl_ulong, &err_internal);
	if (ccl_c_info_unavailable(err_internal)) {
		g_clear_error(&err_internal);
		g_printf("   - Min. private mem. used by each workitem : N/A\n");
	} else {
		g_if_err_propagate_goto(err, err_internal, error_handler);
		g_printf("   - Min. private mem. used by each workitem : %lu bytes\n",
			(unsigned long) k_priv_mem_size);
	}

	g_printf("\n");

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return. */
	return;

}

/**
 * Kernel analyzer main program function.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Vector of command line arguments.
 * @return ::CCL_SUCCESS if program returns with no error, or another
 * ::CCLErrorCode value otherwise.
 */
int main(int argc, char* argv[]) {

	/* Error object. */
	CCLErr* err = NULL, *err_build = NULL;

	/* Program return status. */
	gint status;

	/* Counter for for loops. */
	guint i = 0;

	/* Number of types of files, file names and kernel names. */
	guint n_src_files, n_bin_files, n_src_h_files,
		n_src_h_names, n_kernel_names;

	/* Context wrapper. */
	CCLContext* ctx = NULL;

	/* Device wrapper. */
	CCLDevice* dev = NULL;

	/* Device name. */
	char* dname;

	/* Main program wrapper. */
	CCLProgram* prg = NULL;

	/* Auxiliary program wrapper. */
	CCLProgram* prg_aux = NULL;

	/* Array containing multiple program wrappers. */
	GPtrArray* prgs = NULL;

	/* Build status. */
	cl_build_status build_status;

	/* Build status string. */
	const char* build_status_str;

	/* Build log. */
	const char* build_log;

	/* Parse command line options. */
	ccl_c_args_parse(argc, argv, &err);
	g_if_err_goto(err, error_handler);

	g_printf("\n");

	/* Determine main program goal. */
	if (version) {

		/* If version was requested, show version. */
		ccl_common_version_print("ccl_c");

	} else if (opt_list) {

		/* If user requested a list of available devices, present the list. */
		ccl_devsel_print_device_strings(&err);
		g_if_err_goto(err, error_handler);

	} else {

		/* Otherwise perform a task, which requires at least one input
		 * file and the specification of a device. */

		/* Check for input files. */
		g_if_err_create_goto(err, CCL_ERROR, (src_files == NULL) &&
			(src_h_files == NULL) && (bin_files == NULL),
			CCL_ERROR_ARGS, error_handler,
			"No source or binary input files have been specified.");

		/* How many files of each type? */
		n_src_files = src_files != NULL ? g_strv_length(src_files) : 0;
		n_bin_files = bin_files != NULL ? g_strv_length(bin_files) : 0;
		n_src_h_files = src_h_files != NULL ? g_strv_length(src_h_files) : 0;
		n_src_h_names = src_h_names != NULL ? g_strv_length(src_h_names) : 0;
		n_kernel_names = kernel_names != NULL ? g_strv_length(kernel_names) : 0;

		/* Select a context/device. */
		if (dev_idx == CCL_UTILS_NODEVICE) {
			ctx = ccl_context_new_from_menu(&err);
		} else {
			ctx = ccl_context_new_from_device_index(&dev_idx, &err);
		}
		g_if_err_goto(err, error_handler);

		/* Get device. */
		dev = ccl_context_get_device(ctx, 0, &err);
		g_if_err_goto(err, error_handler);

		 /* Perform task. */
		switch (task) {
			case CCL_C_BUILD:

				/* For direct builds we can only have either one binary
				 * or one or more source files (but not both). */
				g_if_err_create_goto(err, CCL_ERROR,
					(n_src_files > 0) && (n_bin_files > 0),
					CCL_ERROR_ARGS, error_handler,
					"The 'build' task requires either: 1) one or more "
					"source files; or, 2) one binary file.");
				g_if_err_create_goto(err, CCL_ERROR, n_bin_files > 1,
					CCL_ERROR_ARGS, error_handler,
					"The 'build' task accepts at most one binary file.");

				/* Input headers are not accepted by the compile task. */
				g_if_err_create_goto(err, CCL_ERROR,
					(n_src_h_files > 0) || (n_src_h_names > 0),
					CCL_ERROR_ARGS, error_handler,
					"Input headers can only be specified for the 'compile' "
					"task.");

				/* Create program object. */
				if (n_bin_files == 1) {

					/* Create program from binary file. */
					prg = ccl_program_new_from_binary_file(ctx, dev,
						*bin_files, NULL, &err);

				} else {

					/* Create program from source. */
					prg = ccl_program_new_from_source_files(
						ctx, n_src_files, (const char**) src_files, &err);

				}
				g_if_err_goto(err, error_handler);

				/* Build program. */
				ccl_program_build(prg, options, &err_build);

				/* Only check for errors that are not build/compile/link
				 * failures. */
				if (!ccl_c_is_build_error(err_build)) {
					g_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			case CCL_C_COMPILE:

				/* Compilation requires at least one source file. */
				g_if_err_create_goto(err, CCL_ERROR, n_src_files == 0,
					CCL_ERROR_ARGS, error_handler,
					"The 'compile' task requires at least one source file.");

				/* Compilation does not support binaries. */
				g_if_err_create_goto(err, CCL_ERROR, n_bin_files > 0,
					CCL_ERROR_ARGS, error_handler,
					"The 'compile' task does not support binaries.");

				/* Number of header include names must be zero OR be the same as
				 * the number of input headers. */
				g_if_err_create_goto(err, CCL_ERROR,
					(n_src_h_files != n_src_h_names) && (n_src_h_names > 0),
					CCL_ERROR_ARGS, error_handler,
					"Number of header include names must be the same as the "
					"number of input headers.");

				/* Create header programs, if any. */
				if (n_src_h_files) {

					/* Instantiate array of header programs. */
					prgs = g_ptr_array_new_full(
						n_src_h_files,
						(GDestroyNotify) ccl_program_destroy);

					/* Create individual header programs. */
					for (i = 0; i < n_src_h_files; i++) {

						/* Create current header program from source. */
						prg_aux = ccl_program_new_from_source_files(ctx,
							1, (const char**) &(src_h_files[i]), &err);
						g_if_err_goto(err, error_handler);

						/* Add header program to array. */
						g_ptr_array_add(prgs, (gpointer) prg_aux);

					}
				}

				/* Create main program from source. */
				prg = ccl_program_new_from_source_files(ctx,
					n_src_files, (const char**) src_files, &err);
				g_if_err_goto(err, error_handler);

				/* Compile program. */
				ccl_program_compile(prg, 1, &dev, options,
					n_src_h_files,
					(CCLProgram**) (prgs ? prgs->pdata : NULL),
					(const char**) (src_h_names ? src_h_names : src_h_files),
					NULL, NULL, &err_build);

				/* Only check for errors that are not build/compile/link
				 * failures. */
				if (!ccl_c_is_build_error(err_build)) {
					g_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			case CCL_C_LINK:

				/* Linking requires at least one binary file and does
				 * not support source files. */
				g_if_err_create_goto(err, CCL_ERROR, (n_bin_files == 0) ||
					(n_src_files > 0) || (n_src_h_files > 0),
					CCL_ERROR_ARGS, error_handler,
					"The 'link' task requires at least one binary file "
					"and does not support source files.");

				/* Instantiate array of programs. */
				prgs = g_ptr_array_new_full(
					n_bin_files,
					(GDestroyNotify) ccl_program_destroy);

				/* Create programs from binaries. */
				for (i = 0; i < n_bin_files; i++) {

					/* Create program from current binary file. */
					prg_aux = ccl_program_new_from_binary_file(ctx, dev,
						bin_files[i], NULL, &err);
					g_if_err_goto(err, error_handler);

					/* Add binary program to array. */
					g_ptr_array_add(prgs, (gpointer) prg_aux);

				}

				/* Link programs. */
				prg = ccl_program_link(ctx, 1, &dev, options,
					n_bin_files, (CCLProgram**) prgs->pdata, NULL,
					NULL, &err_build);
				/* Only check for errors that are not build/compile/link
				 * failures. */
				if (!ccl_c_is_build_error(err_build)) {
					g_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			default:
				g_if_err_create_goto(err, CCL_ERROR, TRUE,
					CCL_ERROR_ARGS, error_handler, "Unknown task: %d",
					task);
		}

		/* Get and show device name. */
		dname = ccl_device_get_info_array(dev, CL_DEVICE_NAME, char*, &err);
		g_if_err_goto(err, error_handler);
		g_printf("* Device                 : %s\n", dname);

		/* Ir program object exists... */
		if (prg) {

			/* ...get build status and build status string. */
			build_status = ccl_program_get_build_info_scalar(prg, dev,
				CL_PROGRAM_BUILD_STATUS, cl_build_status, &err);
			g_if_err_goto(err, error_handler);
			build_status_str = ccl_c_get_build_status_str(build_status);

		} else {

			/* If program object does not exist, set build status string to
			 * unavailable. */
			build_status = CL_BUILD_NONE;
			build_status_str = "Unavailable";

		}

		/* Show build status. */
		g_printf("* Build status           : %s\n", build_status_str);

		/* If build successful, save binary? */
		if (output && prg && (build_status == CL_BUILD_SUCCESS)) {

			ccl_program_save_binary(prg, dev, output, &err);
			g_if_err_goto(err, error_handler);
			g_printf("* Binary output file     : %s\n", output);

		}

		/* Show build error message, if any. */
		if (err_build) {
			g_printf("* Additional information : %s\n", err_build->message);
		}

		/* Show kernel information? */
		if (kernel_names && !err_build) {

			/* Cycle through kernel names. */
			for (i = 0; i < n_kernel_names; i++) {

				/* Show information for current kernel name. */
				g_printf("* Kernel information     : %s\n", kernel_names[i]);
				ccl_c_kernel_info_show(prg, dev, kernel_names[i], &err);
				g_if_err_goto(err, error_handler);

			}
		}

		/* Show build log, if any. */
		g_printf("* Build log              :");
		if (!prg) {

			/* No build log if program object does not exist. */
			g_printf(" Unavailable.\n");

		} else {

			/* Get build log. */
			build_log = ccl_program_get_device_build_log(prg, dev, &err);
			if (err) {

				/* Not possible to retrieve build log due to error. */
				g_info("Unable to retrieve build log. %s", err->message);
				g_clear_error(&err);

			}

			/* If build log was retrieved successfully and has length greater
			 * than zero, output it. */
			if ((build_log) && (strlen(build_log) > 0)) {

				/* Should we output print log to file or to stderr? */
				if (bld_log_out) {

					/* Output to file. */
					g_printf(" Saved to %s.\n", bld_log_out);
					g_file_set_contents(bld_log_out, build_log, -1, &err);
					g_if_err_goto(err, error_handler);

				} else {

					/* Output to stderr. */
					g_printf(" Printed to error output stream.\n");
					g_fprintf(stderr, "\n%s\n", build_log);

				}

			} else {

				/* No build log or build log is empty. */
				g_printf(" Empty.\n");

			}
		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL);
	status = err_build ? EXIT_FAILURE : EXIT_SUCCESS;
	goto cleanup;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);

	/* Show error message and set return status to failure. */
	g_fprintf(stderr, "* Error                  : %s\n", err->message);
	status = EXIT_FAILURE;

	/* Release error object. */
	g_error_free(err);

cleanup:

	g_printf("\n");

	/* Free stuff! */
	g_clear_error(&err_build);
	if (src_files) g_strfreev(src_files);
	if (src_h_files) g_strfreev(src_h_files);
	if (src_h_names) g_strfreev(src_h_names);
	if (kernel_names) g_strfreev(kernel_names);
	if (bin_files) g_strfreev(bin_files);
	if (options) g_free(options);
	if (bld_log_out) g_free(bld_log_out);
	if (output) g_free(output);
	if (ctx) ccl_context_destroy(ctx);
	if (prg) ccl_program_destroy(prg);
	if (prgs) g_ptr_array_free(prgs, TRUE);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), EXIT_FAILURE);

	/* Return status. */
	return status;

}
