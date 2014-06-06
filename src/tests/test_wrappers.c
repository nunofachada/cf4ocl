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
 * @brief Tests for context, platforms and devices wrappers library.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "platforms.h"
#include "platform.h"
#include "device.h"
//#include "context.h"

/**
 * @brief Tests platforms wrapper.
 * */
static void platforms_test() {
	
	CL4Platforms* platfs = NULL;
	CL4Platform* p = NULL;
	CL4Device** devs = NULL;
	gchar* info;
	guint num_devs;
	guint num_platfs;
	gpointer dev_info;

	platfs = cl4_platforms_new(NULL);
	num_platfs = cl4_platforms_count(platfs);
	
	g_debug("== Found %d OpenCL platforms", num_platfs);
	for (guint i = 0; i < num_platfs; i++) {
	
		p = cl4_platforms_get(platfs, i);
		
		g_debug("==== Platform %d:", i);

		info = cl4_plaform_info(p, CL_PLATFORM_PROFILE, NULL);
		g_debug("======== Profile : %s", info);
		
		info = cl4_plaform_info(p, CL_PLATFORM_VERSION, NULL);
		g_debug("======== Version : %s", info);

		info = cl4_plaform_info(p, CL_PLATFORM_NAME, NULL);
		g_debug("======== Name    : %s", info);

		info = cl4_plaform_info(p, CL_PLATFORM_VENDOR, NULL);
		g_debug("======== Vendor  : %s", info);

		info = cl4_plaform_info(p, CL_PLATFORM_EXTENSIONS, NULL);
		g_debug("======== Extens. : %s", info);
		
		num_devs = cl4_platform_device_count(p, NULL);
		devs = cl4_plaform_devices(p, NULL);
		
		g_debug("======== Devices : %d", num_devs);
		
		for (guint j = 0; j < num_devs; j++) {
			
			dev_info = cl4_device_info(devs[j], CL_DEVICE_NAME, NULL);
			g_debug("============ Name : %s", (gchar*) dev_info);
			
			dev_info = cl4_device_info(devs[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, NULL);
			g_debug("============ Max. workitem sizes : %d, %d, %d", (int) ((size_t*) dev_info)[0], (int) ((size_t*) dev_info)[1], (int) ((size_t*) dev_info)[2]);

		}

	}
	cl4_platforms_destroy(platfs);
}

/**
 * @brief Main function.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/wrappers/platforms", platforms_test);
	return g_test_run();
}

