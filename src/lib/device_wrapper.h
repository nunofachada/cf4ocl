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

#include "common.h"
#include "abstract_wrapper.h"
#include "errors.h"
#include "oclversions.h"
#include <string.h>

/* Forward declaration of CCLPlatform, as we can't include platform.h
 * here due to circular dependency. */
typedef struct ccl_platform CCLPlatform;

/**
 * @defgroup DEVICE_WRAPPER Device wrapper
 *
 * A wrapper object for OpenCL devices and functions to manage 
 * them.
 * 
 * @todo Detailed description of module with code examples.
 * 
 * @{
 */

/** 
 * Device wrapper class. 
 * 
 * @extends ccl_wrapper
 * */
typedef struct ccl_device CCLDevice;

/* Decrements the reference count of the device wrapper object. 
 * If it reaches 0, the device wrapper object is destroyed. */
void ccl_device_destroy(CCLDevice* dev);

/* Get the device wrapper for the given OpenCL device. */
CCLDevice* ccl_device_new_wrap(cl_device_id device);

/**
 * Get a ::CCLWrapperInfo device information object.
 * 
 * @public @memberof ccl_device
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
	ccl_wrapper_get_info((CCLWrapper*) dev, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetDeviceInfo, CL_TRUE, err)

/** 
 * Macro which returns a scalar device information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_device
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
#define ccl_device_get_scalar_info(dev, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) dev, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetDeviceInfo, \
		CL_TRUE, err))

/** 
 * Macro which returns an array device information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_device
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
#define ccl_device_get_array_info(dev, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) dev, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetDeviceInfo, \
		CL_TRUE, err)

/** 
 * Increase the reference count of the device wrapper object.
 * 
 * @public @memberof ccl_device
 * 
 * @param[in] dev The device wrapper object. 
 * */
#define ccl_device_ref(dev) \
	ccl_wrapper_ref((CCLWrapper*) dev)

/**
 * Alias to ccl_device_destroy().
 * 
 * @public @memberof ccl_device
 * 
 * @param[in] dev Device wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_device_unref(dev) ccl_device_destroy(dev)

/**
 * Get the OpenCL device_id object.
 * 
 * @public @memberof ccl_device
 * 
 * @param[in] dev The device wrapper object.
 * @return The OpenCL device_id object.
 * */
#define ccl_device_unwrap(dev) \
	((cl_device_id) ccl_wrapper_unwrap((CCLWrapper*) dev))
	
/** @} */

#endif
