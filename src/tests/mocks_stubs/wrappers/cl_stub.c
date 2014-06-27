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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/** 
 * @file
 * @brief OpenCL stub for testing the wrappers module.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/opencl.h>
#else
	#include <CL/opencl.h>
#endif
#include <glib.h>
#include <string.h>
#include "oclversions.h"

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
	const cl_context_properties* properties;
	const cl_device_id* devices;
	cl_uint num_devices;
	cl_bool d3d;
	cl_uint ref_count;
};

static const cl_uint cl4_test_num_platforms = 3;

static const struct _cl_platform_id cl4_test_platforms[] = {
	{ 
		.profile = "FULL_PROFILE", 
		.version = "OpenCL 1.2",
		.name = "cf4ocl test platform #0",
		.vendor = "FakenMC p0",
		.extensions = "cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing",
		.num_devices = 2,
		.devices = (const struct _cl_device_id[]) {
			{
				.address_bits = 32,
				.available = CL_TRUE,
				.built_in_kernels = "reduce;scan",
				.compiler_available = CL_TRUE,
				.double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_FMA,
				.endian_little = CL_TRUE,
				.error_correction_support = CL_FALSE,
				.execution_capabilities = CL_EXEC_KERNEL,
				.extensions = "cl_khr_int64_base_atomics cl_khr_fp16 cl_khr_gl_sharing cl_khr_gl_event cl_khr_d3d10_sharing cl_khr_dx9_media_sharing cl_khr_d3d11_sharing",
				.global_mem_cache_size = 16384,
				.global_mem_cache_type = CL_READ_ONLY_CACHE,
				.global_mem_cacheline_size = 32,
				.global_mem_size = 1073741824,
				.half_fp_config = 0,
				.host_unified_memory = CL_FALSE,
				.image_support = CL_TRUE,
				.image2d_max_height = 32768,
				.image2d_max_width = 32768,
				.image3d_max_depth = 4096,
				.image3d_max_height = 4096,
				.image3d_max_width = 4096,
				.image_max_buffer_size = 33554432,
				.image_max_array_size = 16384,
				.linker_available = CL_TRUE,
				.local_mem_size =  32768,
				.local_mem_type = CL_LOCAL,
				.max_clock_frequency = 1000,
				.max_compute_units = 16,
				.max_constant_args = 9,
				.max_constant_buffer_size = 65536,
				.max_mem_alloc_size = 268435456,
				.max_parameter_size = 1024,
				.max_read_image_args = 128,
				.max_samplers = 16,
				.max_work_group_size = 512,
				.max_work_item_dimensions = 3,
				.max_work_item_sizes = (const size_t const[]) {512, 256, 16, 0},
				.max_write_image_args = 16,
				.mem_base_addr_align = 1024,
				.min_data_type_align_size = 0, /* Deprecated in OpenCL 1.2 */
				.name = "cf4ocl GPU device",
				.native_vector_width_char = 16,
				.native_vector_width_short = 8,
				.native_vector_width_int = 4,
				.native_vector_width_long = 2,
				.native_vector_width_float = 4,
				.native_vector_width_double = 0,
				.native_vector_width_half = 0,
				.opencl_c_version = "OpenCL C 1.2",
				.parent_device = NULL,
				.partition_max_sub_devices = 16,
				.partition_properties = (const cl_device_partition_property const[]) {CL_DEVICE_PARTITION_EQUALLY, 0},
				.partition_affinity_domain = 0,
				.partition_type = (const cl_device_partition_property const[]) {0},
				.platform_id = (const cl_platform_id) &cl4_test_platforms[0],
				.preferred_vector_width_char = 16,
				.preferred_vector_width_short = 8,
				.preferred_vector_width_int = 4,
				.preferred_vector_width_long = 2,
				.preferred_vector_width_float = 4,
				.preferred_vector_width_double = 0,
				.preferred_vector_width_half = 0,
				.printf_buffer_size = 1048576,
				.preferred_interop_user_sync = CL_TRUE,
				.profile = "FULL_PROFILE",
				.profiling_timer_resolution = 1000,
				.queue_properties = CL_QUEUE_PROFILING_ENABLE,
				.single_fp_config = 0,
				.type = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT,
				.vendor = "FakenMC",
				.vendor_id = 0xFFFF,
				.version = "OpenCL 1.2 cf4ocl",
				.driver_version = "2.0.0"
			},
			{ 
				.address_bits = 64,
				.available = CL_TRUE,
				.built_in_kernels = "",
				.compiler_available = CL_TRUE,
				.double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.endian_little = CL_TRUE,
				.error_correction_support = CL_TRUE,
				.execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
				.extensions = "cl_khr_int64_base_atomics cl_khr_int64_extended_atomics cl_khr_fp16",
				.global_mem_cache_size = 1048576,
				.global_mem_cache_type = CL_READ_WRITE_CACHE,
				.global_mem_cacheline_size = 128,
				.global_mem_size = 17179869184,
				.half_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.host_unified_memory = CL_TRUE,
				.image_support = CL_FALSE,
				.image2d_max_height = 0,
				.image2d_max_width = 0,
				.image3d_max_depth = 0,
				.image3d_max_height = 0,
				.image3d_max_width = 0,
				.image_max_buffer_size = 0,
				.image_max_array_size = 0,
				.linker_available = CL_TRUE,
				.local_mem_size =  65536,
				.local_mem_type = CL_GLOBAL,
				.max_clock_frequency = 3500,
				.max_compute_units = 8,
				.max_constant_args = 9,
				.max_constant_buffer_size = 65536,
				.max_mem_alloc_size = 17179869184,
				.max_parameter_size = 1024,
				.max_read_image_args = 0,
				.max_samplers = 0,
				.max_work_group_size = 1024,
				.max_work_item_dimensions = 3,
				.max_work_item_sizes = (const size_t const[]) {1024, 512, 16, 0},
				.max_write_image_args = 0,
				.mem_base_addr_align = 4096,
				.min_data_type_align_size = 0, /* Deprecated in OpenCL 1.2 */
				.name = "cf4ocl CPU device",
				.native_vector_width_char = 8,
				.native_vector_width_short = 4,
				.native_vector_width_int = 2,
				.native_vector_width_long = 1,
				.native_vector_width_float = 2,
				.native_vector_width_double = 1,
				.native_vector_width_half = 4,
				.opencl_c_version = "OpenCL C 1.2",
				.parent_device = NULL,
				.partition_max_sub_devices = 8,
				.partition_properties = (const cl_device_partition_property const[]) {CL_DEVICE_PARTITION_EQUALLY, CL_DEVICE_PARTITION_BY_COUNTS, CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 0},
				.partition_affinity_domain = CL_DEVICE_AFFINITY_DOMAIN_NUMA | CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE | CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
				.partition_type = (const cl_device_partition_property const[]) {0},
				.platform_id = (const cl_platform_id) &cl4_test_platforms[0],
				.preferred_vector_width_char = 8,
				.preferred_vector_width_short = 4,
				.preferred_vector_width_int = 2,
				.preferred_vector_width_long = 1,
				.preferred_vector_width_float = 2,
				.preferred_vector_width_double = 1,
				.preferred_vector_width_half = 4,
				.printf_buffer_size = 4194304,
				.preferred_interop_user_sync = CL_FALSE,
				.profile = "FULL_PROFILE",
				.profiling_timer_resolution = 1,
				.queue_properties = CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
				.single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.type = CL_DEVICE_TYPE_CPU,
				.vendor = "FakenMC",
				.vendor_id = 0xFFFF,
				.version = "OpenCL 1.2 cf4ocl",
				.driver_version = "2.0.0"
			}
		}
	},
	{ 
		.profile = "FULL_PROFILE", 
		.version = "OpenCL 1.1",
		.name = "cf4ocl test platform #1",
		.vendor = "FakenMC p1",
		.extensions = "cl_khr_byte_addressable_store cl_khr_icd",
		.num_devices = 1,
		.devices = (const struct _cl_device_id[]) {
			{ 
				.address_bits = 32,
				.available = CL_TRUE,
				.built_in_kernels = "", /* Not available in OpenCL 1.1 */
				.compiler_available = CL_FALSE,
				.double_fp_config = CL_FP_DENORM | CL_FP_FMA,
				.endian_little = CL_FALSE,
				.error_correction_support = CL_FALSE,
				.execution_capabilities = CL_EXEC_KERNEL,
				.extensions = "cl_khr_int64_base_atomics",
				.global_mem_cache_size = 0,
				.global_mem_cache_type = CL_NONE,
				.global_mem_cacheline_size = 0,
				.global_mem_size = 268435456,
				.half_fp_config = 0,
				.host_unified_memory = CL_FALSE,
				.image_support = CL_TRUE,
				.image2d_max_height = 8192,
				.image2d_max_width = 8192,
				.image3d_max_depth = 2048,
				.image3d_max_height = 2048,
				.image3d_max_width = 2048,
				.image_max_buffer_size = 0, /* Not available in OpenCL 1.1 */
				.image_max_array_size = 0, /* Not available in OpenCL 1.1 */
				.linker_available = CL_FALSE, /* Not available in OpenCL 1.1 */
				.local_mem_size =  65536,
				.local_mem_type = CL_LOCAL,
				.max_clock_frequency = 1500,
				.max_compute_units = 4,
				.max_constant_args = 15,
				.max_constant_buffer_size = 65536,
				.max_mem_alloc_size = 134217728,
				.max_parameter_size = 1024,
				.max_read_image_args = 128,
				.max_samplers = 16,
				.max_work_group_size = 1024,
				.max_work_item_dimensions = 3,
				.max_work_item_sizes = (const size_t const[]) {1024, 256, 16, 0},
				.max_write_image_args = 16,
				.mem_base_addr_align = 2048,
				.min_data_type_align_size = 8, /* Deprecated in OpenCL 1.2 */
				.name = "cf4ocl Accelerator device",
				.native_vector_width_char = 8,
				.native_vector_width_short = 4,
				.native_vector_width_int = 2,
				.native_vector_width_long = 1,
				.native_vector_width_float = 2,
				.native_vector_width_double = 0,
				.native_vector_width_half = 0,
				.opencl_c_version = "OpenCL C 1.1",
				.parent_device = NULL, /* Not available in OpenCL 1.1 */
				.partition_max_sub_devices = 0, /* Not available in OpenCL 1.1 */
				.partition_properties = (const cl_device_partition_property const[]) {0}, /* Not available in OpenCL 1.1 */
				.partition_affinity_domain = 0, /* Not available in OpenCL 1.1 */
				.partition_type = (const cl_device_partition_property const[]) {0}, /* Not available in OpenCL 1.1 */
				.platform_id = (const cl_platform_id) &cl4_test_platforms[1],
				.preferred_vector_width_char = 4,
				.preferred_vector_width_short = 2,
				.preferred_vector_width_int = 1,
				.preferred_vector_width_long = 1,
				.preferred_vector_width_float = 1,
				.preferred_vector_width_double = 0,
				.preferred_vector_width_half = 0,
				.printf_buffer_size = 1048576,
				.preferred_interop_user_sync = CL_FALSE,
				.profile = "FULL_PROFILE",
				.profiling_timer_resolution = 10000,
				.queue_properties = CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
				.single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_FMA,
				.type = CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT,
				.vendor = "FakenMC",
				.vendor_id = 0xFFFF,
				.version = "OpenCL 1.1 cf4ocl",
				.driver_version = "2.0.0"
			}
		}
	},
	{ 
		.profile = "EMBEDDED_PROFILE", 
		.version = "OpenCL 1.2",
		.name = "cf4ocl test platform #2",
		.vendor = "FakenMC p2",
		.extensions = "cl_khr_icd",
		.num_devices = 1,
		.devices = (const struct _cl_device_id[]) {
			{ 
				.address_bits = 64,
				.available = CL_TRUE,
				.built_in_kernels = "",
				.compiler_available = CL_FALSE,
				.double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.endian_little = CL_TRUE,
				.error_correction_support = CL_TRUE,
				.execution_capabilities = CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
				.extensions = "cl_khr_int64_base_atomics cl_khr_int64_extended_atomics",
				.global_mem_cache_size = 4194304,
				.global_mem_cache_type = CL_READ_WRITE_CACHE,
				.global_mem_cacheline_size = 64,
				.global_mem_size = 2147483648,
				.half_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.host_unified_memory = CL_TRUE,
				.image_support = CL_FALSE,
				.image2d_max_height = 0,
				.image2d_max_width = 0,
				.image3d_max_depth = 0,
				.image3d_max_height = 0,
				.image3d_max_width = 0,
				.image_max_buffer_size = 0,
				.image_max_array_size = 0,
				.linker_available = CL_TRUE,
				.local_mem_size =  65536,
				.local_mem_type = CL_GLOBAL,
				.max_clock_frequency = 2500,
				.max_compute_units = 4,
				.max_constant_args = 9,
				.max_constant_buffer_size = 65536,
				.max_mem_alloc_size = 2147483648,
				.max_parameter_size = 1024,
				.max_read_image_args = 0,
				.max_samplers = 0,
				.max_work_group_size = 1024,
				.max_work_item_dimensions = 3,
				.max_work_item_sizes = (const size_t const[]) {512, 256, 8, 0},
				.max_write_image_args = 0,
				.mem_base_addr_align = 1024,
				.min_data_type_align_size = 0, /* Deprecated in OpenCL 1.2 */
				.name = "cf4ocl CPU device",
				.native_vector_width_char = 8,
				.native_vector_width_short = 4,
				.native_vector_width_int = 2,
				.native_vector_width_long = 1,
				.native_vector_width_float = 2,
				.native_vector_width_double = 1,
				.native_vector_width_half = 4,
				.opencl_c_version = "OpenCL C 1.2",
				.parent_device = NULL,
				.partition_max_sub_devices = 4,
				.partition_properties = (const cl_device_partition_property const[]) {CL_DEVICE_PARTITION_BY_COUNTS, CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 0},
				.partition_affinity_domain = CL_DEVICE_AFFINITY_DOMAIN_NUMA | CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
				.partition_type = (const cl_device_partition_property const[]) {0},
				.platform_id = (const cl_platform_id) &cl4_test_platforms[2],
				.preferred_vector_width_char = 8,
				.preferred_vector_width_short = 4,
				.preferred_vector_width_int = 2,
				.preferred_vector_width_long = 1,
				.preferred_vector_width_float = 2,
				.preferred_vector_width_double = 1,
				.preferred_vector_width_half = 4,
				.printf_buffer_size = 1048576,
				.preferred_interop_user_sync = CL_FALSE,
				.profile = "EMBEDDED_PROFILE",
				.profiling_timer_resolution = 100,
				.queue_properties = CL_QUEUE_PROFILING_ENABLE,
				.single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
				.type = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT,
				.vendor = "FakenMC",
				.vendor_id = 0xFFFF,
				.version = "OpenCL 1.2 cf4ocl",
				.driver_version = "2.0.0"
			}
		}
	}
};


/* Platform API */
cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, 
	cl_uint* num_platforms) {
	
	cl_int status = CL_SUCCESS;
	
	if (platforms == NULL) {
		if (num_platforms != NULL) {
			*num_platforms = cl4_test_num_platforms;
		} else {
			status = CL_INVALID_VALUE;
		}
	} else {
		if (num_entries == 0) {
			status = CL_INVALID_VALUE;
		} else {
			for (guint i = 0; 
				i < MIN(num_entries, cl4_test_num_platforms); i++) {
				platforms[i] = (cl_platform_id) &cl4_test_platforms[i];
			}
		}
	}
	
	return status;
}

static guint veclen(void* vector, size_t elem_size) {
	gulong value;
	guint len = 0;
	do {
		value = 0;
		g_memmove(&value, ((char*) vector) + elem_size * len, elem_size);
		len++;
	} while (value != 0);
	return len - 1;
}

#define cl4_test_char_info(object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = strlen(object->info) + 1; \
		} \
	} else if (param_value_size < strlen(object->info) + 1) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, \
			strlen(object->info) + 1); \
	} \
	break;


#define cl4_test_vector_info(type, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = \
				sizeof(type) * veclen((void*) object->info, sizeof(type)); \
		} \
	} else if (param_value_size < sizeof(type) * veclen((void*) object->info, sizeof(type))) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, \
			sizeof(type) * veclen((void*) object->info, sizeof(type))); \
	} \
	break;

#define cl4_test_predefvector_info(type, qty, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = sizeof(type) * qty; \
		} \
	} else if (param_value_size < sizeof(type) * qty) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, sizeof(type) * qty); \
	} \
	break;
		
#define cl4_test_basic_info(type, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = sizeof(type); \
		} \
	} else if (param_value_size < sizeof(type)) { \
		status = CL_INVALID_VALUE; \
	} else { \
		*((type*) param_value) = object->info; \
	} \
	break;


cl_int clGetPlatformInfo(cl_platform_id platform, 
	cl_platform_info param_name, size_t param_value_size, 
	void* param_value, size_t* param_value_size_ret) {
		
	cl_int status = CL_SUCCESS;

	if (platform == NULL) {
		status = CL_INVALID_PLATFORM;
	} else {
		switch (param_name) {
			case CL_PLATFORM_PROFILE:
				cl4_test_char_info(platform, profile);
			case CL_PLATFORM_VERSION:
				cl4_test_char_info(platform, version);
			case CL_PLATFORM_NAME:
				cl4_test_char_info(platform, name);
			case CL_PLATFORM_VENDOR:
				cl4_test_char_info(platform, vendor);
			case CL_PLATFORM_EXTENSIONS:
				cl4_test_char_info(platform, extensions);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;
}

/* Device APIs */
cl_int clGetDeviceIDs(cl_platform_id platform, 
	cl_device_type device_type, cl_uint num_entries, 
	cl_device_id* devices, cl_uint* num_devices) {
	
	cl_int status = CL_SUCCESS;

	if (platform == NULL) {
		status = CL_INVALID_PLATFORM;
	} else if (((num_entries == 0) && (devices != NULL)) 
			|| ((num_devices == NULL) && (devices == NULL))) {
		status = CL_INVALID_VALUE;
	} else {
		guint num_devices_found = 0;
		for (guint i = 0; i < platform->num_devices; i++) {
			if (platform->devices[i].type || device_type) {
				if (devices != NULL)
					devices[num_devices_found] = 
						(cl_device_id) &platform->devices[i];
				num_devices_found++;
				if ((num_devices_found >= num_entries) 
					&& (devices != NULL))
					break;
			}
		}
		if (num_devices != NULL) 
			*num_devices = num_devices_found;
		if (num_devices_found == 0)
			status = CL_DEVICE_NOT_FOUND;
	}
	return status;
}

cl_int clGetDeviceInfo(cl_device_id device, cl_device_info param_name, 
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) {
		
	cl_int status = CL_SUCCESS;

	if (device == NULL) {
		status = CL_INVALID_DEVICE;
	} else {
		switch (param_name) {
			case CL_DEVICE_ADDRESS_BITS:
				cl4_test_basic_info(cl_uint, device, address_bits);
			case CL_DEVICE_AVAILABLE:
				cl4_test_basic_info(cl_bool, device, available);
			case CL_DEVICE_BUILT_IN_KERNELS:
				cl4_test_char_info(device, built_in_kernels);
			case CL_DEVICE_COMPILER_AVAILABLE:
				cl4_test_basic_info(cl_bool, device, compiler_available);
			case CL_DEVICE_DOUBLE_FP_CONFIG:
				cl4_test_basic_info(cl_device_fp_config, device, double_fp_config);
			case CL_DEVICE_ENDIAN_LITTLE:
				cl4_test_basic_info(cl_bool, device, endian_little);
			case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
				cl4_test_basic_info(cl_bool, device, error_correction_support);
			case CL_DEVICE_EXECUTION_CAPABILITIES:
				cl4_test_basic_info(cl_device_exec_capabilities, device, execution_capabilities);
			case CL_DEVICE_EXTENSIONS:
				cl4_test_char_info(device, extensions);
			case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
				cl4_test_basic_info(cl_ulong, device, global_mem_cache_size);
			case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
				cl4_test_basic_info(cl_device_mem_cache_type, device, global_mem_cache_type);
			case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
				cl4_test_basic_info(cl_uint, device, global_mem_cacheline_size);
			case CL_DEVICE_GLOBAL_MEM_SIZE:
				cl4_test_basic_info(cl_ulong, device, global_mem_size);
			case CL_DEVICE_HALF_FP_CONFIG:
				cl4_test_basic_info(cl_device_fp_config, device, half_fp_config);
			case CL_DEVICE_HOST_UNIFIED_MEMORY:
				cl4_test_basic_info(cl_bool, device, host_unified_memory);
			case CL_DEVICE_IMAGE_SUPPORT:
				cl4_test_basic_info(cl_bool, device, image_support);
			case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
				cl4_test_basic_info(size_t, device, image2d_max_height);
			case CL_DEVICE_IMAGE2D_MAX_WIDTH:
				cl4_test_basic_info(size_t, device, image2d_max_width);
			case CL_DEVICE_IMAGE3D_MAX_DEPTH:
				cl4_test_basic_info(size_t, device, image3d_max_depth);
			case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
				cl4_test_basic_info(size_t, device, image3d_max_height);
			case CL_DEVICE_IMAGE3D_MAX_WIDTH:
				cl4_test_basic_info(size_t, device, image3d_max_width);
			case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
				cl4_test_basic_info(size_t, device, image_max_buffer_size);
			case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
				cl4_test_basic_info(size_t, device, image_max_array_size);
			case CL_DEVICE_LINKER_AVAILABLE:
				cl4_test_basic_info(cl_bool, device, linker_available);
			case CL_DEVICE_LOCAL_MEM_SIZE:
				cl4_test_basic_info(cl_ulong, device, local_mem_size);
			case CL_DEVICE_LOCAL_MEM_TYPE:
				cl4_test_basic_info(cl_device_local_mem_type, device, local_mem_type);
			case CL_DEVICE_MAX_CLOCK_FREQUENCY:
				cl4_test_basic_info(cl_uint, device, max_clock_frequency);
			case CL_DEVICE_MAX_COMPUTE_UNITS:
				cl4_test_basic_info(cl_uint, device, max_compute_units);
			case CL_DEVICE_MAX_CONSTANT_ARGS:
				cl4_test_basic_info(cl_uint, device, max_constant_args);
			case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
				cl4_test_basic_info(cl_ulong, device, max_constant_buffer_size);
			case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
				cl4_test_basic_info(cl_ulong, device, max_mem_alloc_size);
			case CL_DEVICE_MAX_PARAMETER_SIZE:
				cl4_test_basic_info(size_t, device, max_parameter_size);
			case CL_DEVICE_MAX_READ_IMAGE_ARGS:
				cl4_test_basic_info(cl_uint, device, max_read_image_args);
			case CL_DEVICE_MAX_SAMPLERS:
				cl4_test_basic_info(cl_uint, device, max_samplers);
			case CL_DEVICE_MAX_WORK_GROUP_SIZE:
				cl4_test_basic_info(size_t, device, max_work_group_size);
			case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
				cl4_test_basic_info(cl_uint, device, max_work_item_dimensions);
			case CL_DEVICE_MAX_WORK_ITEM_SIZES:
				cl4_test_vector_info(size_t, device, max_work_item_sizes);
			case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
				cl4_test_basic_info(cl_uint, device, max_write_image_args);
			case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
				cl4_test_basic_info(cl_uint, device, mem_base_addr_align);
			case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
				cl4_test_basic_info(cl_uint, device, min_data_type_align_size);
			case CL_DEVICE_NAME:
				cl4_test_char_info(device, name);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
				cl4_test_basic_info(cl_uint, device, native_vector_width_char);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
				cl4_test_basic_info(cl_uint, device, native_vector_width_short);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
				cl4_test_basic_info(cl_uint, device, native_vector_width_int);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
				cl4_test_basic_info(cl_uint, device, native_vector_width_long);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
				cl4_test_basic_info(cl_uint, device, native_vector_width_float);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
				cl4_test_basic_info(cl_uint, device, native_vector_width_double);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
				cl4_test_basic_info(cl_uint, device, native_vector_width_half);
			case CL_DEVICE_OPENCL_C_VERSION:
				cl4_test_char_info(device, opencl_c_version);
			case CL_DEVICE_PARENT_DEVICE:
				cl4_test_basic_info(cl_device_id, device, parent_device);
			case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
				cl4_test_basic_info(cl_uint, device, partition_max_sub_devices);
			case CL_DEVICE_PARTITION_PROPERTIES:
				cl4_test_vector_info(cl_device_partition_property, device, partition_properties);
			case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
				cl4_test_basic_info(cl_device_affinity_domain, device, partition_affinity_domain);
			case CL_DEVICE_PARTITION_TYPE:
				cl4_test_vector_info(cl_device_partition_property, device, partition_properties);
			case CL_DEVICE_PLATFORM:
				cl4_test_basic_info(cl_platform_id, device, platform_id);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_char);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_short);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_int);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_long);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_float);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_double);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
				cl4_test_basic_info(cl_uint, device, preferred_vector_width_half);
			case CL_DEVICE_PRINTF_BUFFER_SIZE:
				cl4_test_basic_info(size_t, device, printf_buffer_size);
			case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
				cl4_test_basic_info(cl_bool, device, preferred_interop_user_sync);
			case CL_DEVICE_PROFILE:
				cl4_test_char_info(device, profile);
			case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
				cl4_test_basic_info(size_t, device, profiling_timer_resolution);
			case CL_DEVICE_QUEUE_PROPERTIES:
				cl4_test_basic_info(cl_command_queue_properties, device, queue_properties);
			case CL_DEVICE_SINGLE_FP_CONFIG:
				cl4_test_basic_info(cl_device_fp_config, device, single_fp_config);
			case CL_DEVICE_TYPE:
				cl4_test_basic_info(cl_device_type, device, type);
			case CL_DEVICE_VENDOR:
				cl4_test_char_info(device, vendor);
			case CL_DEVICE_VENDOR_ID:
				cl4_test_basic_info(cl_uint, device, vendor_id);
			case CL_DEVICE_VERSION:
				cl4_test_char_info(device, version);
			case CL_DRIVER_VERSION:
				cl4_test_char_info(device, driver_version);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;
}

/* Context API */
cl_context clCreateContext(const cl_context_properties* properties,
	cl_uint num_devices, const cl_device_id* devices,
	void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
	void* user_data,
	cl_int* errcode_ret) {
	
	cl_context ctx = g_new(struct _cl_context, 1);
	ctx->properties = properties;
	ctx->devices = devices;
	ctx->num_devices = num_devices;
	ctx->d3d = FALSE;
	ctx->ref_count = 1;
	pfn_notify = pfn_notify;
	user_data = user_data;
	*errcode_ret = CL_SUCCESS;
	
	return ctx;
		
}

cl_int clReleaseContext(cl_context context) {
	
	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&context->ref_count)) {

		g_free(context);
		
	}
	
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) {

	g_atomic_int_inc(&context->ref_count);
	return CL_SUCCESS;

}

cl_int clGetContextInfo(cl_context context, cl_context_info param_name,
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) {
		
	cl_int status = CL_SUCCESS;

	if (context == NULL) {
		status = CL_INVALID_CONTEXT;
	} else {
		switch (param_name) {
			
			case CL_CONTEXT_REFERENCE_COUNT:
				cl4_test_basic_info(cl_uint, context, ref_count);
#ifdef CL_VERSION_1_1
			case CL_CONTEXT_NUM_DEVICES:
				cl4_test_basic_info(cl_uint, context, num_devices);
#endif
			case CL_CONTEXT_DEVICES:
				cl4_test_predefvector_info(
					cl_device_id, context->num_devices, context, devices);
			case CL_CONTEXT_PROPERTIES:
				cl4_test_vector_info(cl_context_properties, context, properties);
			//~ case CL_CONTEXT_D3D10_PREFER_SHARED_RESOURCES_KHR:
				//~ cl4_test_basic_info(cl_bool, context, d3d);
			//~ case CL_CONTEXT_D3D11_PREFER_SHARED_RESOURCES_KHR:			
				//~ cl4_test_basic_info(cl_bool, context, d3d);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;
		
}
