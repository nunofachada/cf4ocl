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
 * @brief Wrapper object for OpenCL platforms. Contains platform and 
 * platform information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "platform.h"

/**
 * @brief Platform wrapper object.
 */
struct cl4_platform {
	cl_platform_id id;   /**< Platform ID. */
	GHashTable* info;    /**< Platform information. */
	guint num_devices;   /**< Number of devices available in platform. */
	CL4Device** devices; /**< Devices available in platform. */
	gint ref_count;      /**< Reference count. */
};


CL4Platform* cl4_platform_new(cl_platform_id id) {
	
	/* The platform wrapper object. */
	CL4Platform* platform;
		
	/* Allocate memory for the platform wrapper object. */
	platform = g_slice_new(CL4Platform);
	
	/* Set the platform ID. */
	platform->id = id;
		
	/* Platform information will be lazy initialized when required. */
	platform->info = NULL;

	/* Platform devices array will be lazy initialized when required. */
	platform->devices = NULL;
	
	/* Set number of devices to zero, initially. */
	platform->num_devices = 0;
	
	/* Reference count is one initially. */
	platform->ref_count = 1;

	/* Return the new platform wrapper object. */
	return platform;
	
}

void cl4_platform_ref(CL4Platform* platform) {
	g_atomic_int_inc(&platform->ref_count);
}

void cl4_platform_destroy(CL4Platform* platform) {
	
	cl4_platform_unref(platform);

}

void cl4_platform_unref(CL4Platform* platform) {
	
	g_return_if_fail(platform != NULL);

	if (g_atomic_int_dec_and_test(&platform->ref_count)) {
		if (platform->info) {
			g_hash_table_destroy(platform->info);
		}
		if (platform->devices) {
			for (guint i = 0; i < platform->num_devices; i++) {
				cl4_device_destroy(platform->devices[i]);
			}
			g_slice_free1(
				sizeof(CL4Device*) * platform->num_devices, 
				platform->devices);

		}
		
		g_slice_free(CL4Platform, platform);
	}	

}

gchar* cl4_plaform_info(CL4Platform* platform, 
	cl_platform_info param_name, GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	gchar* param_value = NULL;
	
	/* If platform information table is not yet initialized, then 
	 * allocate memory for it. */
	if (!platform->info) {
		platform->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, g_free);
	}

	if (g_hash_table_contains(
		platform->info, GUINT_TO_POINTER(param_name))) {
		
		param_value = g_hash_table_lookup(
			platform->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		cl_int ocl_status;
		size_t size_ret;
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get platform info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		param_value = (gchar*) g_malloc(size_ret);
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, size_ret, param_value, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get platform info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
			
		g_hash_table_insert(
			platform->info, GUINT_TO_POINTER(param_name), param_value);
		
	}
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		

	return param_value;

}

cl_platform_id cl4_platform_id(CL4Platform* platform) {
	return platform->id;
}

CL4Device** cl4_plaform_devices(CL4Platform* platform, GError **err) {
	/// @todo Make this return const

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	if (!platform->devices) {
	
		cl_int ocl_status;
		
		size_t dev_ids_size;
		
		cl_device_id* dev_ids;
		
		/* Determine number of devices. */
		ocl_status = clGetDeviceIDs(platform->id, CL_DEVICE_TYPE_ALL, 0,
			NULL, &platform->num_devices);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get number of devices (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Determine size in bytes of array of platform IDs. */
		dev_ids_size = sizeof(cl_device_id) * platform->num_devices;
		
		/* Allocate memory for array of device IDs. */
		dev_ids = (cl_device_id*) g_slice_alloc(dev_ids_size);
		
		/* Get existing device IDs. */
		ocl_status = clGetDeviceIDs(platform->id, CL_DEVICE_TYPE_ALL, 
			platform->num_devices, dev_ids, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device IDs (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
		
		/* Allocate memory for array of device wrapper objects. */
		platform->devices = g_slice_alloc(
			sizeof(CL4Device*) * platform->num_devices);
	
		/* Wrap device IDs in device wrapper objects. */
		for (guint i = 0; i < platform->num_devices; i++) {
			
			/* Add device wrapper object to array of wrapper objects. */
			platform->devices[i] = cl4_device_new(dev_ids[i]);
		}

		/* Free array of device ids. */
		g_slice_free1(dev_ids_size, dev_ids);
		
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		
	
	return platform->devices;
}

guint cl4_platform_device_count(CL4Platform* platform, GError **err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	if (!platform->devices) {
		cl4_plaform_devices(platform, err);
	}
	/// @todo This is a bug, we might not have the device list... an error may have ocurred
	return platform->num_devices;
}
