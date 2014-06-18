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
#include "string.h"
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

/** @brief Device wrapper object. */
typedef struct cl4_device CL4Device;

/**
 * @brief Value type of info hash table.
 * */
typedef struct cl4_device_info_wrapper {
	/** Device information. */
	gpointer value;
	/** Size in bytes of device information. */
	gsize size;
} CL4DeviceInfoWrapper;

/** @brief Creates a new device wrapper object. */
CL4Device* cl4_device_new(cl_device_id id);

/** @brief Increase the reference count of the device wrapper object. */
void cl4_device_ref(CL4Device* device);

/** @brief Alias for cl4_device_unref(). */
void cl4_device_destroy(CL4Device* device);

/** @brief Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed. */
void cl4_device_unref(CL4Device* device);

/** @brief Returns the device wrapper object reference count. For
 * debugging and testing purposes only. */
gint cl4_device_ref_count(CL4Device* device);

/** @brief Get device information. */
CL4DeviceInfoWrapper* cl4_device_info(CL4Device* device, 
	cl_device_info param_name, GError** err);

/** @brief Get the OpenCL device ID object. */
cl_device_id cl4_device_id(CL4Device* device);

#endif
