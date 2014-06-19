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

/** @brief Increase the reference count of the platform wrapper object. */
void cl4_platform_ref(CL4Platform* platform);

/** @brief Alias for cl4_platform_unref(). */
void cl4_platform_destroy(CL4Platform* platform);

/** @brief Decrements the reference count of the platform wrapper object.
 * If it reaches 0, the platform wrapper object is destroyed. */
void cl4_platform_unref(CL4Platform* platform);

/** @brief Returns the platform wrapper object reference count. For
 * debugging and testing purposes only. */
gint cl4_platform_ref_count(CL4Platform* platform);

/** @brief Get platform information. */
gchar* cl4_platform_info(CL4Platform* platform, 
	cl_platform_info param_name, GError **err);

/** @brief Get the OpenCL platform ID object. */
cl_platform_id cl4_platform_id(CL4Platform* platform);

/** @brief Get CL4 device wrapper at given index. */
CL4Device* cl4_platform_get_device(
	CL4Platform* platform, guint index, GError **err);

/** @brief Return number of devices in platform. */
guint cl4_platform_device_count(CL4Platform* platform, GError **err);

#endif
