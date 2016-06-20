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
 * Cannonical example of how to use _cf4ocl_.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

/*
 * Description
 * -----------
 *
 * Cannonical example of how to use _cf4ocl_. Performs an element-wise sum of
 * two vectors, also adding a constant.
 *
 * Optional command-line arguments:
 *
 * 1. Device index
 * 2. Buffer size
 *
 * */

#include <cf4ocl2.h>

/* Kernel source string, will be hardwired in this location during the build
 * process, before compilation. The kernel source is available in canon.cl. */
#define KERNEL_SRC \
@canon_KERNEL_SRC@

/* Kernel name. */
#define KERNEL_NAME "sum"

/* Default buffer size. Final size can be specified as a command-line option. */
#define DEF_BUF_N 16;

/* Error handling macros. */
#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(EXIT_FAILURE); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }

/**
 * Canonical example main function.
 * */
int main(int argc, char** argv) {

	/* Number of elements in buffer. */
	cl_uint buf_n = DEF_BUF_N;

	/* Device selected specified in the command line. */
	int dev_idx = -1;

	/* Check if a device was specified in the command line. */
	if (argc >= 2) {
		dev_idx = atoi(argv[1]);
	}

	/* Check if a new buffer size was specified in the command line. */
	if (argc >= 3) {
		buf_n = atoi(argv[2]);
	}

	/* Wrappers. */
	CCLContext* ctx = NULL;
	CCLProgram* prg = NULL;
	CCLDevice* dev = NULL;
	CCLQueue* queue = NULL;
	CCLBuffer* a_dev;
	CCLBuffer* b_dev;
	CCLBuffer* c_dev;
	CCLEvent* evt_write1;
	CCLEvent* evt_write2;
	CCLEvent* evt_exec;
	CCLEventWaitList ewl = NULL;

	/* Profiler. */
	CCLProf* prof;

	/* Global and local worksizes. */
	size_t gws;
	size_t lws;

	/* Host buffers. */
	cl_uint* a_host = NULL;
	cl_uint* b_host = NULL;
	cl_uint* c_host = NULL;
	cl_uint d_host;

	/* Error reporting object. */
	GError* err = NULL;

	/* Check results flag. */
	cl_bool check_result;

	/* Create a context with device selected from menu. */
	ctx = ccl_context_new_from_menu_full(&dev_idx, &err);
	HANDLE_ERROR(err);

	/* Get the selected device. */
	dev = ccl_context_get_device(ctx, 0, &err);
	HANDLE_ERROR(err);

	/* Create a new program from kernel source. */
	prg = ccl_program_new_from_source(ctx, KERNEL_SRC, &err);
	HANDLE_ERROR(err);

	/* Build program. */
	ccl_program_build(prg, NULL, &err);
	HANDLE_ERROR(err);

	/* Create a command queue. */
	queue = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	HANDLE_ERROR(err);

	/* Get local worksize. */
	lws = ccl_device_get_info_scalar(
		dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, &err);
	HANDLE_ERROR(err);
	lws = MIN(buf_n, lws);

	/* Get global worksize, make it a multiple of local worksize. */
	gws = lws * ((buf_n / lws) + ((buf_n % lws) > 0));
	printf("\n");
	printf(" * Global worksize: %d\n", (int) gws);
	printf(" * Local worksize : %d\n", (int) lws);

	/* Initialize host buffers. */
	a_host = (cl_uint*)malloc(sizeof(cl_uint) * buf_n);
	b_host = (cl_uint*)malloc(sizeof(cl_uint) * buf_n);
	c_host = (cl_uint*)malloc(sizeof(cl_uint) * buf_n);

	/* Fill host buffers. */
	for (cl_uint i = 0; i < buf_n; ++i) {
		a_host[i] = i;
		b_host[i] = buf_n - i;
	}
	d_host = buf_n / 4;

	/* Create device buffers. */
	a_dev = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
		buf_n * sizeof(cl_uint), NULL, &err);
	HANDLE_ERROR(err);
	b_dev = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
		buf_n * sizeof(cl_uint), NULL, &err);
	HANDLE_ERROR(err);
	c_dev = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
		buf_n * sizeof(cl_uint), NULL, &err);
	HANDLE_ERROR(err);

	/* Copy host data to device buffers without waiting for transfer
	 * to terminate before continuing host program. */
	evt_write1 = ccl_buffer_enqueue_write(a_dev, queue, CL_FALSE, 0,
		buf_n * sizeof(cl_uint), a_host, NULL, &err);
	HANDLE_ERROR(err);
	evt_write2 = ccl_buffer_enqueue_write(b_dev, queue, CL_FALSE, 0,
		buf_n * sizeof(cl_uint), b_host, NULL, &err);
	HANDLE_ERROR(err);

	/* Initialize event wait list and add the two transfer events. */
	ccl_event_wait_list_add(&ewl, evt_write1, evt_write2, NULL);

	/* Execute program kernel, waiting for the two transfer events
	 * to terminate (this will empty the event wait list). */
	evt_exec = ccl_program_enqueue_kernel(prg, KERNEL_NAME, queue, 1,
		NULL, &gws, &lws, &ewl, &err,
		/* Kernel arguments. */
		a_dev, b_dev, c_dev,
		ccl_arg_priv(d_host, cl_uint), ccl_arg_priv(buf_n, cl_uint),
		NULL);
	HANDLE_ERROR(err);

	/* Add the kernel termination event to the wait list. */
	ccl_event_wait_list_add(&ewl, evt_exec, NULL);

	/* Sync. queue for events in wait list (just the execute event in
	 * this case) to terminate before going forward... */
	ccl_enqueue_barrier(queue, &ewl, &err);
	HANDLE_ERROR(err);

	/* Read back results from host waiting for transfer to terminate
	 * before continuing host program. */
	ccl_buffer_enqueue_read(c_dev, queue, CL_TRUE, 0,
		buf_n * sizeof(cl_uint), c_host, NULL, &err);
	HANDLE_ERROR(err);

	/* Check results are as expected (not available with OpenCL stub). */
	check_result = CL_TRUE;
	for (cl_uint i = 0; i < buf_n; ++i) {
		if(c_host[i] != a_host[i] + b_host[i] + d_host) {
			check_result = CL_FALSE;
			break;
		}
	}
	if (check_result)
		fprintf(stdout,
			" * Kernel execution produced the expected results.\n");
	else
		fprintf(stderr,
			" * Kernel execution failed to produce the expected results.\n");

	/* Perform profiling. */
	prof = ccl_prof_new();
	ccl_prof_add_queue(prof, "queue1", queue);
	ccl_prof_calc(prof, &err);
	HANDLE_ERROR(err);
	/* Show profiling info. */
	ccl_prof_print_summary(prof);
	/* Export profiling info. */
	ccl_prof_export_info_file(prof, "out.tsv", &err);
	HANDLE_ERROR(err);

	/* Destroy profiler object. */
	ccl_prof_destroy(prof);

	/* Destroy host buffers. */
	free(a_host);
	free(b_host);
	free(c_host);

	/* Destroy wrappers. */
	ccl_buffer_destroy(a_dev);
	ccl_buffer_destroy(b_dev);
	ccl_buffer_destroy(c_dev);
	ccl_queue_destroy(queue);
	ccl_program_destroy(prg);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

	/* Bye. */
	return EXIT_SUCCESS;
}



