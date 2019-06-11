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
 * @internal
 *
 * @file
 * Test the sampler wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"
#include "_ccl_defs.h"

/**
 * @internal
 *
 * @brief Tests creation (using "simple" constructor), getting info from and
 * destruction of sampler wrapper objects.
 * */
static void create_info_destroy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLSampler * s = NULL;
    cl_sampler sampler = NULL;
    CCLErr * err = NULL;
    cl_int ocl_status;
    const cl_sampler_properties sampler_properties[] = {
        CL_SAMPLER_NORMALIZED_COORDS, CL_TRUE,
        CL_SAMPLER_ADDRESSING_MODE, CL_ADDRESS_NONE,
        CL_SAMPLER_FILTER_MODE, CL_FILTER_NEAREST,
        0};

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(&err);
    g_assert_no_error(err);

    /* Test three ways to create a sampler. */
    for (cl_uint i = 0; i < 3; ++i) {

        /* Create sampler wrapper. */
        switch (i) {
            case 0:
                /* Create sampler using "simple" constructor. */
                s = ccl_sampler_new(ctx, CL_TRUE, CL_ADDRESS_NONE,
                    CL_FILTER_NEAREST, &err);
                g_assert_no_error(err);
                break;
            case 1:
                /* Using the "full" constructor. */
                s = ccl_sampler_new_full(ctx, sampler_properties, &err);
                g_assert_no_error(err);
                break;
            case 2:
                /* Using the "wrap" constructor. */
                CCL_BEGIN_IGNORE_DEPRECATIONS
                sampler = clCreateSampler(ccl_context_unwrap(ctx),
                    CL_TRUE, CL_ADDRESS_NONE, CL_FILTER_NEAREST,
                    &ocl_status);
                g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
                CCL_END_IGNORE_DEPRECATIONS
                s = ccl_sampler_new_wrap(sampler);
                g_assert_cmphex(GPOINTER_TO_UINT(sampler), ==,
                    GPOINTER_TO_UINT(ccl_sampler_unwrap(s)));
                break;
        }

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
        g_assert_cmphex(GPOINTER_TO_UINT(context), ==,
            GPOINTER_TO_UINT(ccl_context_unwrap(ctx)));

        /* Destroy sampler. */
        ccl_sampler_destroy(s);
    }

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests creation (using "full" constructor), getting info from and
 * destruction of sampler wrapper objects.
 * */
static void create_full_info_destroy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLSampler * s = NULL;
    const cl_sampler_properties sampler_properties[] = {
        CL_SAMPLER_NORMALIZED_COORDS, CL_FALSE,
        CL_SAMPLER_ADDRESSING_MODE, CL_ADDRESS_CLAMP_TO_EDGE,
        CL_SAMPLER_FILTER_MODE, CL_FILTER_NEAREST,
        0};

    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(&err);
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
    g_assert_cmphex(GPOINTER_TO_UINT(context), ==,
        GPOINTER_TO_UINT(ccl_context_unwrap(ctx)));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy sampler. */
    ccl_sampler_destroy(s);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests sampler wrapper class reference counting.
 * */
static void ref_unref_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLSampler * s = NULL;
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(&err);
    g_assert_no_error(err);

    /* Create sampler. */
    s = ccl_sampler_new(
        ctx, CL_TRUE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST, &err);
    g_assert_no_error(err);

    /* Increase sampler reference count. */
    ccl_sampler_ref(s);

    /* Check that sampler ref count is 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) s));

    /* Unref sampler. */
    ccl_sampler_unref(s);

    /* Check that sampler ref count is 1. */
    g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper *) s));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy stuff. */
    ccl_sampler_unref(s);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char ** argv) {

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
