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

/** Default number of columns in Matrix A. */
#define A_COLS 128
/** Default number of rows in Matrix A. */
#define A_ROWS 256
/** Default number of columns in Matrix B. */
#define B_COLS 16
/** Number of rows in matrix B must be equal to number of columns in matrix A. */
#define B_ROWS A_COLS

/** Default workgroup size, dimension 0. */
#define LWS_X 32
/** Default workgroup size, dimension 1. */
#define LWS_Y 16

/** Default minimum range value (from). */
#define RANGE_MATRIX_FROM -100

/** Default maximum range value (to). */
#define RANGE_MATRIX_TO 100

/** Default kernel selection: 0, 1 or 2 (higher values correspond to more optimized kernels). */
#define KERNEL_ID 0 

/** Default verbose status. */
#define VERBOSE FALSE

/** Default seed. */
#define SEED 0

/** A description of the program. */
#define PROG_DESCRIPTION "Program for testing matrix multiplication on a OpenCL device (GPU or CPU, although optimized for the former) and compare with OpenMP implementation on the CPU."

/* Command line arguments and respective default values. */
static int a_dim[] = {A_COLS, A_ROWS};
static int b_dim[] = {B_COLS, B_ROWS};
static size_t lws[] = {LWS_X, LWS_Y};
static int matrix_range[] = {RANGE_MATRIX_FROM, RANGE_MATRIX_TO};
static gchar* compiler_opts = NULL;
static int dev_idx = -1;
static gchar* dev_name = NULL;
static gchar* dev_vendor = NULL;
static gchar* platf_name = NULL;
static int kernel_id = KERNEL_ID;
static gboolean verbose = VERBOSE;
static guint32 seed = SEED;

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
	{"kernel",    'k', 0, G_OPTION_ARG_INT,      &kernel_id,     "kernel selection: 0-2 (C=AB), 3-4 (C=AA^T) (default is " STR(KERNEL_ID) ")",             "ID"},
	{"asize",     'a', 0, G_OPTION_ARG_CALLBACK, mm_parse_a,     "Size (cols,rows) of matrix A (default is " STR(A_COLS) "," STR(A_ROWS) ")",              "SIZE,SIZE"},
	{"bsize",     'b', 0, G_OPTION_ARG_CALLBACK, mm_parse_b,     "Size (cols,rows) of matrix B (default is " STR(B_COLS) "," STR(B_ROWS) ")",              "SIZE,SIZE"},
	{"localsize", 'l', 0, G_OPTION_ARG_CALLBACK, mm_parse_lws,   "Local work size (default is " STR(LWS_X) "," STR(LWS_Y) ")",                             "SIZE,SIZE"},
	{"range",     'r', 0, G_OPTION_ARG_CALLBACK, mm_parse_rge,   "Matrix range of values (default is " STR(RANGE_MATRIX_FROM) "," STR(RANGE_MATRIX_TO)")", "MIN,MAX"},
	{"seed",      's', 0, G_OPTION_ARG_INT,      &seed,          "RNG seed (default is " STR(SEED)")",                                                     "SEED"},
	{"verbose",   'v', 0, G_OPTION_ARG_NONE,     &verbose,       "Print input and output matrices to stderr",                                              NULL},
	{"device",    'd', 0, G_OPTION_ARG_INT,      &dev_idx,       "Device index, auto-selects device from menu (takes priority on -n and -p options)",      "INDEX"},
	{"dname",     'n', 0, G_OPTION_ARG_STRING,   &dev_name,      "Device name, selects device by name",                                                    "NAME"},
	{"dvendor",   'e', 0, G_OPTION_ARG_STRING,   &dev_vendor,    "Device vendor, selects device by vendor",                                                "VENDOR"},
	{"dplatf",    'p', 0, G_OPTION_ARG_STRING,   &platf_name,    "Platform name, selects device by platform name",                                         "NAME"},
	{"compiler",  'c', 0, G_OPTION_ARG_STRING,   &compiler_opts, "Extra OpenCL compiler options",                                                          "OPTS"},
	{ NULL, 0, 0, 0, NULL, NULL, NULL }	
};

/* Kernel file. */
static const char* kernelFiles[] = {"matmult.cl"};

/** 
 * @brief OpenCL and OpenMP matrix multiplication main function. 
 * 
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return #CLEXP_SUCCESS if program returns with no error, or 
 * #CLEXP_FAIL otherwise.
 * */
int main(int argc, char *argv[])
{

	/* ************* */
	/* Program vars. */
	/* ************* */

	int status;                                    /* Function and program return status. */
	CLUDeviceInfo deviceInfo;                      /* Select device by returned information strings. */
	GError *err = NULL;                            /* Error management */
	GRand* rng = NULL;	                           /* Random number generator. */
	ProfCLProfile *profile_dev = NULL,             /* Profiler for OpenCL device implementation */
		*profile_cpu = NULL;                       /* Profiler for OpenMP CPU implementation */
	CLUZone* zone = NULL;                          /* OpenCL zone (context, platform, program, queues, etc.) */
	cl_event events[4] = {NULL, NULL, NULL, NULL}; /* OpenCL events */
	cl_kernel kernel_matmult = NULL;               /* Kernel object */
	gchar* kernelName = NULL;                      /* Kernel name */
	cl_int *matrixA_host = NULL,                   /* Host matrix A */
		*matrixB_host = NULL,                      /* Host matrix B */
		*matrixC_host = NULL;                      /* Host matrix C (calcutated on OpenCL device). */
	int *matrixC_test = NULL;                      /* Host matrix C (calculated on the CPU). */
	cl_mem matrixA_device = NULL,                  /* Device matrix A. */
		matrixB_device = NULL,                     /* Device matrix B. */
		matrixC_device = NULL;                     /* Device matrix C. */
	size_t gws[2];                                 /* Global work sizes */
	size_t sizeMatrixAInBytes = 0,                 /* Size of matrix A in bytes. */
		sizeMatrixBInBytes = 0,                    /* Size of matrix B in bytes. */
		sizeMatrixCInBytes = 0;                    /* Size of matrix C in bytes. */
	size_t globalMemSizeInBytes,                   /* Size of device global memory required. */
		localMemSizeAInBytes,                      /* Size of local memory required by matrix A (depends on kernel id). */
		localMemSizeBInBytes;                      /* Size of local memory required by matrix B (depends on kernel id). */
	
	/* ************************** */
	/* Parse command line options */
	/* ************************** */

	matmult_args_parse(argc, argv, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* ******************************************************* */
	/* Initialize profiler, OpenCL variables and build program */
	/* ******************************************************* */
	
	/* Initialize RNG. */
	rng = g_rand_new_with_seed(seed);
		
	/* Profiling / Timmings */
	profile_dev = profcl_profile_new();
	gef_if_error_create_goto(err, CLEXP_ERROR, profile_dev == NULL, CLEXP_FAIL, error_handler, "Unable to create device profiler object.");
	profile_cpu = profcl_profile_new();
	gef_if_error_create_goto(err, CLEXP_ERROR, profile_cpu == NULL, CLEXP_FAIL, error_handler, "Unable to create CPU profiler object.");

	/* Get the required CL zone. */
	if ((dev_idx != -1) || ((dev_name == NULL) && (platf_name ==NULL))) {
		/* Select device by index or user choice. */
		zone = clu_zone_new(CL_DEVICE_TYPE_ALL, 1, CL_QUEUE_PROFILING_ENABLE, clu_menu_device_selector, (dev_idx != -1 ? &dev_idx : NULL), &err);
	} else {
		/* Select device by device name and/or platform name. */
		if (dev_name != NULL) 
			g_strlcpy(deviceInfo.device_name, dev_name, CLU_MAX_AUX_BUFF);
		else 
			deviceInfo.device_name[0] = '\0';
		if (dev_vendor != NULL) 
			g_strlcpy(deviceInfo.device_vendor, dev_vendor, CLU_MAX_AUX_BUFF);
		else 
			deviceInfo.device_vendor[0] = '\0';
		if (platf_name != NULL) 
			g_strlcpy(deviceInfo.platform_name, platf_name, CLU_MAX_AUX_BUFF);
		else 
			deviceInfo.platform_name[0] = '\0';
		zone = clu_zone_new(CL_DEVICE_TYPE_ALL, 1, CL_QUEUE_PROFILING_ENABLE, clu_info_device_selector, &deviceInfo, &err);
	}
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	printf("\n   == Using device '%s' from '%s' (platform is '%s')\n", zone->device_info.device_name, zone->device_info.device_vendor, zone->device_info.platform_name);
	
	/* Build program. */
	status = clu_program_create(zone, kernelFiles, 1, compiler_opts, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	
	/* Kernel */
	kernelName = g_strdup_printf("matmult%d", kernel_id);
	kernel_matmult = clCreateKernel(zone->program, kernelName, &status);
	gef_if_error_create_goto(err, CLEXP_ERROR, CL_SUCCESS != status, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to create '%s' kernel.", status, kernelName);

	/* ********************************** */	
	/* Create and initialize host buffers */
	/* ********************************** */	
	
	/* Matrix A */
	sizeMatrixAInBytes = a_dim[0] * a_dim[1] * sizeof(cl_int);
	matrixA_host = matmult_matrix_new(a_dim[0], a_dim[1], matrix_range, rng);
	gef_if_error_create_goto(err, CLEXP_ERROR, matrixA_host == NULL, CLEXP_FAIL, error_handler, "Unable to create host matrix A.");
	
	/* Matrix B */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		sizeMatrixBInBytes = b_dim[0] * b_dim[1] * sizeof(cl_int);
		matrixB_host = matmult_matrix_new(b_dim[0], b_dim[1], matrix_range, rng);
		gef_if_error_create_goto(err, CLEXP_ERROR, matrixB_host == NULL, CLEXP_FAIL, error_handler, "Unable to create host matrix B.");
	}
	
	/* Matrix C (result) */
	sizeMatrixCInBytes = b_dim[0] * a_dim[1] * sizeof(cl_int);
	matrixC_host = matmult_matrix_new(b_dim[0], a_dim[1], NULL, NULL);
	gef_if_error_create_goto(err, CLEXP_ERROR, matrixC_host == NULL, CLEXP_FAIL, error_handler, "Unable to create host matrix C.");

	/* ********************* */
	/* Create device buffers */
	/* ********************* */

	/* Matrix A */
	matrixA_device = clCreateBuffer(zone->context, CL_MEM_READ_ONLY, sizeMatrixAInBytes, NULL, &status );
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to create device buffer for matrix A.", status);
	
	/* Matrix B */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		matrixB_device = clCreateBuffer(zone->context, CL_MEM_READ_ONLY, sizeMatrixBInBytes, NULL, &status );
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to create device buffer for matrix B.", status);
	}

	/* Matrix C */
	matrixC_device = clCreateBuffer(zone->context, CL_MEM_WRITE_ONLY, sizeMatrixCInBytes, NULL, &status );
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to create device buffer for matrix C.", status);
	
	/* ************************* */
	/* Initialize device buffers */
	/* ************************* */
	
	/* Start basic timming / profiling. */
	profcl_profile_start(profile_dev);
	
	/* Copy matrix A to device. */
	status = clEnqueueWriteBuffer (	
		zone->queues[0], 
		matrixA_device, 
		CL_TRUE, 
		0, 
		sizeMatrixAInBytes, 
		matrixA_host, 
		0, 
		NULL, 
		&(events[0]) 
	);
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to write matrix A on device.", status);

	/* Copy matrix B to device. */
	if (!IS_AAT(kernel_id)) {
		/* Only required if we're not multiplying the transpose. */
		status = clEnqueueWriteBuffer (	
			zone->queues[0], 
			matrixB_device, 
			CL_TRUE, 
			0, 
			sizeMatrixBInBytes, 
			matrixB_host, 
			0, 
			NULL, 
			&(events[1]) 
		);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to write matrix B on device.", status);
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
	globalMemSizeInBytes = sizeMatrixAInBytes + sizeMatrixBInBytes + sizeMatrixCInBytes;
	
	/* Local memory requirements. Default is 0 for non-optimized kernels 0 and 3. */
	localMemSizeAInBytes = 0;
	localMemSizeBInBytes = 0;
	if (kernel_id >= 1)
		/* Optimized matrix mult. 1*/
		localMemSizeAInBytes = a_dim[0] * lws[1] * sizeof(cl_int);
	if (kernel_id == 2)
		/* Optimized matrix mult. 2*/
		localMemSizeBInBytes = lws[0] * b_dim[1] * sizeof(cl_int);
	if (kernel_id == 4) {
		/* Optimized matrix transpose mult. */
		localMemSizeAInBytes = lws[1] * a_dim[0] * sizeof(cl_int);
		localMemSizeBInBytes = lws[0] * a_dim[0] * sizeof(cl_int);
	}
	
	/* ****************************** */
	/* Print requirements information */
	/* ****************************** */

	clexp_reqs_print(gws, lws, globalMemSizeInBytes, localMemSizeAInBytes + localMemSizeBInBytes);

	/* *************************** */
	/*  Set fixed kernel arguments */
	/* *************************** */

	/* Generic arguments */
	status = clSetKernelArg(kernel_matmult, 0, sizeof(cl_mem), (void *) &matrixA_device);
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 0 of '%s' kernel.", status, kernelName);

	if (!IS_AAT(kernel_id)) {
		/* Arguments only for C=AB */
		status = clSetKernelArg(kernel_matmult, 1, sizeof(cl_mem), (void *) &matrixB_device);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 1 of '%s' kernel.", status, kernelName);

		status = clSetKernelArg(kernel_matmult, 2, sizeof(cl_mem), (void *) &matrixC_device);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 2 of '%s' kernel.", status, kernelName);
		
		status = clSetKernelArg(kernel_matmult, 3, sizeof(cl_int2), (void *) &a_dim);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 3 of '%s' kernel.", status, kernelName);
		
		status = clSetKernelArg(kernel_matmult, 4, sizeof(cl_int2), (void *) &b_dim);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 4 of '%s' kernel.", status, kernelName);
		
		if (kernel_id >= 1) {
			status = clSetKernelArg(kernel_matmult, 5, localMemSizeAInBytes, NULL);
			gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 5 of '%s' kernel.", status, kernelName);
		}
		if (kernel_id == 2) {
			status = clSetKernelArg(kernel_matmult, 6, localMemSizeBInBytes, NULL);
			gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 6 of '%s' kernel.", status, kernelName);
		}
	} else {
		/* Arguments only for C=AA^T */
		status = clSetKernelArg(kernel_matmult, 1, sizeof(cl_mem), (void *) &matrixC_device);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 1 of '%s' kernel.", status, kernelName);
		
		status = clSetKernelArg(kernel_matmult, 2, sizeof(cl_int2), (void *) &a_dim);
		gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 2 of '%s' kernel.", status, kernelName);
		
		if (kernel_id == 4) {
			status = clSetKernelArg(kernel_matmult, 3, localMemSizeAInBytes, NULL);
			gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 3 of '%s' kernel.", status, kernelName);
			status = clSetKernelArg(kernel_matmult, 4, localMemSizeBInBytes, NULL);
			gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable set arg 4 of '%s' kernel.", status, kernelName);
		}

	}
	
	/* ************ */
	/*  Run kernel! */
	/* ************ */
	
	status = clEnqueueNDRangeKernel(
		zone->queues[0], 
		kernel_matmult, 
		2, 
		NULL, 
		gws, 
		lws, 
		0, 
		NULL, 
		&(events[2]) 
	);
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d while executing kernel '%s'.", status, kernelName);

	/* *********************** */
	/*  Get result from device */
	/* *********************** */

	status = clEnqueueReadBuffer(
		zone->queues[0], 
		matrixC_device, 
		CL_TRUE, 
		0, 
		sizeMatrixCInBytes, 
		matrixC_host, 
		0, 
		NULL, 
		&(events[3]) 
	);
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d: unable to read matrix C from device.", status);

	/* Finish execution. */
	status = clFinish(zone->queues[0]); 
	gef_if_error_create_goto(err, CLEXP_ERROR, status != CL_SUCCESS, CLEXP_FAIL, error_handler, "OpenCL error %d on clFinish() function.", status);

	/* ************************************** */
	/*  Manage profiling of OpenCL operations */
	/* ************************************** */
	
	/* Stop profiler. */
	profcl_profile_stop(profile_dev); 

	profcl_profile_add(profile_dev, "Transfer matrix A to device", events[0], &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	if (!IS_AAT(kernel_id)) {
		profcl_profile_add(profile_dev, "Transfer matrix B to device", events[1], &err);
		gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
	}

	profcl_profile_add(profile_dev, "Kernel execution (Matmult)", events[2], &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	profcl_profile_add(profile_dev, "Transfer matrix C to host", events[3], &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	profcl_profile_aggregate(profile_dev, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);

	/* Show profiling info. */
	profcl_print_info(profile_dev, PROFCL_AGGEVDATA_SORT_TIME, &err);
	gef_if_error_goto(err, CLEXP_FAIL, status, error_handler);
		
	/* ********************************************************* */
	/* Perform multiplication on the CPU with the help of OpenMP */
	/* ********************************************************* */

	/* Allocate memory for a new results matrix. */
	matrixC_test = matmult_matrix_new(b_dim[0], a_dim[1], NULL, NULL);
	gef_if_error_create_goto(err, CLEXP_ERROR, matrixC_test == NULL, CLEXP_FAIL, error_handler, "Unable to create host matrix C (OpenMP test).");

	/* Start basic timming / profiling. */
	profcl_profile_start(profile_cpu);

	if (!IS_AAT(kernel_id)) {
		/* C=AB */
		/* The pragma below makes the loop parallel with threads = number of cores */
		#pragma omp parallel for
		for (int col = 0; col < b_dim[0]; col++) {
			for (int row = 0; row < a_dim[1]; row++) {
				matrixC_test[row * b_dim[0] + col] = 0;
				for (int i = 0; i < a_dim[0]; i++) {
					matrixC_test[row * b_dim[0] + col] += matrixA_host[row * a_dim[0] + i] * matrixB_host[i * b_dim[0] + col];
				}
			}
		}
	} else {
		/* C=AA^T */
		/* The pragma below makes the loop parallel with threads = number of cores */
		#pragma omp parallel for
		for (int row = 0; row < a_dim[1]; row++) {
			for (int col = 0; col < a_dim[1]; col++) {
				matrixC_test[row * a_dim[1] + col] = 0;
				for (int i = 0; i < a_dim[0]; i++) {
					matrixC_test[row * a_dim[1] + col] += matrixA_host[row * a_dim[0] + i] * matrixA_host[col * a_dim[0] + i];
				}
			}
		}	
	}
	/* Get finishing time */
	profcl_profile_stop(profile_cpu);
	
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
	printf("     Total CPU Time (OpenMP)     : %fs\n", profcl_time_elapsed(profile_cpu));
	printf("     SpeedUp (OpenCL vs. OpenMP) : %fx\n", profcl_time_elapsed(profile_cpu) / profcl_time_elapsed(profile_dev));
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
	fprintf(stderr, "Error %d from domain '%s' with message: \"%s\"\n", err->code, g_quark_to_string(err->domain), err->message);
	g_error_free(err);
	status = CLEXP_FAIL;

cleanup:	

	/* *********** */
	/* Free stuff! */
	/* *********** */
	
	/* Free profile and cpu timer */
	if (profile_dev) profcl_profile_free(profile_dev);
	if (profile_cpu) profcl_profile_free(profile_cpu);
	
	/* Free string command line options. */
	if (compiler_opts) g_free(compiler_opts);
	if (dev_name) g_free(dev_name);
	if (dev_vendor) g_free(dev_vendor);
	if (platf_name) g_free(platf_name);
		
	/* Free miscelaneous objects. */
	if (kernelName) g_free(kernelName);

	/* Free RNG */
	if (rng) g_rand_free(rng);
		
	/* Release events */
	for (unsigned int i = 0; i < 4; i++) {
		if (events[i]) status = clReleaseEvent(events[i]);
	}

	/* Release OpenCL kernels */
	if (kernel_matmult) clReleaseKernel(kernel_matmult);
	
	/* Release OpenCL memory objects */
	if (matrixA_device) clReleaseMemObject(matrixA_device);
	if (matrixB_device) clReleaseMemObject(matrixB_device);
	if (matrixC_device) clReleaseMemObject(matrixC_device);

	/* Free OpenCL zone */
	if (zone) clu_zone_free(zone);

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
 * @param matrix_range Array containing the min. and max. matrix values; if NULL, matrix is only allocated.
 * @param rng Random number generator; can be NULL if `matrix_range` is NULL.
 * @return The new matrix or NULL if memory allocation failed.
 * */
int* matmult_matrix_new(int cols, int rows, int* matrix_range, GRand* rng) {
	int *matrix = (int*) malloc(cols * rows * sizeof(int));
	if ((matrix != NULL) && (matrix_range != NULL)) {
		g_assert(rng != NULL);
		for (int i = 0; i < cols * rows; i++) {
			matrix[i] = g_rand_int_range(rng, matrix_range[0], matrix_range[1]);
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
	g_assert(matrix != NULL);
	free(matrix);
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
	gef_if_error_create_goto(*err, CLEXP_ERROR, context == NULL, CLEXP_FAIL, error_handler, "Unable to create command line parsing context.");
	
	/* Add acceptable command line options to context. */ 
	g_option_context_add_main_entries(context, entries, NULL);
	
	/* Use context to parse command line options. */
	g_option_context_parse(context, &argc, &argv, err);
	gef_if_error_goto(*err, CLEXP_FAIL, status, error_handler);
	
	/* Make checks which depend if the multiplication is AB or AA^T (transpose) */
	if (!IS_AAT(kernel_id)) {
		/* Check if number of rows in B is the same as the number of columns in A. */
		gef_if_error_create_goto(*err, CLEXP_ERROR, (b_dim[1] != a_dim[0]), CLEXP_FAIL, error_handler, "Number of rows in B must the same as the number of columns in A.");	
	} else {
		/* In this case (transpose multiplication), dimensions of B are considered to be of dimensions of A^T. */
		b_dim[0] = a_dim[1];
		b_dim[1] = a_dim[0];
	}

	/* Check if kernel ID is within 0 to 4. */
	gef_if_error_create_goto(*err, CLEXP_ERROR, ((kernel_id < 0) || (kernel_id > 4)), CLEXP_FAIL, error_handler, "Kernel selection must be 0, 1, 2 (for C=AB kernels), 3 or 4 (for C=AA^T kernels).");	

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
