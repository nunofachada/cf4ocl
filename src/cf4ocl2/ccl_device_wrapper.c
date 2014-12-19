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
 * Implementation of a wrapper class and its methods for OpenCL device
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 *
 * */

#include "ccl_device_wrapper.h"
#include "ccl_platform_wrapper.h"
#include "_ccl_abstract_wrapper.h"

/**
 * Device wrapper class.
 *
 * @extends ccl_wrapper
 *
 * @todo Add support for sub-devices.
 */
struct ccl_device {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

#ifdef CL_VERSION_1_2
	/**
	 * List of sub-device arrays.
	 * @private
	 * */
	GSList* subdev_arrays;
#endif

};

#ifdef CL_VERSION_1_2

/**
 * @internal
 * Release an array of sub-devices and the respective sub-devices.
 *
 * @private @memberof ccl_device
 *
 * @param[in] data An array of sub-devices to be released.
 * */
static void ccl_device_release_subdev_arrays(gpointer data) {

	/* Make sure data is not NULL. */
	g_return_if_fail(data != NULL);

	/* Convert data to array of sub-devices. */
	CCLDevice** subdevs = (CCLDevice**) data;

	/* Destroy sub-devices. */
	for (cl_uint i = 0; subdevs[i] != NULL; ++i)
		ccl_device_destroy(subdevs[i]);

	/* Destroy array holding sub-devices. */
	g_free(subdevs);
}

/**
 * @internal
 * Implementation of ccl_wrapper_release_fields() function for
 * ::CCLDevice wrapper objects.
 *
 * @private @memberof ccl_device
 *
 * @param[in] dev A ::CCLDevice wrapper object.
 * */
static void ccl_device_release_fields(CCLDevice* dev) {

	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(dev != NULL);

	/* Release list of arrays of sub-devices. */
	g_slist_free_full(dev->subdev_arrays,
		ccl_device_release_subdev_arrays);
}

#endif

/**
 * @addtogroup CCL_DEVICE_WRAPPER
 * @{
 */

/**
 * Get the device wrapper for the given OpenCL device.
 *
 * If the wrapper doesn't exist, it's created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to get the OpenCL device directly (using the
 * clGetDeviceIDs() function) and then wrap the OpenCL device in a
 * ::CCLDevice wrapper object.
 *
 * @public @memberof ccl_device
 *
 * @param[in] device The OpenCL device to be wrapped.
 * @return The device wrapper for the given OpenCL device.
 * */
CCL_EXPORT
CCLDevice* ccl_device_new_wrap(cl_device_id device) {

	return (CCLDevice*) ccl_wrapper_new(
		(void*) device, sizeof(CCLDevice));

}

/**
 * Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed.
 *
 * @public @memberof ccl_device
 *
 * @param[in] dev The device wrapper object.
 * */
CCL_EXPORT
void ccl_device_destroy(CCLDevice* dev) {

#ifdef CL_VERSION_1_2
	ccl_wrapper_unref((CCLWrapper*) dev, sizeof(CCLDevice),
		(ccl_wrapper_release_fields) ccl_device_release_fields,
		(ccl_wrapper_release_cl_object) clReleaseDevice, NULL);
#else
	ccl_wrapper_unref((CCLWrapper*) dev, sizeof(CCLDevice),
		NULL, NULL, NULL);
#endif
}

#ifdef CL_VERSION_1_2
/**
 * Creates a `NULL`-terminated array of sub-devices that each reference
 * a non-intersecting set of compute units within the given parent
 * device.
 *
 * @public @memberof ccl_device
 *
 * @param[in] dev The parent device to be partitioned.
 * @param[in] properties Specifies how `dev` is to be partitioned. See
 * the documentation for the clCreateSubDevices() function.
 * @param[out] num_devs_ret Returns how many sub-devices were created
 * in the returned array of sub-devices. If `NULL`, this parameter is
 * ignored and client code will have to check for the `NULL` termination
 * in the returned array.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A `NULL`-terminated array of sub-devices. Sub-devices will
 * be automatically disposed of when parent device is destroyed.
 * */
CCL_EXPORT
CCLDevice* const* ccl_device_create_subdevices(CCLDevice* dev,
	const cl_device_partition_property *properties,
	cl_uint *num_devs_ret, GError** err) {

	/* Make sure device wrapper object is not NULL. */
	g_return_val_if_fail(dev != NULL, NULL);

	/* OpenCL function return status. */
	cl_int ocl_status;
	/* Number of sub-devices the partition may yield. */
	cl_uint num_devs;
	/* Array for holding sub-devices. */
	void** subdevs = NULL;

	/* How many sub-devices will this partition yield? */
	ocl_status = clCreateSubDevices(
		ccl_device_unwrap(dev), properties, 0, NULL, &num_devs);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to determine how many sub-devices can be created " \
		"(OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Create array for holding sub-devices. The last element should
	 * be NULL. */
	subdevs = g_new0(void*, num_devs + 1);

	/* Create sub-devices. */
	ocl_status = clCreateSubDevices(
		ccl_device_unwrap(dev), properties, num_devs,
		(cl_device_id*) subdevs, NULL);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create sub-devices (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Wrap sub-devices. */
	for (cl_uint i = 0; subdevs[i] != NULL; ++i)
		subdevs[i] = ccl_device_new_wrap(subdevs[i]);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	if (num_devs_ret != NULL) *num_devs_ret = num_devs;
	dev->subdev_arrays = g_slist_prepend(dev->subdev_arrays, subdevs);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	if (num_devs_ret != NULL) *num_devs_ret = 0;
	if (subdevs != NULL) g_free(subdevs);
	subdevs = NULL;

finish:

	/* Return event. */
	return (CCLDevice* const*) subdevs;
}
#endif

/** @} */
