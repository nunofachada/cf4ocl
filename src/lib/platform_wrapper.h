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
 * Wrapper object for OpenCL platforms. Contains platform and 
 * platform information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_PLATFORM_WRAPPER_H_
#define _CCL_PLATFORM_WRAPPER_H_

#include "common.h"
#include "device_wrapper.h"
#include "gerrorf.h"
#include "errors.h"
#include "abstract_dev_container_wrapper.h"

/**
 * @defgroup PLATFORM_WRAPPER Platform wrapper
 *
 * A wrapper object for OpenCL platforms and functions to manage 
 * them.
 * 
 * Todo: detailed description of module.
 * 
 * @{
 */
 
/** 
 * The platform wrapper class. 
 * 
 * @extends ccl_dev_container
 * */
typedef struct ccl_platform CCLPlatform;

/** Get the platform wrapper for the given OpenCL platform. */
CCLPlatform* ccl_platform_new_wrap(cl_platform_id platform);

/** Get the platform wrapper for the given device wrapper. */
CCLPlatform* ccl_platform_new_from_device(CCLDevice* dev, GError** err);

/** Decrements the reference count of the platform wrapper 
 * object. If it reaches 0, the platform wrapper object is destroyed. */
void ccl_platform_destroy(CCLPlatform* platf);

/** Get numeric OpenCL version of platform. */
double ccl_platform_get_opencl_version(
	CCLPlatform* platf, GError** err);

/**
 * Get a ::CCLWrapperInfo platform information object.
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested platform information object. This object will
 * be automatically freed when the platform wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_platform_get_info(platf, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) platf, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetPlatformInfo, CL_TRUE, err)

/** 
 * Macro which returns a scalar platform information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested platform information value. This value will be 
 * automatically freed when the platform wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define ccl_platform_get_scalar_info(platf, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) platf, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetPlatformInfo, \
		CL_TRUE, err))

/** 
 * Macro which returns an array platform information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested platform information value. This value will be 
 * automatically freed when the platform wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define ccl_platform_get_array_info(platf, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) platf, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetPlatformInfo, \
		CL_TRUE, err)

/**
 * Helper macro which gets a platform information string. This 
 * macro simply wraps the ccl_platform_get_array_info() macro, because
 * (as of OpenCL 2.0) all platform information return types are char*.
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platf The platform wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested platform information string. This information 
 * will be automatically freed when the platform wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_platform_get_info_string(platf, param_name, err) \
	ccl_platform_get_array_info(platf, param_name, char*, err)

/** 
 * Increase the reference count of the platform wrapper object.
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platform The platform wrapper object.
 * */
#define ccl_platform_ref(platform) \
	ccl_wrapper_ref((CCLWrapper*) platform)

/**
 * Alias to ccl_platform_destroy().
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platform Platform wrapper object to destroy if reference
 * count is 1, otherwise just decrement the reference count.
 * */
#define ccl_platform_unref(platform) ccl_platform_destroy(platform)

/**
 * Get the OpenCL platform object.
 * 
 * @public @memberof ccl_platform
 * 
 * @param[in] platform The platform wrapper object.
 * @return The OpenCL platform object.
 * */
#define ccl_platform_unwrap(platform) \
	((cl_platform) ccl_wrapper_unwrap((CCLWrapper*) platform))
	
/** Get all device wrappers in platform. */
CCLDevice* const* ccl_platform_get_all_devices(
	CCLPlatform* platf, GError** err);
 
/** Get ::CCLDevice wrapper at given index. */
CCLDevice* ccl_platform_get_device(
	CCLPlatform* platf, cl_uint index, GError** err);

/** Return number of devices in platform. */
cl_uint ccl_platform_get_num_devices(CCLPlatform* platf, GError** err);
 
/** @} */

#endif
