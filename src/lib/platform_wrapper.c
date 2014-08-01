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
 
#include "platform_wrapper.h"

/**
 * @brief Platform wrapper object.
 */
struct ccl_platform {

	/** Parent wrapper object. */
	CCLDevContainer base;

};

/** 
 * @brief Implementation of ccl_dev_container_get_cldevices() for the
 * platform wrapper. 
 * 
 * @param devcon A ::CCLPlatform wrapper, passed as a ::CCLDevContainer .
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A list of cl_device_id objects inside a ::CCLWrapperInfo
 * object.
 * */
static CCLWrapperInfo* ccl_platform_get_cldevices(
	CCLDevContainer* devcon, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure devcon is not NULL. */
	g_return_val_if_fail(devcon != NULL, NULL);
	
	CCLWrapperInfo* info = NULL;
	
	cl_int ocl_status;
	
	GError* err_internal = NULL;
	
	/* Determine number of devices. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object, 
		CL_DEVICE_TYPE_ALL, 0, NULL, &devcon->num_devices);
	gef_if_err_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: get number of devices (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
		
	/* Create info object with size in bytes of array of device IDs. */
	info = ccl_wrapper_info_new(
		sizeof(cl_device_id) * devcon->num_devices);
		
	/* Get existing device IDs. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object, 
		CL_DEVICE_TYPE_ALL, devcon->num_devices, info->value, NULL);
	gef_if_err_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: get device IDs (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
		
	/* Dirty trick to allow for automatic memory release of this info 
	 * when platform object is destroyed. */
	 
	/* 1 - Make sure info table is initialized by requesting some info. */
	ccl_platform_get_info_string(devcon, CL_PLATFORM_NAME, &err_internal);
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
	if (info != NULL) ccl_wrapper_info_destroy(info);
	
finish:		
	
	/* Terminate function. */
	return info;
}

/** 
 * @addtogroup PLATFORM_WRAPPER
 * @{
 */

/**
 * @brief Get the platform wrapper for the given OpenCL platform.
 * 
 * If the wrapper doesn't exist, its created with a reference count 
 * of 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL platform directly (using the
 * clGetPlatformIDs() function) and then wrap the OpenCL platform in a 
 * ::CCLPlatform wrapper object.
 * 
 * @param platform The OpenCL platform to be wrapped.
 * @return The ::CCLPlatform wrapper for the given OpenCL platform.
 * */
CCLPlatform* ccl_platform_new_wrap(cl_platform_id platform) {
	
	return (CCLPlatform*) ccl_wrapper_new(
		(void*) platform, sizeof(CCLPlatform));
		
}

/** 
 * @brief Get the platform wrapper for the given device wrapper. 
 * 
 * @param dev The device wrapper from where to get a platform wrapper.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The platform wrapper for the given device wrapper or NULL in
 * case an error occurs. 
 * */
CCLPlatform* ccl_platform_new_from_device(CCLDevice* dev, GError** err) {

	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* The OpenCL platform_id object. */
	cl_platform_id platform_id;
	/* The platform wrapper to return. */
	CCLPlatform* platf = NULL;
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Get OpenCL platform_id object from device. */
	platform_id = ccl_device_get_scalar_info(
		dev, CL_DEVICE_PLATFORM, cl_platform_id, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);

	/* Create/get the platform wrapper. */
	platf = ccl_platform_new_wrap(platform_id);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:		

	/* Return the device platform wrapper. */
	return platf;	

}

/** 
 * @brief Decrements the reference count of the platform wrapper object. 
 * If it reaches 0, the platform wrapper object is destroyed.
 *
 * @param platf The platform wrapper object.
 * */
void ccl_platform_destroy(CCLPlatform* platf) {
	
	ccl_wrapper_unref((CCLWrapper*) platf, sizeof(CCLPlatform),
		(ccl_wrapper_release_fields) ccl_dev_container_release_devices, 
		NULL, NULL); 

}

/** 
 * @brief Get all device wrappers in platform. 
 * 
 * This function returns the internal array containing the platform
 * device wrappers. As such, clients should not modify the returned 
 * array (e.g. they should not free it directly).
 * 
 * @param platf The platform wrapper object.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return An array containing the ::CCLDevice wrappers which belong to
 * this platform, or NULL if an error occurs.
 */
const CCLDevice** ccl_platform_get_all_devices(
	CCLPlatform* platf, GError** err) {
	
	return ccl_dev_container_get_all_devices(
		(CCLDevContainer*) platf, ccl_platform_get_cldevices, err);
}
 
/** 
 * @brief Get ::CCLDevice wrapper at given index. 
 * 
 * @param platf The platform wrapper object.
 * @param index Index of device in platform.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The ::CCLDevice wrapper at given index or NULL if an error 
 * occurs.
 * */
CCLDevice* ccl_platform_get_device(
	CCLPlatform* platf, cl_uint index, GError** err) {

	return ccl_dev_container_get_device((CCLDevContainer*) platf,
		ccl_platform_get_cldevices, index, err);

}

/**
 * @brief Return number of devices in platform.
 * 
 * @param platf The platform wrapper object.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The number of devices in platform or 0 if an error occurs or 
 * is otherwise not possible to get any device.
 * */
cl_uint ccl_platform_get_num_devices(CCLPlatform* platf, GError** err) {

	return ccl_dev_container_get_num_devices((CCLDevContainer*) platf,
		ccl_platform_get_cldevices, err);

}

/** @}*/
