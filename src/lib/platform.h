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

/** @brief The platform wrapper object. */
typedef struct cl4_platform CL4Platform;

/** @brief Creates a new platform wrapper object. */
CL4Platform* cl4_platform_new(cl_platform_id id);

/** @brief Decrements the reference count of the platform wrapper 
 * object. If it reaches 0, the platform wrapper object is destroyed. */
void cl4_platform_destroy(CL4Platform* platform);

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
		param_name, (cl4_wrapper_info_fp) clGetPlatformInfo, err))

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
 
/** @brief Get CL4 device wrapper at given index. */
CL4Device* cl4_platform_get_device(
	CL4Platform* platform, guint index, GError **err);

/** @brief Get all device wrappers in platform. */
CL4Device** cl4_platform_get_all_devices(
	CL4Platform* platform, GError **err);

/** @brief Return number of devices in platform. */
guint cl4_platform_num_devices(CL4Platform* platform, GError **err);

#endif
