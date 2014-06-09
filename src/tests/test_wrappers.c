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

/* Max. length of information string. */
#define CL4_TEST_WRAPPERS_MAXINFOSTR 200

/* Test utility macro. Presents either the required information, or 
 * the error message, if it occurred. Also frees the error object if 
 * an error occurred. */
#define cl4_test_wrappers_msg(base_msg, format, ...) \
	if (err == NULL) { \
		g_snprintf(info_str, CL4_TEST_WRAPPERS_MAXINFOSTR, \
			format, ##__VA_ARGS__); \
	} else { \
		g_snprintf(info_str, CL4_TEST_WRAPPERS_MAXINFOSTR, \
			"%s", err->message); \
		g_clear_error(&err); \
	} \
	g_debug("%s %s", base_msg, info_str);
		
/**
 * @brief Tests platforms wrapper.
 * */
static void platforms_test() {
	
	CL4Platforms* platfs = NULL;
	CL4Platform* p = NULL;
	CL4Device** devs = NULL;
	GError* err = NULL;
	gpointer info;
	guint num_devs;
	guint num_platfs;
	gchar info_str[CL4_TEST_WRAPPERS_MAXINFOSTR];

	/* Get platforms. */
	platfs = cl4_platforms_new(&err);
	if (err == NULL) {
		
		/* Number of platforms. */
		num_platfs = cl4_platforms_count(platfs);
		g_debug("* Found %d OpenCL platforms", num_platfs);
		
		/* Cycle through platforms. */
		for (guint i = 0; i < num_platfs; i++) {
		
			/* Get current platform. */
			p = cl4_platforms_get(platfs, i);
			g_debug(">> Platform %d:", i);

			/* Get platform profile. */
			info = cl4_plaform_info(p, CL_PLATFORM_PROFILE, &err);
			cl4_test_wrappers_msg("==== Profile :", "%s", (gchar*) info);

			/* Get platform version. */
			info = cl4_plaform_info(p, CL_PLATFORM_VERSION, &err);
			cl4_test_wrappers_msg("==== Version :", "%s", (gchar*) info);
			
			/* Get platform name. */
			info = cl4_plaform_info(p, CL_PLATFORM_NAME, &err);
			cl4_test_wrappers_msg("==== Name    :", "%s", (gchar*) info);

			/* Get platform vendor. */
			info = cl4_plaform_info(p, CL_PLATFORM_VENDOR, &err);
			cl4_test_wrappers_msg("==== Vendor  :", "%s", (gchar*) info);

			/* Get platform extensions. */
			info = cl4_plaform_info(p, CL_PLATFORM_EXTENSIONS, &err);
			cl4_test_wrappers_msg("==== Extens. :", "%s", (gchar*) info);
			
			/* Get number of devices. */
			num_devs = cl4_platform_device_count(p, &err);
			
			/* Only test for device information if device count was 
			 * successfully obtained. */
			if (err != NULL) {
				g_test_message("Error obtaining number of devices for platform %d (%s).",
					i, err->message);
				g_error_free(err);
			} else {
				
				g_debug("==== # Devs  : %d", num_devs);

				/* Internally, the CL4Device wrapper already has the 
				 * device list, so we won't check for errors here. */
				devs = cl4_plaform_devices(p, NULL);
				
				/* Cycle through devices in platform. */
				for (guint j = 0; j < num_devs; j++) {
					
					g_debug("====== Device #%d", j);

					info = cl4_device_info(devs[j], CL_DEVICE_NAME, &err);
					cl4_test_wrappers_msg("...... Name :", "%s", (gchar*) info);
					
					info = cl4_device_info(devs[j], CL_DEVICE_ADDRESS_BITS, &err);
					cl4_test_wrappers_msg("...... Address bits :", "%d", *((cl_uint*) info));

					info = cl4_device_info(devs[j], CL_DEVICE_COMPILER_AVAILABLE, &err);
					cl4_test_wrappers_msg("...... Available :", "%d", *((cl_bool*) info));

					info = cl4_device_info(devs[j], CL_DEVICE_BUILT_IN_KERNELS, &err);
					cl4_test_wrappers_msg("...... Built-in kernels :", "%s", (gchar*) info);

					info = cl4_device_info(devs[j], CL_DEVICE_COMPILER_AVAILABLE, &err);
					cl4_test_wrappers_msg("...... Compiler available :", "%d", *((cl_bool*) info));

					info = cl4_device_info(devs[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, &err);
					cl4_test_wrappers_msg("...... Max wkitem sizes :", "%d, %d, %d", (int) ((size_t*) info)[0], (int) ((size_t*) info)[1], (int) ((size_t*) info)[2]);

					info = cl4_device_info(devs[j], CL_DEVICE_TYPE, &err);
					cl4_test_wrappers_msg("...... Type :", "%s", cl4_device_type2str(*((cl_device_type*) info)));
				}
			}
		}

		/* Destroy list of platforms. */
		cl4_platforms_destroy(platfs);
		
	} else {
		
		/* Unable to get any OpenCL platforms, test can't pass. */
		g_test_message("Test failed due to following error: %s", 
			err->message);
		g_test_fail();
	}
	
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

