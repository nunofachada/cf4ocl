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
 * @brief OpenCL utilities function headers.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef CL4_MAN_H
#define CL4_MAN_H

#include <string.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include <glib.h>
#include "common.h"
#include "gerrorf.h"
#include "errors.h"

/**
 * @defgroup CL4_MAN_DEVICE_TYPE_STR Device type string descriptions.
 *
 * @{
 */

/** Default device type long (OpenCL) description. */
#define CL4_MAN_DEVICE_TYPE_DEFAULT_STR_FULL "CL_DEVICE_TYPE_DEFAULT"
/** CPU device type long (OpenCL) description. */
#define CL4_MAN_DEVICE_TYPE_CPU_STR_FULL "CL_DEVICE_TYPE_CPU"
/** GPU device type long (OpenCL) description. */
#define CL4_MAN_DEVICE_TYPE_GPU_STR_FULL "CL_DEVICE_TYPE_GPU"
/** Accelerator device type long (OpenCL) description. */
#define CL4_MAN_DEVICE_TYPE_ACCELERATOR_STR_FULL "CL_DEVICE_TYPE_ACCELERATOR"
/** All device types long (OpenCL) description. */
#define CL4_MAN_DEVICE_TYPE_ALL_STR_FULL "CL_DEVICE_TYPE_ALL"

/** Default device type description. */
#define CL4_MAN_DEVICE_TYPE_DEFAULT_STR "Default"
/** CPU device type description. */
#define CL4_MAN_DEVICE_TYPE_CPU_STR "CPU"
/** GPU device type description. */
#define CL4_MAN_DEVICE_TYPE_GPU_STR "GPU"
/** Accelerator device type description. */
#define CL4_MAN_DEVICE_TYPE_ACCELERATOR_STR "Accelerator"
/** All device types description. */
#define CL4_MAN_DEVICE_TYPE_ALL_STR "All"

/** @} */



/**
 * @defgroup CL4_MAN_MAX Maximum values/sizes for certain operations/data structures.
 *
 * @{
 */

/** Auxiliary maximum buffer size. */
#define CL4_MAN_MAX_AUX_BUFF 500
/** Maximum number of platforms. */
#define CL4_MAN_MAX_PLATFORMS 10
/** Maximum number of devices per platform. */
#define CL4_MAN_MAX_DEVICES_PER_PLATFORM 10
/** Maximum number of total devices. */
#define CL4_MAN_MAX_DEVICES_TOTAL 20

/** @} */

/**
 * @brief Information about OpenCL device.
 */
typedef struct cl4_man_device_info {
	cl_device_id device_id;               /**< Device ID. */
	cl_platform_id platform_id;           /**< Platform ID. */
	char device_name[CL4_MAN_MAX_AUX_BUFF];   /**< Device name string. */
	char device_vendor[CL4_MAN_MAX_AUX_BUFF]; /**< Device vendor string. */
	char platform_name[CL4_MAN_MAX_AUX_BUFF]; /**< Platform name string. */
} CL4ManDeviceInfo;

/**
 * @brief Complete information for an OpenCL execution session on a specific device.
 */
typedef struct cl4_man_zone {
	cl_uint device_type;       /**< OpenCL device type. */
	cl_uint cu;                /**< Compute units of device. */
	cl_context context;        /**< OpenCL context. */
	cl_command_queue* queues;  /**< Command queues. */
	cl_program program;        /**< OpenCL program. */
	cl_uint numQueues;         /**< Number of command queues. */
	CL4ManDeviceInfo device_info; /**< Device information. */
} CL4ManZone;


/**
 * @brief Pointer to function which will select device, if more than one
 * is available.
 *
 * Implementations of this function must always return a value between
 * 0 and @a numDevices.
 *
 * @param devInfos Array of device information.
 * @param numDevices Number of devices on list.
 * @param extraArg Extra arguments which may be required by function
 * implementations.
 * @return The array index of the selected device or -1 if no device is
 * selectable.
 */
typedef cl_uint (*cl4_man_device_selector)(CL4ManDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

/** @brief Get a string identifying the type of device. */
char* cl4_man_device_type_str_get(cl_device_type cldt, int full, char* str, int strSize);

/** @brief Create a new OpenCL zone, which will contain complete
 * information for an OpenCL execution session on a specific device. */
CL4ManZone* cl4_man_zone_new(cl_uint deviceType, cl_uint numQueues, cl_int queueProperties, cl4_man_device_selector devSel, void* dsExtraArg, GError **err);

/** @brief Create an OpenCL program given a set of source kernel files. */
int cl4_man_program_create(CL4ManZone* zone, char** kernelFiles, cl_uint numKernelFiles, char* compilerOpts, GError **err);

/** @brief Create an OpenCL program given a set of source kernel files. */
cl_program cl4_man_program_create_indep(cl_context context, 
	cl_device_id device, char** kernelFiles, cl_uint numKernelFiles, 
	char* compilerOpts, GError **err);

/** @brief Free a previously created OpenCL zone. */
void cl4_man_zone_free(CL4ManZone* zone);

/** @brief Load kernel source from given file. */
char* cl4_man_source_load(char* filename, GError** err);

/** @brief Free kernel source. */
void cl4_man_source_free(char* source);

/** @brief Queries the user to select a device from a list. */
cl_uint cl4_man_menu_device_selector(CL4ManDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

/** @brief Implementation of ::cl4_man_device_selector function which selects a
 * device based on device information such as device name, device vendor
 * and platform name. */
cl_uint cl4_man_info_device_selector(CL4ManDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

#endif
