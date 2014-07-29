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
 * @brief OpenCL platform stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, 
	cl_uint* num_platforms) {
	
	cl_int status = CL_SUCCESS;
	
	if (platforms == NULL) {
		if (num_platforms != NULL) {
			*num_platforms = ccl_test_num_platforms;
		} else {
			status = CL_INVALID_VALUE;
		}
	} else {
		if (num_entries == 0) {
			status = CL_INVALID_VALUE;
		} else {
			for (guint i = 0; 
				i < MIN(num_entries, ccl_test_num_platforms); i++) {
				platforms[i] = (cl_platform_id) &ccl_test_platforms[i];
			}
		}
	}
	
	return status;
}

cl_int clGetPlatformInfo(cl_platform_id platform, 
	cl_platform_info param_name, size_t param_value_size, 
	void* param_value, size_t* param_value_size_ret) {
		
	cl_int status = CL_SUCCESS;

	if (platform == NULL) {
		status = CL_INVALID_PLATFORM;
	} else {
		switch (param_name) {
			case CL_PLATFORM_PROFILE:
				ccl_test_char_info(platform, profile);
			case CL_PLATFORM_VERSION:
				ccl_test_char_info(platform, version);
			case CL_PLATFORM_NAME:
				ccl_test_char_info(platform, name);
			case CL_PLATFORM_VENDOR:
				ccl_test_char_info(platform, vendor);
			case CL_PLATFORM_EXTENSIONS:
				ccl_test_char_info(platform, extensions);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;
}



