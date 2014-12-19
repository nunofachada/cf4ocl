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
 * Test the device wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

/**
 * Tests the creation of sub-devices.
 * */
static void sub_devices_test() {

#ifdef CL_VERSION_1_2

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* pdev = NULL;
	GError* err = NULL;
	cl_uint ocl_ver;
	cl_device_partition_property* dpp;
	cl_uint i;
	cl_bool supported;
	CCLDevice* const* subdevs;
	cl_uint num_subdevs;
	cl_uint max_subdevs;

	/* Get the test context with the pre-defined device. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Check OpenCL version of the platform. */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err);
	g_assert_no_error(err);

	/* This test can only be performed with devices supporting OpenCL
	 * 1.2 or higher. */
	if (ocl_ver < 120) {
		g_test_message("OpenCL version of parent device does not "\
			"support sub-devices. Sub-devices test not performed.");
		return;
	}

	/* Get parent device. */
	pdev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Get device partition properties. */
	dpp = ccl_device_get_info_array(pdev, CL_DEVICE_PARTITION_PROPERTIES,
		cl_device_partition_property*, &err);
	g_assert_no_error(err);

	/* Get maximum number sub-devices. */
	max_subdevs = ccl_device_get_info_scalar(
		pdev, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, cl_uint, &err);
	g_assert_no_error(err);

	/* Test partition equally, if supported by device. */
	supported = CL_FALSE;
	for (i = 0; dpp[i] != 0; ++i) {
		if (dpp[i] == CL_DEVICE_PARTITION_EQUALLY) {
			supported = CL_TRUE;
			break;
		}
	}
	if (supported) {

		/* Find an appropriate number of compute units for each
		 * sub-device. */
		cl_uint cu;
		for (i = 8; (cu = max_subdevs / i) == 0; i /= 2);

		/* Set partition properties. */
		const cl_device_partition_property eqprop[] =
			{CL_DEVICE_PARTITION_EQUALLY, cu, 0};

		/* Partition device. */
		subdevs = ccl_device_create_subdevices(
			pdev, eqprop, &num_subdevs, &err);
		g_assert_no_error(err);

		/* Check sub-devices. */
		for (i = 0; i < num_subdevs; ++i) {

			/* Check the number of compute units. */
			cl_uint subcu = ccl_device_get_info_scalar(subdevs[i],
				CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, &err);
			g_assert_no_error(err);
			g_assert_cmpuint(subcu, ==, cu);

			/* Check the parent device. */
			cl_device_id parent_device = ccl_device_get_info_scalar(
				subdevs[i], CL_DEVICE_PARENT_DEVICE, cl_device_id, &err);
			g_assert_no_error(err);
			g_assert_cmphex(GPOINTER_TO_UINT(parent_device), ==,
				GPOINTER_TO_UINT(ccl_device_unwrap(pdev)));
		}

	}

	//~ /* Test partition by counts, if supported by device. */
	//~ supported = CL_FALSE;
	//~ for (i = 0; dpp[i] != 0; ++i) {
		//~ if (dpp[i] == CL_DEVICE_PARTITION_BY_COUNTS) {
			//~ supported = CL_TRUE;
			//~ break;
		//~ }
	//~ }
	//~ if (supported) {
	//~ }

	/* Destroy stuff. */
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

#else
	g_test_message("OpenCL version of platform does not support " \
		"sub-devices. Sub-devices test not performed.");
#endif

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
		"/wrappers/device/sub-devices",
		sub_devices_test);

	return g_test_run();
}



