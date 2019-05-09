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
 * Definition of a wrapper class and its methods for OpenCL platform objects.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_PLATFORM_WRAPPER_H_
#define _CCL_PLATFORM_WRAPPER_H_

#include "ccl_common.h"
#include "ccl_device_wrapper.h"
#include "ccl_errors.h"

/**
 * @defgroup CCL_PLATFORM_WRAPPER Platform wrapper
 *
 * The platform wrapper module provides functionality for simple handling of
 * OpenCL platform objects.
 *
 * Platform wrappers can be obtained in two ways:
 *
 * 1. From a given ::CCLDevice* device wrapper, using the
 * ::ccl_platform_new_from_device() function (in which case, the object
 * must be destroyed with ::ccl_platform_destroy(), following the
 * _cf4ocl_ @ref ug_new_destroy "new/destroy" rule).
 * 2. From a ::CCLPlatforms* object (which contains the list of platforms
 * in the system), using the ::ccl_platforms_get() function. In this
 * case the platform wrapper is automatically destroyed when the
 * ::CCLPlatforms* object is freed; as such, client code should not
 * directly destroy the platform wrapper. See the
 * @ref CCL_PLATFORMS "platforms module" for more details.
 *
 * The provided functions allow to get the device wrappers associated
 * with the given platform wrapper, as well as to obtain the supported
 * OpenCL version of the underlying OpenCL platform object.
 *
 * Information about platform objects can be fetched using the
 * platform @ref ug_getinfo "info macros":
 *
 * * ::ccl_platform_get_info_scalar()
 * * ::ccl_platform_get_info_array()
 * * ::ccl_platform_get_info()
 *
 * However, because the platform info return type is always `char *`,
 * _cf4ocl_ provides an additional helper macro for platform wrappers,
 * ::ccl_platform_get_info_string(), which is simpler to use.
 *
 * The ::CCLPlatform* class extends the ::CCLDevContainer* class; as
 * such, it provides methods for handling a list of devices associated
 * with the platform:
 *
 * * ::ccl_platform_get_all_devices()
 * * ::ccl_platform_get_device()
 * * ::ccl_platform_get_num_devices()
 *
 * _Usage example:_
 *
 * ```c
 * CCLPlatform * platf;
 * CCLDevice * dev;
 * char * platf_name;
 * cl_uint platf_ver;
 * ```
 *
 * ```c
 * platf = ccl_platform_new_from_device(dev, NULL);
 * ```
 *
 * ```c
 * platf_name = ccl_platform_get_info_string(platf, CL_PLATFORM_NAME, NULL);
 * ```
 *
 * ```c
 * platf_ver = ccl_platform_get_opencl_version(platf, NULL);
 * ```
 *
 * ```c
 * printf("Platform name is %s\n: ", platf_name);
 * printf("Platform version is %f\n: ", platf_ver / 100.0f);
 * ```
 *
 * ```c
 * ccl_platform_destroy(platf);
 * ```
 *
 * @{
 */

/* Get the platform wrapper for the given OpenCL platform. */
CCL_EXPORT
CCLPlatform * ccl_platform_new_wrap(cl_platform_id platform);

/* Get the platform wrapper for the given device wrapper. */
CCL_EXPORT
CCLPlatform * ccl_platform_new_from_device(CCLDevice * dev, CCLErr ** err);

/* Decrements the reference count of the platform wrapper
 * object. If it reaches 0, the platform wrapper object is destroyed. */
CCL_EXPORT
void ccl_platform_destroy(CCLPlatform * platf);

/* Get integer OpenCL version of platform. */
CCL_EXPORT
cl_uint ccl_platform_get_opencl_version(CCLPlatform * platf, CCLErr ** err);

/**
 * Get a ::CCLWrapperInfo platform information object.
 *
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested platform information object. This object will
 * be automatically freed when the platform wrapper object is
 * destroyed. If an error occurs, `NULL` is returned.
 * */
#define ccl_platform_get_info(platf, param_name, err) \
    ccl_wrapper_get_info((CCLWrapper *) platf, NULL, param_name, 0, \
        CCL_INFO_PLATFORM, CL_FALSE, err)

/**
 * Macro which returns a scalar platform information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. `cl_uint`, `size_t`, etc.).
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested platform information value. This value will be
 * automatically freed when the platform wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_platform_get_info_scalar(platf, param_name, param_type, err) \
    *((param_type *) ccl_wrapper_get_info_value((CCLWrapper *) platf, \
        NULL, param_name, sizeof(param_type), \
        CCL_INFO_PLATFORM, CL_FALSE, err))

/**
 * Macro which returns an array platform information value.
 *
 * Use with care. In case an error occurs, `NULL` is returned, which
 * might be ambiguous if `NULL` is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter in array (e.g. `char`, `size_t`,
 * etc.).
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested platform information value. This value will be
 * automatically freed when the platform wrapper object is destroyed.
 * If an error occurs, `NULL` is returned.
 * */
#define ccl_platform_get_info_array(platf, param_name, param_type, err) \
    (param_type *) ccl_wrapper_get_info_value((CCLWrapper *) platf, \
        NULL, param_name, sizeof(param_type), \
        CCL_INFO_PLATFORM, CL_FALSE, err)

/**
 * Helper macro which gets a platform information string. This
 * macro simply wraps the ccl_platform_get_info_array() macro, because
 * (as of OpenCL 2.0) all platform information return types are `char *`.
 *
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested platform information string. This information
 * will be automatically freed when the platform wrapper object is
 * destroyed. If an error occurs, `NULL` is returned.
 * */
#define ccl_platform_get_info_string(platf, param_name, err) \
    ccl_platform_get_info_array(platf, param_name, char, err)

/**
 * Increase the reference count of the platform wrapper object.
 *
 * @param[in] platform The platform wrapper object.
 * */
#define ccl_platform_ref(platform) ccl_wrapper_ref((CCLWrapper *) platform)

/**
 * Alias to ccl_platform_destroy().
 *
 * @param[in] platform Platform wrapper object to destroy if reference
 * count is 1, otherwise just decrement the reference count.
 * */
#define ccl_platform_unref(platform) ccl_platform_destroy(platform)

/**
 * Get the OpenCL platform object.
 *
 * @param[in] platform The platform wrapper object.
 * @return The OpenCL platform object.
 * */
#define ccl_platform_unwrap(platform) \
    ((cl_platform_id) ccl_wrapper_unwrap((CCLWrapper *) platform))

/* Get all device wrappers in platform. */
CCL_EXPORT
CCLDevice * const * ccl_platform_get_all_devices(
    CCLPlatform * platf, CCLErr ** err);

/* Get ::CCLDevice wrapper at given index. */
CCL_EXPORT
CCLDevice * ccl_platform_get_device(
    CCLPlatform * platf, cl_uint index, CCLErr ** err);

/* Return number of devices in platform. */
CCL_EXPORT
cl_uint ccl_platform_get_num_devices(CCLPlatform * platf, CCLErr ** err);

/** @} */

#endif
