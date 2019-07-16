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
 * Tests functionality offered by the abstract wrappers.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"
#include "_ccl_abstract_dev_container_wrapper.h"

/**
 * @internal
 *
 * @brief Used as a mock function for getting devices in a device container
 * wrapper.
 */
static CCLWrapperInfo * mock_get_devices(
        CCLDevContainer * devcon, CCLErr ** err) {

    CCL_UNUSED(devcon);
    g_set_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE, "Mock error");
    return NULL;
}

/**
 * @internal
 *
 * @brief Tests errors in device container functions.
 * */
static void device_container_errors_test() {

    /* Test variables. */
    CCLDevContainer mock_devcon = { { 0, NULL, NULL, 0 }, 0, NULL };
    CCLDevice * dev;
    CCLDevice * const * dev_lst;
    CCLErr * err = NULL;

    /* Try and get device from mock device container. */
    dev = ccl_dev_container_get_device(
        &mock_devcon, mock_get_devices, 0, &err);

    /* Check that dev is NULL */
    g_assert(dev == NULL);

    /* Check the error domain and code, and clear the error. */
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    ccl_err_clear(&err);

    /* Try and get device list from mock device container. */
    dev_lst = ccl_dev_container_get_all_devices(
        &mock_devcon, mock_get_devices, &err);

    /* Check that dev is NULL */
    g_assert(dev_lst == NULL);

    /* Check the error domain and code, and clear the error. */
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    ccl_err_clear(&err);

    /* Confirm that no memory was allocated for wrappers. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Used as a mock function releasing an OpenCL object.
 */
static cl_int mock_cl_release(void * cl_object) {

    CCL_UNUSED(cl_object);
    return CL_OUT_OF_RESOURCES;
}

/**
 * @internal
 *
 * @brief Tests errors in abstract wrapper functions.
 * */
static void errors_test() {

    /* Test variables. */
    void * var;
    size_t size = sizeof(CCLWrapper);
    CCLWrapper * mock_wrapper;
    CCLErr * err = NULL;

    /* Create a mock wrapper. */
    mock_wrapper = ccl_wrapper_new(CCL_NONE, (void *) &var, size);

    /* Destroy the mock wrapper, provoking an error while doing so. */
    ccl_wrapper_unref(mock_wrapper, size, NULL, mock_cl_release, &err);

    /* Check the error domain and code, and clear the error. */
    g_assert_error(err, CCL_OCL_ERROR, CL_OUT_OF_RESOURCES);
    ccl_err_clear(&err);

    /* Confirm that no memory was allocated for wrappers. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests the creating of wrapper info with zero size.
 * */
static void info_zero_size_test() {

    /* Test variables. */
    CCLWrapperInfo * info;

    /* Create wrapper info object with zero size for the information itself. */
    info = ccl_wrapper_info_new(0);

    /* Confirm size is zero and info value is NULL. */
    g_assert_cmpint(info->size, ==, 0);
    g_assert_null(info->value);

    /* Destroy wrapper info object. */
    ccl_wrapper_info_destroy(info);

    /* Confirm that no memory was allocated for wrappers. */
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
        "/wrappers/abstract/device-container-errors",
        device_container_errors_test);

    g_test_add_func(
        "/wrappers/abstract/errors",
        errors_test);

    g_test_add_func(
        "/wrappers/abstract/info_zero_size",
        info_zero_size_test);

    return g_test_run();
}
