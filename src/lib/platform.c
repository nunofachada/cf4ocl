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
	/** Platform ID. */
	cl_platform_id id;
	/** Platform information. */
	GHashTable* info;
	/** Number of devices available in platform. */
	guint num_devices;
	/** Devices available in platform. */
	CL4Device** devices;
	/** Reference count. */
	gint ref_count;
};

/** 
 * @brief Initialize internal device list of platform wrapper object. 
 * 
 * @param platform The platform wrapper object.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * */
static void cl4_plaform_init_devices(
	CL4Platform* platform, GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, NULL);
	
	/* Check if device list is already initialized. */
	if (!platform->devices) {
		/* Not initialized, initialize it. */
		
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
	
	/* Terminate function. */
	return;
}

/**
 * @brief Creates a new platform wrapper object.
 * 
 * @param id The OpenCL platform ID object.
 * @return A new platform wrapper object.
 * */
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

/** 
 * @brief Increase the reference count of the platform wrapper object.
 * 
 * @param platform The platform wrapper object. 
 * */
void cl4_platform_ref(CL4Platform* platform) {
	
	/* Make sure platform wrapper object is not NULL. */
	g_return_if_fail(platform != NULL);

	/* Increase reference count. */
	g_atomic_int_inc(&platform->ref_count);
	
}

/** 
 * @brief Alias for cl4_platform_unref().
 *
 * @param platform The platform wrapper object. 
 * */
void cl4_platform_destroy(CL4Platform* platform) {
	
	cl4_platform_unref(platform);

}

/** 
 * @brief Decrements the reference count of the platform wrapper object.
 * If it reaches 0, the platform wrapper object is destroyed.
 *
 * @param platform The platform wrapper object. 
 * */
 void cl4_platform_unref(CL4Platform* platform) {
	
	/* Make sure platform wrapper object is not NULL. */
	g_return_if_fail(platform != NULL);

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&platform->ref_count)) {
		
		/* Destroy hash table containing platform information. */
		if (platform->info) {
			g_hash_table_destroy(platform->info);
		}
		
		/* Reduce reference count of devices in device list, free the
		 * device list. */
		if (platform->devices) {
			
			/* Reduce reference count of devices in device list. */
			for (guint i = 0; i < platform->num_devices; i++) {
				cl4_device_unref(platform->devices[i]);
			}
			
			/* Free the device list. */
			g_slice_free1(
				sizeof(CL4Device*) * platform->num_devices, 
				platform->devices);

		}
		
		/* Free the platform wrapper object. */
		g_slice_free(CL4Platform, platform);
	}	

}

/**
 * @brief Returns the platform wrapper object reference count. For
 * debugging and testing purposes only.
 * 
 * @param platform The platform wrapper object.
 * @return The platform wrapper object reference count or -1 if platform
 * is NULL.
 * */
gint cl4_platform_ref_count(CL4Platform* platform) {

	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, -1);
	
	/* Return the reference count. */
	return platform->ref_count;

}

/**
 * @brief Get platform information.
 * 
 * @param platform The platform wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested platform information. This information will
 * be automatically freed when the platform wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
gchar* cl4_plaform_info(CL4Platform* platform, 
	cl_platform_info param_name, GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, NULL);
	
	/* Platform information placeholder. */
	gchar* param_value = NULL;
	
	/* If platform information table is not yet initialized, then 
	 * initialize it. */
	if (!platform->info) {
		platform->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, g_free);
	}

	/* Check if requested information is already present in the 
	 * platform information table. */
	if (g_hash_table_contains(
		platform->info, GUINT_TO_POINTER(param_name))) {
		
		/* If so, retrieve it from there. */
		param_value = g_hash_table_lookup(
			platform->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		/* Otherwise, get it from OpenCL platform.*/
		cl_int ocl_status;
		size_t size_ret;
		
		/* Get size of information. */
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get platform info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Allocate memory for information. */
		param_value = (gchar*) g_malloc(size_ret);
		
		/* Get information. */
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, size_ret, param_value, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
			CL4_OCL_ERROR, error_handler, 
			"Function '%s': get platform info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Keep information in platform information table. */
		g_hash_table_insert(
			platform->info, GUINT_TO_POINTER(param_name), param_value);
		
	}
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	param_value = NULL;
	
finish:		

	/* Return the requested platform information. */
	return param_value;

}

/**
 * @brief Get the OpenCL platform ID object.
 * 
 * @param platform The platform wrapper object.
 * @return The OpenCL platform ID object.
 * */
cl_platform_id cl4_platform_id(CL4Platform* platform) {
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, NULL);
	
	/* Return the OpenCL platform ID. */
	return platform->id;
}
 
/** 
 * @brief Get CL4 device wrapper at given index. 
 * 
 * @param platform The platform wrapper object.
 * @param index Index of device in platform.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The CL4 device wrapper at given index or NULL if an error 
 * occurs.
 * */
CL4Device* cl4_platform_get_device(
	CL4Platform* platform, guint index, GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, NULL);
	
	/* The return value. */
	CL4Device* device_ret;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Check if device list is already initialized. */
	if (!platform->devices) {
		
		/* Not initialized, initialize it. */
		cl4_plaform_init_devices(platform, &err_internal);
		
		/* Check for errors. */
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
	}

	/* Make sure device index is less than the number of devices. */
	g_return_val_if_fail(index < platform->num_devices, NULL);	

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	device_ret = platform->devices[index];
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	device_ret = NULL;
	
finish:		
	
	/* Return list of device wrappers. */
	return device_ret;
}

/**
 * @brief Return number of devices in platform.
 * 
 * @param platform The platform wrapper object.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The number of devices in platform or 0 if an error occurs or
 * is otherwise not possible to get any device.
 * */
guint cl4_platform_device_count(CL4Platform* platform, GError **err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(platform != NULL, 0);
	
	/* Check if device list is already initialized. */
	if (!platform->devices) {
		
		/* Not initialized, initialize it. */
		cl4_plaform_init_devices(platform, err);
		
	}
	
	/* Return the number of devices in platform. */
	return platform->num_devices;
}
