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
				.name = "cf4ocl GPU device",
				.type = CL_DEVICE_TYPE_GPU
			},
			{ 
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
				.name = "cf4ocl Accelerator device",
				.type = CL_DEVICE_TYPE_ACCELERATOR
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
				.name = "cf4ocl CPU device",
				.type = CL_DEVICE_TYPE_CPU
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
			for (guint i = 0; i < MIN(num_entries, cl4_test_num_platforms); i++)
				platforms[i] = &cl4_test_platforms[i];
		}
	}
	
	return status;
}

#define cl4_test_platform_info(info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = strlen(platform->info) + 1; \
		} \
	} else if (param_value_size < strlen(platform->info) + 1) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, platform->info, \
			strlen(platform->info) + 1); \
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
				cl4_test_platform_info(profile);
			case CL_PLATFORM_VERSION:
				cl4_test_platform_info(version);
			case CL_PLATFORM_NAME:
				cl4_test_platform_info(name);
			case CL_PLATFORM_VENDOR:
				cl4_test_platform_info(vendor);
			case CL_PLATFORM_EXTENSIONS:
				cl4_test_platform_info(extensions);
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

	/// Fill
	
	return status;
}

cl_int clGetDeviceInfo(cl_device_id device, cl_device_info param_name, 
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) {
		
	cl_int status = CL_SUCCESS;

	/// Fill
	
	return status;
}
