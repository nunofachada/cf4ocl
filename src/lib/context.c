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
 * @brief Objects and functions which assist on setting up the OpenCL 
 * runtime environment.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "context.h"

/*
 * OpenCL runtime context and associated objects.
 */
struct cl4_context {
	/* Platform. */
	cl_platform_id platform;
	/* Context. */
	cl_context context;
	/* Number of devices in context. */
	cl_uint num_devices;
	/* Devices in context. */
	cl_device_id* devices;
	/* Number of programs in context. */
	cl_uint num_programs;
	/* Programs in context. */
	cl_program* programs;
	/* Number of queues in context. */
	cl_uint num_queues;
	/* Command queues */
	cl_command_queue* queues;
	/* Number of kernels in context. */
	cl_uint num_kernels;
	/* Kernels. */
	cl_kernel* kernels;
};

CL4Context* cl4_context_new(cl4_devsel dev_sel, void* ds_info, GError **err) {

	/* The OpenCL scene to create. */
	CL4Context* scene;
	
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	
	/* Create scene. */
	scene = (CL4Context*) g_try_malloc0(sizeof(CL4Context));
	gef_if_error_create_goto(*err, CL4_ERROR, scene == NULL, 
		CL4_ERROR_NOALLOC, error_handler, 
		"Function '%s': unable to allocate memory for scene object.",
		__func__);
	
	/* Get the context from the devices/context selector. */
	scene->context = dev_sel(ds_info, err);
	gef_if_error_goto(*err, GEF_USE_GERROR, (*err)->code, error_handler);

	/* Get number of devices in context. */
	ocl_status = clGetContextInfo(scene->context, CL_CONTEXT_NUM_DEVICES,
		sizeof(cl_uint), &(scene->num_devices), NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': get number of devices in context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
	g_assert_cmpuint(scene->num_devices, >, 0);
		
	/* Get devices in context. */
	scene->devices = (cl_device_id*) g_try_malloc0_n(
		scene->num_devices, sizeof(cl_device_id));
	gef_if_error_create_goto(*err, CL4_ERROR, scene->devices == NULL, 
		CL4_ERROR_NOALLOC, error_handler, 
		"Function '%s': unable to allocate memory for devices.", 
		__func__);
	ocl_status = clGetContextInfo(scene->context, CL_CONTEXT_DEVICES,
		scene->num_devices * sizeof(cl_device_id), scene->devices, NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': get devices in context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
		
	/* Get context platform using first device. */
	ocl_status = clGetDeviceInfo(scene->devices[0], CL_DEVICE_PLATFORM,
		sizeof(cl_platform_id), &(scene->platform), NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': unable to get platform (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* Destroy the scene, or what was possible to build of it. */
	cl4_context_destroy(scene);
	scene = NULL;

finish:	

	/* Return scene. */
	return scene;
	
}

void cl4_context_destroy(CL4Context* scene) {
	
	/* Aux. var. */
	unsigned int i;
	
	/* If scene is not NULL */
	if (scene) {
		
		/* Release kernels in scene. */
		for (i = 0; i < scene->num_kernels; i++) {
			/* Only release kernel if it's non-NULL */
			if (scene->kernels[i]) {
				clReleaseKernel(scene->kernels[i]);
			}
		}
		/* Free kernel array. */
		g_free(scene->kernels);
		
		/* Release queues in scene. */
		for (i = 0; i < scene->num_queues; i++) {
			/* Only release queue if it's non-NULL */
			if (scene->queues[i]) {
				clReleaseCommandQueue(scene->queues[i]);
			}
		}
		/* Free queue array. */
		g_free(scene->queues);

		/* Release programs in scene. */
		for (i = 0; i < scene->num_programs; i++) {
			/* Only release program if it's non-NULL */
			if (scene->programs[i]) {
				clReleaseProgram(scene->programs[i]);
			}
		}
		/* Free program array. */
		g_free(scene->programs);

		/* Release devices in scene. */
		for (i = 0; i < scene->num_devices; i++) {
			/* Only release device if it's non-NULL */
			if (scene->devices[i]) {
				clReleaseDevice(scene->devices[i]);
			}
		}
		/* Free device array. */
		g_free(scene->devices);

		/* Release context. */
		if (scene->context) {
			clReleaseContext(scene->context);
		}
		
		/* Release platform. */
		if (scene->platform) {
			clReleasePlatform(scene->platform);
		}
		
		/* Release scene. */
		g_free(scene);
	}
}


