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
 *
 * Implementation of a wrapper class and its methods for OpenCL platform
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_platform_wrapper.h"
#include "_ccl_abstract_dev_container_wrapper.h"

/**
 * The platform wrapper class.
 *
 * @extends ccl_dev_container
 * */
struct ccl_platform {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLDevContainer base;

};

/**
 * @internal
 * Implementation of ::ccl_dev_container_get_cldevices() for the
 * platform wrapper.
 *
 * @private @memberof ccl_platform
 *
 * @param[in] devcon A ::CCLPlatform wrapper, passed as a
 * ::CCLDevContainer.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
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

	/* Determine number of devices. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object,
		CL_DEVICE_TYPE_ALL, 0, NULL, &devcon->num_devices);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: get number of devices (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Create info object with size in bytes of array of device IDs. */
	info = ccl_wrapper_info_new(
		sizeof(cl_device_id) * devcon->num_devices);

	/* Get existing device IDs. */
	ocl_status = clGetDeviceIDs(devcon->base.cl_object,
		CL_DEVICE_TYPE_ALL, devcon->num_devices, info->value, NULL);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: get device IDs (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Add device list to info table, so that it will be
	 * automatically released. Because the cl_platform_id object
	 * doesn't have a CL_PLATFORM_DEVICES parameter, we keep this info
	 * referenced has CL_CONTEXT_DEVICES. */
	ccl_wrapper_add_info(
		(CCLWrapper*) devcon, CL_CONTEXT_DEVICES, info);

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
 * @addtogroup CCL_PLATFORM_WRAPPER
 * @{
 */

/**
 * Get the platform wrapper for the given OpenCL platform.
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
 * @public @memberof ccl_platform
 *
 * @param[in] platform The OpenCL platform to be wrapped.
 * @return The ::CCLPlatform wrapper for the given OpenCL platform.
 * */
CCL_EXPORT
CCLPlatform* ccl_platform_new_wrap(cl_platform_id platform) {

	return (CCLPlatform*) ccl_wrapper_new(
		(void*) platform, sizeof(CCLPlatform));

}

/**
 * Get the platform wrapper for the given device wrapper.
 *
 * @public @memberof ccl_platform
 *
 * @param[in] dev The device wrapper from where to get a platform
 * wrapper.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The platform wrapper for the given device wrapper or NULL in
 * case an error occurs.
 * */
CCL_EXPORT
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
	platform_id = ccl_device_get_info_scalar(
		dev, CL_DEVICE_PLATFORM, cl_platform_id, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Create/get the platform wrapper. */
	platf = ccl_platform_new_wrap(platform_id);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return the device platform wrapper. */
	return platf;

}

/**
 * Decrements the reference count of the platform wrapper object.
 * If it reaches 0, the platform wrapper object is destroyed.
 *
 * @public @memberof ccl_platform
 *
 * @param[in] platf The platform wrapper object.
 * */
CCL_EXPORT
void ccl_platform_destroy(CCLPlatform* platf) {

	ccl_wrapper_unref((CCLWrapper*) platf, sizeof(CCLPlatform),
		(ccl_wrapper_release_fields) ccl_dev_container_release_devices,
		NULL, NULL);

}

/**
 * Get the OpenCL version of this platform
 *
 * @public @memberof ccl_platform
 *
 * @param[in] platf The platform wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return OpenCL version of platform as an integer. If an error
 * occurs, 0 is returned.
 * */
CCL_EXPORT
cl_uint ccl_platform_get_opencl_version(
	CCLPlatform* platf, GError** err) {

	/* Make sure platf is not NULL. */
	g_return_val_if_fail(platf != NULL, 0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

	char* ver_str;
	cl_uint ver = 0;

	/* Get version string which has the format "OpenCL x.x ..." */
	ver_str = ccl_platform_get_info_string(
		platf, CL_PLATFORM_VERSION, err);

	if (ver_str != NULL) {
		ver = /* strlen("OpenCL ") == 7 */
			atoi(ver_str + 7) * 100 + /* Major version. */
			atoi(ver_str + 9) * 10; /* Minor version. */
	}
	return ver;
}

/**
 * Get all device wrappers in platform.
 *
 * This function returns the internal array containing the platform
 * device wrappers. As such, clients should not modify the returned
 * array (e.g. they should not free it directly).
 *
 * @public @memberof ccl_platform
 *
 * @param[in] platf The platform wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An array containing the ::CCLDevice wrappers which belong to
 * this platform, or `NULL` if an error occurs.
 */
CCLDevice* const* ccl_platform_get_all_devices(
	CCLPlatform* platf, GError** err) {

	return ccl_dev_container_get_all_devices(
		(CCLDevContainer*) platf, ccl_platform_get_cldevices, err);
}

/**
 * Get ::CCLDevice wrapper at given index.
 *
 * @public @memberof ccl_platform
 *
 * @param[in] platf The platform wrapper object.
 * @param[in] index Index of device in platform.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLDevice wrapper at given index or `NULL` if an error
 * occurs.
 * */
CCL_EXPORT
CCLDevice* ccl_platform_get_device(
	CCLPlatform* platf, cl_uint index, GError** err) {

	return ccl_dev_container_get_device((CCLDevContainer*) platf,
		ccl_platform_get_cldevices, index, err);

}

/**
 * Return number of devices in platform.
 *
 * @public @memberof ccl_platform
 *
 * @param[in] platf The platform wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The number of devices in platform or 0 if an error occurs or
 * is otherwise not possible to get any device.
 * */
CCL_EXPORT
cl_uint ccl_platform_get_num_devices(CCLPlatform* platf, GError** err) {

	return ccl_dev_container_get_num_devices((CCLDevContainer*) platf,
		ccl_platform_get_cldevices, err);

}

/** @}*/
