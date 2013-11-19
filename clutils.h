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
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CLUTILS_H
#define CLUTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <glib.h>
#include "gerrorf.h"



/**
 * @defgroup CLU_DEVICE_TYPE_STR Device type string descriptions.
 *
 * @{
 */

/** Default device type long (OpenCL) description. */
#define CLU_DEVICE_TYPE_DEFAULT_STR_FULL "CL_DEVICE_TYPE_DEFAULT"
/** CPU device type long (OpenCL) description. */
#define CLU_DEVICE_TYPE_CPU_STR_FULL "CL_DEVICE_TYPE_CPU"
/** GPU device type long (OpenCL) description. */
#define CLU_DEVICE_TYPE_GPU_STR_FULL "CL_DEVICE_TYPE_GPU"
/** Accelerator device type long (OpenCL) description. */
#define CLU_DEVICE_TYPE_ACCELERATOR_STR_FULL "CL_DEVICE_TYPE_ACCELERATOR"
/** All device types long (OpenCL) description. */
#define CLU_DEVICE_TYPE_ALL_STR_FULL "CL_DEVICE_TYPE_ALL"

/** Default device type description. */
#define CLU_DEVICE_TYPE_DEFAULT_STR "Default"
/** CPU device type description. */
#define CLU_DEVICE_TYPE_CPU_STR "CPU"
/** GPU device type description. */
#define CLU_DEVICE_TYPE_GPU_STR "GPU"
/** Accelerator device type description. */
#define CLU_DEVICE_TYPE_ACCELERATOR_STR "Accelerator"
/** All device types description. */
#define CLU_DEVICE_TYPE_ALL_STR "All"

/** @} */



/**
 * @defgroup CLU_MAX Maximum values/sizes for certain operations/data structures.
 *
 * @{
 */

/** Auxiliary maximum buffer size. */ 
#define CLU_MAX_AUX_BUFF 500
/** Maximum number of platforms. */
#define CLU_MAX_PLATFORMS 10
/** Maximum number of devices per platform. */
#define CLU_MAX_DEVICES_PER_PLATFORM 10
/** Maximum number of total devices. */
#define CLU_MAX_DEVICES_TOTAL 20

/** @} */

/**
 * @brief Error codes.
 * */ 
enum clu_error_codes {
	CLU_SUCCESS = 0,        /**< Successful operation. */
	CLU_ERROR_NOALLOC = 1,  /**< Error code thrown when no memory allocation is possible. */
	CLU_ERROR_OPENFILE = 2, /**< Error code thrown when it's not possible to open file. */
	CLU_ERROR_ARGS = 3,     /**< Error code thrown when passed arguments are invalid. */
	CLU_ERROR_DEVICE_NOT_FOUND = 4, /**< Error code thrown when no OpenCL device is found. */
	CLU_OCL_ERROR = 10      /**< An OpenCL error ocurred. */
};

/** Resolves to error category identifying string, in this case an error in the OpenCL utilities library. */
#define CLU_UTILS_ERROR clu_utils_error_quark()

/** 
 * @brief Kernel work group information.
 */
typedef struct clu_kernel_work_group_info {
	size_t preferred_work_group_size_multiple; /**< Preferred multiple of workgroup size for launch. */
	size_t compile_work_group_size[3];         /**< Work-group size specified by the @code __attribute__((reqd_work_gr oup_size(X, Y, Z))) @endcode qualifier. If the work-group size is not specified using the above attribute qualifier (0, 0, 0) is returned. */
	size_t max_work_group_size;                /**< Maximum work-group size that can be used to execute a kernel on a specific device. */
	cl_ulong local_mem_size;                   /**< Amount of local memory in bytes being used by a kernel. */
	cl_ulong private_mem_size;                 /**< Minimum amount of private memory, in bytes, used by each workitem in the kernel.  */
} CLUKernelWorkgroupInfo;

/**
 * @brief Information about OpenCL device.
 */
typedef struct clu_device_info {
	cl_device_id device_id;               /**< Device ID. */
	cl_platform_id platform_id;           /**< Platform ID. */
	char device_name[CLU_MAX_AUX_BUFF];   /**< Device name string. */
	char device_vendor[CLU_MAX_AUX_BUFF]; /**< Device vendor string. */
	char platform_name[CLU_MAX_AUX_BUFF]; /**< Platform name string. */
} CLUDeviceInfo;

/** 
 * @brief Complete information for an OpenCL execution session on a specific device.
 */
typedef struct clu_zone {
	cl_uint device_type;       /**< OpenCL device type. */
	cl_uint cu;                /**< Compute units of device. */
	cl_context context;        /**< OpenCL context. */
	cl_command_queue* queues;  /**< Command queues. */
	cl_program program;        /**< OpenCL program. */
	cl_uint numQueues;         /**< Number of command queues. */
	CLUDeviceInfo device_info; /**< Device information. */
} CLUZone;


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
typedef cl_uint (*clu_device_selector)(CLUDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

/** @brief Get kernel workgroup info. */
int clu_workgroup_info_get(cl_kernel kernel, cl_device_id device, CLUKernelWorkgroupInfo* kwgi, GError **err);

/** @brief Print kernel workgroup info. */
void clu_workgroup_info_print(CLUKernelWorkgroupInfo* kwgi);

/** @brief Get a string identifying the type of device. */
char* clu_device_type_str_get(cl_device_type cldt, int full, char* str, int strSize);

/** @brief Create a new OpenCL zone, which will contain complete 
 * information for an OpenCL execution session on a specific device. */
CLUZone* clu_zone_new(cl_uint deviceType, cl_uint numQueues, cl_int queueProperties, clu_device_selector devSel, void* dsExtraArg, GError **err);

/** @brief Create an OpenCL program given a set of source kernel files. */
int clu_program_create(CLUZone* zone, const char** kernelFiles, cl_uint numKernelFiles, const char* compilerOpts, GError **err);

/** @brief Free a previously created OpenCL zone. */
void clu_zone_free(CLUZone* zone);

/** @brief Load kernel source from given file. */
char* clu_source_load(const char* filename, GError** err);

/** @brief Free kernel source. */
void clu_source_free(char* source);

/** @brief Queries the user to select a device from a list. */
cl_uint clu_menu_device_selector(CLUDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

/** @brief Implementation of ::clu_device_selector function which selects a 
 * device based on device information such as device name, device vendor
 * and platform name. */
cl_uint clu_info_device_selector(CLUDeviceInfo* devInfos, cl_uint numDevices, void* extraArg);

/** @brief Resolves to error category identifying string, in this case 
 * an error in the OpenCL utilities library. */
GQuark clu_utils_error_quark(void);

#endif
