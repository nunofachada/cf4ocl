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
    g_set_error(err, CCL_ERROR, CL_INVALID_VALUE, "Mock error");
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
    CCLErr * err = NULL;

    /* Try and get device from mock device container. */
    dev = ccl_dev_container_get_device(
        &mock_devcon, mock_get_devices, 0, &err);

    /* Check that dev is NULL */
    g_assert(dev == NULL);

    /* Check the error domain and code, and clear the error. */
    g_assert_error(err, CCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

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

    return g_test_run();
}
