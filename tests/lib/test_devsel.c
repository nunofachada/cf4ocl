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
 * Tests for the device selection module. This module is mostly tested in the
 * context tests. This file contains tests that don't fit in a context
 * creation criteria.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

/**
 * @internal
 *
 * @brief Test dependent filters in the device selection module.
 * */
static void dependent_filters_test() {

    /* Variables. */
    CCLErr * err = NULL;
    CCLDevSelDevices devs = NULL;
    CCLContext * ctx = NULL;

    /* Create test context. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Create object containing device wrappers for all OpenCL devices in the
     * system. */
    devs = ccl_devsel_devices_new(&err);
    /* Check that no error occurred. */
    g_assert_no_error(err);

    /* Check that devs is not null. */
    g_assert_cmphex(GPOINTER_TO_SIZE(devs), !=, GPOINTER_TO_SIZE(NULL));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* TO DO: Test dependent filters. */

    /* Destroy object containing device wrappers for all OpenCL devices in the
     * system. */
    ccl_devsel_devices_destroy(devs);

    /* Destroy test context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Test independent filters in the device selection module.
 * */
static void independent_filters_test() {

    /* Variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLPlatform * platf = NULL;
    CCLErr * err = NULL;
    cl_bool abool;
    char * dname;

    /* Get the test context, platform and device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);
    platf = ccl_context_get_platform(ctx, &err);
    g_assert_no_error(err);
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* 1. Test ccl_devsel_indep_type() with an error condition (NULL data). */
    abool = ccl_devsel_indep_type(dev, NULL, &err);
    g_assert_false(abool);
    g_assert_error(err, CCL_ERROR, CCL_ERROR_INVALID_DATA);
    g_clear_error(&err);

    /* 2. Test ccl_devsel_indep_platform() with an error condition (NULL
     * data). */
    abool = ccl_devsel_indep_platform(dev, NULL, &err);
    g_assert_false(abool);
    g_assert_error(err, CCL_ERROR, CCL_ERROR_INVALID_DATA);
    g_clear_error(&err);

    /* 3. Test ccl_devsel_indep_string() with a pass condition (device
     * name). */
    dname = ccl_device_get_info_array(dev, CL_DEVICE_NAME, char, &err);
    g_assert_no_error(err);
    abool = ccl_devsel_indep_string(dev, dname, &err);
    g_assert_true(abool);
    g_assert_no_error(err);

    /* 4. Test ccl_devsel_indep_string() with a pass condition (device
     * vendor). */
    dname = ccl_device_get_info_array(dev, CL_DEVICE_VENDOR, char, &err);
    g_assert_no_error(err);
    abool = ccl_devsel_indep_string(dev, dname, &err);
    g_assert_true(abool);
    g_assert_no_error(err);

    /* 5. Test ccl_devsel_indep_string() with a pass condition (device
     * name). */
    dname = ccl_platform_get_info_string(platf, CL_PLATFORM_NAME, &err);
    g_assert_no_error(err);
    abool = ccl_devsel_indep_string(dev, dname, &err);
    g_assert_true(abool);
    g_assert_no_error(err);

    /* 6. Test ccl_devsel_indep_string() with a no pass condition (unknown
     * name). */
    dname = "!!!! This is a long name and no device, platform or vendor will "
        "ever have such a long name, probably ????";
    abool = ccl_devsel_indep_string(dev, dname, &err);
    g_assert_false(abool);
    g_assert_no_error(err);

    /* 7. Test ccl_devsel_indep_string() with an error condition (NULL
     * data). */
    abool = ccl_devsel_indep_string(dev, NULL, &err);
    g_assert_error(err, CCL_ERROR, CCL_ERROR_INVALID_DATA);
    g_assert_false(abool);
    g_clear_error(&err);

    /* Free context. */
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

    g_test_add_func("/devsel/dependent_filters",
        dependent_filters_test);

    g_test_add_func("/devsel/independent_filters",
        independent_filters_test);

    return g_test_run();
}
