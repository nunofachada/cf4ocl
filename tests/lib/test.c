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
 * Common stuff for cf4ocl2 tests.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "test.h"
#include "_g_err_macros.h"

/**
 * @internal
 *
 * @brief Print handler which redirects output to debug stream.
 *
 * @param[in] string Output to redirect to debug stream.
 * */
void ccl_print_to_debug(const gchar * string) {
    g_debug("%s", string);
}

/**
 * @internal
 *
 * @brief Create a context with a device specified at compile time or using the
 * `CCL_TEST_DEVICE_INDEX` environment variable.
 *
 * @param[in] min_ocl_ver Minimum OpenCL version that should be supported by
 * the device.
 * @param[out] err Error handling object.
 * @return A context with a device specified at compile time or using the
 * `CCL_TEST_DEVICE_INDEX` environment variable.
 * */
CCLContext * ccl_test_context_new(cl_uint min_ocl_ver, CCLErr ** err) {

    /* Make sure there is no error set. */
    g_assert_no_error(*err);

    /* Context wrapper. */
    CCLContext * ctx = NULL;

    /* Internal error reporting object. */
    CCLErr * err_internal = NULL;

    /* Device index string taken from the environment. */
    const char * devidx_str;

    /* Final device index integer. */
    cl_uint ccl_tests_devidx;

    /* Is a device index defined as an environment variable? */
    devidx_str = g_getenv("CCL_TEST_DEVICE_INDEX");
    if (devidx_str) {

        /* If so, use it. */
        ccl_tests_devidx = atoi(devidx_str);

    } else {

        /* Otherwise use default device index specified at compile time. */
        ccl_tests_devidx = CCL_TEST_DEFAULT_DEVICE_IDX;

    }

    /* Create context. */
    ctx = ccl_context_new_from_device_index(&ccl_tests_devidx, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* Does the test require a minimum OpenCL version? */
    if (min_ocl_ver > 0) {

        /* Variable to hold OpenCL version of platform and device. */
        cl_uint ocl_ver = 0;
        /* Device associated with context. */
        CCLDevice * dev = NULL;

        /* Get OpenCL version of platform associated with context. */
        ocl_ver = ccl_context_get_opencl_version(ctx, &err_internal);
        g_if_err_propagate_goto(err, err_internal, error_handler);

        /* Does the platform support the required OpenCL version? */
        if (ocl_ver < min_ocl_ver) {
            g_test_skip(
                "Platform does not support the required OpenCL version");
            goto set_context_to_null;
        }

        /* Get selected device. */
        dev = ccl_context_get_device(ctx, 0, &err_internal);
        g_if_err_propagate_goto(err, err_internal, error_handler);

        /* Does the device support the required minimum OpenCL version? */
        ocl_ver = ccl_device_get_opencl_version(dev, &err_internal);
        g_if_err_propagate_goto(err, err_internal, error_handler);
        if (ocl_ver < min_ocl_ver)
        {
            g_test_skip(
                "Device does not support the required OpenCL version");
            goto set_context_to_null;
        }
    }

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:

    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

set_context_to_null:

    /* If an error occurred or the platform or device associated with the
     * context don't support the required OpenCL version, return NULL to signal
     * it. */
    if (ctx) {
        ccl_context_destroy(ctx);
        ctx = NULL;
    }

finish:

    return ctx;
}

/**
 * @internal
 *
 * @brief Create a context with an image-supporting device and support for a
 * minimum OpenCL version.
 *
 * @param[in] min_ocl_ver Minimum OpenCL version to be supported by device.
 * @param[out] err Error handling object.
 * @return A context for the image tests.
 * */
CCLContext * ccl_test_context_new_with_image_support(
    cl_uint min_ocl_ver, CCLErr ** err) {

    /* Make sure there is no error set. */
    g_assert_no_error(*err);

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    GError * err_internal = NULL;
    cl_bool image_support;

    /* Create a context with the devices specified in CCL_TEST_DEVICE_INDEX. */
    ctx = ccl_test_context_new(min_ocl_ver, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* Was the context created successfully? */
    if (ctx) {

        /* Get the first device associated with the context. */
        dev = ccl_context_get_device(ctx, 0, &err_internal);
        g_if_err_propagate_goto(err, err_internal, error_handler);

        /* Get device image support. */
        image_support = ccl_device_get_info_scalar(
            dev, CL_DEVICE_IMAGE_SUPPORT, cl_bool, &err_internal);
        g_if_err_propagate_goto(err, err_internal, error_handler);

        /* Does the device support images? */
        if (!image_support) {
            g_test_skip("Device does not support images");
            ccl_context_destroy(ctx);
            ctx = NULL;
        }
    }

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:

    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

finish:

    /* Return the context. */
    return ctx;
}
