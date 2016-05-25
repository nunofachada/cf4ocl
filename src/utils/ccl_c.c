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
 * Utility for static compilation and linking of OpenCL kernels.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/**
 * @page ccl_c ccl_devinfo
 *
 * Man page will be placed here.
 *
 * */

#include <cf4ocl2.h>
#include <string.h>

#define CCL_C_DESCRIPTION "Static kernel compiler and analyzer"
#define CCL_C_NODEVICE G_MAXUINT
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

/* Available tasks. */
typedef enum ccl_c_tasks {
	CCL_C_BUILD = 0,
	CCL_C_COMPILE = 1,
	CCL_C_LINK = 2
} CCLCTasks;

/* Command line arguments and respective default values. */
static gboolean opt_list = FALSE;
static guint dev_idx = CCL_C_NODEVICE;
static guint task = CCL_C_BUILD;
static gchar* options = NULL;
static gchar** src_files = NULL;
static gchar** bin_files = NULL;
static gchar** src_h_files = NULL;
static gchar** src_h_names = NULL;
static gchar* output = NULL;
static gboolean version = FALSE;
static gboolean silent = FALSE;

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
	{"input-headers",        'h', 0, G_OPTION_ARG_FILENAME_ARRAY, &src_h_files,
	 "Embedded header input files for the compile task. This option can be "
	 "specified multiple times.",                                 "FILE"},
	{"header-include-names", 'n', 0, G_OPTION_ARG_STRING_ARRAY,   &src_h_names,
	 "Embedded header include names for the compile task. This option can be "
	 "specified multiple and has a one to one correspondence with "
	 "--input-headers.",                                          "STRING"},
	{"bin",                  'b', 0, G_OPTION_ARG_FILENAME_ARRAY, &bin_files,
	 "Binary input file. This option can be specified multiple times.",
	                                                              "FILE"},
	{"output",               'o', 0, G_OPTION_ARG_FILENAME,       &output,
	 "Binary output file.",                                       "FILE"},
	{"silent",               'i', 0, G_OPTION_ARG_NONE,           &silent,
	 "Work silently, suppress output.",                           NULL},
	{"version",               0,  0, G_OPTION_ARG_NONE,           &version,
	 "Output version information and exit.",                      NULL},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }
};

/**
 * Parse and verify command line arguments.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * */
void ccl_c_args_parse(int argc, char* argv[], GError** err) {

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
	ccl_if_err_goto(*err, error_handler);

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
 * Output handler which discards output.
 *
 * @param[in] string Output to discard.
 * */
void ccl_c_print_silent(const gchar* string) {
	/* Do nothing. */
	CCL_UNUSED(string);
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
	GError* err = NULL, *err_build = NULL;

	/* Program return status. */
	gint status;

	/* Counter for for loops. */
	guint i = 0;

	/* Number of types of files and file names. */
	guint n_src_files, n_bin_files, n_src_h_files, n_src_h_names;

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
	ccl_if_err_goto(err, error_handler);

	/* Suppress output? */
	if (silent) {
		g_set_print_handler(ccl_c_print_silent);
		g_set_printerr_handler(ccl_c_print_silent);
	}

	g_print("\n");

	/* Determine main program goal. */
	if (version) {

		/* If version was requested, show version. */
		ccl_common_version_print("ccl_c");

	} else if (opt_list) {

		/* If user requested a list of available devices,
		 * present the list. */
		ccl_devsel_print_device_strings(&err);
		ccl_if_err_goto(err, error_handler);

	} else {

		/* Otherwise perform a task, which requires at least one input
		 * file and the specification of a device. */

		/* Check for input files. */
		ccl_if_err_create_goto(err, CCL_ERROR, (src_files == NULL) &&
			(src_h_files == NULL) && (bin_files == NULL),
			CCL_ERROR_ARGS, error_handler,
			"No source or binary input files have been specified.");

		/* How many files of each type? */
		n_src_files = src_files != NULL
			? g_strv_length(src_files) : 0;
		n_bin_files = bin_files != NULL
			? g_strv_length(bin_files) : 0;
		n_src_h_files = src_h_files != NULL
			? g_strv_length(src_h_files) : 0;
		n_src_h_names = src_h_names != NULL
			? g_strv_length(src_h_names) : 0;

		/* Select a context/device. */
		if (dev_idx == CCL_C_NODEVICE) {
			ctx = ccl_context_new_from_menu(&err);
		} else {
			ctx = ccl_context_new_from_device_index(&dev_idx, &err);
		}
		ccl_if_err_goto(err, error_handler);
		dev = ccl_context_get_device(ctx, 0, &err);
		ccl_if_err_goto(err, error_handler);

		/* Get and show device name. */
		dname = ccl_device_get_info_array(dev, CL_DEVICE_NAME, char*, &err);
		ccl_if_err_goto(err, error_handler);
		g_print("* Device: %s\n", dname);

		 /* Perform task. */
		switch (task) {
			case CCL_C_BUILD:

				/* For direct builds we can only have either one binary
				 * or one or more source files (but not both). */
				ccl_if_err_create_goto(err, CCL_ERROR,
					(n_src_files > 0) && (n_bin_files > 0),
					CCL_ERROR_ARGS, error_handler,
					"The 'build' task requires either: 1) one or more "
					"source files; or, 2) one binary file.");
				ccl_if_err_create_goto(err, CCL_ERROR, n_bin_files > 1,
					CCL_ERROR_ARGS, error_handler,
					"The 'build' task accepts at most one binary file.");

				/* Input headers are not accepted by the compile task. */
				ccl_if_err_create_goto(err, CCL_ERROR,
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
				ccl_if_err_goto(err, error_handler);

				/* Build program. */
				ccl_program_build(prg, options, &err_build);

				/* Only check for errors that are not build/compile/link
				 * failures. */
				if (!ccl_c_is_build_error(err_build)) {
					ccl_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			case CCL_C_COMPILE:

				/* Compilation requires at least one source file. */
				ccl_if_err_create_goto(err, CCL_ERROR, n_src_files == 0,
					CCL_ERROR_ARGS, error_handler,
					"The 'compile' task requires at least one source file.");

				/* Compilation does not support binaries. */
				ccl_if_err_create_goto(err, CCL_ERROR, n_bin_files > 0,
					CCL_ERROR_ARGS, error_handler,
					"The 'compile' task does not support binaries.");

				/* Number of header include names must be zero OR be the same as
				 * the number of input headers. */
				ccl_if_err_create_goto(err, CCL_ERROR,
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
						ccl_if_err_goto(err, error_handler);

						/* Add header program to array. */
						g_ptr_array_add(prgs, (gpointer) prg_aux);

					}
				}

				/* Create main program from source. */
				prg = ccl_program_new_from_source_files(ctx,
					n_src_files, (const char**) src_files, &err);
				ccl_if_err_goto(err, error_handler);

				/* Compile program. */
				ccl_program_compile(prg, 1, &dev, options,
					n_src_h_files,
					(CCLProgram**) (prgs ? prgs->pdata : NULL),
					(const char**) (src_h_names ? src_h_names : src_h_files),
					NULL, NULL, &err_build);

				/* Only check for errors that are not build/compile/link
				 * failures. */
				if (!ccl_c_is_build_error(err_build)) {
					ccl_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			case CCL_C_LINK:

				/* Linking requires at least one binary file and does
				 * not support source files. */
				ccl_if_err_create_goto(err, CCL_ERROR, (n_bin_files == 0) ||
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
						*bin_files, NULL, &err);
					ccl_if_err_goto(err, error_handler);

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
					ccl_if_err_propagate_goto(&err, err_build, error_handler);
				}

				break;

			default:
				ccl_if_err_create_goto(err, CCL_ERROR, TRUE,
					CCL_ERROR_ARGS, error_handler, "Unknown task: %d",
					task);
		}

		/* Get build status. */
		if (prg) {
			build_status = ccl_program_get_build_info_scalar(prg, dev,
				CL_PROGRAM_BUILD_STATUS, cl_build_status, &err);
			ccl_if_err_goto(err, error_handler);
			build_status_str = ccl_c_get_build_status_str(build_status);
		} else {
			build_status_str = "Unavailable";
		}

		/* Show build status. */
		g_print("* Build status: %s\n", build_status_str);

		/* If build successful, save binary? */
		if (output && prg && (build_status == CL_BUILD_SUCCESS)) {
			ccl_program_save_binary(prg, dev, output, &err);
			ccl_if_err_goto(err, error_handler);
			g_print("* Binary output file: %s\n", output);
		}

		/* Show build error message, if any. */
		if (err_build) {
			g_print("* Additional information: %s\n", err_build->message);
		}

		/* Show build log, if any. */
		g_print("* Build log:");
		if (!prg) {
			g_print(" Unavailable\n");
		} else {
			build_log = ccl_program_get_device_build_log(prg, dev);
			if ((build_log) && (strlen(build_log) > 0)) {
				g_print("\n%s\n", build_log);
			} else {
				g_print(" Empty\n");
			}
		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL);
	status = err_build ? CCL_ERROR_PROGRAM_BUILD : CCL_SUCCESS;
	goto cleanup;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);

	g_printerr("* Error: %s\n", err->message);
	status = (err->domain == CCL_ERROR) ? err->code : CCL_ERROR_OTHER;
	g_error_free(err);

cleanup:

	g_print("\n");

	/* Free stuff! */
	g_clear_error(&err_build);
	if (src_files) g_strfreev(src_files);
	if (src_h_files) g_strfreev(src_h_files);
	if (src_h_names) g_strfreev(src_h_names);
	if (bin_files) g_strfreev(bin_files);
	if (options) g_free(options);
	if (output) g_free(output);
	if (ctx) ccl_context_destroy(ctx);
	if (prg) ccl_program_destroy(prg);
	if (prgs) g_ptr_array_free(prgs, TRUE);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

	/* Return status. */
	return status;

}
