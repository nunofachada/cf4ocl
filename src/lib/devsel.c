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

struct cl4_devsel_filter {
	cl4_devsel function;
	gpointer data;
};


void cl4_devsel_add_filter(
	CL4DevSelFilters* filters, cl4_devsel filt_fun, gpointer filt_data) {

	/* Initialize filters if required. */
	if (*filters == NULL)
		*filters = g_ptr_array_new();
	
	/* Allocate space for new filter. */	
	CL4DevSelFilter* filter = g_slice_new0(CL4DevSelFilter);
	
	/* Set filter function and filter data. */
	filter->function = filt_fun;
	filter->data = filt_data;
	
	/* Add filter to filters array. */
	g_ptr_array_add(*filters, filter);
	
}

CL4DevSelDevices cl4_devsel_select(CL4DevSelFilters* filters, GError **err) {

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
		guint num_devices = cl4_platform_device_count(
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

	/* Cycle through all devices. */
	for (guint i = 0; i < devices->len; i++) {
		
		/* Get current device wrapper. */
		CL4Device* curr_device = (CL4Device*) g_ptr_array_index(devices, i);
		
		/* Cycle through all filters. */
		for (guint j = 0; j < (*filters)->len; j++) {

			/* Get current filter. */
			CL4DevSelFilter* curr_filter = g_ptr_array_index(*filters, j);

			/* Check if current device is accepted by current filter. */
			gboolean pass = curr_filter->function(
				curr_device, curr_filter->data, &err_internal);
			gef_if_err_propagate_goto(err, err_internal, error_handler);
			
			/* If current device didn't pass current filter... */
			if (!pass) {
				
				/* Remove device wrapper from device wrapper array. */
				g_ptr_array_remove_index(devices, i);
				
				/* Force device index decrement, because next device to
				 * be checked will be in this index. */
				i--;
				
				/* Get out of the filtering loop, device didn't pass,
				 * no need to test with other filters. */
				break;
			}
		};
		
	};

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


static gboolean cl4_devsel_type(
	CL4Device* device, cl_device_type type_to_check, GError **err) {
	
	/* Make sure device is not NULL. */ 
	g_return_val_if_fail(device != NULL, FALSE);
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
	
	GError* err_internal = NULL;
	
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

gboolean cl4_devsel_gpu(CL4Device* device, void *select_data, GError **err) {

	select_data = select_data;
	return cl4_devsel_type(device, CL_DEVICE_TYPE_GPU, err);
	
}

gboolean cl4_devsel_cpu(CL4Device* device, void *select_data, GError **err) {

	select_data = select_data;
	return cl4_devsel_type(device, CL_DEVICE_TYPE_CPU, err);

}

gboolean cl4_devsel_accel(CL4Device* device, void *select_data, GError **err) {

	select_data = select_data;
	return cl4_devsel_type(device, CL_DEVICE_TYPE_ACCELERATOR, err);

}


