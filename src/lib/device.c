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
 * @brief Wrapper object for OpenCL devices. Contains device and device
 * information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "device.h"

/**
 * @brief Device wrapper object.
 */
struct cl4_device {

	cl_device_id id;
	GHashTable* info;  /**< Device information. */
	gint ref_count;    /**< Reference count. */
	
};


CL4Device* cl4_device_new(cl_device_id id) {
	
	/* The device wrapper object. */
	CL4Device* device;
		
	/* Allocate memory for the device wrapper object. */
	device = g_slice_new(CL4Device);
	
	/* Set the device ID. */
	device->id = id;
		
	/* Device information will be lazy initialized when required. */
	device->info = NULL;

	/* Reference count is one initially. */
	device->ref_count = 1;

	/* Return the new device wrapper object. */
	return device;
	
}

void cl4_device_ref(CL4Device* device) {
	g_atomic_int_inc(&device->ref_count);
}

void cl4_device_destroy(CL4Device* device) {
	
	cl4_device_unref(device);

}

void cl4_device_unref(CL4Device* device) {
	
	g_return_if_fail (device != NULL);

	if (g_atomic_int_dec_and_test(&device->ref_count)) {
		if (device->info) {
			g_hash_table_destroy(device->info);
		}
		
		g_slice_free(CL4Device, device);
	}	

}

gpointer cl4_device_get_info(CL4Device* device, 
	cl_device_info param_name) {

	gpointer param_value;
	
	/* If device information table is not yet initialized, then 
	 * allocate memory for it. */
	if (!device->info) {
		device->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, g_free);
	}

	if (g_hash_table_contains(
		device->info, GUINT_TO_POINTER(param_name))) {
		
		param_value = g_hash_table_lookup(
			device->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		cl_int ocl_status;
		size_t size_ret;
		
		ocl_status = clGetDeviceInfo(
			device->id, param_name, 0, NULL, &size_ret);
		/// @todo verify status here and in platform
		
		param_value = g_malloc(size_ret);
		
		ocl_status = clGetDeviceInfo(
			device->id, param_name, size_ret, param_value, NULL);
			
		g_hash_table_insert(
			device->info, GUINT_TO_POINTER(param_name), param_value);
		
	}
	
	return param_value;

}

cl_device_id cl4_device_get_cl_device_id(CL4Device* device) {
	return device->id;
}

