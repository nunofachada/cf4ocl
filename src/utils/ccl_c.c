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
static gchar** src_h_files = NULL;
static gchar** bin_files = NULL;
static gboolean hide_log = FALSE;
static gchar* output = NULL;
static gboolean version = FALSE;

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"list",     'l', 0, G_OPTION_ARG_NONE,               &opt_list,
	 "List available devices and exit",                   NULL},
	{"device",   'd', 0, G_OPTION_ARG_INT,                &dev_idx,
	 "Specify a device on which to perform the task",     "DEV"},
	{"task",     't', 0, G_OPTION_ARG_INT,                &task,
	 "0: Compile + Link (default); 1: Compile; 2: Link",  "TASK"},
	{"options",  '0', 0, G_OPTION_ARG_STRING,             &options,
	 "Compiler/linker options",                           "OPTIONS"},
	{"src",      's', 0, G_OPTION_ARG_FILENAME_ARRAY,     &src_files,
	 "Source input file",                                 "FILE"},
	{"src-h",    'h', 0, G_OPTION_ARG_FILENAME_ARRAY,     &src_h_files,
	 "Source header input file",                          "FILE"},
	{"bin",      'b', 0, G_OPTION_ARG_FILENAME_ARRAY,     &bin_files,
	 "Binary input file",                                 "FILE"},
	{"hidelog",  'h', 0, G_OPTION_ARG_NONE,               &hide_log,
	 "Hide build log",                                    NULL},
	{"output",   'o', 0, G_OPTION_ARG_FILENAME,           &output,
	 "Binary output file",                               "FILE"},
	{"version",   0, 0, G_OPTION_ARG_NONE,                &version,
	 "Output version information and exit",               NULL},
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
 * Kernel analyzer main program function.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Vector of command line arguments.
 * @return ::CCL_SUCCESS if program returns with no error, or another
 * ::CCLErrorCode value otherwise.
 */
int main(int argc, char* argv[]) {

	/* Error object. */
	GError* err = NULL;

	/* Program return status. */
	gint status;

	/* Number of types of files. */
	guint n_src_files, n_src_h_files, n_bin_files;

	/* All source files. */
	gchar** src_files_all = NULL;

	/* Device filters. */
	CCLDevSelFilters filters = NULL;

	/* Context wrapper. */
	CCLContext* ctx = NULL;

	/* Device wrapper. */
	CCLDevice* dev = NULL;

	/* Program wrapper. */
	CCLProgram* prg = NULL;

	/* Parse command line options. */
	ccl_c_args_parse(argc, argv, &err);
	ccl_if_err_goto(err, error_handler);

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
		n_src_h_files = src_h_files != NULL
			? g_strv_length(src_h_files) : 0;
		n_bin_files = bin_files != NULL
			? g_strv_length(bin_files) : 0;

		/* Select a context/device. */
		ccl_devsel_add_dep_filter(
			&filters, ccl_devsel_dep_menu,
			(dev_idx == CCL_C_NODEVICE) ? NULL : (void*) &dev_idx);
		ctx = ccl_context_new_from_filters(&filters, &err);
		ccl_if_err_goto(err, error_handler);
		dev = ccl_context_get_device(ctx, 0, &err);
		ccl_if_err_goto(err, error_handler);

		 /* Perform task. */
		switch (task) {
			case CCL_C_BUILD:

				/* For direct builds we can only have either binary or
				 * source files, but not both. */
				ccl_if_err_create_goto(err, CCL_ERROR,
					((n_src_files + n_src_h_files > 0) &&
					(n_bin_files > 0)) || (n_bin_files > 1),
					CCL_ERROR_ARGS, error_handler,
					"The 'build' task requires either: 1) one or more "
					"source files; or, 2) one binary file.");

				/* Create program object. */
				if (n_bin_files == 1) {

					/* Create program from binary file. */
					prg = ccl_program_new_from_binary_file(ctx, dev,
						*bin_files, NULL, &err);

				} else {

					/* Join header sources + program sources. */
					src_files_all = (gchar**)
						g_slice_alloc(sizeof(gchar*) *
							(n_src_files + n_src_h_files));
					g_memmove(src_files_all, src_h_files,
						n_src_h_files * sizeof(char*));
					g_memmove(src_files_all + n_src_h_files, src_files,
						n_src_files * sizeof(char*));

					/* Create program from source. */
					prg = ccl_program_new_from_source_files(ctx,
						n_src_h_files + n_src_files,
						(const char**) src_files_all, &err);

					/* Free joined header + program sources. */
					g_slice_free1(sizeof(gchar*) *
							(n_src_files + n_src_h_files),
							src_files_all);

				}
				ccl_if_err_goto(err, error_handler);

				/* Build program. */
				ccl_program_build(prg, options, &err);
				ccl_if_err_goto(err, error_handler);

				break;

			case CCL_C_COMPILE:

				g_printf("Compile kernel\n");
				break;

			case CCL_C_LINK:

				g_printf("Link kernel\n");
				break;

			default:
				ccl_if_err_create_goto(err, CCL_ERROR, TRUE,
					CCL_ERROR_ARGS, error_handler, "Unknown task: %d",
					task);
		}

		/* Output binary? */
		if (output) {
			ccl_program_save_binary(prg, dev, output, &err);
			ccl_if_err_goto(err, error_handler);
		}

		/* Show build log? */
		if (!hide_log) {

			printf("%s", ccl_program_get_build_log(prg));

		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL);
	status = CCL_SUCCESS;
	goto cleanup;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);

	if (err->code == CL_BUILD_PROGRAM_FAILURE) {
		g_fprintf(stderr, "%s", ccl_program_get_build_log(prg));
	} else {
		g_fprintf(stderr, "%s\n", err->message);
	}
	status = (err->domain == CCL_ERROR) ? err->code : CCL_ERROR_OTHER;
	g_error_free(err);

cleanup:

	/* Free stuff! */
	if (src_files) g_strfreev(src_files);
	if (src_h_files) g_strfreev(src_h_files);
	if (bin_files) g_strfreev(bin_files);
	if (options) g_free(options);
	if (output) g_free(output);
	if (ctx) ccl_context_destroy(ctx);
	if (prg) ccl_program_destroy(prg);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

	/* Return status. */
	return status;

}
