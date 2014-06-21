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

/**
 * Context wrapper object.
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
	
};

//~ CL4Context* cl4_context_new(
	//~ cl4_devsel* filters, guint num_filters, GError **err) {
//~ 
	//~ /* Make sure number ds is not NULL. */
	//~ g_return_val_if_fail(num_devices > 0, NULL);
	//~ 
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
	//~ /* Error reporting object. */
	//~ GError* err_internal = NULL;
	//~ 
	//~ /* Complete list of devices. */
	//~ GSList* devices = NULL;  
	//~ 
	//~ /* Context wrapper to create. */
	//~ CL4Context* ctx;
//~ 
	//~ /* Initialize complete list of devices. */
	//~ devices = cl4_devsel_list(&err_internal);
	//~ 
	//~ /* Filter devices. */
	//~ for (guint i = 0; i < num_filters; i++) {
		//~ devices = filters[i](devices, &err_internal);
	//~ }
	//~ 
	//~ /* Create context wrapper object with remaining devices. */
	//~ 
	//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert (err == NULL || *err != NULL);
//~ 
	//~ /* Destroy the ctx, or what was possible to build of it. */
	//~ if (ctx != NULL)
		//~ cl4_context_destroy(ctx);
	//~ ctx = NULL;
//~ 
//~ finish:
//~ 
	//~ /* Free list of devices. */
	//~ if (devices != NULL)
		//~ g_slist_free(devices);
//~ 
	//~ /* Return ctx. */
	//~ return ctx;
//~ }

/**
 * @brief Creates a context wrapper using the exact parameters received
 * by the clCreateContext function.
 * 
 * If the properties parameter is NULL, this function obtains the 
 * cl_platform_id object from the first device.
 * 
 * @param properties
 * @param num_devices Number of cl_devices_id's in list.
 * @param devices List of cl_device_id's.
 * @param pfn_notify
 * @param user_data
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new context wrapper object.
 * */
CL4Context* cl4_context_new_from_cldevices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	const cl_device_id* devices,
	void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
    void* user_data,
    GError** err) {
		
	/* Make sure number of devices is not zero. */
	g_return_val_if_fail(num_devices > 0, NULL);
	
	/* Make sure device list is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* The OpenCL scene to create. */
	CL4Context* ctx = NULL;
	
	/* Error reporting object. */
	GError* err_internal = NULL;
	
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	
	/* Was memory allocated for ctx_props? */
	gboolean ctx_props_alloc = FALSE;
	
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	
	/* OpenCL platform ID. */
	cl_platform_id platform = NULL;
	
	/* Create ctx. */
	ctx = g_slice_new0(CL4Context);
	
	/* Set number of devices. */
	ctx->num_devices = num_devices;
	
	/* Allocate space for device wrappers. */
	ctx->devices = g_slice_alloc0(num_devices * sizeof(CL4Device*));
	
	/* Add device wrappers to list of device wrappers. */
	for (guint i = 0; i < num_devices; i++) {

		/* Make sure devices in list are not NULL. */
		g_return_val_if_fail(devices[i] != NULL, NULL);

		/* Create new device wrapper, add it to list. */
		ctx->devices[i] = cl4_device_new(devices[i]);

	}
		
	/* If the properties parameter is NULL, assume some default context 
	 * properties. */
	if (properties == NULL) {
		
		/* Allocate memory for default properties. */
		ctx_props = g_slice_alloc(3 * sizeof(cl_context_properties));
		ctx_props_alloc = TRUE;

		/* Get context platform using first device. */
		platform = *((cl_platform_id*) cl4_device_info_value(
			ctx->devices[0], CL_DEVICE_PLATFORM, &err_internal));
		gef_if_err_propagate_goto(err, err_internal, error_handler);

		/* Set context properties using discovered platform. */
		ctx_props[0] = CL_CONTEXT_PLATFORM;
		ctx_props[1] = (cl_context_properties) platform;
		ctx_props[2] = 0;
		
	} else {
		
		/* If properties parameter is not NULL, use it. */
		ctx_props = (cl_context_properties*) properties;
	}
	
	/* Create a platform wrapper object and keep it. */
	ctx->platform = cl4_platform_new(platform);
	
	/* Create OpenCL context. */
	ctx->context = clCreateContext(
		(const cl_context_properties*) ctx_props, num_devices, devices, 
		pfn_notify, user_data, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_OCL_ERROR, error_handler, 
		"Function '%s': unable to create cl_context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* Destroy the ctx, or what was possible to build of it. */
	if (ctx != NULL)
		cl4_context_destroy(ctx);
	ctx = NULL;

finish:

	/* Free stuff. */
	if (ctx_props_alloc) g_slice_free1(
		3 * sizeof(cl_context_properties), ctx_props);
	
	/* Return ctx. */
	return ctx;
	
}

//~ /** 
 //~ * @brief Creates a context wrapper from a cl_context object.
 //~ * 
 //~ * This function is useful when the client wants more control over the
 //~ * cl_context object creation.
 //~ * 
 //~ * @param ctx The OpenCL cl_context object to wrap.
 //~ * @param err Return location for a GError, or NULL if error reporting
 //~ * is to be ignored.
 //~ * @return A new context wrapper object.
 //~ * */
//~ CL4Context* cl4_context_new_from_clcontext(cl_context ctx, GError **err) {
//~ 
	//~ /// @todo
	//~ return NULL;
//~ }

/**
 * @brief Destroy a context wrapper object.
 * 
 * @param ctx Context wrapper object to destroy.
 * */
void cl4_context_destroy(CL4Context* ctx) {
	
	/* Make sure context wrapper object is not NULL. */
	g_return_if_fail(ctx != NULL);

	/* Release devices in ctx. */
	for (guint i = 0; i < ctx->num_devices; i++) {
		cl4_device_unref(ctx->devices[i]);
	}
	
	/* Free device array. */
	g_slice_free1(
		ctx->num_devices * sizeof(CL4Device*), ctx->devices);
              
	/* Release context. */
	if (ctx->context) {
		clReleaseContext(ctx->context);
	}
	
	/* Release platform. */
	if (ctx->platform) {
		cl4_platform_unref(ctx->platform);
	}
	
	/* Release ctx. */
	g_slice_free(CL4Context, ctx);

}
