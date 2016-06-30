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
 * Convert OpenCL error codes to readable strings.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_errors.h"

static const char* ccl_errors[] = {
	"Successful operation", /* CL_SUCCESS - 0 */
	"Device not found", /* CL_DEVICE_NOT_FOUND - 1 */
	"Device not available", /* CL_DEVICE_NOT_AVAILABLE - 2 */
	"Compiler not available", /* CL_COMPILER_NOT_AVAILABLE - 3 */
	"Memory object allocation failure", /* CL_MEM_OBJECT_ALLOCATION_FAILURE - 4 */
	"Out of resources", /* CL_OUT_OF_RESOURCES - 5 */
	"Out of host memory", /* CL_OUT_OF_HOST_MEMORY - 6 */
	"Profiling info not available", /* CL_PROFILING_INFO_NOT_AVAILABLE - 7 */
	"Memory copy overlap", /* CL_MEM_COPY_OVERLAP - 8 */
	"Image format mismatch", /* CL_IMAGE_FORMAT_MISMATCH - 9 */
	"Image format not supported", /* CL_IMAGE_FORMAT_NOT_SUPPORTED - 10 */
	"Build program failure", /* CL_BUILD_PROGRAM_FAILURE - 11 */
	"Map failure", /* CL_MAP_FAILURE - 12 */
	"Misaligned sub-buffer offset", /* CL_MISALIGNED_SUB_BUFFER_OFFSET - 13 */
	"Execution status error for events in wait list", /* CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST - 14 */
	"Compile program failure", /* CL_COMPILE_PROGRAM_FAILURE - 15 */
	"Linker not available", /* CL_LINKER_NOT_AVAILABLE - 16 */
	"Link program failure", /* CL_LINK_PROGRAM_FAILURE - 17 */
	"Device partition failed", /* CL_DEVICE_PARTITION_FAILED - 18 */
	"Argument information not available", /* CL_KERNEL_ARG_INFO_NOT_AVAILABLE - 19 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 20 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 21 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 22 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 23 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 24 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 25 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 26 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 27 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 28 */
	"Unassigned error code", /* Unassigned as of OpenCL 2.1 - 29 */
	"Invalid value", /* CL_INVALID_VALUE - 30 */
	"Invalid device type", /* CL_INVALID_DEVICE_TYPE - 31 */
	"Invalid platform", /* CL_INVALID_PLATFORM - 32 */
	"Invalid device", /* CL_INVALID_DEVICE - 33 */
	"Invalid context", /* CL_INVALID_CONTEXT - 34 */
	"Invalid queue properties", /* CL_INVALID_QUEUE_PROPERTIES - 35 */
	"Invalid command queue", /* CL_INVALID_COMMAND_QUEUE - 36 */
	"Invalid host pointer", /* CL_INVALID_HOST_PTR - 37 */
	"Invalid memory object", /* CL_INVALID_MEM_OBJECT - 38 */
	"Invalid image format descriptor", /* CL_INVALID_IMAGE_FORMAT_DESCRIPTOR - 39 */
	"Invalid image size", /* CL_INVALID_IMAGE_SIZE - 40 */
	"Invalid sampler", /* CL_INVALID_SAMPLER - 41 */
	"Invalid binary", /* CL_INVALID_BINARY - 42 */
	"Invalid build options", /* CL_INVALID_BUILD_OPTIONS - 43 */
	"Invalid program", /* CL_INVALID_PROGRAM - 44 */
	"Invalid program executable", /* CL_INVALID_PROGRAM_EXECUTABLE - 45 */
	"Invalid kernel name", /* CL_INVALID_KERNEL_NAME - 46 */
	"Invalid kernel definition", /* CL_INVALID_KERNEL_DEFINITION - 47 */
	"Invalid kernel", /* CL_INVALID_KERNEL - 48 */
	"Invalid argument index", /* CL_INVALID_ARG_INDEX - 49 */
	"Invalid argument value", /* CL_INVALID_ARG_VALUE - 50 */
	"Invalid argument size", /* CL_INVALID_ARG_SIZE - 51 */
	"Invalid kernel arguments", /* CL_INVALID_CCL_KERNEL_ARGS - 52 */
	"Invalid work dimension", /* CL_INVALID_WORK_DIMENSION - 53 */
	"Invalid work-group size", /* CL_INVALID_WORK_GROUP_SIZE - 54 */
	"Invalid work-item size", /* CL_INVALID_WORK_ITEM_SIZE - 55 */
	"Invalid global offset", /* CL_INVALID_GLOBAL_OFFSET - 56  */
	"Invalid event wait list", /* CL_INVALID_EVENT_WAIT_LIST - 57 */
	"Invalid event", /* CL_INVALID_EVENT - 58 */
	"Invalid operation", /* CL_INVALID_OPERATION - 59 */
	"Invalid GL object", /* CL_INVALID_GL_OBJECT - 60 */
	"Invalid buffer size", /* CL_INVALID_BUFFER_SIZE - 61 */
	"Invalid MIP level", /* CL_INVALID_MIP_LEVEL - 62 */
	"Invalid global work size", /* CL_INVALID_GLOBAL_WORK_SIZE - 63 */
	"Invalid property", /* CL_INVALID_PROPERTY - 64 */
	"Invalid image descriptor", /* CL_INVALID_IMAGE_DESCRIPTOR - 65 */
	"Invalid compiler options", /* CL_INVALID_COMPILER_OPTIONS - 66 */
	"Invalid linker options", /* CL_INVALID_LINKER_OPTIONS - 67 */
	"Invalid device partition count", /* CL_INVALID_DEVICE_PARTITION_COUNT - 68 */
	"Invalid pipe size", /* CL_INVALID_PIPE_SIZE - 69 */
	"Invalid device queue" /* CL_INVALID_DEVICE_QUEUE - 70 */
};

static const int ccl_errors_count = 71;

/**
 * @addtogroup CCL_ERRORS
 * @{
 */

/**
 * Convert OpenCL error code to a readable string.
 *
 * @param[in] code OpenCL error code.
 * @return A readable string.
 * */
CCL_EXPORT
const char* ccl_err(int code) {
	int index = -1 * code;
	return (index >= 0) && (index < ccl_errors_count)
		? ccl_errors[index]
		: "Unknown OpenCL error code";
}

/** @} */
