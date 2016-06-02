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
 * Test the profiling module.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

/**
 * Tests creation, getting info from and destruction of
 * profiler objects, and their relationship with context, device and
 * queue wrapper objects.
 * */
static void create_add_destroy_test() {

	/* Test variables. */
	GError* err = NULL;
	CCLBuffer* buf1 = NULL;
	CCLBuffer* buf2 = NULL;
	CCLProf* prof = NULL;
	CCLContext* ctx = NULL;
	CCLDevice* d = NULL;
	CCLQueue* cq1 = NULL;
	CCLQueue* cq2 = NULL;
	CCLEvent* evt = NULL;
	CCLEventWaitList ewl = NULL;
	size_t buf_size = 8 * sizeof(cl_short);
	cl_short hbuf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	cl_ulong duration, eff_duration;
	double time_ellapsed;

	/* Create a new profile object. */
	prof = ccl_prof_new();

	/* Get a context and a device. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create two command queue wrappers. */
	cq1 = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);

	cq2 = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);

	/* Create device buffers. */
	buf1 = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, buf_size, NULL, &err);
	g_assert_no_error(err);
	buf2 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
	g_assert_no_error(err);

	/* Start profile object timer. */
	ccl_prof_start(prof);

	/* Transfer data to buffer. */
	evt = ccl_buffer_enqueue_write(
		buf1, cq1, CL_FALSE, 0, buf_size, hbuf, NULL, &err);
	g_assert_no_error(err);

	/* Transfer data from one buffer to another. */
	evt = ccl_buffer_enqueue_copy(buf1, buf2, cq2, 0, 0, buf_size,
		ccl_ewl(&ewl, evt, NULL), &err);
	g_assert_no_error(err);

	/* Wait for copy. */
	ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
	g_assert_no_error(err);

	/* Stop profile object timer. */
	ccl_prof_stop(prof);

	/* Add both queues to profile object. */
	ccl_prof_add_queue(prof, "A Queue", cq1);
	ccl_prof_add_queue(prof, "Another Queue", cq2);

	/* Process queues. */
	ccl_prof_calc(prof, &err);
	g_assert_no_error(err);

	/* Request some profiling information. */
	time_ellapsed = ccl_prof_time_elapsed(prof);
	duration = ccl_prof_get_duration(prof);
	eff_duration = ccl_prof_get_eff_duration(prof);

	g_debug("Profilling time ellapsed: %lf", time_ellapsed);
	g_debug("Profilling duration: %d", (cl_int) duration);
	g_debug("Profilling eff. duration: %d", (cl_int) eff_duration);

	/* Destroy buffers. */
	ccl_buffer_destroy(buf1);
	ccl_buffer_destroy(buf2);

	/* Unref cq1, which should not be destroyed because it is held
	 * by the profile object. */
	ccl_queue_destroy(cq1);

	/* Destroy the profile object, which will also destroy cq1. cq2
	 * will me merely unrefed and must still be explicitly destroyed. */
	ccl_prof_destroy(prof);

	/* Destroy cq2. */
	ccl_queue_destroy(cq2);

	/* Destroy the context. */
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
		"/profiler/create-add-destroy", create_add_destroy_test);

	return g_test_run();

}

