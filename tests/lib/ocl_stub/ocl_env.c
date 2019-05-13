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
 * OpenCL fake environment.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"

const cl_uint ccl_test_num_platforms = 3;

const struct _cl_platform_id ccl_test_platforms[] = {
    {
        .profile = "FULL_PROFILE",
        .version = "OpenCL 1.2",
        .name = "cf4ocl test platform #0",
        .vendor = "FakenMC p0",
        .extensions = "cl_khr_byte_addressable_store cl_khr_icd cl_khr_gl_sharing",
        .image_formats = (const cl_image_format[]) {
            {
                .image_channel_order = CL_RGBA,
                .image_channel_data_type = CL_UNORM_INT8
            },
            {
                .image_channel_order = CL_INTENSITY,
                .image_channel_data_type = CL_UNORM_INT16
            }
        },
        .num_image_formats = 2,
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
                .extensions = "cl_khr_int64_base_atomics cl_khr_fp16 "
                    "cl_khr_gl_sharing cl_khr_gl_event cl_khr_d3d10_sharing "
                    "cl_khr_dx9_media_sharing cl_khr_d3d11_sharing",
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
                .max_work_item_sizes = (size_t const[]) {512, 256, 16, 0},
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
                .partition_properties =
                    (cl_device_partition_property const[])
                    {CL_DEVICE_PARTITION_EQUALLY, 0},
                .partition_affinity_domain = 0,
                .partition_type = (cl_device_partition_property[]) {0},
                .platform_id = (const cl_platform_id) &ccl_test_platforms[0],
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
                .driver_version = "2.0.0",
                .ref_count = 1
            },
            {
                .address_bits = 64,
                .available = CL_TRUE,
                .built_in_kernels = "",
                .compiler_available = CL_TRUE,
                .double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_INF | CL_FP_FMA,
                .endian_little = CL_TRUE,
                .error_correction_support = CL_TRUE,
                .execution_capabilities =
                    CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
                .extensions = "cl_khr_int64_base_atomics "
                    "cl_khr_int64_extended_atomics cl_khr_fp16",
                .global_mem_cache_size = 1048576,
                .global_mem_cache_type = CL_READ_WRITE_CACHE,
                .global_mem_cacheline_size = 128,
                .global_mem_size = 17179869184,
                .half_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_INF | CL_FP_FMA,
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
                .max_work_item_sizes = (size_t const[]) {1024, 512, 16, 0},
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
                .partition_properties =
                    (cl_device_partition_property const[]) {
                        CL_DEVICE_PARTITION_EQUALLY,
                        CL_DEVICE_PARTITION_BY_COUNTS,
                        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
                        0
                    },
                .partition_affinity_domain =
                    CL_DEVICE_AFFINITY_DOMAIN_NUMA |
                    CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE |
                    CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
                .partition_type = (cl_device_partition_property[]) {0},
                .platform_id = (const cl_platform_id) &ccl_test_platforms[0],
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
                .queue_properties =
                    CL_QUEUE_PROFILING_ENABLE |
                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                .single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_INF | CL_FP_FMA,
                .type = CL_DEVICE_TYPE_CPU,
                .vendor = "FakenMC",
                .vendor_id = 0xFFFF,
                .version = "OpenCL 1.2 cf4ocl",
                .driver_version = "2.0.0",
                .ref_count = 1
            }
        }
    },
    {
        .profile = "FULL_PROFILE",
        .version = "OpenCL 1.1",
        .name = "cf4ocl test platform #1",
        .vendor = "FakenMC p1",
        .extensions = "cl_khr_byte_addressable_store cl_khr_icd",
        .image_formats = (const cl_image_format[]) {
            {
                .image_channel_order = CL_RGBA,
                .image_channel_data_type = CL_UNORM_INT8
            },
            {
                .image_channel_order = CL_LUMINANCE,
                .image_channel_data_type = CL_FLOAT
            }
        },
        .num_image_formats = 2,
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
                .max_work_item_sizes = (size_t const[]) {1024, 256, 16, 0},
                .max_write_image_args = 16,
                .mem_base_addr_align = 2048,
                /* Deprecated in OpenCL 1.2 */
                .min_data_type_align_size = 8,
                .name = "cf4ocl Accelerator device",
                .native_vector_width_char = 8,
                .native_vector_width_short = 4,
                .native_vector_width_int = 2,
                .native_vector_width_long = 1,
                .native_vector_width_float = 2,
                .native_vector_width_double = 0,
                .native_vector_width_half = 0,
                .opencl_c_version = "OpenCL C 1.1",
                /* Not available in OpenCL 1.1 */
                .parent_device = NULL,
                /* Not available in OpenCL 1.1 */
                .partition_max_sub_devices = 0,
                /* Not available in OpenCL 1.1 */
                .partition_properties =
                    (cl_device_partition_property const[]) {0},
                /* Not available in OpenCL 1.1 */
                .partition_affinity_domain = 0,
                /* Not available in OpenCL 1.1 */
                .partition_type = (cl_device_partition_property[]) {0},
                .platform_id = (const cl_platform_id) &ccl_test_platforms[1],
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
                .queue_properties =
                    CL_QUEUE_PROFILING_ENABLE |
                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                .single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN | CL_FP_FMA,
                .type = CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT,
                .vendor = "FakenMC",
                .vendor_id = 0xFFFF,
                .version = "OpenCL 1.1 cf4ocl",
                .driver_version = "2.0.0",
                .ref_count = 1
            }
        }
    },
    {
        .profile = "EMBEDDED_PROFILE",
        .version = "OpenCL 1.2",
        .name = "cf4ocl test platform #2",
        .vendor = "FakenMC p2",
        .extensions = "cl_khr_icd",
        .image_formats = (const cl_image_format[]) {
            {
                .image_channel_order = CL_RGBA,
                .image_channel_data_type = CL_SNORM_INT16
            }
        },
        .num_image_formats = 1,
        .num_devices = 1,
        .devices = (const struct _cl_device_id[]) {
            {
                .address_bits = 64,
                .available = CL_TRUE,
                .built_in_kernels = "",
                .compiler_available = CL_FALSE,
                .double_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
                .endian_little = CL_TRUE,
                .error_correction_support = CL_TRUE,
                .execution_capabilities =
                    CL_EXEC_KERNEL | CL_EXEC_NATIVE_KERNEL,
                .extensions = "cl_khr_int64_base_atomics "
                    "cl_khr_int64_extended_atomics",
                .global_mem_cache_size = 4194304,
                .global_mem_cache_type = CL_READ_WRITE_CACHE,
                .global_mem_cacheline_size = 64,
                .global_mem_size = 2147483648,
                .half_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
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
                .max_work_item_sizes = (size_t const[]) {512, 256, 8, 0},
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
                .partition_properties =
                    (cl_device_partition_property const[]) {
                        CL_DEVICE_PARTITION_BY_COUNTS,
                        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN,
                        0
                    },
                .partition_affinity_domain = CL_DEVICE_AFFINITY_DOMAIN_NUMA |
                    CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE,
                .partition_type = (cl_device_partition_property[]) {0},
                .platform_id = (const cl_platform_id) &ccl_test_platforms[2],
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
                .single_fp_config = CL_FP_DENORM | CL_FP_INF_NAN |
                    CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA,
                .type = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT,
                .vendor = "FakenMC",
                .vendor_id = 0xFFFF,
                .version = "OpenCL 1.2 cf4ocl",
                .driver_version = "2.0.0",
                .ref_count = 1
            }
        }
    }
};
