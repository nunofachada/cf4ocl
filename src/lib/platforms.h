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
 * @brief Object which represents the list of OpenCL platforms available
 * in the system and respective functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_PLATFORMS_H
#define CL4_PLATFORMS_H

#include "common.h"
#include "platform_wrapper.h"
#include "gerrorf.h"
#include "errors.h"

/**
 * @brief Object which contains the list of OpenCL platforms available
 * in the system.
 */
typedef struct cl4_platforms CL4Platforms;

/** @brief Creates a new CL4Platforms* object, which contains the list 
 * of OpenCL platforms available in the system. */
CL4Platforms* cl4_platforms_new(GError **err);

/** @brief Destroy a CL4Platforms* object, including all underlying
 * platforms, devices and data. */
void cl4_platforms_destroy(CL4Platforms* platforms);

/** @brief Return number of OpenCL platforms found in CL4Platforms* 
 * object. */
guint cl4_platforms_count(CL4Platforms* platforms);

/** @brief Get CL4 platform wrapper object at given index. */
CL4Platform* cl4_platforms_get_platform(CL4Platforms* platforms, guint index);

#endif
