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
 * Bank conflicts test example. Control the level of conflicts 
 * using the stride `-s` parameter. 
 * 
 * The frequency of bank conflicts can be increased by doubling the 
 * stride `-s` parameter, e.g. 1, 2, 4, 16, 32. The maximum number of 
 * conflicts is obtained  with `s=16` or `s=32`, depending if the GPU 
 * has 16 or 32 banks of local memory.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include "bank_conflicts.h"

/** Default global work size, dimension 0. */
#define GWS_X 4096
/** Default global work size, dimension 1. */
#define GWS_Y 4096
/** Default local work size, dimension 0. */
#define LWS_X 32
/** Default local work size, dimension 1. */
#define LWS_Y 16
/** Default stride. */
#define STRIDE 1

/** A description of the program. */
#define PROG_DESCRIPTION "Program for testing bank conflicts on the GPU"

/* Command line arguments and respective default values. */
static size_t gws[] = {GWS_X, GWS_Y};
static size_t lws[] = {LWS_X, LWS_Y};
static gchar* compiler_opts = NULL;
static gboolean dev_list = FALSE;
static int dev_idx = -1;
static int stride = STRIDE;
static gboolean version;

/* Callback functions to parse gws and lws. */
static gboolean bct_parse_gws(const gchar *option_name, 
	const gchar *value, gpointer data, GError **err) {
	ccl_ex_parse_pairs(value, gws, option_name, data, err);
}
static gboolean bct_parse_lws(const gchar *option_name, 
	const gchar *value, gpointer data, GError **err) {
	ccl_ex_parse_pairs(value, lws, option_name, data, err);
}

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"compiler",   'c', 0, G_OPTION_ARG_STRING,   &compiler_opts,
		"Extra OpenCL compiler options", 
		"OPTS"},
	{"globalsize", 'g', 0, G_OPTION_ARG_CALLBACK, bct_parse_gws,
		"Work size (default is " \
		G_STRINGIFY(GWS_X) "," G_STRINGIFY(GWS_Y) ")", 
		"SIZE,SIZE"},
	{"localsize",  'l', 0, G_OPTION_ARG_CALLBACK, bct_parse_lws,
		"Local work size (default is " \
		G_STRINGIFY(LWS_X) "," G_STRINGIFY(LWS_Y) ")", 
		"SIZE,SIZE"},
	{"stride",     's', 0, G_OPTION_ARG_INT,      &stride,
		"Stride (default is " G_STRINGIFY(STRIDE) ")", 
		"STRIDE"},
	{"list",      'i', 0, G_OPTION_ARG_NONE,      &dev_list,
		"List available devices (selectable with -d) and exit",
		NULL},
	{"device",     'd', 0, G_OPTION_ARG_INT,      &dev_idx,
		"Device index (if not given and more than one device is "\
		"available, chose device from menu)", 
		"INDEX"},
	{"version",     0,  0, G_OPTION_ARG_NONE,     &version,
		"Output version information and exit",
		NULL},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }	
};

/* Kernel file. */
static char* kernel_files[] = {"bank_conflicts.cl"};

/** 
 * Bank conflicts example main function.
 * 
 * The frequency of bank conflicts can be increased by doubling the 
 * stride `-s` parameter, e.g. 1, 2, 4, 16, 32. The maximum number of 
 * conflicts is obtained  with `s=16` or `s=32`, depending if the GPU 
 * has 16 or 32 banks of local memory.
 * 
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return #CCL_EX_SUCCESS if program returns with no error, or 
 * #CCL_EX_FAIL otherwise.
 * */
int main(int argc, char *argv[]) {

	/* ***************** */	
	/* Program variables */
	/* ***************** */	
	
	/* Function and program return status. */
	int status;
	/* Error management. */
	GError *err = NULL;
	/* Profiling / Timmings. */
	CCLProf* prof = NULL;
	/* Context wrapper. */
	CCLContext* ctx = NULL;
	/* Program wrapper. */
	CCLProgram* prg = NULL;
	/* Command queue wrapper. */
	CCLQueue* cq = NULL;
	/* Data in device. */
	CCLBuffer* buf_data_dev = NULL;
	/* Full kernel path. */
	gchar* kernel_path = NULL;
	/* Data in host. */
	cl_int *data_host = NULL;
	/* Size of data to be transfered to device. */
	size_t size_data_in_bytes;
	/* Size of local memory required. */
	size_t local_mem_size_in_bytes;
	/* Command line options context. */
	GOptionContext* opt_ctx = NULL;
	/* Random number generator. */
	GRand* rng = NULL;
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	/* Create parsing context. */
	opt_ctx = g_option_context_new (" - " PROG_DESCRIPTION);
	/* Add acceptable command line options to context. */ 
	g_option_context_add_main_entries(opt_ctx, entries, NULL);
	/* Use context to parse command line options. */
	g_option_context_parse(opt_ctx, &argc, &argv, &err);
	ccl_if_err_goto(err, error_handler);
	
	/* If version was requested, output version and exit. */
	if (version) {
		ccl_common_version_print("cf4ocl Bankconf example");
		exit(0);
	}
	/* If device list was requested, present list of devices and
	 * exit. */
	if (dev_list) {
		g_printf("\n");
		ccl_devsel_print_device_strings(&err);
		g_printf("\n");
		ccl_if_err_goto(err, error_handler);
		exit(0);
	}
	
	/* ******************************************************* */
	/* Initialize profiler, OpenCL variables and build program */
	/* ******************************************************* */
	
	/* Initialize RNG. */
	rng = g_rand_new_with_seed(0);
		
	/* Initialize profiling object. */
	prof = ccl_prof_new();

	/* Create a GPU context. */
	ctx = ccl_context_new_from_menu_full(
		dev_idx == -1 ? NULL : &dev_idx, &err);
	ccl_if_err_goto(err, error_handler);

	/* Get location of kernel file, which should be in the same location 
	 * of the bankconf executable. */
	kernel_path = ccl_ex_kernelpath_get(kernel_files[0], argv[0]);
	
	/* Create program. */
	prg = ccl_program_new_from_source_file(ctx, kernel_path, &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Build program. */
	status = ccl_program_build(prg, compiler_opts, &err);
	ccl_if_err_goto(err, error_handler);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, NULL, CL_QUEUE_PROFILING_ENABLE, &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Start basic timming / profiling. */
	ccl_prof_start(prof);

	/* Allocate data in host */
	size_data_in_bytes = gws[0] * gws[1] * sizeof(cl_int);
	data_host = (cl_int*) g_malloc(size_data_in_bytes);
	
	/* Allocate data in device */
	buf_data_dev = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, 
		size_data_in_bytes, NULL, &err);
	ccl_if_err_goto(err, error_handler);
	
	/* Copy data from host to device. */
	ccl_buffer_enqueue_write(cq, buf_data_dev, CL_TRUE, 0, size_data_in_bytes,
		data_host, NULL, &err);
	ccl_if_err_goto(err, error_handler);

	/* ************************************************** */
	/* Determine and print required memory and work sizes */
	/* ************************************************** */

	local_mem_size_in_bytes = lws[1] * lws[0] * sizeof(cl_int);
	ccl_ex_reqs_print(gws, lws, size_data_in_bytes, local_mem_size_in_bytes);

	/* ************************************ */
	/*  Set kernel arguments and run kernel */
	/* ************************************ */
	
	ccl_program_enqueue_kernel(prg, "bankconf", cq, 2, NULL, gws, lws, NULL, &err,
		buf_data_dev, ccl_arg_local(lws[1] * lws[0], cl_int),
		ccl_arg_priv(stride, cl_uint), NULL);
	ccl_if_err_goto(err, error_handler);
	
	/* Wait... */
	ccl_queue_finish(cq, &err);
	ccl_if_err_goto(err, error_handler);

	/* ******************** */
	/*  Show profiling info */
	/* ******************** */
	
	ccl_prof_stop(prof); 

	ccl_prof_add_queue(prof, "Q1", cq);
	
	ccl_prof_calc(prof, &err);
	ccl_if_err_goto(err, error_handler);

	ccl_prof_print_summary(prof);
	
	/* If we get here, no need for error checking, jump to cleanup. */
	g_assert(err == NULL);
	status = CCL_EX_SUCCESS;
	goto cleanup;
	
	/* ************** */
	/* Error handling */
	/* ************** */
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err != NULL);
	fprintf(stderr, "Error %d from domain '%s' with message: \"%s\"\n", 
		err->code, g_quark_to_string(err->domain), err->message);
	g_error_free(err);
	status = CCL_EX_FAIL;

cleanup:
		
	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	/* Free command line variables. */
	if (opt_ctx) g_option_context_free(opt_ctx);
	if (compiler_opts) g_free(compiler_opts);
	
	/* Free RNG */
	if (rng != NULL) g_rand_free(rng);
	
	/* Free profile */
	if (prof) ccl_prof_destroy(prof);
	
	/* Free wrappers. */
	if (buf_data_dev) ccl_buffer_destroy(buf_data_dev);
	if (cq) ccl_queue_destroy(cq);
	if (prg) ccl_program_destroy(prg);
	if (ctx) ccl_context_destroy(ctx);
	
	/* Free host resources */
	if (data_host) g_free(data_host);

	/* Free kernel path. */
	if (kernel_path) g_free(kernel_path);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_return_val_if_fail(ccl_wrapper_memcheck(), CCL_EX_FAIL);
	
	/* Return status. */
	return status;

}

