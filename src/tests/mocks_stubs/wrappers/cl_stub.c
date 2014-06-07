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
 * @brief OpenCL stubs for testing the profile module.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.h>
#else
	#include <CL/cl.h>
#endif
#include <glib.h>
#include <string.h>

struct _cl_device_id {
	const cl_uint address_bits;
	const cl_bool available;
	const char* built_in_kernels;
	const cl_bool compiler_available;
	const size_t const* max_work_item_sizes;
	const char* name;
	const cl_device_type type;
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
				.max_work_item_sizes = (const size_t const[]) {4096, 2048, 2048, 0},
				.name = "cf4ocl GPU device",
				.type = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT
			},
			{ 
				.address_bits = 64,
				.available = CL_TRUE,
				.built_in_kernels = "",
				.compiler_available = CL_TRUE,
				.max_work_item_sizes = (const size_t const[]) {256, 64, 16, 0},
				.name = "cf4ocl CPU device",
				.type = CL_DEVICE_TYPE_CPU
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
				.max_work_item_sizes = (const size_t const[]) {1024, 256, 16, 0},
				.name = "cf4ocl Accelerator device",
				.type = CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT
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
				.max_work_item_sizes = (const size_t const[]) {512, 256, 8, 0},
				.name = "cf4ocl CPU device",
				.type = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT
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
			case CL_DEVICE_MAX_WORK_ITEM_SIZES:
				cl4_test_vector_info(size_t, device, max_work_item_sizes);
			case CL_DEVICE_NAME:
				cl4_test_char_info(device, name);
			case CL_DEVICE_TYPE:
				cl4_test_basic_info(cl_device_type, device, type);
			default:
				status = CL_INVALID_VALUE;
		}
	}
	
	
		
	return status;
}
