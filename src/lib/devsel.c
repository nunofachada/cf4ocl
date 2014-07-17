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
 * @brief Functions for filtering and selecting OpenCL devices.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "devsel.h"

	
/** 
 * @brief Filter type. 
 * */
typedef enum cl4_devsel_filter_type {
	
	/** Independent filter, filters one device at a time. */
	CL4_DEVSEL_INDEP,
	/** Dependent filter, filters devices depending on the currently
	 * available device choices. */
	CL4_DEVSEL_DEP
	
} CL4DevSelFilterType;

/** 
 * @brief Filter object, includes a filter function (independent or 
 * dependent) and the respective filter data.
 * */
typedef struct cl4_devsel_filter {
	
	/** Filter function. */
	gpointer function;
	/** Filter data. */
	gpointer data;
	/** Filter type. */
	CL4DevSelFilterType type;
	
} CL4DevSelFilter;

/**
 * @brief Add any filter to the filter set.
 * 
 * This private function is used by the public cl4_devsel_add_*_filter()
 * functions.
 * 
 * @param filters The filter set.
 * @param function Filter function (independent or dependent).
 * @param data Filter data.
 * @param type Type of filter: independent or dependent.
 * */
static void cl4_devsel_add_filter(CL4DevSelFilters* filters, 
	gpointer function, gpointer data, CL4DevSelFilterType type) {

	/* Initialize filters if required. */
	if (*filters == NULL)
		*filters = g_ptr_array_new();
	
	/* Allocate space for new filter. */
	CL4DevSelFilter* filter = g_slice_new0(CL4DevSelFilter);
	
	/* Set filter function, filter data and type. */
	filter->function = function;
	filter->data = data;
	filter->type = type;
	
	/* Add filter to filters array. */
	g_ptr_array_add(*filters, filter);
	
}

/**
 * @brief Add an independent filter to the filter set.
 * 
 * @param filters The filter set.
 * @param function Indendent filter function.
 * @param data Filter data.
 * */
void cl4_devsel_add_indep_filter(
	CL4DevSelFilters* filters, cl4_devsel_indep function, gpointer data) {

	cl4_devsel_add_filter(filters, function, data, CL4_DEVSEL_INDEP);
}

/**
 * @brief Add a dependent filter to the filter set.
 * 
 * @param filters The filter set.
 * @param function Indendent filter function.
 * @param data Filter data.
 * */
void cl4_devsel_add_dep_filter(
	CL4DevSelFilters* filters, cl4_devsel_dep function, gpointer data) {

	cl4_devsel_add_filter(filters, function, data, CL4_DEVSEL_DEP);
}

/** 
 * @brief Select one or more OpenCL devices based on the provided
 * filters.
 * 
 * This function is internally used by the 
 * cl4_context_new_from_filters_full() function for selecting context
 * devices. Clients should not need to use it frequently.
 * 
 * @param filters Filters used to select device(s).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return One or more OpenCL devices selected based on the provided
 * filters.
 *  */
CL4DevSelDevices cl4_devsel_select(
	CL4DevSelFilters* filters, GError **err) {

	/* Make sure filters is not NULL. */
	g_return_val_if_fail(filters != NULL, NULL);
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Platforms wrapper object. */
	CL4Platforms* platforms = NULL;
	
	/* Platform wrapper object. */
	CL4Platform* platform = NULL;
	
	/* Device wrapper object. */
	CL4Device* device = NULL;
	
	/* Array of device wrapper objects. Devices will be selected from
	 * this array.  */
	GPtrArray* devices = NULL;

	/* Number of platforms. */
	guint num_platfs;
	
	/* Internal error handling object. */
	GError* err_internal = NULL;
	
	/* Get all OpenCL platforms in system wrapped in a CL4Platforms
	 * object. */
	platforms = cl4_platforms_new(&err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Determine number of platforms. */
	num_platfs = cl4_platforms_count(platforms);
	
	/* Create array of device wrapper objects. */
	devices = g_ptr_array_new_with_free_func(
		(GDestroyNotify) cl4_device_destroy);
	
	/* *** Populate array of device wrapper objects with all OpenCL ***
	 * *** devices present in the system. *** */
	 
	/* Cycle through OpenCL platforms. */
	for (guint i = 0; i < num_platfs; i++) {
		
		/* Get next platform wrapper. */
		platform = cl4_platforms_get_platform(platforms, i);
		
		/* Get number of devices in current platform.*/
		guint num_devices = cl4_platform_get_num_devices(
			platform, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		/* Cycle through devices in current platform. */
		for (guint j = 0; j < num_devices; j++) {
			
			/* Get current device wrapper. */
			device = cl4_platform_get_device(platform, j, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
			
			/* Add device wrapper to array of device wrapper objects. */
			g_ptr_array_add(devices, (gpointer) device);
			
			/* Update device reference (because it is kept also in
			 * the array of device wrapper objects). */
			cl4_device_ref(device);
			
		} 
		
	}
	
	/* *** Filter devices. *** */

	/* Cycle through all filters. */
	for (guint i = 0; i < (*filters)->len; i++) {
		
		/* Get current filter. */
		CL4DevSelFilter* curr_filter = g_ptr_array_index(*filters, i);
		
		/* Check type of filter, proceed accordingly. */
		if (curr_filter->type == CL4_DEVSEL_DEP) {
			
			/* It's a dependent filter.*/
			devices = ((cl4_devsel_dep) (curr_filter->function))(
				devices, curr_filter->data, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
				
		} else {
			/* It's an independent filter. */
			
			/* Cycle through all devices. */
			for (guint j = 0; j < devices->len; j++) {
				
				/* Get current device wrapper. */
				CL4Device* curr_device = 
					(CL4Device*) g_ptr_array_index(devices, j);

				/* Check if current device is accepted by current 
				 * filter. */
				gboolean pass = 
					((cl4_devsel_indep) curr_filter->function)(
						curr_device, curr_filter->data, &err_internal);
				gef_if_err_propagate_goto(
					err, err_internal, error_handler);

				/* If current device didn't pass current filter... */
				if (!pass) {
					
					/* Remove device wrapper from device wrapper array. */
					g_ptr_array_remove_index(devices, j);
					
					/* Force device index decrement, because next device to
					 * be checked will be in this index. */
					j--;
					
				}
			}			
		}
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:

	/* Free platforms wrapper object. */
	cl4_platforms_destroy(platforms);
	
	/* Free individual filters. */
	for (guint i = 0; i < (*filters)->len; i++)
		g_slice_free(CL4DevSelFilter, g_ptr_array_index(*filters, i));
		
	/* Free filter array. */
	g_ptr_array_free(*filters, TRUE);

	/* Return the selected devices. */
	return devices;

}

/**
 * @brief Independent filter function which accepts devices of the type
 * given in the data parameter.
 * 
 * This function is used by the cl4_devsel_indep_type_*() group of
 * functions. Using these is simpler than using this function directly.
 * 
 * @param dev OpenCL device to filter depending on type.
 * @param data Filter data, must point to a cl_device_type value.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is of the given type, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type(
	CL4Device* dev, void* data, GError **err) {
	
	/* Make sure dev is not NULL. */ 
	g_return_val_if_fail(dev != NULL, FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Make sure data is not NULL. */
	gef_if_error_create_goto(*err, CL4_ERROR, data == NULL, 
		CL4_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", G_STRLOC); 
	
	/* Get type to check for. */
	cl_device_type type_to_check = *((cl_device_type*) data);
	
	/* Get device type. */
	cl_device_type type = cl4_device_info_value_scalar(
		dev, CL_DEVICE_TYPE, cl_device_type, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:
	
	/* Return the selected devices. */
	return (gboolean) (type & type_to_check);
}

/**
 * @brief Independent filter function which only accepts GPU devices.
 * 
 * @param dev OpenCL device to check for GPU type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a GPU, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_gpu(
	CL4Device* dev, void *data, GError **err) {

	/* Set device type to GPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_GPU;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(dev, (void*) &type_to_check, err);
	
}

/**
 * @brief Independent filter function which only accepts CPU devices.
 * 
 * @param dev OpenCL device to check for CPU type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a CPU, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_cpu(
	CL4Device* dev, void *data, GError **err) {

	/* Set device type to CPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_CPU;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(dev, (void*) &type_to_check, err);

}

/**
 * @brief Independent filter function which only accepts accelerator 
 * devices.
 * 
 * @param dev OpenCL device to check for accelerator type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a accelerator, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_accel(
	CL4Device* dev, void *data, GError **err) {

	/* Set device type to Accelerator. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_ACCELERATOR;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(dev, (void*) &type_to_check, err);

}

/**
 * @brief Independent filter which selects devices based on device name, 
 * device vendor and/or platform name.
 * 
 * @param dev OpenCL device to filter by platform.
 * @param data Filter data, must be a string.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is accepted by filter, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_string(
	CL4Device* dev, void *data, GError **err) {
	
	/* Make sure device is not NULL. */ 
	g_return_val_if_fail(dev != NULL, FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Return value, i.e., flag indicating if device belongs to the 
	 * specified platform. */
	gboolean pass = FALSE;
	
	/* Partial name must be a substring of complete name. */
	gchar *complt_info, *complt_info_lowr, *part_info;
	
	/* Make sure data is not NULL. */
	gef_if_error_create_goto(*err, CL4_ERROR, data == NULL, 
		CL4_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", G_STRLOC); 
	
	/* Lower-case partial name for comparison. */
	part_info = g_ascii_strdown((gchar*) data, -1);

	/* Compare with device name. */
	complt_info = cl4_device_info_value_array(
		dev, CL_DEVICE_NAME, char*, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	complt_info_lowr = g_ascii_strdown(complt_info, -1);

	if (!g_strrstr(complt_info_lowr, part_info)) {
		/* Device name does not match, check device vendor. */
		
		/* Free string allocated for previous comparison. */
		g_free(complt_info_lowr);
		
		/* Compare with device vendor. */
		complt_info = cl4_device_info_value_array(
			dev, CL_DEVICE_VENDOR, char*, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);

		complt_info_lowr = g_ascii_strdown(complt_info, -1);

		if (!g_strrstr(complt_info_lowr, part_info)) {
			/* Device vendor does not match, check platform name. */

			/* Free string allocated for previous comparison. */
			g_free(complt_info_lowr);
			
			/* Compare with platform name. */
			CL4Platform* platf;
			
			/* Get device platform. */
			platf = cl4_device_get_platform(dev, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
			
			/* Get platform name. */
			complt_info = cl4_platform_get_info_string(
				platf, CL_PLATFORM_NAME, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
			
			complt_info_lowr = g_ascii_strdown(complt_info, -1);
			
			/* Compare. */
			if (g_strrstr(complt_info_lowr, part_info)) {
				pass = TRUE;
			}
			
		} else {
			pass = TRUE;
		}
		
	} else {
		pass = TRUE;
	}

	g_free(complt_info_lowr);
	g_free(part_info);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* Filter will not accept device in case an error occurs. */
	pass = FALSE;
	
finish:

	/* Return filtering result. */
	return pass;

}

/**
 * @brief Independent filter function which only accepts devices of a
 * specified platform.
 * 
 * @param device OpenCL device to filter by platform.
 * @param data Filter data, must be a cl_platform_id.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device belongs to the specified platform, FALSE 
 * otherwise.
 * */
gboolean cl4_devsel_indep_platform(
	CL4Device* device, void *data, GError **err) {
		
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

	/* Device platform. */
	cl_platform_id platf;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Return value, i.e., flag indicating if device belongs to the 
	 * specified platform. */
	gboolean pass;

	/* Check if data is NULL, throw error if so. */
	gef_if_error_create_goto(*err, CL4_ERROR, data == NULL, 
		CL4_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", G_STRLOC); 
	
	/* Get device platform. */
	platf = cl4_device_info_value_scalar(device, CL_DEVICE_PLATFORM,
		cl_platform_id, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Determine filtering result, i.e. if device platform is the same
	 * as the specified platform. */
	pass = (platf == (cl_platform_id) data) ? TRUE : FALSE;

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* Filter will not accept device in case an error occurs. */
	pass = FALSE;
	
finish:

	/* Return filtering result. */
	return pass;
}

/**
 * @brief Dependent filter function which only accepts devices of the
 * same platform (the platform to which the first device belong to).
 * 
 * @param devices Currently available OpenCL devices.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 * */
CL4DevSelDevices cl4_devsel_dep_platform(
	CL4DevSelDevices devices, void *data, GError **err) {
		
	/* Make sure devices is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Current device. */
	CL4Device* dev;
	
	/* Reference platform and current device platform.*/
	cl_platform_id platf_ref, platf_curr;
	
	/* Internal error object. */
	GError *err_internal = NULL;
	
	/* Filter data is ignored by this filter. */
	data = data;
	
	/* Get first device, which will determine the reference platform. */
	dev = (CL4Device*) g_ptr_array_index(devices, 0);
	
	/* Determine reference platform (i.e. platform of first device). */
	platf_ref = cl4_device_info_value_scalar(dev, CL_DEVICE_PLATFORM,
		cl_platform_id, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
		
	/* Check if devices belong to the reference platform, remove them if 
	 * they don't. */
	for (guint i = 1; i < devices->len; i++) {
		
		/* Get current device. */
		dev = (CL4Device*) g_ptr_array_index(devices, i);
		
		/* Get current device platform. */
		platf_curr = cl4_device_info_value_scalar(
			dev, CL_DEVICE_PLATFORM, cl_platform_id, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);

		/* If current device doesn't belong to the reference 
		 * platform... */
		if (platf_ref != platf_curr) {
			
			/* Remove device wrapper from device wrapper array. */
			g_ptr_array_remove_index(devices, i);
			
			/* Force device index decrement, because next device to
			 * be checked will be in this index. */
			i--;
			
		}
		
	}
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* Set return value to NULL to conform to specification. */
	devices = NULL;
	
finish:

	/* Return filtered devices. */
	return devices;
}

/** 
 * @brief Private helper function, prints a list of the devices 
 * specified in the given list.
 * 
 * @param devices List of devices.
 * @param selected Index of selected device (a message will appear near
 * the device name indicating the device is selected). Pass -1 to 
 * ignore it.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * */
static void cl4_devsel_dep_menu_list(CL4DevSelDevices devices, 
	cl_int selected, GError** err) {
	
	char* sel_str;
	g_print("\n   " \
		"=========================== Device Selection" \
		"============================\n\n");
		
	for (cl_int i = 0; i < (cl_int) devices->len; i++) {
		
		char* name = cl4_device_info_value_array(
				devices->pdata[i], CL_DEVICE_NAME, char*, err);
		if ((err != NULL) && (*err != NULL)) return;
		char* vendor = cl4_device_info_value_array(
				devices->pdata[i], CL_DEVICE_VENDOR, char*, err);
		if ((err != NULL) && (*err != NULL)) return;
		sel_str = "            ";
		if (i == selected) {
			sel_str = "  [SELECTED]";
		}
		g_print(" %s %d. %s\n                 %s\n", 
			sel_str, i, name, vendor);
		
	}
}

/** 
 * @brief Private helper function, asks the user to select a device 
 * from a list. 
 * 
 * @param devices List of devices.
 * @return The list index of the selected device.
 * */
static cl_int cl4_devsel_dep_menu_query(CL4DevSelDevices devices,
	GError** err) {
	
	/* Index of selected device. */
	cl_int index = -1;
	/* Number of results read from user input. */
	int result;
	/* Internal error handler. */
	GError* err_internal = NULL;
	
	/* Print available devices */
	cl4_devsel_dep_menu_list(devices, -1, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If only one device exists, return that one. */
	if (devices->len == 1) {
		index = 0;
	} else {
		/* Otherwise, query the user. */
		do {
			g_print("   (?) Select device (0-%d) > ", devices->len - 1);
			result = scanf("%u", &index);
			/* Clean keyboard buffer */
			int c;
			do { c = getchar(); } while (c != '\n' && c != EOF);
			/* Check if result is Ok and break the loop if so */
			if (1 == result) {
				if ((index >= 0) && (index < (cl_int) devices->len))
					break;
			}
			/* Result not Ok, print error message */
			g_print("   (!) Invalid choice, please insert a value " \
				"between 0 and %u.\n", devices->len - 1);
		} while (1);
	}
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	index = -1;
	
finish:
	
	/* Return device index. */
	return index;

}

/**
 * @brief Dependent filter function which presents a menu to the user
 * allowing him to select the desired device.
 * 
 * @param devices List of devices.
 * @param data If not NULL, can contain a device index, such that the
 * device is automatically selected by this filter.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 * */
CL4DevSelDevices cl4_devsel_dep_menu(
	CL4DevSelDevices devices, void *data, GError **err) {
		
	/* Make sure devices is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Internal error object. */
	GError *err_internal = NULL;

	/* Index of selected device. */
	cl_int index = -1;

	/* If data argument is given, perform auto-selection. */
	if (data != NULL) {
		/* Check if data contains a valid device index. */
		index = *((cl_uint*) data);
		/* Check if index is within bounds. */
		if ((index >= 0) && (index < (cl_int) devices->len)) {
			/* Device index is within bounds, print list with selection. */
			cl4_devsel_dep_menu_list(devices, index, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
		} else {
			/* If we get here, an invalid device index was given. */
			g_print("\n   (!) No device at index %d!\n", index);
			index = -1;
		}
	}
	
	/* If no proper index was given ask the user for the correct index. */
	if (index == -1) {
		index = cl4_devsel_dep_menu_query(devices, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}
	
	/* Remove all devices except the selected device. */
	cl_int num_devs = devices->len;
	for (cl_int i = 0; i < num_devs; ++i) {
		if (i != index)
			g_ptr_array_remove_index(devices, 0);
	}
	g_assert_cmpint(1, ==, devices->len);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* Set return value to NULL to conform to specification. */
	devices = NULL;
	
finish:

	/* Return filtered devices. */
	return devices;
}


