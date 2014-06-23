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

/* Macro which allocates space for a CL4Context object. */
#define cl4_context_new() \
	g_slice_new0(CL4Context);

/* Macro which allocates space for the devices wrappers kept in a 
 * CL4Context object. */
#define cl4_context_device_wrappers_new(num_devices) \
	g_new0(CL4Device*, num_devices)


#define cl4_context_properties_default_free(properties) \
	g_slice_free1(3 * sizeof(cl_context_properties), properties)
	
static cl_context_properties* cl4_context_properties_default(
	CL4Device* device, GError** err) {
	
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Context properties. */
	cl_context_properties* ctx_props = NULL;
	
	cl_platform_id platform;
	
	/* Internal error handler. */
	GError* err_internal = NULL;

	/* Allocate memory for default properties. */
	ctx_props = g_slice_alloc(3 * sizeof(cl_context_properties));

	/* Get context platform using first device. */
	platform = *((cl_platform_id*) cl4_device_info_value(
		device, CL_DEVICE_PLATFORM, &err_internal));
	gef_if_err_propagate_goto(err, err_internal, error_handler);

	/* Set context properties using discovered platform. */
	ctx_props[0] = CL_CONTEXT_PLATFORM;
	ctx_props[1] = (cl_context_properties) platform;
	ctx_props[2] = 0;

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:
	
	/* Return properties. */
	return ctx_props;

}

CL4Context* cl4_context_new_from_filters_full(
	const cl_context_properties* properties, 
	CL4DevSelFilters* filters,
	void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
    void* user_data,
	GError **err) {

	/* Make sure number ds is not NULL. */
	g_return_val_if_fail(filters != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Error reporting object. */
	GError* err_internal = NULL;
	
	/* Array of selected/filtered CL4 device wrappers. */
	GPtrArray* devices = NULL;
	/* Array of selected/filtered devices (unwrapped). */
	cl_device_id* cl_devices = NULL;
	/* Context wrapper to create. */
	CL4Context* ctx = NULL;
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	/* Was memory allocated for ctx_props? */
	gboolean ctx_props_alloc = FALSE;
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;

	/* Create ctx. */
	ctx = cl4_context_new();

	/* Get selected/filtered devices. */
	devices = cl4_devsel_select(filters, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Check if any device was found. */
	gef_if_error_create_goto(*err, CL4_ERROR, devices->len == 0, 
		CL4_ERROR_DEVICE_NOT_FOUND, error_handler, 
		"Function '%s': no device found for selected filters.", 
		__func__);

	/* Set device wrappers. */
	ctx->devices = (CL4Device**) devices->pdata;
	ctx->num_devices = devices->len;
	
	/* Create an array for the selected cl_device_id's. */
	cl_devices = g_slice_alloc(devices->len * sizeof(cl_device_id));
	
	/* Unwrap selected devices and add them to array. */
	for (guint i = 0; i < devices->len; i++) {
		cl_devices[i] = cl4_device_id((CL4Device*) g_ptr_array_index(devices, i));
	}

	/* If the properties parameter is NULL, assume some default context 
	 * properties. */
	if (properties == NULL) {
		
		ctx_props_alloc = TRUE;
		ctx_props = cl4_context_properties_default(
			ctx->devices[0], &err_internal);
		
	} else {
		
		/* If properties parameter is not NULL, use it. */
		ctx_props = (cl_context_properties*) properties;
	}
	  
	/* Lazy initialization of the platform wrapper object. */
	ctx->platform = NULL;
	
	/// @todo Check if devices belong to same platform
	
	/* Create OpenCL context. */
	ctx->context = clCreateContext(
		(const cl_context_properties*) ctx_props, devices->len, cl_devices, 
		pfn_notify, user_data, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
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

	if (ctx_props_alloc) cl4_context_properties_default_free(ctx_props);
	
	g_ptr_array_free(devices, FALSE);

	/* Return ctx. */
	return ctx;
}

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
	
	/* Create ctx. */
	ctx = cl4_context_new();
	
	/* Set number of devices. */
	ctx->num_devices = num_devices;
	
	/* Allocate space for device wrappers. */
	ctx->devices = cl4_context_device_wrappers_new(num_devices);
	
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
		
		ctx_props_alloc = TRUE;
		ctx_props = cl4_context_properties_default(
			ctx->devices[0], &err_internal);
		
	} else {
		
		/* If properties parameter is not NULL, use it. */
		ctx_props = (cl_context_properties*) properties;
	}
	
	/* Lazy initialization of the platform wrapper object. */
	ctx->platform = NULL;
	
	/* Create OpenCL context. */
	ctx->context = clCreateContext(
		(const cl_context_properties*) ctx_props, num_devices, devices, 
		pfn_notify, user_data, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
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
	if (ctx_props_alloc) cl4_context_properties_default_free(ctx_props);
	
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
	
	/* Free device wrapper array. */
	g_free(ctx->devices);
              
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
