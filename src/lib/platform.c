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

	/** Parent wrapper object. */
	CL4Wrapper base;

	/** Number of devices available in platform. */
	guint num_devices;

	/** Devices available in platform. */
	CL4Device** devices;

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
		ocl_status = clGetDeviceIDs(platform->base.cl_object, 
			CL_DEVICE_TYPE_ALL, 0, NULL, &platform->num_devices);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
			"Function '%s': get number of devices (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Determine size in bytes of array of device IDs. */
		dev_ids_size = sizeof(cl_device_id) * platform->num_devices;
		
		/* Allocate memory for array of device IDs. */
		dev_ids = (cl_device_id*) g_slice_alloc(dev_ids_size);
		
		/* Get existing device IDs. */
		ocl_status = clGetDeviceIDs(platform->base.cl_object, 
			CL_DEVICE_TYPE_ALL, platform->num_devices, dev_ids, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
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
	
	/* Initialize parent object. */
	cl4_wrapper_init(&platform->base);
	
	/* Set the platform ID. */
	platform->base.cl_object = id;

	/* Platform devices array will be lazy initialized when required. */
	platform->devices = NULL;
	
	/* Set number of devices to zero, initially. */
	platform->num_devices = 0;

	/* Return the new platform wrapper object. */
	return platform;
	
}

/** 
 * @brief Decrements the reference count of the platform wrapper object.
 * If it reaches 0, the platform wrapper object is destroyed.
 *
 * @param platform The platform wrapper object. 
 * */
 void cl4_platform_destroy(CL4Platform* platform) {
	
	/* Make sure platform wrapper object is not NULL. */
	g_return_if_fail(platform != NULL);
	
	/* Wrapped OpenCL object (a platform_id in this case), returned by 
	 * the parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_platform_id platform_id;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	platform_id = 
		(cl_platform_id) cl4_wrapper_unref((CL4Wrapper*) platform);
	
	/* If an OpenCL platform was returned, the reference count of the
	 * wrapper object reached 0, so we must destroy remaining platform
	 * wrapper properties. */
	if (platform_id != NULL) {

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
guint cl4_platform_num_devices(CL4Platform* platform, GError **err) {
	
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
