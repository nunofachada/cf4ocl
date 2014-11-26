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
 * Tests for device query module.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

#define CCL_TEST_DEVQUERY_MAXINFOLEN 500

/**
 * Tests devquery module helper functions.
 * */
static void helpers_test() {

	CCLPlatforms* platfs = NULL;
	CCLPlatform* p = NULL;
	CCLDevice* d = NULL;
	GError* err = NULL;
	guint num_devs;
	guint num_platfs;
	CCLWrapperInfo* info;
	gchar param_value_str[CCL_TEST_DEVQUERY_MAXINFOLEN];

	/* Get platforms. */
	platfs = ccl_platforms_new(&err);
	if (err == NULL) {

		/* Number of platforms. */
		num_platfs = ccl_platforms_count(platfs);
		g_debug("* Found %d OpenCL platforms", num_platfs);

		/* Cycle through platforms. */
		for (guint i = 0; i < num_platfs; i++) {

			/* Get current platform. */
			p = ccl_platforms_get(platfs, i);
			g_debug(">> Platform %d:", i);

			/* Get number of devices. */
			num_devs = ccl_platform_get_num_devices(p, &err);

			/* Only test for device information if device count was
			 * successfully obtained. */
			if (err != NULL) {
				g_test_message("Error obtaining number of devices for platform %d (%s).",
					i, err->message);
				g_clear_error(&err);
			} else {

				g_debug("==== # Devs  : %d", num_devs);

				/* Cycle through devices in platform. */
				for (guint j = 0; j < num_devs; j++) {

					/* Get current device. */
					d = ccl_platform_get_device(p, j, &err);
					g_assert_no_error(err);
					g_debug("====== Device #%d", j);

					for (gint k = 0; k < ccl_devquery_info_map_size; k++) {
						info = ccl_device_get_info(d, ccl_devquery_info_map[k].device_info, &err);
						if (err == NULL) {
							g_debug("\t%s : %s",
								ccl_devquery_info_map[k].param_name,
								ccl_devquery_info_map[k].format(
									info, param_value_str,
									CCL_TEST_DEVQUERY_MAXINFOLEN,
									ccl_devquery_info_map[k].units));
						} else {
							g_clear_error(&err);
							g_debug("\t%s : %s",
								ccl_devquery_info_map[k].param_name, "N/A");

						}
					}

				}
			}
		}

		/* Destroy list of platforms. */
		ccl_platforms_destroy(platfs);

	} else {

		/* Unable to get any OpenCL platforms, test can't pass. */
		g_test_message("Test failed due to following error: %s",
			err->message);
		g_test_fail();
	}

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Test the ccl_devquery_name function of the device module.
 * */
static void name_test() {

	/* Device information. */
	cl_device_info info;

	/* Test exact parameter name. */
	info = ccl_devquery_name("CL_DEVICE_ENDIAN_LITTLE");
	g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
	info = ccl_devquery_name("CL_DEVICE_EXTENSIONS");
	g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
	info = ccl_devquery_name("CL_DRIVER_VERSION");
	g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

	/* Test mixed parameter name. */
	info = ccl_devquery_name("cl_Device_Endian_Little");
	g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
	info = ccl_devquery_name("CL_device_Extensions");
	g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
	info = ccl_devquery_name("cl_DRIVer_version");
	g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

	/* Test lowercase parameter name without cl_device_ or cl_ prefix. */
	info = ccl_devquery_name("endian_little");
	g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
	info = ccl_devquery_name("extensions");
	g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
	info = ccl_devquery_name("driver_version");
	g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

	/* Test parameter name without CL_DEVICE_ or CL_ prefix. */
	info = ccl_devquery_name("ENDIAN_LITTLE");
	g_assert_cmphex(info, ==, CL_DEVICE_ENDIAN_LITTLE);
	info = ccl_devquery_name("EXTENSIONS");
	g_assert_cmphex(info, ==, CL_DEVICE_EXTENSIONS);
	info = ccl_devquery_name("DRIVER_VERSION");
	g_assert_cmphex(info, ==, CL_DRIVER_VERSION);

}

/**
 * Tests if the ccl_devquery_info_map array is well built,
 * namely (i) if the param_name fields are alphabetically ordered, and
 * (ii) if the size of the array corresponds to the
 * ccl_devquery_info_map_size variable.
 * */
static void infomap_test() {

	/* Determined size of info map. */
	gint imsize;

	/* Cycle through info map. */
	for (imsize = 0;
		ccl_devquery_info_map[imsize].param_name != NULL;
		imsize++) {

		if ((imsize > 0)
			&&
			(ccl_devquery_info_map[imsize].param_name != NULL)) {

			/* Test if parameter names are alphabetically ordered. */
			g_assert_cmpstr(
				ccl_devquery_info_map[imsize - 1].param_name,
				<,
				ccl_devquery_info_map[imsize].param_name);
		}
	}

	/* Test if size corresponds. */
	g_assert_cmpint(imsize, ==, ccl_devquery_info_map_size);


}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/devquery/helpers", helpers_test);
	g_test_add_func("/devquery/name", name_test);
	g_test_add_func("/devquery/infomap", infomap_test);
	return g_test_run();
}

