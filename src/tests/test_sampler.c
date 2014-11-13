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
 * Test the sampler wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

/**
 * Tests creation (using "simple" constructor), getting info from and
 * destruction of sampler wrapper objects.
 * */
static void create_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLSampler* s = NULL;
	GError* err = NULL;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create sampler using "simple" constructor. */
	s = ccl_sampler_new(ctx, CL_TRUE, CL_ADDRESS_NONE,
		CL_FILTER_NEAREST, &err);
	g_assert_no_error(err);

	/* Get some info and check if the return value is as expected. */
	cl_addressing_mode am;
	am = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_ADDRESSING_MODE, cl_addressing_mode, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(am, ==, CL_ADDRESS_NONE);

	cl_filter_mode fm;
	fm = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_FILTER_MODE, cl_filter_mode, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(fm, ==, CL_FILTER_NEAREST);

	cl_bool nc;
	nc = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_NORMALIZED_COORDS, cl_bool, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(nc, ==, CL_TRUE);

	cl_context context;
	context = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_CONTEXT, cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex((gulong) context, ==, (gulong) ccl_context_unwrap(ctx));

	/* Destroy sampler. */
	ccl_sampler_destroy(s);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests creation (using "full" constructor), getting info from and
 * destruction of sampler wrapper objects.
 * */
static void create_full_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLSampler* s = NULL;
	const cl_sampler_properties sampler_properties[] = {
		CL_SAMPLER_NORMALIZED_COORDS, CL_FALSE,
		CL_SAMPLER_ADDRESSING_MODE, CL_ADDRESS_CLAMP_TO_EDGE,
		CL_SAMPLER_FILTER_MODE, CL_FILTER_NEAREST,
		0};

	GError* err = NULL;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create sampler using "full" constructor. */
	s = ccl_sampler_new_full(ctx, sampler_properties, &err);
	g_assert_no_error(err);

	/* Get some info and check if the return value is as expected. */
	cl_addressing_mode am;
	am = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_ADDRESSING_MODE, cl_addressing_mode, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(am, ==, CL_ADDRESS_CLAMP_TO_EDGE);

	cl_filter_mode fm;
	fm = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_FILTER_MODE, cl_filter_mode, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(fm, ==, CL_FILTER_NEAREST);

	cl_bool nc;
	nc = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_NORMALIZED_COORDS, cl_bool, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(nc, ==, CL_FALSE);

	cl_context context;
	context = ccl_sampler_get_info_scalar(
		s, CL_SAMPLER_CONTEXT, cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex((gulong) context, ==, (gulong) ccl_context_unwrap(ctx));

	/* Destroy sampler. */
	ccl_sampler_destroy(s);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests sampler wrapper class reference counting.
 * */
static void ref_unref_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLSampler* s = NULL;
	GError* err = NULL;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create sampler. */
	s = ccl_sampler_new(ctx, CL_TRUE, CL_ADDRESS_CLAMP,
		CL_FILTER_NEAREST, &err);
	g_assert_no_error(err);

	/* Increase sampler reference count. */
	ccl_sampler_ref(s);

	/* Check that sampler ref count is 2. */
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) s));

	/* Unref sampler. */
	ccl_sampler_unref(s);

	/* Check that sampler ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) s));

	/* Destroy stuff. */
	ccl_sampler_unref(s);
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
		"/wrappers/sampler/create-info-destroy",
		create_info_destroy_test);

	g_test_add_func(
		"/wrappers/sampler/create-full-info-destroy",
		create_full_info_destroy_test);

	g_test_add_func(
		"/wrappers/sampler/ref-unref",
		ref_unref_test);

	return g_test_run();
}



