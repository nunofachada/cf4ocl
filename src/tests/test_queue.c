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
 * Test the command queue wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

/**
 * Tests creation (using "simple" constructor), getting info from and
 * destruction of command queue wrapper objects.
 * */
static void create_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLContext* ctx_aux = NULL;
	cl_context context = NULL;
	CCLDevice* dev = NULL;
	CCLDevice* dev_aux = NULL;
	cl_device_id device = NULL;
	CCLQueue* cq = NULL;
	cl_command_queue command_queue = NULL;
	GError* err = NULL;
	cl_command_queue_properties prop;
	const cl_queue_properties prop_full[] =
		{ CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
	cl_int ocl_status;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Test three ways to create a queue. */
	for (cl_uint i = 0; i < 3; ++i) {

		/* Create command queue wrapper. */
		switch (i) {
			case 0:
				/* The regular way. */
				cq = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE,
					&err);
				g_assert_no_error(err);
				break;
			case 1:
				/* Using the "full" constructor. */
				cq = ccl_queue_new_full(ctx, dev, prop_full, &err);
				g_assert_no_error(err);
				break;
			case 2:
				/* Using the "wrap" constructor. */
				command_queue = clCreateCommandQueue(
					ccl_context_unwrap(ctx), ccl_device_unwrap(dev),
					CL_QUEUE_PROFILING_ENABLE, &ocl_status);
				g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
				cq = ccl_queue_new_wrap(command_queue);
				g_assert_cmphex(GPOINTER_TO_UINT(command_queue), ==,
					GPOINTER_TO_UINT(ccl_queue_unwrap(cq)));
				break;
		}

		/* Get some info and check if the return value is as expected. */
		context = ccl_queue_get_info_scalar(
			cq, CL_QUEUE_CONTEXT, cl_context, &err);
		g_assert_no_error(err);
		g_assert_cmphex(
			GPOINTER_TO_UINT(context),
			==,
			GPOINTER_TO_UINT(ccl_context_unwrap(ctx)));

		ctx_aux = ccl_queue_get_context(cq, &err);
		g_assert_no_error(err);
		g_assert_cmphex(
			GPOINTER_TO_UINT(ctx), ==, GPOINTER_TO_UINT(ctx_aux));

		device = ccl_queue_get_info_scalar(
			cq, CL_QUEUE_DEVICE, cl_device_id, &err);
		g_assert_no_error(err);
		g_assert_cmphex(
			GPOINTER_TO_UINT(device),
			==,
			GPOINTER_TO_UINT(ccl_device_unwrap(dev)));

		dev_aux = ccl_queue_get_device(cq, &err);
		g_assert_no_error(err);
		g_assert_cmphex(
			GPOINTER_TO_UINT(dev), ==, GPOINTER_TO_UINT(dev_aux));

		prop = ccl_queue_get_info_scalar(
			cq, CL_QUEUE_PROPERTIES, cl_command_queue_properties, &err);
		g_assert_no_error(err);
		g_assert_cmphex(prop, ==, CL_QUEUE_PROFILING_ENABLE);

		/* Destroy queue. */
		ccl_queue_destroy(cq);
		cq = NULL;
	}

	/* Destroy context. */
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests the command queue wrapper class reference counting.
 * */
static void ref_unref_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLQueue* cq = NULL;
	CCLDevice* dev = NULL;
	GError* err = NULL;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create command queue wrapper. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Increase queue reference count. */
	ccl_queue_ref(cq);

	/* Check that queue ref count is 2. */
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) cq));

	/* Unref queue. */
	ccl_queue_unref(cq);

	/* Check that queue ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) cq));

	/* Destroy stuff. */
	ccl_queue_unref(cq);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Tests the ::ccl_enqueue_barrier() and ::ccl_enqueue_marker()
 * functions.
 * */
static void barrier_marker_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLQueue* cq = NULL;
	CCLBuffer* buf = NULL;
	CCLEvent* evt_cq = NULL;
	CCLEvent* evt_map = NULL;
	CCLEvent* evt_unmap = NULL;
	CCLEvent* evt_barrier = NULL;
	CCLEvent* evt_marker = NULL;
	GError* err = NULL;
	cl_ulong* host_buf;
	CCLEventWaitList ewl = NULL;
	cl_int exec_status = -1;
	cl_command_type ct = 0;
	cl_uint ocl_ver;
	cl_bool evt_in_cq;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Get OpenCL version for context. */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Create a device buffer. */
	buf = ccl_buffer_new(
		ctx, CL_MEM_READ_WRITE, 8 * sizeof(cl_ulong), NULL, &err);
	g_assert_no_error(err);

	/* Map device buffer, get an event and analise it. */
	host_buf = ccl_buffer_enqueue_map(buf, cq, CL_FALSE, CL_MAP_WRITE,
		0, 8 * sizeof(cl_ulong), NULL, &evt_map, &err);
	g_assert_no_error(err);

	/* Enqueue a barrier for waiting on the map event. */
	evt_barrier =
		ccl_enqueue_barrier(cq, ccl_ewl(&ewl, evt_map, NULL), &err);
	g_assert_no_error(err);

	/* Check that the barrier event is CL_COMMAND_BARRIER (OpenCL >=
	 * 1.2) or CL_COMMAND_MARKER (OpenCL <= 1.1). */
	ct = ccl_event_get_command_type(evt_barrier, &err);
	g_assert_no_error(err);
#ifdef CL_VERSION_1_2
	if (ocl_ver >= 120)
		g_assert_cmpuint(ct, ==, CL_COMMAND_BARRIER);
	else
#endif
		g_assert_cmpuint(ct, ==, CL_COMMAND_MARKER);

	/* Unmap buffer, get resulting event. */
	evt_unmap = ccl_buffer_enqueue_unmap(buf, cq, host_buf, NULL, &err);
	g_assert_no_error(err);

#ifdef CL_VERSION_1_2
	if (ocl_ver >= 120) {
		/* For OpenCL >= 1.2 the marker event is complete when either
		 * a) all previous enqueued events have completed, or b) the
		 * events specified in the event wait list have completed. */
		ccl_event_wait_list_add(
			&ewl, evt_map, evt_barrier, evt_unmap, NULL);
		evt_marker = ccl_enqueue_marker(cq, &ewl, &err);
		g_assert_no_error(err);
	} else {
#endif
		/* For OpenCL <= 1.1, the marker event is complete ONLY when all
		 * previous enqueued events have completed. */
		evt_marker = ccl_enqueue_marker(cq, NULL, &err);
		g_assert_no_error(err);
#ifdef CL_VERSION_1_2
	}
#endif

	/* Wait for marker event to complete (which means all previous
	 * enqueued events have also completed). */
	ccl_event_wait(ccl_ewl(&ewl, evt_marker, NULL), &err);

	/* Test queue flush. */
	ccl_queue_flush(cq, &err);
	g_assert_no_error(err);

	/* Check that the unmap event is CL_COMPLETE. */
	exec_status = ccl_event_get_info_scalar(
		evt_unmap, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_COMPLETE);

	/* Check that the event is a marker event. */
	ct = ccl_event_get_command_type(evt_marker, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(ct, ==, CL_COMMAND_MARKER);

	/* Check that generated events are in queue. */
	CCLEvent* evts[] =
		{evt_map, evt_barrier, evt_unmap, evt_marker, NULL};
	for (cl_uint i = 0; evts[i] != NULL; ++i) {
		ccl_queue_iter_event_init(cq);
		evt_in_cq = CL_FALSE;
		while ((evt_cq = ccl_queue_iter_event_next(cq)) != NULL) {
			if (evt_cq == evt_map) {
				evt_in_cq = CL_TRUE;
				break;
			}
		}
		g_assert(evt_in_cq);
	}

	/* Clear events in queue and check that they were really destroyed. */
	ccl_queue_gc(cq);
	ccl_queue_iter_event_init(cq);
	g_assert(ccl_queue_iter_event_next(cq) == NULL);

	/* Release wrappers. */
	ccl_buffer_destroy(buf);
	ccl_queue_destroy(cq);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add_func(
		"/wrappers/queue/create-info-destroy",
		create_info_destroy_test);

	g_test_add_func(
		"/wrappers/queue/ref-unref",
		ref_unref_test);

	g_test_add_func(
		"/wrappers/queue/barrier-marker",
		barrier_marker_test);

	return g_test_run();
}



