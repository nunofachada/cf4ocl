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
	cl_context context = NULL;
	CCLDevice* dev = NULL;
	cl_device_id device = NULL;
	CCLQueue* cq = NULL;
	GError* err = NULL;
	cl_command_queue_properties prop;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create command queue wrapper. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Get some info and check if the return value is as expected. */
	context = ccl_queue_get_info_scalar(
		cq, CL_QUEUE_CONTEXT, cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex(
		GPOINTER_TO_UINT(context),
		==,
		GPOINTER_TO_UINT(ccl_context_unwrap(ctx)));

	device = ccl_queue_get_info_scalar(
		cq, CL_QUEUE_DEVICE, cl_device_id, &err);
	g_assert_no_error(err);
	g_assert_cmphex(
		GPOINTER_TO_UINT(device),
		==,
		GPOINTER_TO_UINT(ccl_device_unwrap(dev)));

	prop = ccl_queue_get_info_scalar(
		cq, CL_QUEUE_PROPERTIES, cl_command_queue_properties, &err);
	g_assert_no_error(err);
	g_assert_cmphex(prop, ==, 0);

	/* Destroy stuff. */
	ccl_queue_destroy(cq);
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

	return g_test_run();
}



