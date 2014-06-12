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
 * @brief OpenCL context wrapper.
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
	CL4Platform* platform;
	
	/* Context. */
	cl_context context;
	
	/* Number of devices in context. */
	cl_uint num_devices;
	
	/* Devices in context. */
	CL4Device** devices;
	
	//~ /* Number of programs in context. */
	//~ cl_uint num_programs;
	//~ /* Programs in context. */
	//~ cl_program* programs;
	//~ /* Number of queues in context. */
	//~ cl_uint num_queues;
	//~ /* Command queues */
	//~ cl_command_queue* queues;
	//~ /* Number of kernels in context. */
	//~ cl_uint num_kernels;
	//~ /* Kernels. */
	//~ cl_kernel* kernels;
};

CL4Context* cl4_context_new(cl4_devsel dev_sel, void* ds_info, GError **err) {

	/* The OpenCL scene to create. */
	CL4Context* ctx;
	
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	
	/* Create ctx. */
	ctx = g_slice_new0(CL4Context);
	
	/* Get the context from the devices/context selector. */
	ctx->context = dev_sel(ds_info, err);
	gef_if_error_goto(*err, GEF_USE_GERROR, (*err)->code, error_handler);

	/* Get number of devices in context. */
	ocl_status = clGetContextInfo(ctx->context, CL_CONTEXT_NUM_DEVICES,
		sizeof(cl_uint), &(ctx->num_devices), NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': get number of devices in context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
	g_assert_cmpuint(ctx->num_devices, >, 0);
		
	/* Get devices in context. */
	ctx->devices = (cl_device_id*) g_try_malloc0_n(
		ctx->num_devices, sizeof(cl_device_id));
	gef_if_error_create_goto(*err, CL4_ERROR, ctx->devices == NULL, 
		CL4_ERROR_NOALLOC, error_handler, 
		"Function '%s': unable to allocate memory for devices.", 
		__func__);
	ocl_status = clGetContextInfo(ctx->context, CL_CONTEXT_DEVICES,
		ctx->num_devices * sizeof(cl_device_id), ctx->devices, NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': get devices in context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
		
	/* Get context platform using first device. */
	ocl_status = clGetDeviceInfo(ctx->devices[0], CL_DEVICE_PLATFORM,
		sizeof(cl_platform_id), &(ctx->platform), NULL);
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
	
	/* Destroy the ctx, or what was possible to build of it. */
	cl4_context_destroy(ctx);
	ctx = NULL;

finish:	

	/* Return ctx. */
	return ctx;
	
}

void cl4_context_destroy(CL4Context* ctx) {
	
	/* Aux. var. */
	unsigned int i;
	
	/* If scene is not NULL */
	if (ctx) {
		
		/* Release kernels in ctx. */
		for (i = 0; i < ctx->num_kernels; i++) {
			/* Only release kernel if it's non-NULL */
			if (ctx->kernels[i]) {
				clReleaseKernel(ctx->kernels[i]);
			}
		}
		/* Free kernel array. */
		g_free(ctx->kernels);
		
		/* Release queues in ctx. */
		for (i = 0; i < ctx->num_queues; i++) {
			/* Only release queue if it's non-NULL */
			if (ctx->queues[i]) {
				clReleaseCommandQueue(ctx->queues[i]);
			}
		}
		/* Free queue array. */
		g_free(ctx->queues);

		/* Release programs in ctx. */
		for (i = 0; i < ctx->num_programs; i++) {
			/* Only release program if it's non-NULL */
			if (ctx->programs[i]) {
				clReleaseProgram(ctx->programs[i]);
			}
		}
		/* Free program array. */
		g_free(ctx->programs);

		/* Release devices in ctx. */
		for (i = 0; i < ctx->num_devices; i++) {
			/* Only release device if it's non-NULL */
			if (ctx->devices[i]) {
				clReleaseDevice(ctx->devices[i]);
			}
		}
		/* Free device array. */
		g_free(ctx->devices);

		/* Release context. */
		if (ctx->context) {
			clReleaseContext(ctx->context);
		}
		
		/* Release platform. */
		if (ctx->platform) {
			clReleasePlatform(ctx->platform);
		}
		
		/* Release ctx. */
		g_slice_free(CL4Context, ctx);
	}
}


