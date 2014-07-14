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
 
#ifndef CL4_PLATFORM_H
#define CL4_PLATFORM_H

#include "common.h"
#include "device.h"
#include "gerrorf.h"
#include "errors.h"
#include "abstract_dev_container_wrapper.h"

/**
 * @defgroup PLATFORM The platform wrapper module.
 *
 * @brief A wrapper object for OpenCL platforms and functions to manage 
 * it.
 * 
 * Todo: detailed description of module.
 * 
 * @{
 */
 
/** @brief The platform wrapper object. */
typedef struct cl4_platform CL4Platform;

/** @brief Get the platform wrapper for the given OpenCL platform. */
CL4Platform* cl4_platform_new_wrap(cl_platform_id platform);

/** @brief Decrements the reference count of the platform wrapper 
 * object. If it reaches 0, the platform wrapper object is destroyed. */
void cl4_platform_destroy(CL4Platform* platf);

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
#define cl4_platform_info(platform, param_name, err) \
	((gchar*) cl4_wrapper_get_info_value((CL4Wrapper*) platform, NULL, \
		param_name, (cl4_wrapper_info_fp) clGetPlatformInfo, CL_TRUE, \
		err))

/** 
 * @brief Increase the reference count of the platform wrapper object.
 * 
 * @param platform The platform wrapper object.
 * */
#define cl4_platform_ref(platform) \
	cl4_wrapper_ref((CL4Wrapper*) platform)

/**
 * @brief Alias to cl4_platform_destroy().
 * 
 * @param platform Platform wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define cl4_platform_unref(platform) cl4_platform_destroy(platform)

/**
 * @brief Get the OpenCL platform object.
 * 
 * @param platform The platform wrapper object.
 * @return The OpenCL platform object.
 * */
#define cl4_platform_unwrap(platform) \
	((cl_platform) cl4_wrapper_unwrap((CL4Wrapper*) platform))
 
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
 * @return An array containing the ::CL4Device wrappers which belong to
 * this platform, or NULL if an error occurs.
 */
#define cl4_platform_get_all_devices(platf, err) \
	cl4_dev_container_get_all_devices((CL4DevContainer*) platf, \
		cl4_platform_get_cldevices, err)
 
/** 
 * @brief Get ::CL4Device wrapper at given index. 
 * 
 * @param platf The platform wrapper object.
 * @param index Index of device in platform.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The ::CL4Device wrapper at given index or NULL if an error 
 * occurs.
 * */
#define cl4_platform_get_device(platf, index, err) \
	cl4_dev_container_get_device((CL4DevContainer*) platf, \
		cl4_platform_get_cldevices, index, err)

/**
 * @brief Return number of devices in platform.
 * 
 * @param platf The platform wrapper object.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The number of devices in platform or 0 if an error occurs or 
 * is otherwise not possible to get any device.
 * */
#define cl4_platform_get_num_devices(platf, err) \
	cl4_dev_container_get_num_devices((CL4DevContainer*) platf, \
		cl4_platform_get_cldevices, err)

/** @} */

/** @brief Implementation of cl4_dev_container_get_cldevices() for the
 * platform wrapper. */
CL4WrapperInfo* cl4_platform_get_cldevices(
	CL4DevContainer* devcon, GError** err);

#endif
