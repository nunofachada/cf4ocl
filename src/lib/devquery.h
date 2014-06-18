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
#include "device.h"
#include "errors.h"
#include "string.h"
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

/* Some of these AMD query constants may not be defined in standard 
 * OpenCL headers, so we defined them here if necessary. */
#ifndef CL_DEVICE_TOPOLOGY_AMD
	#define CL_DEVICE_TOPOLOGY_AMD                      0x4037
#endif
#ifndef CL_DEVICE_BOARD_NAME_AMD
	#define CL_DEVICE_BOARD_NAME_AMD                    0x4038
#endif
#ifndef CL_DEVICE_GLOBAL_FREE_MEMORY_AMD
	#define CL_DEVICE_GLOBAL_FREE_MEMORY_AMD            0x4039
#endif
#ifndef CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD
	#define CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD         0x4040
#endif
#ifndef CL_DEVICE_SIMD_WIDTH_AMD
	#define CL_DEVICE_SIMD_WIDTH_AMD                    0x4041
#endif
#ifndef CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD
	#define CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD        0x4042
#endif
#ifndef CL_DEVICE_WAVEFRONT_WIDTH_AMD
	#define CL_DEVICE_WAVEFRONT_WIDTH_AMD               0x4043
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD           0x4044
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD      0x4045
#endif
#ifndef CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD
	#define CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD 0x4046
#endif
#ifndef CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD
	#define CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD   0x4047
#endif
#ifndef CL_DEVICE_LOCAL_MEM_BANKS_AMD
	#define CL_DEVICE_LOCAL_MEM_BANKS_AMD               0x4048
#endif
#ifndef CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD
	#define CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD        0x4049
#endif

/** 
 * @brief Output formatting function.
 * 
 * @param info CL device information bitfield.
 * @param out Char buffer (pre-allocated) were to write formatted output.
 * @param size Maximum output length.
 * @param units Parameter units suffix.
 * @return Formatted output (same address as parameter out).
 * */
typedef gchar* (*cl4_devquery_format)(
	CL4DeviceInfoWrapper* info, gchar* out, guint size, 
	const gchar const* units);

/** 
 * @brief Maps a string to a cl_device_info bitfield. 
 * */
typedef struct cl4_devquery_map {
	
	/** Parameter name string. */
	const gchar const* param_name;
	/** CL device information bitfield. */
	const cl_device_info device_info;
	/** Long description of parameter. */
	const gchar const* description;
	/** Output formatting function. */
	const cl4_devquery_format format;
	/** Parameter units suffix. */
	const gchar const* units;
	
} CL4DevQueryMap;

/** @brief Size of parameter information map. */
extern const gint cl4_devquery_info_map_size;

/** @brief Map of parameter name strings to respective cl_device_info 
 * bitfields, long description string, format output function and a
 * units suffix. */
extern const CL4DevQueryMap cl4_devquery_info_map[];

/** @brief Return a cl_device_info object given its name. */	
cl_device_info cl4_devquery_name(gchar* name);

/** @brief Get a list of device information parameters which have the 
 * given prefix. */
const CL4DevQueryMap* cl4_devquery_prefix(
	gchar* prefix, gint* size);

/** @brief Search for a device information parameter by matching part
 * of its name. This function is supposed to be used in a loop. */
const CL4DevQueryMap* cl4_devquery_match(gchar* substr, gint* idx);

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
