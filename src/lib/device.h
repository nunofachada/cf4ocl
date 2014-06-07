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
 * @brief Wrapper object for OpenCL devices. Contains device and device
 * information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_DEVICE_H
#define CL4_DEVICE_H

#include "common.h"
#include "gerrorf.h"
#include "errors.h"
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif

/**
 * @brief Device wrapper object.
 */
typedef struct cl4_device CL4Device;

#define cl4_device_type2str(type) \
	(((type) & CL_DEVICE_TYPE_CPU) ? "CPU" : \
		(((type) & CL_DEVICE_TYPE_GPU) ? "GPU" : \
			(((type) & CL_DEVICE_TYPE_ACCELERATOR) ? "Accelerator" : \
				(((type) & CL_DEVICE_TYPE_CUSTOM) ? "Custom" : \
					"Unknown"))))

CL4Device* cl4_device_new(cl_device_id id);

void cl4_device_ref(CL4Device* device);

void cl4_device_destroy(CL4Device* device);

void cl4_device_unref(CL4Device* device);

gpointer cl4_device_info(CL4Device* device, 
	cl_device_info param_name, GError** err);
	
cl_device_id cl4_device_id(CL4Device* device);

#endif
