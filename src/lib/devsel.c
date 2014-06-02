/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with cf4ocl. If not, see 
 * <http://www.gnu.org/licenses/>.
 * */
 
/** 
 * @file
 * @brief Functions for selection OpenCL devices and the associated 
 * context.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "devsel.h"

/** @brief Implementation of ::cl4_devsel function which selects one or
 * more devices based on device information such as device name, device 
 * vendor and platform name.
 *  
 * `devInfos` is expected to be a NULL-terminated array of strings 
 * (char**), with at most three strings, each representing (partially or
 * in full) the following information:
 * 
 * 1. Platform name
 * 2. Device name
 * 3. Device vendor
 * 
 * Empty strings will be ignored.
 * 
 * The function will return an OpenCL context encompassing the first 
 * device or set of devices (within the same OpenCL platform) in 
 * accordance with the given information.
 * 
 * @param select_info A NULL-terminated array of strings, with at most 
 * three strings, each respectively representing (partially or in full) 
 * the platform name, the device name and the device vendor. Empty 
 * strings will be ignored.
 * @param err Error structure, to be populated if an error occurs.
 * @return An OpenCL context or NULL if no adequate context was found.
 */
cl_context cl4_devsel_devname(void *select_info, GError **err) {
	
	
	cl_uint num_devices = 0;
	cl_device_id *devices;
	cl_context = NULL;
	
	/* Partial name must be a substring of complete name. */
	gchar *partialName, *completeName;
	
	cl4_platform platforms;
	cl4_device devices_aux;
	
	gchar* name_select_info[];
	
	platforms = cl4_platform_all();
	name_select_info = (gchar**) select_info;
	
	if (platforms) {
		for (unsigned int i = 0; platforms[i]; i++) {
			
			completeName = g_ascii_strdown(cl4_platform_, -1);
			partialName = g_ascii_strdown(name_select_info[0], -1);
		
		}
	}
	
	/* numDevices must be greater than 0. */
	g_assert_cmpuint(numDevices, >, 0);
	
	/* Index of selected device. */
	cl_int index = -1;
	/* Pointer to CL4ManDeviceInfo structure. */
	CL4ManDeviceInfo* info;
	/* Number of devices found which conform to the information. */
	int numValidDevs = 0;
	/* If more than 1 device is found this aux. struct. will be passed 
	 * to a user query function. */
    CL4ManDeviceInfo validDevInfos[CL4_MAN_MAX_DEVICES_TOTAL];
    /* Maps the aux. struct. dev. index to the main struct. dev. index. */
	int map[CL4_MAN_MAX_DEVICES_TOTAL];
	/* Flag to check if a device is conformant with the information. */
	gboolean validDev;
	/* Partial name must be a substring of complete name. */
	gchar *partialName, *completeName;

	/* Check if extraArg contains a valid NULL-terminated array of strings. */
	if (extraArg != NULL) {
		info = (CL4ManDeviceInfo*) extraArg;
		g_assert(info != NULL);
		/* Cycle through available devices. */
		for (unsigned int i = 0; i < numDevices; i++) {
			/* Check for the three types of information: device name.
			 * device vendor and platform name. */
			for (unsigned int j = 0; j < 3; j++) {
				/* Not known if device will pass this filter. */
				validDev = FALSE;
				/* Obtain complete and partial information for comparison. */
				switch (j) {
					case 0:
					if (info->device_name != NULL) {
						completeName = g_ascii_strdown(devInfos[i].device_name, -1);
						partialName = g_ascii_strdown(info->device_name, -1);
					}
					break;
					case 1:
					if (info->device_vendor != NULL) {
						completeName = g_ascii_strdown(devInfos[i].device_vendor, -1);
						partialName = g_ascii_strdown(info->device_vendor, -1);
					}
					break;
					case 2:
					if (info->platform_name != NULL) {
						completeName = g_ascii_strdown(devInfos[i].platform_name, -1);
						partialName = g_ascii_strdown(info->platform_name, -1);
					}
					break;
					default:
					g_assert_not_reached();
				}
				/* Check if partial name is within in the complete name. */
				if (g_strrstr(completeName, partialName)) {
					/* Valid device so far. */
					validDev = TRUE;
				}
				/* Free temporary strings. */
				g_free(partialName);
				g_free(completeName);
				/* If device didn't conform to info, go to next device. */
				if (!validDev) {
					break;
				}
			}
			/* If device passed filters, add it to valid devices. */
			if (validDev) {
				validDevInfos[numValidDevs] = devInfos[i];
				map[numValidDevs] = i;
				numValidDevs++;
			}
		}
	}
	
	/* Check result. */
	if (numValidDevs == 1) {
		/* Only one valid device found, return that one. */
		index = map[0];
	} else if (numValidDevs > 1) {
		/* Several valid devices found, ask the user to chose one. */
		index = map[cl4_man_menu_device_selector_query(validDevInfos, numValidDevs)];
	}
	
	/* Return device index. */
	return index;
}


