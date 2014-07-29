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
 * @brief Matrix multiplication examples.
 * 
 * The OpenCL matrix multiplication kernels 0 to 4 are based on the 
 * [CUDA best practices guide](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html). 
 * More specifically:
 * 
 * * 0. [Unoptimized matrix multiplication](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__unoptimized-matrix-multiplication)
 * * 1. [Using shared memory to improve the global memory load efficiency in matrix multiplication](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__using-shared-memory-improve-global-memory-load-efficiency-matrix-multiplication)
 * * 2. [Improvement by reading additional data into shared memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-ab__improvement-reading-additional-data-shared-memory)
 * * 3. [Unoptimized handling of strided accesses to global memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-aa__unoptimized-handling-strided-accesses-global-memory)
 * * 4. [An optimized handling of strided accesses using coalesced reads from global memory](http://docs.nvidia.com/cuda/cuda-c-best-practices-guide/index.html#shared-memory-in-matrix-multiplication-c-aa__optimized-version-coalesced-reads-global-memory)
 *
 * Kernels 0 to 2 perform multiplication of matrices A and B (@f$C=AB@f$), 
 * while kernels 3 and 4 perform multiplication of matrix A by its
 * transpose (@f$C=AA^T@f$).
 * 
 * The kernels are optimized for a GPU device, however they also 
 * run on a CPU device.
 * 
 * The OpenMP implementation is a basic parallelized for loop, which
 * runs on the CPU.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html) 
 * */

#include "matmult.h"

/* Default number of columns in Matrix A. */
#define A_COLS 128
/* Default number of rows in Matrix A. */
#define A_ROWS 256
/* Default number of columns in Matrix B. */
#define B_COLS 16
/* Number of rows in matrix B must be equal to number of columns in 
 * matrix A. */
#define B_ROWS A_COLS

/* Default workgroup size, dimension 0. */
#define LWS_X 32
/* Default workgroup size, dimension 1. */
#define LWS_Y 16

/* Default minimum range value (from). */
#define RANGE_MATRIX_FROM -100

/* Default maximum range value (to). */
#define RANGE_MATRIX_TO 100

/* Default kernel selection: 0, 1 or 2 (higher values correspond to 
 * more optimized kernels). */
#define KERNEL_ID 0 

/* Default verbose status. */
#define VERBOSE FALSE

/* Default seed. */
#define SEED 0

/* A description of the program. */
#define PROG_DESCRIPTION "Program for testing matrix multiplication on \
	a OpenCL device (GPU or CPU, although optimized for the former) \
	and compare with OpenMP implementation on the CPU."

/* Command line arguments and respective default values. */
static int a_dim[] = {A_COLS, A_ROWS};
static int b_dim[] = {B_COLS, B_ROWS};
static size_t lws[] = {LWS_X, LWS_Y};
static int matrix_range[] = {RANGE_MATRIX_FROM, RANGE_MATRIX_TO};
static gchar* compiler_opts = NULL;
static gboolean dev_list = FALSE;
static int dev_idx = -1;
static gchar* name = NULL;
static int kernel_id = KERNEL_ID;
static gboolean verbose = VERBOSE;
static guint32 seed = SEED;
static gchar* output_export = NULL;

/* Callback functions to parse pairs of numbers. */
static gboolean mm_parse_a(const gchar *option_name, const gchar *value, gpointer data, GError **err) {
	clexp_parse_pairs(value, a_dim, option_name, data, err);
}
static gboolean mm_parse_b(const gchar *option_name, const gchar *value, gpointer data, GError **err) {
	clexp_parse_pairs(value, b_dim, option_name, data, err);
}
static gboolean mm_parse_lws(const gchar *option_name, const gchar *value, gpointer data, GError **err) {
	clexp_parse_pairs(value, lws, option_name, data, err);
}
static gboolean mm_parse_rge(const gchar *option_name, const gchar *value, gpointer data, GError **err) {
	clexp_parse_pairs(value, matrix_range, option_name, data, err);
}

/* Valid command line options. */
static GOptionEntry entries[] = {
	{"kernel",    'k', 0, G_OPTION_ARG_INT,      &kernel_id,
		"kernel selection: 0-2 (C=AB), 3-4 (C=AA^T) (default is " \
		G_STRINGIFY(KERNEL_ID) ")",
		"ID"},
	{"asize",     'a', 0, G_OPTION_ARG_CALLBACK, mm_parse_a,
		"Size (cols,rows) of matrix A (default is " G_STRINGIFY(A_COLS) \
		 "," G_STRINGIFY(A_ROWS) ")",
		 "SIZE,SIZE"},
	{"bsize",     'b', 0, G_OPTION_ARG_CALLBACK, mm_parse_b,
		"Size (cols,rows) of matrix B (default is " G_STRINGIFY(B_COLS) \
		"," G_STRINGIFY(B_ROWS) ")",
		"SIZE,SIZE"},
	{"localsize", 'l', 0, G_OPTION_ARG_CALLBACK, mm_parse_lws,
		"Local work size (default is " G_STRINGIFY(LWS_X) "," \
		G_STRINGIFY(LWS_Y) ")",
		"SIZE,SIZE"},
	{"range",     'r', 0, G_OPTION_ARG_CALLBACK, mm_parse_rge,
		"Matrix range of values (default is " \
		G_STRINGIFY(RANGE_MATRIX_FROM) "," \
		G_STRINGIFY(RANGE_MATRIX_TO)")", 
		"MIN,MAX"},
	{"seed",      's', 0, G_OPTION_ARG_INT,      &seed,
		"RNG seed (default is " G_STRINGIFY(SEED)")",
		"SEED"},
	{"verbose",   'v', 0, G_OPTION_ARG_NONE,     &verbose,
		"Print input and output matrices to stderr",
		NULL},
	{"list",      'i', 0, G_OPTION_ARG_NONE,      &dev_list,
		"List available devices (selectable with -d) and exit",
		NULL},
	{"device",    'd', 0, G_OPTION_ARG_INT,      &dev_idx,
		"Device index, auto-selects device from menu (takes priority " \
		"over -n option)",
		"INDEX"},
	{"name",     'n', 0, G_OPTION_ARG_STRING,   &name,
		"Selects device by device, platform or vendor name",
		"NAME"},
	{"compiler",  'c', 0, G_OPTION_ARG_STRING,   &compiler_opts,
		"Extra OpenCL compiler options",
		"OPTS"},
	{"output",    'o', 0, G_OPTION_ARG_FILENAME, &output_export,
		"File where to export profiling info (default is none)",
		"FILE"},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }	
};

/* Kernel file. */
static char* kernel_files[] = {"matmult.cl"};

/** 
 * @brief OpenCL and OpenMP matrix multiplication main function. 
 * 
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return #CLEXP_SUCCESS if program returns with no error, or 
 * #CLEXP_FAIL otherwise.
 * */
int main(int argc, char *argv[]) {

	/* ************* */
	/* Program vars. */
	/* ************* */

	/* Function and program return status. */
	int status;
	/* Error management */
	GError* err = NULL;
	/* Random number generator. */
	GRand* rng = NULL;
	/* Profiler for OpenCL device implementation */
	CL4Prof* prof_dev = NULL;
	/* Profiler for OpenMP CPU implementation */
	CL4Prof* prof_cpu = NULL;
	/* Device name */
	char* dev_name;
	/* Device vendor. */
	char* dev_vendor;
	/* Context wrapper. */
	CL4Context* ctx = NULL;
	/* Device wrapper. */
	CL4Device* dev = NULL;
	/* Program wrapper. */
	CL4Program* prg = NULL;
	/* Command queue wrapper. */
	CL4CQueue* cq = NULL;
	/* Kernel wrapper. */
	CL4Kernel* krnl = NULL;
	/* Kernel name */
	gchar* kernel_name = NULL;
	/* Full kernel path. */
	gchar* kernel_path = NULL;
	/* Host matrix A */
	cl_int* matrixA_host = NULL;
	/* Host matrix B */
	cl_int* matrixB_host = NULL;
	/* Host matrix C (calcutated on OpenCL device). */
	cl_int* matrixC_host = NULL;
	/* Host matrix C (calculated on the CPU). */
	int *matrixC_test = NULL;
	/* Device matrix A. */
	CL4Buffer* matrixA_dev = NULL;
	/* Device matrix B. */
	CL4Buffer* matrixB_dev = NULL;
	/* Device matrix C. */
	CL4Buffer* matrixC_dev = NULL;
	/* Global work sizes */
	size_t gws[2];
	/* Size of matrix A in bytes. */
	size_t size_matA_in_bytes = 0;
	/* Size of matrix B in bytes. */
	size_t size_matB_in_bytes = 0;
	/* Size of matrix C in bytes. */
	size_t size_matC_in_bytes = 0;
	/* Size of device global memory required. */
	size_t g_mem_size_in_bytes;
	/* Size of local memory required by matrix A (depends on kernel id). */
	size_t l_mem_sizeA_in_bytes;
	/* Size of local memory required by matrix B (depends on kernel id). */
	size_t l_mem_sizeB_in_bytes;
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	matmult_args_parse(argc, argv, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* If device list was required, present list of devices and
	 * exit. */
	if (dev_list) {
		g_printf("\n");
		cl4_devsel_print_device_strings(&err);
		g_printf("\n");
		gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
		exit(0);
	}

	/* ******************************************************* */
	/* Initialize profiler, OpenCL variables and build program */
	/* ******************************************************* */
	
	/* Initialize RNG. */
	rng = g_rand_new_with_seed(seed);
		
	/* Profiling / Timmings */
	prof_dev = cl4_prof_new();
	prof_cpu = cl4_prof_new();

	/* Create the context wrapper. */
	if ((dev_idx != -1) || (name == NULL)) {
		/* Select device by index or user choice. */
		ctx = cl4_context_new_from_menu_full(
			dev_idx != -1 ? &dev_idx : NULL, &err);
	} else {
		/* Select device by device name, platform name or vendor name. */
		ctx = cl4_context_new_from_indep_filter(cl4_devsel_indep_string, name, &err);
	}
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* Print information about selected device. */
	dev = cl4_context_get_device(ctx, 0, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	dev_name = cl4_device_get_array_info(dev, CL_DEVICE_NAME, char*, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	dev_vendor = cl4_device_get_array_info(dev, CL_DEVICE_VENDOR, char*, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	g_printf("\n   == Using device '%s' from '%s'\n", dev_name, dev_vendor);
	
	/* Get location of kernel file, which should be in the same location 
	 * has the matmult executable. */
	kernel_path = clexp_kernelpath_get(kernel_files[0], argv[0]);
	
	/* Create and build program. */
	prg = cl4_program_new_from_source_file(ctx, kernel_path, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	cl4_program_build(prg, compiler_opts, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* Determine kernel name. */
	kernel_name = g_strdup_printf("matmult%d", kernel_id);

	/* Get kernel. */
	krnl = cl4_program_get_kernel(prg, kernel_name, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* Create command queue wrapper. */
	cq = cl4_cqueue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* ********************************** */	
	/* Create and initialize host buffers */
	/* ********************************** */	
	
	/* Matrix A */
	size_matA_in_bytes = a_dim[0] * a_dim[1] * sizeof(cl_int);
	matrixA_host = matmult_matrix_new(
		a_dim[0], a_dim[1], matrix_range, rng);
	
	/* Matrix B */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		size_matB_in_bytes = b_dim[0] * b_dim[1] * sizeof(cl_int);
		matrixB_host = matmult_matrix_new(
			b_dim[0], b_dim[1], matrix_range, rng);
	}
	
	/* Matrix C (result) */
	size_matC_in_bytes = b_dim[0] * a_dim[1] * sizeof(cl_int);
	matrixC_host = matmult_matrix_new(b_dim[0], a_dim[1], NULL, NULL);

	/* ********************* */
	/* Create device buffers */
	/* ********************* */

	/* Matrix A */
	matrixA_dev = cl4_buffer_new(ctx, CL_MEM_READ_ONLY, 
		size_matA_in_bytes, NULL, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
		
	/* Matrix B */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		matrixB_dev = cl4_buffer_new(ctx, CL_MEM_READ_ONLY, 
			size_matB_in_bytes, NULL, &err);
		gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	}

	/* Matrix C */
	matrixC_dev = cl4_buffer_new(ctx, CL_MEM_WRITE_ONLY, 
			size_matC_in_bytes, NULL, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* ************************* */
	/* Initialize device buffers */
	/* ************************* */
	
	/* Start basic timming / profiling. */
	cl4_prof_start(prof_dev);
	
	/* Copy matrix A to device. */
	cl4_buffer_write(cq, matrixA_dev, CL_TRUE, 0, size_matA_in_bytes, 
		matrixA_host, NULL, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* Copy matrix B to device. */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		cl4_buffer_write(cq, matrixB_dev, CL_TRUE, 0, 
			size_matB_in_bytes, matrixB_host, NULL, &err); 
		gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	}
	
	/* ******************** */
	/*  Determine worksizes */
	/* ******************** */
	
	gws[0] = lws[0]* ceil(((float) b_dim[0]) / lws[0]);
	gws[1] = lws[1] * ceil(((float) a_dim[1]) / lws[1]);
		
	/* ************************* */
	/* Determine required memory */
	/* ************************* */

	/* Global memory requirements. */
	g_mem_size_in_bytes = 
		size_matA_in_bytes + size_matB_in_bytes + size_matC_in_bytes;
	
	/* Local memory requirements. Default is 0 for non-optimized kernels 0 and 3. */
	l_mem_sizeA_in_bytes = 0;
	l_mem_sizeB_in_bytes = 0;
	if (kernel_id >= 1)
		/* Optimized matrix mult. 1*/
		l_mem_sizeA_in_bytes = a_dim[0] * lws[1] * sizeof(cl_int);
	if (kernel_id == 2)
		/* Optimized matrix mult. 2*/
		l_mem_sizeB_in_bytes = lws[0] * b_dim[1] * sizeof(cl_int);
	if (kernel_id == 4) {
		/* Optimized matrix transpose mult. */
		l_mem_sizeA_in_bytes = lws[1] * a_dim[0] * sizeof(cl_int);
		l_mem_sizeB_in_bytes = lws[0] * a_dim[0] * sizeof(cl_int);
	}
	
	/* ****************************** */
	/* Print requirements information */
	/* ****************************** */

	clexp_reqs_print(gws, lws, g_mem_size_in_bytes, 
		l_mem_sizeA_in_bytes + l_mem_sizeB_in_bytes);

	/* *************************** */
	/*  Set fixed kernel arguments */
	/* *************************** */

	if (!IS_AAT(kernel_id)) {

		/* Arguments for C=AB */
		if (kernel_id == 0) {
			cl4_kernel_set_args(krnl, matrixA_dev, matrixB_dev, 
				matrixC_dev, cl4_arg_private(a_dim, cl_int2), 
				cl4_arg_private(b_dim, cl_int2), NULL);
		} else if (kernel_id == 1) {
			cl4_kernel_set_args(krnl, matrixA_dev, matrixB_dev, 
				matrixC_dev, cl4_arg_private(a_dim, cl_int2), 
				cl4_arg_private(b_dim, cl_int2), 
				cl4_arg_new(NULL, l_mem_sizeA_in_bytes), NULL);
		} else if (kernel_id == 2) {
			cl4_kernel_set_args(krnl, matrixA_dev, matrixB_dev, 
				matrixC_dev, cl4_arg_private(a_dim, cl_int2), 
				cl4_arg_private(b_dim, cl_int2), 
				cl4_arg_new(NULL, l_mem_sizeA_in_bytes),
				cl4_arg_new(NULL, l_mem_sizeB_in_bytes), NULL);
		}

	} else {

		/* Arguments only for C=AA^T */
		if (kernel_id < 4) {
			cl4_kernel_set_args(krnl, matrixA_dev, matrixC_dev, 
				cl4_arg_private(a_dim, cl_int2), NULL);
		} else if (kernel_id == 4) {
			cl4_kernel_set_args(krnl, matrixA_dev, matrixC_dev, 
				cl4_arg_private(a_dim, cl_int2),
				cl4_arg_new(NULL, l_mem_sizeA_in_bytes),
				cl4_arg_new(NULL, l_mem_sizeB_in_bytes), NULL);
		}
	}
	
	/* ************ */
	/*  Run kernel! */
	/* ************ */
	
	cl4_kernel_run(krnl, cq, 2, NULL, gws, lws, NULL, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* *********************** */
	/*  Get result from device */
	/* *********************** */

	cl4_buffer_read(cq, matrixC_dev, CL_TRUE, 0, size_matC_in_bytes, 
		matrixC_host, NULL, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* Finish execution. */
	cl4_cqueue_finish(cq, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* ************************************** */
	/*  Manage profiling of OpenCL operations */
	/* ************************************** */
	
	/* Stop profiler. */
	cl4_prof_stop(prof_dev); 

	/* Add queue for profiling. */
	cl4_prof_add_queue(prof_dev, "Queue1", cq);

	/* Process profiling data. */
	cl4_prof_calc(prof_dev, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* Show profiling info. */
	cl4_prof_print_summary(prof_dev);
	
	/* Export profiling info if a filename was given. */
	if (output_export) {
		cl4_prof_export_info_file(prof_dev, output_export, &err);
		gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	}
		
	/* ********************************************************* */
	/* Perform multiplication on the CPU with the help of OpenMP */
	/* ********************************************************* */

	/* Allocate memory for a new results matrix. */
	matrixC_test = matmult_matrix_new(b_dim[0], a_dim[1], NULL, NULL);
		
	/* Start basic timming / profiling. */
	cl4_prof_start(prof_cpu);

	if (!IS_AAT(kernel_id)) {
		/* C=AB */
		/* The pragma below makes the loop parallel with 
		 * threads = number of cores */
#ifdef CF4OCL_USE_OPENMP		
		#pragma omp parallel for
#endif
		for (int col = 0; col < b_dim[0]; col++) {
			for (int row = 0; row < a_dim[1]; row++) {
				matrixC_test[row * b_dim[0] + col] = 0;
				for (int i = 0; i < a_dim[0]; i++) {
					matrixC_test[row * b_dim[0] + col] += 
						matrixA_host[row * a_dim[0] + i] 
						* 
						matrixB_host[i * b_dim[0] + col];
				}
			}
		}
	} else {
		/* C=AA^T */
		/* The pragma below makes the loop parallel with threads = number of cores */
#ifdef CF4OCL_USE_OPENMP		
		#pragma omp parallel for
#endif
		for (int row = 0; row < a_dim[1]; row++) {
			for (int col = 0; col < a_dim[1]; col++) {
				matrixC_test[row * a_dim[1] + col] = 0;
				for (int i = 0; i < a_dim[0]; i++) {
					matrixC_test[row * a_dim[1] + col] += 
						matrixA_host[row * a_dim[0] + i] 
						* 
						matrixA_host[col * a_dim[0] + i];
				}
			}
		}	
	}
	/* Get finishing time */
	cl4_prof_stop(prof_cpu);
	
	/* ******************************************************** */
	/* Determine and print OpenCL/OpenMP comparison information */
	/* ******************************************************** */

	/* Check for correctness */
	int error = 0;
	unsigned int sizeC = b_dim[0] * a_dim[1];
	for (unsigned int index = 0; index < sizeC; index++) {
		error += matrixC_host[index] - matrixC_test[index];
	}
	
	printf("\n   ============================== Results ==================================\n\n");
	printf("     Total CPU Time %s: %fs\n", 
#ifdef CF4OCL_USE_OPENMP	
		"(Parallel)   ",
#else
		"(Serial)     ",
#endif
		cl4_prof_time_elapsed(prof_cpu));
	printf("     SpeedUp (OpenCL vs. %s) : %fx\n",
#ifdef CF4OCL_USE_OPENMP	
		"OpenMP",
#else
		"1x CPU",
#endif
		cl4_prof_time_elapsed(prof_cpu) / cl4_prof_time_elapsed(prof_dev));
	printf("     Error (Device-CPU)          : %d\n", error);
	printf("\n");
	

	/* Show matrices messages if verbose == TRUE */
	if (verbose) {
		fprintf(stderr, "\n\"Matrix A\"\n");
		for (int i = 0; i < a_dim[1]; i++) {
			for (int j = 0; j < a_dim[0]; j++) {
				fprintf(stderr, "%d\t", matrixA_host[a_dim[0] * i + j]);
			}
			fprintf(stderr, "\n");
		}

		if (!IS_AAT(kernel_id)) {
			fprintf(stderr, "\n\"Matrix B\"\n");
			for (int i = 0; i < b_dim[1]; i++) {
				for (int j = 0; j < b_dim[0]; j++) {
					fprintf(stderr, "%d\t", matrixB_host[b_dim[0] * i + j]);
				}
				fprintf(stderr, "\n");
			}
		}

		fprintf(stderr, "\n\"Device matrix C\"\n");
		for (int row = 0; row < a_dim[1]; row++) {
			for (int col = 0; col < b_dim[0]; col++) {
				fprintf(stderr, "%d\t", matrixC_host[b_dim[0] * row + col]);
			}
			fprintf(stderr, "\n");
		}

		fprintf(stderr, "\n\"CPU matrix C\"\n");
		for (int row = 0; row < a_dim[1]; row++) {
			for (int col = 0; col < b_dim[0]; col++) {
				fprintf(stderr, "%d\t", matrixC_test[b_dim[0] * row + col]);
			}
			fprintf(stderr, "\n");
		}
	}
	
	/* If we get here, no need for error treatment, jump to cleanup. */
	g_assert(err == NULL);
	status = CLEXP_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err != NULL);
	fprintf(
		stderr, 
		"Error %d from domain '%s' with message: \"%s\"\n", 
		err->code, 
		g_quark_to_string(err->domain), 
		err->message);
	g_error_free(err);
	status = CLEXP_FAIL;

cleanup:	

	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	/* Free profile and cpu timer */
	if (prof_dev) cl4_prof_destroy(prof_dev);
	if (prof_cpu) cl4_prof_destroy(prof_cpu);
	
	/* Free string command line options. */
	if (compiler_opts) g_free(compiler_opts);
	//~ if (output_export) g_free(output_export);
		
	/* Free miscelaneous objects. */
	if (kernel_name) g_free(kernel_name);
	if (kernel_path) g_free(kernel_path);

	/* Free RNG */
	if (rng) g_rand_free(rng);
		
	/* Release wrappers. */
	if (matrixC_dev) cl4_buffer_destroy(matrixC_dev);
	if (matrixB_dev) cl4_buffer_destroy(matrixB_dev);
	if (matrixA_dev) cl4_buffer_destroy(matrixA_dev);
	if (prg) cl4_program_destroy(prg);
	if (cq) cl4_cqueue_destroy(cq);
	if (ctx) cl4_context_destroy(ctx);

	/* Free host resources */
	if (matrixA_host) matmult_matrix_free(matrixA_host);
	if (matrixB_host) matmult_matrix_free(matrixB_host);
	if (matrixC_host) matmult_matrix_free(matrixC_host);
	if (matrixC_test) matmult_matrix_free(matrixC_test);
	
	/* Return program status. */
	return status;

}

/**
 * @brief Create a new matrix with random values.
 * 
 * @param cols Number of columns in matrix.
 * @param rows Number of rows in matrix.
 * @param matrix_range Array containing the min. and max. matrix values;
 * if NULL, matrix is only allocated.
 * @param rng Random number generator; can be NULL if `matrix_range` is NULL.
 * @return The new matrix or NULL if memory allocation failed.
 * */
int* matmult_matrix_new(int cols, int rows, int* matrix_range, GRand* rng) {
	int *matrix = (int*) g_malloc(cols * rows * sizeof(int));
	if (matrix_range != NULL) {
		for (int i = 0; i < cols * rows; i++) {
			matrix[i] = g_rand_int_range(
				rng, matrix_range[0], matrix_range[1]);
		}
	}
	return matrix;
}

/**
 * @brief Free's a matrix created with matmult_matrix_new().
 * 
 * @param matrix The matrix to free.
 * */
void matmult_matrix_free(int* matrix) {
	g_free(matrix);
}

/**
 * @brief Parse and verify command line arguments.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @param err GLib error object for error reporting.
 * @return #CLEXP_SUCCESS if program returns with no error, or 
 * #CLEXP_FAIL otherwise.
 * */
int matmult_args_parse(int argc, char* argv[], GError** err) {
	
	/* Aux. var. */
	int status;

	/* Command line options context. */
	GOptionContext* context = NULL;

	/* Create parsing context. */
	context = g_option_context_new (" - " PROG_DESCRIPTION);
	gef_if_error_create_goto(*err, CL4_ERROR,  context == NULL, 
		CLEXP_FAIL, error_handler, 
		"Unable to create command line parsing context.");
	
	/* Add acceptable command line options to context. */ 
	g_option_context_add_main_entries(context, entries, NULL);
	
	/* Use context to parse command line options. */
	g_option_context_parse(context, &argc, &argv, err);
	gef_if_error_goto(*err, CLEXP_FAIL, status, error_handler);
	
	/* Make checks which depend if the multiplication is AB or AA^T 
	 * (transpose) */
	if (!IS_AAT(kernel_id)) {
		/* Check if number of rows in B is the same as the number of 
		 * columns in A. */
		gef_if_error_create_goto(*err, CL4_ERROR, 
			(b_dim[1] != a_dim[0]), CLEXP_FAIL, error_handler, 
			"Number of rows in B must the same as the number of columns in A.");
	} else {
		/* In this case (transpose multiplication), dimensions of B are 
		 * considered to be of dimensions of A^T. */
		b_dim[0] = a_dim[1];
		b_dim[1] = a_dim[0];
	}

	/* Check if kernel ID is within 0 to 4. */
	gef_if_error_create_goto(*err, CL4_ERROR, 
		((kernel_id < 0) || (kernel_id > 4)), CLEXP_FAIL, error_handler, 
		"Kernel selection must be 0, 1, 2 (for C=AB kernels), 3 or 4 \
		(for C=AA^T kernels).");

	/* If we get here, no need for error treatment, jump to cleanup. */
	g_assert (err == NULL || *err == NULL);
	status = CLEXP_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	status = CLEXP_FAIL;

cleanup:	

	/* Free context. */
	if (context) g_option_context_free(context);
	
	/* Return function status. */
	return status;

}
