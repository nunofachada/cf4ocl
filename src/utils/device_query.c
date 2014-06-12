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
 * @brief Utility to query OpenCL platforms and devices.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include "device_query.h"

#define CL4_DEVICE_QUERY_DESCRIPTION "Utility for querying OpenCL \
platforms and devices"

/* Command line arguments and respective default values. */
static gboolean opt_all = FALSE;
static gboolean opt_basic = FALSE;
static gchar** opt_custom = NULL;

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"all",    'a', 0, G_OPTION_ARG_NONE,         &opt_all,   "Show all the available device information",      NULL},
	{"basic",  'b', 0, G_OPTION_ARG_NONE,         &opt_basic, "Show basic device information",                  NULL},
	{"custom", 'c', 0, G_OPTION_ARG_STRING_ARRAY, &opt_custom, "Show specific information, repeat as necessary", "cl_device_info"},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }	
};

/**
 * @brief Device query main program function.
 * 
 * @param argc Number of command line arguments.
 * @param argv Vector of command line arguments.
 * @return #CL4_SUCCESS if program returns with no error, or an error
 * code otherwise.
 */
int main(int argc, char* argv[]) {
	
	/* Error object. */
	GError* err = NULL;
	
	/* List of platform wrapper objects. */
	CL4Platforms* platforms = NULL;
	gchar* str;
	
	/* Function status. */
	gint status;
	
	/* Parse command line options */
	cl4_device_query_args_parse(argc, argv, &err);
	gef_if_error_goto(err, GEF_USE_GERROR, status, error_handler);
	
	printf("a : %s\n", opt_all ? "yes" : "no");
	printf("b : %s\n", opt_basic ? "yes" : "no");
	if (opt_custom != NULL)
		for (int i = 0; (str = opt_custom[i]) != NULL; i++)
			printf("c%d : %s\n", i, str);

	
	/* If we got here, everything is OK. */
	g_assert(err == NULL);
	status = CL4_SUCCESS;
	goto cleanup;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);
	fprintf(stderr, "%s\n", err->message);
	status = err->code;
	g_error_free(err);

cleanup:
		
	/* Free stuff! */
	if (platforms) cl4_platforms_destroy(platforms);
	
	g_strfreev(opt_custom);

	/* Return status. */
	return status;

}

/**
 * @brief Parse and verify command line arguments.
 * 
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @param err GLib error object for error reporting.
 * @return #CL4_SUCCESS if function returns with no error, or 
 * an error code otherwise.
 * */
gint cl4_device_query_args_parse(int argc, char* argv[], GError** err) {
	
	/* Aux. var. */
	gint status;

	/* Command line options context. */
	GOptionContext* context = NULL;

	/* Create parsing context. */
	context = g_option_context_new(" - " CL4_DEVICE_QUERY_DESCRIPTION);
	gef_if_error_create_goto(*err, CL4_ERROR, context == NULL, 
		CL4_ERROR_ARGS, error_handler, 
		"Unable to create command line parsing context.");
	
	/* Add acceptable command line options to context. */ 
	g_option_context_add_main_entries(context, entries, NULL);
	
	/* Use context to parse command line options. */
	g_option_context_parse(context, &argc, &argv, err);
	gef_if_error_goto(*err, CL4_ERROR_ARGS, status, error_handler);
	
	/* If we get here, no need for error treatment, jump to cleanup. */
	g_assert(*err == NULL);
	status = CL4_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here, everything is OK. */
	g_assert(*err != NULL);

cleanup:	

	/* Free context. */
	if (context) g_option_context_free(context);
	
	/* Return function status. */
	return status;

}
