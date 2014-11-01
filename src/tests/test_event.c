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
 * Test the event wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

/**
 * Tests user events.
 * */
static void user_event_test() {

	/* Test variables. */
	CCLEvent* uevt = NULL;
	CCLContext* ctx = NULL;
	GError* err = NULL;
	cl_command_queue clcq = NULL;
	cl_context clctx = NULL;
	cl_command_type clct = 0;
	cl_int exec_status;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create user event. */
	uevt = ccl_user_event_new(ctx, &err);
	g_assert_no_error(err);

	/* Check that event ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Increase ref count, check it is 2. */
	ccl_event_ref(uevt);
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Decrease ref count, check it is 1. */
	ccl_event_unref(uevt);
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Confirm that event command queue is NULL. */
	clcq = ccl_event_get_info_scalar(uevt, CL_EVENT_COMMAND_QUEUE,
		cl_command_queue, &err);
	g_assert_no_error(err);
	g_assert_cmphex(((cl_ulong) clcq), ==, ((cl_ulong) NULL));

	/* Confirm that event returns the correct context. */
	clctx = ccl_event_get_info_scalar(uevt, CL_EVENT_CONTEXT,
		cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex(((cl_ulong) clctx), ==, ((cl_ulong) ccl_context_unwrap(ctx)));

	/* Confirm command type is user event. */
	clct = ccl_event_get_info_scalar(uevt, CL_EVENT_COMMAND_TYPE,
		cl_command_type, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(clct, ==, CL_COMMAND_USER);

	/* Confirm execution status is "submitted". */
	exec_status = ccl_event_get_info_scalar(uevt,
		CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_SUBMITTED);

	/* Change execution status to "complete". */
	ccl_user_event_set_status(uevt, CL_COMPLETE, &err);
	g_assert_no_error(err);

	/* Confirm execution status is "complete". */
	exec_status = ccl_event_get_info_scalar(uevt,
		CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_COMPLETE);

	/* Destroy stuff. */
	ccl_event_destroy(uevt);
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
		"/wrappers/event/user",
		user_event_test);

	return g_test_run();
}




