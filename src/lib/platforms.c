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
 * @brief Object which represents the list of OpenCL platforms available
 * in the system and respective functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "platforms.h"

/**
 * @brief Object which contains the list of OpenCL platforms available
 * in the system.
 */
struct cl4_platforms {
	/** Platforms available in the system. */
	CL4Platform** platfs;
	/** Number of platforms available in the system. */
	cl_uint num_platfs;
};

/**
 * @brief Creates a new CL4Platforms* object, which contains the list 
 * of OpenCL platforms available in the system.
 * 
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new CL4Platforms* object, or NULL in case an error occurs.
 * */
CL4Platforms* cl4_platforms_new(GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Return status of OpenCL functions. */
	cl_int ocl_status;
	
	/* Object which represents the list of OpenCL platforms available 
	 * in the system. */
	CL4Platforms* platforms = NULL;
	
	/* Size in bytes of array of platform IDs. */
	gsize platf_ids_size;
	
	/* Array of platform IDs. */
	cl_platform_id* platf_ids = NULL;
	
	/* Allocate memory for the CL4Platforms object. */
	platforms = g_slice_new0(CL4Platforms);
	
	/* Get number of platforms */
	ocl_status = clGetPlatformIDs(0, NULL, &platforms->num_platfs);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: get number of platforms (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, cl4_err(ocl_status));
		
	/* Determine size in bytes of array of platform IDs. */
	platf_ids_size = sizeof(cl_platform_id) * platforms->num_platfs;
	
	/* Allocate memory for array of platform IDs. */
	platf_ids = (cl_platform_id*) g_slice_alloc(platf_ids_size); 

	/* Get existing platform IDs. */
	ocl_status = clGetPlatformIDs(
		platforms->num_platfs, platf_ids, NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status,
		CL4_ERROR_OCL, error_handler, 
		"%s: get platforms IDs (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, cl4_err(ocl_status));
		
	/* Allocate memory for array of platform wrapper objects. */
	platforms->platfs = 
		g_slice_alloc(sizeof(CL4Platform*) * platforms->num_platfs);
	
	/* Wrap platform IDs in platform wrapper objects. */
	for (guint i = 0; i < platforms->num_platfs; i++) {
		/* Add platform wrapper object to array of wrapper objects. */
		platforms->platfs[i] = cl4_platform_new_wrap(platf_ids[i]);
	}

	/* Free array of platform ids. */
	g_slice_free1(platf_ids_size, platf_ids);
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
	/* Destroy the CL4Platforms object, or what was possible to build 
	 * of it. */
	if (platforms) {
		cl4_platforms_destroy(platforms);
	}
	
	/* Set platforms to NULL, indicating an error occurred.*/
	platforms = NULL;

finish:		
	
	/* Return the CL4Platforms object. */
	return platforms;

}

/**
 * @brief Destroy a CL4Platforms* object, including all underlying
 * platforms, devices and data.
 * 
 * @param platforms CL4Platforms* object to destroy.
 * */
void cl4_platforms_destroy(CL4Platforms* platforms) {

	/* Platforms object can't be NULL. */
	g_return_if_fail(platforms != NULL);
	
	/* Destroy underlying platforms. */
	for (guint i = 0; i < platforms->num_platfs; i++) {
		cl4_platform_unref(platforms->platfs[i]);
	}
	
	/* Free underlying platforms array. */
	g_slice_free1(
		sizeof(CL4Platform*) * platforms->num_platfs, platforms->platfs);
		
	/* Free CL4Platforms object. */	
	g_slice_free(CL4Platforms, platforms);
}


/**
 * @brief Return number of OpenCL platforms found in CL4Platforms* 
 * object.
 * 
 * @param platforms Object containing the OpenCL platforms.
 * @return The number of OpenCL platforms found.
 * */
guint cl4_platforms_count(CL4Platforms* platforms) {
	
	/* Platforms object can't be NULL. */
	g_return_val_if_fail(platforms != NULL, 0);
	
	/* Return number of OpenCL platforms. */
	return platforms->num_platfs;
}

/**
 * @brief Get CL4 platform wrapper object at given index.
 * 
 * @param platforms Object containing the OpenCL platforms.
 * @param index Index of platform to return.
 * @return Platform wrapper object at given index.
 * */
CL4Platform* cl4_platforms_get_platform(CL4Platforms* platforms, guint index) {
	
	/* Platforms object can't be NULL. */
	g_return_val_if_fail(platforms != NULL, NULL);

	/* Index of platform to return must be smaller than the number of
	 * available platforms. */
	g_return_val_if_fail(index < platforms->num_platfs, NULL);
	 
	/* Return platform at given index. */
	return platforms->platfs[index];
}
