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
	GHashTable* info   /**< Platform information. */
};


CL4Platform* cl4_platform_new(cl_platform_id id) {
	
	/* The platform wrapper object. */
	CL4Platform* platform;
		
	/* Allocate memory for the platform wrapper object. */
	platform = g_slice_new(CL4Platform);
	
	/* Set the platform ID. */
	platform->id = id;
		
	/* Allocate memory for the platform information. */
	platform->info = g_hash_table_new_full(
		g_direct_hash, g_direct_equal, NULL, g_free);

	/* Return the new platform wrapper object. */
	return platform;
	
}

void cl4_platform_destroy(CL4Platform* platform) {
	
	g_hash_table_destroy(platform->info);
	g_slice_free(CL4Platform, platform);

}

gchar* cl4_plaform_get_info(CL4Platform* platform, 
	cl_platform_info param_name) {

	gchar* param_value;
	
	if (g_hash_table_contains(platform->info, param_name) {
		
		param_value = g_hash_table_lookup(platform->info, param_name);
		
	} else {
		
		cl_int ocl_status;
		size_t size_ret;
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, 0, NULL, &size_ret);
		
		param_value = g_new(gchar, size_ret);
		
		ocl_status = clGetPlatformInfo(
			platform->id, param_name, size_ret, param_value, NULL);
			
		g_hash_table_insert(
			platform->info, GUINT_TO_POINTER(param_name), param_value);
		
	}
	
	return param_value;

}
