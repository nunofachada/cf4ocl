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

	CL4DevSelFilter* filter = g_slice_new0(CL4DevSelFilter);
	
	filter->function = filt_fun;
	filter->data = filt_data;
	
	g_ptr_array_add(filters, filter);
	
}

GPtrArray* cl4_devsel_select(CL4DevSelFilters* filters, GError **err) {


	/// @todo Assert that filters and filters->list are not NULL, also check err
	
	CL4Platforms* platforms = NULL;
	CL4Platform* platform = NULL;
	CL4Device* device = NULL;
	guint num_platfs;
	guint num_devices;
	GError* err_internal = NULL;
	GPtrArray* devices = NULL;
	
	platforms = cl4_platforms_new(&err_internal);
	/// @todo Check error
	
	num_platfs = cl4_platforms_count(platforms);
	
	devices = g_ptr_array_new_with_free_func(
		(GDestroyNotify) cl4_device_destroy);
	
	for (guint i = 0; i < num_platfs; i++) {
		
		platform = cl4_platforms_get_platform(platforms, i);
		
		num_devices = cl4_platform_device_count(platform, &err_internal);
		/// @todo Check error
		
		for (guint j = 0; j < num_devices; j++) {
			
			device = cl4_platform_get_device(platform, j, &err_internal);
			/// @todo Check error
			
			cl4_device_ref(device);
			
			g_ptr_array_add(devices, (gpointer) device);
			
		} 
		
	}
	
	/* Filter devices. */
	for (guint i = 0; i < devices->len; i++) {
		
		CL4Device* curr_device = (CL4Device*) g_ptr_array_index(devices, i);
		
		/* Check if current device is accepted by all filters. */
		for (guint j = 0; j < filters->len; j++) {

			CL4DevSelFilter* curr_filter = g_ptr_array_index(filters, j);

			gboolean pass = curr_filter->function(curr_device, curr_filter->data, &err_internal);
			/// @todo Check error
			if (!pass) {
				g_ptr_array_remove_index_fast(devices, i);
				i--; /// @todo There is surely a less hacky way to do this
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

	/* Free allocated stuff. */
	cl4_platforms_destroy(platforms);
	

	/// @todo Free filters
	
	return devices;


}





