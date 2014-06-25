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

	/** OpenCL device ID. */
	cl_device_id cl_object;
	/** Device information. */
	GHashTable* info;
	/** Reference count. */
	gint ref_count;    
	
};

/**
 * @brief Creates a new device wrapper object.
 * 
 * @param id The OpenCL device ID object.
 * @return A new device wrapper object.
 * */
CL4Device* cl4_device_new(cl_device_id id) {
	
	/* The device wrapper object. */
	CL4Device* device;
		
	/* Allocate memory for the device wrapper object. */
	device = g_slice_new(CL4Device);
	
	/* Set the device ID. */
	device->cl_object = id;
		
	/* Device information will be lazy initialized when required. */
	device->info = NULL;

	/* Reference count is one initially. */
	device->ref_count = 1;

	/* Return the new device wrapper object. */
	return device;
	
}

/** 
 * @brief Increase the reference count of the device wrapper object.
 * 
 * @param device The device wrapper object. 
 * */
void cl4_device_ref(CL4Device* device) {
	
	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(device != NULL);
	
	/* Increase reference count. */
	g_atomic_int_inc(&device->ref_count);
	
}

/** 
 * @brief Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed.
 *
 * @param device The device wrapper object. 
 * */
void cl4_device_unref(CL4Device* device) {
	
	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(device != NULL);

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&device->ref_count)) {

		/* Destroy hash table containing device information. */
		if (device->info) {
			g_hash_table_destroy(device->info);
		}
		
		/* Free the device wrapper object. */
		g_slice_free(CL4Device, device);
	}	

}

/**
 * @brief Returns the device wrapper object reference count. For
 * debugging and testing purposes only.
 * 
 * @param device The device wrapper object.
 * @return The device wrapper object reference count or -1 if device
 * is NULL.
 * */
gint cl4_device_ref_count(CL4Device* device) {
	
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, -1);
	
	/* Return reference count. */
	return device->ref_count;

}

/**
 * @brief Get device information object.
 * 
 * @param device The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested device information object. This object will
 * be automatically freed when the device wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
CL4Info* cl4_device_info(CL4Device* device, 
	cl_device_info param_name, GError** err) {

	/* Use the generic get info macro. */
	cl4_info_get(device, param_name, clGetDeviceInfo, err);

}

/** 
 * @brief Get pointer to device information value.
 * 
 * @param device The device wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A pointer to the requested device information value. This 
 * value will be automatically freed when the device wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
gpointer cl4_device_info_value(CL4Device* device, 
	cl_device_info param_name, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Get information wrapper. */
	CL4Info* diw = cl4_device_info(device, param_name, err);
	
	/* Return value if information wrapper is not NULL. */	
	return diw != NULL ? diw->value : NULL;
}

/** 
 * @brief Get device information size.
 * 
 * @param device The device wrapper object.
 * @param param_name Name of information/parameter to get size of.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested device information size. If an error occurs, 
 * a size of 0 is returned.
 * */
 gsize cl4_device_info_size(CL4Device* device, 
	cl_device_info param_name, GError** err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Get information wrapper. */
	CL4Info* diw = cl4_device_info(device, param_name, err);
	
	/* Return value if information wrapper is not NULL. */	
	return diw != NULL ? diw->size : 0;
}


/**
 * @brief Get the OpenCL device ID object.
 * 
 * @param device The device wrapper object.
 * @return The OpenCL device ID object.
 * */
cl_device_id cl4_device_unwrap(CL4Device* device) {

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Return the OpenCL device ID. */
	return device->cl_object;
}
