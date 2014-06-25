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
		(GDestroyNotify) cl4_device_unref);
	
	/* *** Populate array of device wrapper objects with all OpenCL ***
	 * *** devices present in the system. *** */
	 
	/* Cycle through OpenCL platforms. */
	for (guint i = 0; i < num_platfs; i++) {
		
		/* Get next platform wrapper. */
		platform = cl4_platforms_get_platform(platforms, i);
		
		/* Get number of devices in current platform.*/
		guint num_devices = cl4_platform_num_devices(
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
 * @param device OpenCL device to filter depending on type.
 * @param data Filter data, must point to a cl_device_type value.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is of the given type, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type(
	CL4Device* device, void* data, GError **err) {
	
	/* Make sure device is not NULL. */ 
	g_return_val_if_fail(device != NULL, FALSE);
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Make sure data is not NULL. */
	gef_if_error_create_goto(*err, CL4_ERROR, data == NULL, 
		CL4_ERROR_INVALID_DATA, error_handler,
		"Function '%s': invalid filter data", __func__); 
	
	/* Get type to check for. */
	cl_device_type type_to_check = *((cl_device_type*) data);
	
	/* Get device type. */
	cl_device_type type = cl4_device_info_value_scalar(
		device, CL_DEVICE_TYPE, cl_device_type, &err_internal);
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
 * @param device OpenCL device to check for GPU type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a GPU, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_gpu(
	CL4Device* device, void *data, GError **err) {

	/* Set device type to GPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_GPU;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(device, (void*) &type_to_check, err);
	
}

/**
 * @brief Independent filter function which only accepts CPU devices.
 * 
 * @param device OpenCL device to check for CPU type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a CPU, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_cpu(
	CL4Device* device, void *data, GError **err) {

	/* Set device type to CPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_CPU;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(device, (void*) &type_to_check, err);

}

/**
 * @brief Independent filter function which only accepts accelerator 
 * devices.
 * 
 * @param device OpenCL device to check for accelerator type.
 * @param data Filter data, ignored.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if device is a accelerator, FALSE otherwise.
 * */
gboolean cl4_devsel_indep_type_accel(
	CL4Device* device, void *data, GError **err) {

	/* Set device type to Accelerator. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_ACCELERATOR;
	
	/* Data not used, ignore compiler warnings. */
	data = data;
	
	/* Return result. */
	return cl4_devsel_indep_type(device, (void*) &type_to_check, err);

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
		"Function '%s': invalid filter data", __func__); 
	
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
