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
 * Tests for device selection module.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

/**
 * Tests devsel creation and destruction of object containing device wrappers
 * for all OpenCL devices in the system.
 * */
static void devices_new_destroy_test() {

	/* Error reporting object. */
	GError* err = NULL;

	/* Internal pointer array. */
	gpointer* ptr_array = NULL;

	/* Object containing device wrappers. */
	CCLDevSelDevices devs = NULL;

	/* Create object containing device wrappers for all OpenCL devices in the
	 * system. */
	devs = ccl_devsel_devices_new(&err);

	/* Check that no error occurred. */
	g_assert_no_error(err);

	/* Check that devs is not null. */
	g_assert_cmphex((guint64) devs, !=, (guint64) NULL);

	/* Destroy object containing device wrappers for all OpenCL devices in the
	 * system. */
	ptr_array = ccl_devsel_devices_destroy(devs);

	/* Check that devs is null. */
	g_assert_cmphex((guint64) ptr_array, ==, (guint64) NULL);

	/* Confirm that memory allocated by wrappers has been properly freed. */
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

	g_test_add_func("/devsel/devices_new_destroy_test",
		devices_new_destroy_test);

	return g_test_run();

}

