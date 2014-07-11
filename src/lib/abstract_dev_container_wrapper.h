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
 * @brief Abstract wrapper for OpenCL objects which contain a list of
 * devices. Extends abstract_wrapper.c
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_ABSTRACT_DEV_CONTAINER_WRAPPER_H
#define CL4_ABSTRACT_DEV_CONTAINER_WRAPPER_H 

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include "gerrorf.h"
#include "common.h"
#include "errors.h"
#include "device.h"
#include "wrapper.h"

typedef struct cl4_dev_container {

	/** Parent wrapper object. */
	CL4Wrapper base;

	/** Number of devices in context (can be lazy initialized). */
	cl_uint num_devices;
	
	/** Devices in context (can be lazy initialized). */
	CL4Device** devices;
	
} CL4DevContainer;

typedef CL4WrapperInfo* (*cl4_dev_container_get_cldevices)(
	CL4DevContainer* devcon, GError** err);

/** @brief Release the devices held by the given #CL4DevContainer 
 * object. */
void cl4_dev_container_release_devices(CL4DevContainer* devcon);

/** @brief Get all ::CL4Device wrappers in device container. */
CL4Device** cl4_dev_container_get_all_devices(CL4DevContainer* devcon,
	cl4_dev_container_get_cldevices get_devices, GError** err);

/** @brief Get ::CL4Device wrapper at given index. */
CL4Device* cl4_dev_container_get_device(CL4DevContainer* devcon, 
	cl4_dev_container_get_cldevices get_devices, 
	unsigned int index, GError** err);

/** @brief Return number of devices in device container. */
unsigned int cl4_dev_container_get_num_devices(CL4DevContainer* devcon, 
	cl4_dev_container_get_cldevices get_devices, GError** err);

#endif


