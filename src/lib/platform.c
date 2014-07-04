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
	CL4DevContainer base;

};

/** 
 * @addtogroup PLATFORM
 * @{
 */
 
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
	cl4_dev_container_init(&platform->base);
	
	/* Set the platform ID. */
	platform->base.base.cl_object = id;

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
		(cl_platform_id) cl4_dev_container_unref(
			(CL4DevContainer*) platform);
	
	/* If an OpenCL platform was returned, the reference count of the
	 * wrapper object reached 0, so we must destroy remaining platform
	 * wrapper properties. */
	if (platform_id != NULL) {

		/* Free the platform wrapper object. */
		g_slice_free(CL4Platform, platform);
		
	}
	
}

/** @}*/

/** 
 * @brief Implementation of cl4_dev_container_get_cldevices() for the
 * platform wrapper. 
 * 
 * @param devcon A ::CL4Platform wrapper, passed as a ::CL4DevContainer .
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A list of cl_device_id objects inside a ::CL4WrapperInfo
 * object.
 * */
CL4WrapperInfo* cl4_platform_get_cldevices(
	CL4DevContainer* devcon, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure devcon is not NULL. */
	g_return_val_if_fail(devcon != NULL, NULL);
	
	CL4WrapperInfo* info = NULL;
	
	cl_int ocl_status;
	
	GError* err_internal = NULL;
	
	/* Determine number of devices. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object, 
		CL_DEVICE_TYPE_ALL, 0, NULL, &devcon->num_devices);
	gef_if_error_create_goto(*err, CL4_ERROR, 
		CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
		"Function '%s': get number of devices (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
		
	/* Create info object with size in bytes of array of device IDs. */
	info = cl4_wrapper_info_new(
		sizeof(cl_device_id) * devcon->num_devices);
		
	/* Get existing device IDs. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object, 
		CL_DEVICE_TYPE_ALL, devcon->num_devices, info->value, NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, 
		CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
		"Function '%s': get device IDs (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
		
	/* Dirty trick to allow for automatic memory release of this info 
	 * when platform object is destroyed. */
	 
	/* 1 - Make sure info table is initialized by requesting some info. */
	cl4_platform_info(devcon, CL_PLATFORM_NAME, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	/* 2 - Insert device list in info table, so that it will be 
	 * automatically released. */
	g_hash_table_insert(devcon->base.info, GINT_TO_POINTER(-1), info);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
	/* Free info if it was created. */
	if (info != NULL) cl4_wrapper_info_destroy(info);
	
finish:		
	
	/* Terminate function. */
	return info;
}
