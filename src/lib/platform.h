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

/**
 * @brief Platform wrapper object.
 */
typedef struct cl4_platform CL4Platform;


CL4Platform* cl4_platform_new(cl_platform_id id);

void cl4_platform_ref(CL4Platform* platform);

void cl4_platform_destroy(CL4Platform* platform);

void cl4_platform_unref(CL4Platform* platform);

gchar* cl4_plaform_info(CL4Platform* platform, 
	cl_platform_info param_name, GError **err);

cl_platform_id cl4_platform_id(CL4Platform* platform);

CL4Device** cl4_plaform_devices(CL4Platform* platform, GError **err);

guint cl4_platform_device_count(CL4Platform* platform, GError **err);

#endif
