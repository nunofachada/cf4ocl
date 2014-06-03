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
 * @brief Wrapper object for OpenCL platforms. Contains platform and 
 * platform information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "platform.h"

/**
 * @brief Platform wrapper object.
 */
struct cl4_platform {
	cl_platform_id id; /**< Platform ID. */
	GHashTable* info;  /**< Platform information. */
	CL4Device** devs;  /**< Devices available in platform. */
	gint ref_count;    /**< Reference count. */
};


CL4Platform* cl4_platform_new(cl_platform_id id) {
	
	/* The platform wrapper object. */
	CL4Platform* platform;
		
	/* Allocate memory for the platform wrapper object. */
	platform = g_slice_new(CL4Platform);
	
	/* Set the platform ID. */
	platform->id = id;
		
	/* Platform information will be lazy initialized when required. */
	platform->info = NULL;

	/* Platform devices array will be lazy initialized when required. */
	platform->devs = NULL;
	
	/* Reference count is one initially. */
	platform->ref_count = 1;

	/* Return the new platform wrapper object. */
	return platform;
	
}

void cl4_platform_ref(CL4Platform* platform) {
	g_atomic_int_inc(&platform->ref_count);
}

void cl4_platform_destroy(CL4Platform* platform) {
	
	cl4_platform_unref(platform);

}

void cl4_platform_unref(CL4Platform* platform) {
	
	g_return_if_fail (platform != NULL);

	if (g_atomic_int_dec_and_test(&platform->ref_count)) {
		if (platform->info) {
			g_hash_table_destroy(platform->info);
		}
		if (platform->devs) {
			for (guint i = 0; platform->devs[i]; i++) {
				cl4_device_destroy(platform->devs[i]);
			}
			g_free(platform->devs);
		}
		
		g_slice_free(CL4Platform, platform);
	}	

}

gchar* cl4_plaform_get_info(CL4Platform* platform, 
	cl_platform_info param_name) {

	gchar* param_value;
	
	/* If platform information table is not yet initialized, then 
	 * allocate memory for it. */
	if (!platform->info) {
		platform->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, g_free);
	}

	if (g_hash_table_contains(
		platform->info, GUINT_TO_POINTER(param_name))) {
		
		param_value = g_hash_table_lookup(
			platform->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		cl_int ocl_status;
		size_t size_ret;
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, 0, NULL, &size_ret);
		
		param_value = (gchar*) g_malloc(size_ret);
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, size_ret, param_value, NULL);
		/// @todo Check ocl status	
			
		g_hash_table_insert(
			platform->info, GUINT_TO_POINTER(param_name), param_value);
		
	}
	
	return param_value;

}

cl_platform_id cl4_platform_get_cl_platform_id(CL4Platform* platform) {
	return platform->id;
}


