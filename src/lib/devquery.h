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
 * @brief Functions for querying OpenCL devices.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_DEVQUERY_H
#define CL4_DEVQUERY_H

#include "common.h"
#include "gerrorf.h"
#include "errors.h"
#include "string.h"
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif

#ifndef CL_DEVICE_HALF_FP_CONFIG
	#define CL_DEVICE_HALF_FP_CONFIG 0x1033
#endif

/** @brief Maps a string to a cl_device_info bitfield. */
typedef struct cl4_devquery_map {
	
	const gchar const* param_name;
	const cl_device_info device_info;
	const gchar const* description;
	
} CL4DevQueryMap;

/** @brief Return a cl_device_info object given its name. */	
cl_device_info cl4_devquery_name(gchar* name);

/** @brief Get a list of device information parameters which have the 
 * given prefix. */
const CL4DevQueryMap* cl4_devquery_list_prefix(
	gchar* prefix, gint* size);

/** 
 * @brief Map an OpenCL cl_device_type object to a string identifying
 * the device type.
 * 
 * @param type The OpenCL cl_device_type.
 * */
#define cl4_devquery_type2str(type) \
	(((type) & CL_DEVICE_TYPE_CPU) ? "CPU" : \
		(((type) & CL_DEVICE_TYPE_GPU) ? "GPU" : \
			(((type) & CL_DEVICE_TYPE_ACCELERATOR) ? "Accelerator" : \
				(((type) & CL_DEVICE_TYPE_CUSTOM) ? "Custom" : \
					"Unknown"))))

#endif
