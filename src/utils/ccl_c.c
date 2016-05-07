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
static guint opt_task = CCL_C_BUILD;
static gchar** inputs = NULL;
static gboolean hide_log = FALSE;
static gchar** kernel_names = NULL;
static gchar* opt_output = NULL;
static gboolean version = FALSE;

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"list",     'l', 0, G_OPTION_ARG_NONE,               &opt_list,
	 "List available devices",                            NULL},
	{"device",   'd', 0, G_OPTION_ARG_INT,                &dev_idx,
	 "Specify a device on which to perform the task",     "DEV"},
	{"task",     't', 0, G_OPTION_ARG_INT,                &opt_task,
	 "0: Compile + Link (default); 1: Compile; 2: Link",  "TASK"},
	{"input",    'i', 0, G_OPTION_ARG_FILENAME_ARRAY,     &inputs,
	 "Input file for build, compile or link tasks",       "FILE"},
	{"hidelog",  'h', 0, G_OPTION_ARG_NONE,               &hide_log,
	 "Hide build log",                                    NULL},
	{"kerninfo", 'k', 0, G_OPTION_ARG_STRING_ARRAY,       &kernel_names,
	 "Show information for KERNEL",                       "KERNEL"},
	{"output",   'o', 0, G_OPTION_ARG_FILENAME,           &opt_output,
	 "Output file for compile and/or link tasks",         "FILE"},
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

	/* Counter. */
	int i;

	/* Device filters. */
	CCLDevSelFilters filters = NULL;

	/* Context wrapper. */
	CCLContext* ctx = NULL;

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
		ccl_if_err_create_goto(err, CCL_ERROR, inputs == NULL,
					CCL_ERROR_ARGS, error_handler,
					"No input files have been specified");

		/* Select a context/device. */
		ccl_devsel_add_dep_filter(
			&filters, ccl_devsel_dep_menu,
			(dev_idx == CCL_C_NODEVICE) ? NULL : (void*) &dev_idx);
		ctx = ccl_context_new_from_filters(&filters, &err);
		ccl_if_err_goto(err, error_handler);

		 /* Perform task. */
		switch (opt_task) {
			case CCL_C_BUILD:
				g_printf("Build kernel\n");
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
					opt_task);
		}

		/* Show build log? */
		if (!hide_log) {

			printf("Build log not hidden!\n");

		}

		/* Show information about kernels? */
		if (kernel_names) {

			/* Cycle through the specified kernels. */
			for (i = 0; kernel_names[i] != NULL; i++) {

				printf("Kernel name: %s\n", kernel_names[i]);

			}

		}

	}

	/* If we got here, everything is OK. */
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

	/* Free stuff! */
	if (inputs) g_strfreev(inputs);
	if (kernel_names) g_strfreev(kernel_names);
	if (opt_output) g_free(opt_output);
	if (ctx) ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_ERROR_OTHER);

	/* Return status. */
	return status;

}
