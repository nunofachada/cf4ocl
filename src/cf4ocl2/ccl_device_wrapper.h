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
 * Definition of a wrapper class and its methods for OpenCL device
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_DEVICE_WRAPPER_H_
#define _CCL_DEVICE_WRAPPER_H_

#include "ccl_common.h"
#include "ccl_abstract_wrapper.h"
#include "ccl_errors.h"
#include "ccl_oclversions.h"
#include <string.h>

/**
 * @defgroup CCL_DEVICE_WRAPPER Device wrapper
 *
 * The device wrapper module provides functionality for simple
 * handling of OpenCL device objects.
 *
 * In most cases, device wrapper objects should not be directly
 * instanced by client code. They are usually fetched from
 * @ref CCLDevContainer "device container" objects such as
 * ::CCLPlatform* or ::CCLContext* instances, or created as sub-devices
 * using the ::ccl_device_create_subdevices() function. In either case,
 * when the parent object is destroyed, the associated devices (or
 * sub-devices) are also destroyed. As such, and in accordance with the
 * _cf4ocl_ @ref ug_new_destroy "new/destroy" rule, the
 * ::ccl_device_destroy() destructor function will rarely be used.
 *
 * Information about device objects can be fetched using the
 * device @ref ug_getinfo "info macros":
 *
 * * ::ccl_device_get_info_scalar()
 * * ::ccl_device_get_info_array()
 * * ::ccl_device_get_info()
 *
 * _Example: getting the first device in a context_
 *
 * @code{.c}
 * CCLContext* ctx;
 * CCLDevice* dev;
 * @endcode
 * @code{.c}
 * dev = ccl_context_get_device(ctx, 0, NULL);
 * @endcode
 *
 * @{
 */

/* Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed. */
CCL_EXPORT
void ccl_device_destroy(CCLDevice* dev);

/* Get the device wrapper for the given OpenCL device. */
CCL_EXPORT
CCLDevice* ccl_device_new_wrap(cl_device_id device);

/* Get the OpenCL version of the device. */
CCL_EXPORT
cl_uint ccl_device_get_opencl_c_version(CCLDevice* dev, GError** err);

#ifdef CL_VERSION_1_2

/* Creates an array of sub-devices that each reference a
 * non-intersecting set of compute units within the given device. */
CCL_EXPORT
CCLDevice* const* ccl_device_create_subdevices(CCLDevice* dev,
	const cl_device_partition_property *properties,
	cl_uint *num_devs_ret, GError** err);

#endif

/**
 * Get a ::CCLWrapperInfo device information object.
 *
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested device information object. This object will
 * be automatically freed when the device wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_device_get_info(dev, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) dev, NULL, param_name, 0, \
		(ccl_wrapper_info_fp) clGetDeviceInfo, CL_FALSE, err)

/**
 * Macro which returns a scalar device information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested device information value. This value will be
 * automatically freed when the device wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_device_get_info_scalar(dev, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) dev, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetDeviceInfo, CL_FALSE, err))

/**
 * Macro which returns an array device information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested device information value. This value will be
 * automatically freed when the device wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_device_get_info_array(dev, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) dev, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetDeviceInfo, CL_FALSE, err)

/**
 * Increase the reference count of the device wrapper object.
 *
 * @param[in] dev The device wrapper object.
 * */
#define ccl_device_ref(dev) \
	ccl_wrapper_ref((CCLWrapper*) dev)

/**
 * Alias to ccl_device_destroy().
 *
 * @param[in] dev Device wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_device_unref(dev) ccl_device_destroy(dev)

/**
 * Get the OpenCL device_id object.
 *
 * @param[in] dev The device wrapper object.
 * @return The OpenCL device_id object.
 * */
#define ccl_device_unwrap(dev) \
	((cl_device_id) ccl_wrapper_unwrap((CCLWrapper*) dev))

/** @} */

#endif
