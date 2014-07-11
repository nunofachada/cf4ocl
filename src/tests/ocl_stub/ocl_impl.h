/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */
 
 /** 
 * @file
 * @brief OpenCL stub object implementations.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#ifndef CL4_OCL_STUB_H
#define CL4_OCL_STUB_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/opencl.h>
#else
	#include <CL/opencl.h>
#endif
#include "oclversions.h"


/** @brief Stub for cl_event objects. */ 
struct _cl_event {
	cl_ulong start;
	cl_ulong end;
	cl_command_queue queue;
	cl_uint ref_count;
};

/** @brief Stub for cl_command_queue objects. */ 
struct _cl_command_queue {
	cl_context context;
	cl_device_id device;
	cl_uint ref_count;
	cl_command_queue_properties properties;
};

struct _cl_device_id {
	const cl_uint address_bits;
	const cl_bool available;
	const char* built_in_kernels;
	const cl_bool compiler_available;
	const cl_device_fp_config double_fp_config;
	const cl_bool endian_little;
	const cl_bool error_correction_support;
	const cl_device_exec_capabilities execution_capabilities;
	const char* extensions;
	const cl_ulong global_mem_cache_size;
	const cl_device_mem_cache_type global_mem_cache_type;
	const cl_uint global_mem_cacheline_size;
	const cl_ulong global_mem_size;
	const cl_device_fp_config half_fp_config;
	const cl_bool host_unified_memory;
	const cl_bool image_support;
	const size_t image2d_max_height;
	const size_t image2d_max_width;
	const size_t image3d_max_depth;
	const size_t image3d_max_height;
	const size_t image3d_max_width;
	const size_t image_max_buffer_size;
	const size_t image_max_array_size;
	const cl_bool linker_available;
	const cl_ulong local_mem_size;
	const cl_device_local_mem_type local_mem_type;
	const cl_uint max_clock_frequency;
	const cl_uint max_compute_units;
	const cl_uint max_constant_args;
	const cl_ulong max_constant_buffer_size;
	const cl_ulong max_mem_alloc_size;
	const size_t max_parameter_size;
	const cl_uint max_read_image_args;
	const cl_uint max_samplers;
	const size_t max_work_group_size;
	const cl_uint max_work_item_dimensions;
	const size_t const* max_work_item_sizes;
	const cl_uint max_write_image_args;
	const cl_uint mem_base_addr_align;
	const cl_uint min_data_type_align_size;
	const char* name;
	const cl_uint native_vector_width_char;
	const cl_uint native_vector_width_short;
	const cl_uint native_vector_width_int;
	const cl_uint native_vector_width_long;
	const cl_uint native_vector_width_float;
	const cl_uint native_vector_width_double;
	const cl_uint native_vector_width_half;
	const char* opencl_c_version;
	const cl_device_id parent_device;
	const cl_uint partition_max_sub_devices;
	const cl_device_partition_property const* partition_properties;
	const cl_device_affinity_domain partition_affinity_domain;
	const cl_device_partition_property const* partition_type;
	const cl_platform_id platform_id;
	const cl_uint preferred_vector_width_char;
	const cl_uint preferred_vector_width_short;
	const cl_uint preferred_vector_width_int;
	const cl_uint preferred_vector_width_long;
	const cl_uint preferred_vector_width_float;
	const cl_uint preferred_vector_width_double;
	const cl_uint preferred_vector_width_half;
	const size_t printf_buffer_size;
	const cl_bool preferred_interop_user_sync;
	const char* profile;
	const size_t profiling_timer_resolution;
	const cl_command_queue_properties queue_properties;
	const cl_device_fp_config single_fp_config;
	const cl_device_type type;
	const char* vendor;
	const cl_uint vendor_id;
	const char* version;
	const char* driver_version;
};

struct _cl_platform_id {
	const char* profile;
	const char* version;
	const char* name;
	const char* vendor;
	const char* extensions;
	const guint num_devices;
	const struct _cl_device_id const* devices;
};

struct _cl_context {
	cl_context_properties* properties;
	cl_uint prop_len;
	cl_device_id* devices;
	cl_uint num_devices;
	cl_bool d3d;
	cl_uint ref_count;
};

struct _cl_program {
	cl_uint ref_count;
	cl_context context;
	cl_uint num_devices;
	cl_device_id* devices;
	char* source;
	size_t* binary_sizes;
	unsigned char** binaries;
	cl_program_binary_type* binary_type;
	size_t num_kernels;
	char* kernel_names;
	cl_build_status* build_status;
	char** build_options;
	char** build_log;
};

struct _cl_kernel {
	cl_uint ref_count;
	cl_program program;
	cl_context context;
	const char* function_name;
	cl_uint num_args;
	const char* attributes;
};

struct _cl_mem {
	cl_uint ref_count;
	void (*release)(cl_mem);
	cl_mem_object_type type;
	cl_mem_flags flags;
	size_t size;
	void* host_ptr;
	cl_uint map_count;
	cl_context context;
	cl_mem associated_object;
	size_t offset;
};

struct _cl_image {
	struct _cl_mem buffer;
	struct _cl_image_desc desc;
};

#endif

