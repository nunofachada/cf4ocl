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
 * OpenCL device stub functions.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type,
	cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices) {

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

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device, cl_device_info param_name,
	size_t param_value_size, void* param_value,
	size_t* param_value_size_ret) {

	cl_int status = CL_SUCCESS;

	if (device == NULL) {
		status = CL_INVALID_DEVICE;
	} else {
		switch (param_name) {
			case CL_DEVICE_ADDRESS_BITS:
				ccl_test_basic_info(cl_uint, device, address_bits);
			case CL_DEVICE_AVAILABLE:
				ccl_test_basic_info(cl_bool, device, available);
			case CL_DEVICE_BUILT_IN_KERNELS:
				ccl_test_char_info(device, built_in_kernels);
			case CL_DEVICE_COMPILER_AVAILABLE:
				ccl_test_basic_info(cl_bool, device, compiler_available);
			case CL_DEVICE_DOUBLE_FP_CONFIG:
				ccl_test_basic_info(cl_device_fp_config, device, double_fp_config);
			case CL_DEVICE_ENDIAN_LITTLE:
				ccl_test_basic_info(cl_bool, device, endian_little);
			case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
				ccl_test_basic_info(cl_bool, device, error_correction_support);
			case CL_DEVICE_EXECUTION_CAPABILITIES:
				ccl_test_basic_info(cl_device_exec_capabilities, device, execution_capabilities);
			case CL_DEVICE_EXTENSIONS:
				ccl_test_char_info(device, extensions);
			case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
				ccl_test_basic_info(cl_ulong, device, global_mem_cache_size);
			case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
				ccl_test_basic_info(cl_device_mem_cache_type, device, global_mem_cache_type);
			case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
				ccl_test_basic_info(cl_uint, device, global_mem_cacheline_size);
			case CL_DEVICE_GLOBAL_MEM_SIZE:
				ccl_test_basic_info(cl_ulong, device, global_mem_size);
			case CL_DEVICE_HALF_FP_CONFIG:
				ccl_test_basic_info(cl_device_fp_config, device, half_fp_config);
			case CL_DEVICE_HOST_UNIFIED_MEMORY:
				ccl_test_basic_info(cl_bool, device, host_unified_memory);
			case CL_DEVICE_IMAGE_SUPPORT:
				ccl_test_basic_info(cl_bool, device, image_support);
			case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
				ccl_test_basic_info(size_t, device, image2d_max_height);
			case CL_DEVICE_IMAGE2D_MAX_WIDTH:
				ccl_test_basic_info(size_t, device, image2d_max_width);
			case CL_DEVICE_IMAGE3D_MAX_DEPTH:
				ccl_test_basic_info(size_t, device, image3d_max_depth);
			case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
				ccl_test_basic_info(size_t, device, image3d_max_height);
			case CL_DEVICE_IMAGE3D_MAX_WIDTH:
				ccl_test_basic_info(size_t, device, image3d_max_width);
			case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
				ccl_test_basic_info(size_t, device, image_max_buffer_size);
			case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
				ccl_test_basic_info(size_t, device, image_max_array_size);
			case CL_DEVICE_LINKER_AVAILABLE:
				ccl_test_basic_info(cl_bool, device, linker_available);
			case CL_DEVICE_LOCAL_MEM_SIZE:
				ccl_test_basic_info(cl_ulong, device, local_mem_size);
			case CL_DEVICE_LOCAL_MEM_TYPE:
				ccl_test_basic_info(cl_device_local_mem_type, device, local_mem_type);
			case CL_DEVICE_MAX_CLOCK_FREQUENCY:
				ccl_test_basic_info(cl_uint, device, max_clock_frequency);
			case CL_DEVICE_MAX_COMPUTE_UNITS:
				ccl_test_basic_info(cl_uint, device, max_compute_units);
			case CL_DEVICE_MAX_CONSTANT_ARGS:
				ccl_test_basic_info(cl_uint, device, max_constant_args);
			case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
				ccl_test_basic_info(cl_ulong, device, max_constant_buffer_size);
			case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
				ccl_test_basic_info(cl_ulong, device, max_mem_alloc_size);
			case CL_DEVICE_MAX_PARAMETER_SIZE:
				ccl_test_basic_info(size_t, device, max_parameter_size);
			case CL_DEVICE_MAX_READ_IMAGE_ARGS:
				ccl_test_basic_info(cl_uint, device, max_read_image_args);
			case CL_DEVICE_MAX_SAMPLERS:
				ccl_test_basic_info(cl_uint, device, max_samplers);
			case CL_DEVICE_MAX_WORK_GROUP_SIZE:
				ccl_test_basic_info(size_t, device, max_work_group_size);
			case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
				ccl_test_basic_info(cl_uint, device, max_work_item_dimensions);
			case CL_DEVICE_MAX_WORK_ITEM_SIZES:
				ccl_test_vector_info(size_t, device, max_work_item_sizes);
			case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
				ccl_test_basic_info(cl_uint, device, max_write_image_args);
			case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
				ccl_test_basic_info(cl_uint, device, mem_base_addr_align);
			case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
				ccl_test_basic_info(cl_uint, device, min_data_type_align_size);
			case CL_DEVICE_NAME:
				ccl_test_char_info(device, name);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
				ccl_test_basic_info(cl_uint, device, native_vector_width_char);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
				ccl_test_basic_info(cl_uint, device, native_vector_width_short);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
				ccl_test_basic_info(cl_uint, device, native_vector_width_int);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
				ccl_test_basic_info(cl_uint, device, native_vector_width_long);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
				ccl_test_basic_info(cl_uint, device, native_vector_width_float);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
				ccl_test_basic_info(cl_uint, device, native_vector_width_double);
			case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
				ccl_test_basic_info(cl_uint, device, native_vector_width_half);
			case CL_DEVICE_OPENCL_C_VERSION:
				ccl_test_char_info(device, opencl_c_version);
			case CL_DEVICE_PARENT_DEVICE:
				ccl_test_basic_info(cl_device_id, device, parent_device);
			case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
				ccl_test_basic_info(cl_uint, device, partition_max_sub_devices);
			case CL_DEVICE_PARTITION_PROPERTIES:
				ccl_test_vector_info(cl_device_partition_property, device, partition_properties);
			case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
				ccl_test_basic_info(cl_device_affinity_domain, device, partition_affinity_domain);
			case CL_DEVICE_PARTITION_TYPE:
				ccl_test_vector_info(cl_device_partition_property, device, partition_properties);
			case CL_DEVICE_PLATFORM:
				ccl_test_basic_info(cl_platform_id, device, platform_id);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_char);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_short);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_int);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_long);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_float);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_double);
			case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
				ccl_test_basic_info(cl_uint, device, preferred_vector_width_half);
			case CL_DEVICE_PRINTF_BUFFER_SIZE:
				ccl_test_basic_info(size_t, device, printf_buffer_size);
			case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
				ccl_test_basic_info(cl_bool, device, preferred_interop_user_sync);
			case CL_DEVICE_PROFILE:
				ccl_test_char_info(device, profile);
			case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
				ccl_test_basic_info(size_t, device, profiling_timer_resolution);
			case CL_DEVICE_QUEUE_PROPERTIES:
				ccl_test_basic_info(cl_command_queue_properties, device, queue_properties);
			case CL_DEVICE_SINGLE_FP_CONFIG:
				ccl_test_basic_info(cl_device_fp_config, device, single_fp_config);
			case CL_DEVICE_TYPE:
				ccl_test_basic_info(cl_device_type, device, type);
			case CL_DEVICE_VENDOR:
				ccl_test_char_info(device, vendor);
			case CL_DEVICE_VENDOR_ID:
				ccl_test_basic_info(cl_uint, device, vendor_id);
			case CL_DEVICE_VERSION:
				ccl_test_char_info(device, version);
			case CL_DRIVER_VERSION:
				ccl_test_char_info(device, driver_version);
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;
}

#ifdef CL_VERSION_1_2

CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id in_device,
	const cl_device_partition_property* properties, cl_uint num_devices,
	cl_device_id* out_devices, cl_uint* num_devices_ret) {

	(void)in_device;
	(void)properties;
	(void)num_devices;
	(void)out_devices;
	(void)num_devices_ret;

	return CL_SUCCESS;
}



CL_API_ENTRY cl_int CL_API_CALL
clRetainDevice(cl_device_id device) {

	/* Only increment ref count if it's a sub-device. */
	if (device->parent_device != NULL) {
		g_atomic_int_inc(&device->ref_count);
	}
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseDevice(cl_device_id device) {

	/* Only decrement ref count if it's a sub--device. */
	if (device->parent_device != NULL) {
		/* Decrement reference count and check if it reaches 0. */
		if (g_atomic_int_dec_and_test(&device->ref_count)) {

			g_slice_free(struct _cl_device_id, device);

		}
	}
	return CL_SUCCESS;
}
#endif
