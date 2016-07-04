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
 * Header for cf4ocl2 tests.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

/* A test kernel. */
#define CCL_TEST_PROGRAM_SUM_CONTENT "@TEST_KERNEL_SUM_SRC@"

/* The default device to use in tests is set in the CMake configuration. */
#define CCL_TEST_DEFAULT_DEVICE_IDX @TESTS_DEVICE_INDEX@

/* Print handler which redirects output to debug stream. */
void ccl_print_to_debug(const gchar* string);

/* Create a context with a device specified at compile time. */
CCLContext* ccl_test_context_new(CCLErr** err);
