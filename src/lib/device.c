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
	cl_device_id id;
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
	device->id = id;
		
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
 * @brief Alias for cl4_device_unref().
 *
 * @param device The device wrapper object. 
 * */
void cl4_device_destroy(CL4Device* device) {
	
	cl4_device_unref(device);

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
 * @brief Create a new CL4DeviceInfoValue* object.
 * 
 * @param value Parameter value.
 * @param size Parameter size in bytes.
 * @return A new CL4DeviceInfoValue* object.
 * */
static CL4DeviceInfoValue* cl4_device_info_value_new(
	gpointer value, gsize size) {
	
	CL4DeviceInfoValue* info_value = g_slice_new(CL4DeviceInfoValue);
	
	info_value->value = value;
	info_value->size = size;
	
	return info_value;
	
}

/**
 * @brief Destroy a CL4DeviceInfoValue* object.
 * 
 * @param info_value Object to destroy.
 * */
static void cl4_device_info_value_destroy(
	void* info_value) {
		
	g_free(((CL4DeviceInfoValue*) info_value)->value);
	g_slice_free(CL4DeviceInfoValue, info_value);
}

/**
 * @brief Get device information.
 * 
 * @param device The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested device information. This information will
 * be automatically freed when the device wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
CL4DeviceInfoValue* cl4_device_info(CL4Device* device, 
	cl_device_info param_name, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Device information value object. */
	CL4DeviceInfoValue* info_value = NULL;
	
	/* If device information table is not yet initialized, then 
	 * initialize it. */
	if (!device->info) {
		device->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, 
			NULL, cl4_device_info_value_destroy);
	}

	/* Check if requested information is already present in the 
	 * device information table. */
	if (g_hash_table_contains(
		device->info, GUINT_TO_POINTER(param_name))) {
		
		/* If so, retrieve it from there. */
		info_value = g_hash_table_lookup(
			device->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		/* Otherwise, get it from OpenCL device.*/
		cl_int ocl_status;
		/* Device information placeholder. */
		gpointer param_value;
		/* Size of device information in bytes. */
		gsize size_ret;
		
		/* Get size of information. */
		ocl_status = clGetDeviceInfo(
			device->id, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		gef_if_error_create_goto(*err, CL4_ERROR, 
			size_ret == 0, CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device info [size] (size is 0).",
			__func__);
		
		/* Allocate memory for information. */
		param_value = g_malloc(size_ret);
		
		/* Get information. */
		ocl_status = clGetDeviceInfo(
			device->id, param_name, size_ret, param_value, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
			
		/* Keep information in device information table. */
		info_value = cl4_device_info_value_new(param_value, size_ret);
		g_hash_table_insert(device->info, 
			GUINT_TO_POINTER(param_name), 
			info_value);
		
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	info_value = NULL;

finish:
	
	/* Return the requested device information. */
	return info_value;

}

/**
 * @brief Get the OpenCL device ID object.
 * 
 * @param device The device wrapper object.
 * @return The OpenCL device ID object.
 * */
cl_device_id cl4_device_id(CL4Device* device) {

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Return the OpenCL device ID. */
	return device->id;
}
