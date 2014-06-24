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
	
	/** Platform (can be lazy initialized). */
	CL4Platform* platform;
	
	/** Context. */
	cl_context context;
	
	/** Context information. */
	GHashTable* info;
	
	/** Number of devices in context. */
	cl_uint num_devices;
	
	/** Devices in context (can be lazy initialized). */
	CL4Device** devices;
	
	/** Reference count. */
	gint ref_count;    	
	
};

/** 
 * @brief Macro which allocates space for the devices wrappers kept in a 
 * CL4Context object.
 * 
 * @param num_devices Number of device wrapper objects to allocate space
 * for.
 * @return Pointer to newly allocated memory for the given number of
 * device wrapper objects. 
 * */
#define cl4_context_device_wrappers_new(num_devices) \
	g_new0(CL4Device*, num_devices)

/**
 * @brief Internal context wrapper build function.
 * 
 * To be called by the different cl4_context_new_* public functions.
 * 
 * @return A new context wrapper object with empty fields and a 
 * reference count of 1.
 * */
static CL4Context* cl4_context_new_internal() {
	
	/* The context wrapper object. */
	CL4Context* ctx;
	
	/* Allocate memory for context wrapper object. */
	ctx = g_slice_new0(CL4Context);
	
	/* Set fields empty and reference count to 1. */
	ctx->context = NULL;
	ctx->info = NULL;
	ctx->platform = NULL;
	ctx->num_devices = 0;
	ctx->devices = NULL;
	ctx->ref_count = 1;
	
	/* Return new context wrapper object. */
	return ctx;
		
}

/**
 * @brief Free the default context properties if required.
 * 
 * @param properties The original const properties, may be NULL, in 
 * which case the ctx_props parameter will be freed.
 * @param ctx_props Context properties to be freed, if different than
 * the original const properties parameter.
 * */
#define cl4_context_properties_default_free(properties, ctx_props) \
	if (properties == NULL) /* Could also be (properties != ctx_props) */ \
		g_slice_free1(3 * sizeof(cl_context_properties), ctx_props)

/**
 * @brief Create a default context properties object, if required. The
 * only property set in the default properties object is the OpenCL
 * cl_platform_id object.
 * 
 * @param properties Original const properties, which if NULL imply that
 * a new default properties object should be created.
 * @param device Reference device to build the context properties for.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The properties parameter if not NULL, or a default set of 
 * context properties.
 * */	
static cl_context_properties* cl4_context_properties_default(
	const cl_context_properties* properties, 
	CL4Device* device, GError** err) {
	
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Context properties. */
	cl_context_properties* ctx_props = NULL;
	
	/* The OpenCL platform object. */
	cl_platform_id platform;
	
	/* Internal error handler. */
	GError* err_internal = NULL;
	
	/* Check if the original const properties object is NULL... */
	if (properties == NULL) {
		
		/* ...if so, create a default set of context properties. */
		
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

	} else {
		
		/* If properties parameter is not NULL, use it instead. */
		ctx_props = (cl_context_properties*) properties;
	}

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

/** 
 * @brief Initialize internal device list of context wrapper object. 
 * 
 * @param ctx The context wrapper object.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * */
static void cl4_context_init_devices(
	CL4Context* ctx, GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure platform is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	
	/* Check if device list is already initialized. */
	if (!ctx->devices) {
		/* Not initialized, initialize it. */
		
		CL4Info* info_devs;
		GError* err_internal = NULL;
		
		/* Get device IDs. */
		info_devs = cl4_context_info(
			ctx, CL_CONTEXT_DEVICES, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		/* Allocate memory for array of device wrapper objects. */
		ctx->devices = cl4_context_device_wrappers_new(ctx->num_devices);
	
		/* Wrap device IDs in device wrapper objects. */
		for (guint i = 0; i < ctx->num_devices; i++) {
			
			/* Add device wrapper object to array of wrapper objects. */
			ctx->devices[i] = cl4_device_new(
				((cl_device_id*) info_devs->value)[i]);
		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		
	
	/* Terminate function. */
	return;
}

/**
 * @brief Create a new context wrapper object selecting devices using
 * the given set of filters. 
 * 
 * This function accepts all the parameters
 * required for the clCreateContext() OpenCL function. For simple
 * context creation use the cl4_context_new_from_filters() macro
 * instead.
 * 
 * @param properties A set of OpenCL context properties.
 * @param filters Filters for selecting device.
 * @param pfn_notify A callback function used by the OpenCL 
 * implementation to report information on errors during context 
 * creation as well as errors that occur at runtime in this context.
 * Ignored if NULL.
 * @param user_data Passed as argument to pfn_notify, can be NULL.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new context wrapper object.
 * */
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
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	/* Context wrapper to create. */
	CL4Context* ctx = cl4_context_new_internal();

	/* Get selected/filtered devices. */
	devices = cl4_devsel_select(filters, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Check if any device was found. */
	gef_if_error_create_goto(*err, CL4_ERROR, devices->len == 0, 
		CL4_ERROR_DEVICE_NOT_FOUND, error_handler, 
		"Function '%s': no device found for selected filters.", 
		__func__);

	/* Create an array for the selected cl_device_id's. */
	cl_devices = g_slice_alloc(devices->len * sizeof(cl_device_id));
	
	/* Unwrap selected devices and add them to array. */
	for (guint i = 0; i < devices->len; i++) {
		cl_devices[i] = cl4_device_unwrap(
			(CL4Device*) g_ptr_array_index(devices, i));
	}

	/* Determine context properties. */
	ctx_props = cl4_context_properties_default(
			properties, g_ptr_array_index(devices, 0), &err_internal);
	 
	/// @todo Check if devices belong to same platform
	
	/* Create OpenCL context. */
	ctx->context = clCreateContext(
		(const cl_context_properties*) ctx_props, devices->len, cl_devices, 
		pfn_notify, user_data, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"Function '%s': unable to create cl_context (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
		
	/* Set number of devices and device wrappers in context wrapper 
	 * object. */
	ctx->num_devices = devices->len;
	ctx->devices = (CL4Device**) devices->pdata;

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* Destroy what was built for the context wrapper. */
	cl4_context_destroy(ctx);
	ctx = NULL;
	
finish:

	/* Free default context properties if required. */
	cl4_context_properties_default_free(properties, ctx_props);
	
	/* Destroy array of selected cl_device_id's. */
	g_slice_free1(devices->len * sizeof(cl_device_id), cl_devices);
	
	/* Free array object containing device wrappers but not the
	 * underlying device wrappers (now reference in the context
	 * wrapper object). */
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

	/* Error reporting object. */
	GError* err_internal = NULL;
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	/* Context wrapper to create. */
	CL4Context* ctx = cl4_context_new_internal();
	
	/* Set number of devices in the context wrapper object. */
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
	
	/* Get a set of default context properties, if required. */
	ctx_props = cl4_context_properties_default(
			properties, ctx->devices[0], &err_internal);
	
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

	/* Destroy what was built for the context wrapper. */
	cl4_context_destroy(ctx);
	ctx = NULL;

finish:

	/* Free stuff. */
	cl4_context_properties_default_free(properties, ctx_props);
	
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
 * @brief Increase the reference count of the context wrapper object.
 * 
 * @param context The context wrapper object. 
 * */
void cl4_context_ref(CL4Context* ctx) {
	
	/* Make sure platform wrapper object is not NULL. */
	g_return_if_fail(ctx != NULL);

	/* Increase reference count. */
	g_atomic_int_inc(&ctx->ref_count);
	
}

/** 
 * @brief Decrements the reference count of the context wrapper object.
 * If it reaches 0, the context wrapper object is destroyed.
 *
 * @param ctx The context wrapper object. 
 * */
void cl4_context_unref(CL4Context* ctx) {
	
	/* Make sure context wrapper object is not NULL. */
	g_return_if_fail(ctx != NULL);

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&ctx->ref_count)) {

		if (ctx->devices != NULL) {

			/* Release devices in ctx. */
			for (guint i = 0; i < ctx->num_devices; i++) {
				if (ctx->devices[i])
					cl4_device_unref(ctx->devices[i]);
			}
			
			/* Free device wrapper array. */
			g_free(ctx->devices);
		}
				  
		/* Release context. */
		if (ctx->context) {
			clReleaseContext(ctx->context);
		}
		
		/* Release platform. */
		if (ctx->platform) {
			cl4_platform_unref(ctx->platform);
		}
		
		/* Destroy hash table containing context information. */
		if (ctx->info) {
			g_hash_table_destroy(ctx->info);
		}
		
		/* Release ctx. */
		g_slice_free(CL4Context, ctx);

	}	

}

/**
 * @brief Returns the context wrapper object reference count. For
 * debugging and testing purposes only.
 * 
 * @param ctx The context wrapper object.
 * @return The context wrapper object reference count or -1 if device
 * is NULL.
 * */
gint cl4_context_ref_count(CL4Context* ctx) {
	
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, -1);
	
	/* Return reference count. */
	return ctx->ref_count;

}

/**
 * @brief Get context information object.
 * 
 * @param ctx The context wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested context information object. This object will
 * be automatically freed when the device wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
CL4Info* cl4_context_info(CL4Context* ctx, 
	cl_context_info param_name, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	
	/* Context information object. */
	CL4Info* info = NULL;
	
	/* If context information table is not yet initialized, then 
	 * initialize it. */
	if (!ctx->info) {
		ctx->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, 
			NULL, cl4_info_destroy);
	}

	/* Check if requested information is already present in the 
	 * context information table. */
	if (g_hash_table_contains(
		ctx->info, GUINT_TO_POINTER(param_name))) {
		
		/* If so, retrieve it from there. */
		info = g_hash_table_lookup(
			ctx->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		/* Otherwise, get it from OpenCL device.*/
		cl_int ocl_status;
		/* Device information placeholder. */
		gpointer param_value;
		/* Size of device information in bytes. */
		gsize size_ret;
		
		/* Get size of information. */
		ocl_status = clGetContextInfo(
			ctx->context, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
			"Function '%s': get context info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		gef_if_error_create_goto(*err, CL4_ERROR, 
			size_ret == 0, CL4_ERROR_OCL, error_handler, 
			"Function '%s': get context info [size] (size is 0).",
			__func__);
		
		/* Allocate memory for information. */
		param_value = g_malloc(size_ret);
		
		/* Get information. */
		ocl_status = clGetContextInfo(
			ctx->context, param_name, size_ret, param_value, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
			"Function '%s': get context info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
			
		/* Keep information in context information table. */
		info = cl4_info_new(param_value, size_ret);
		g_hash_table_insert(ctx->info, 
			GUINT_TO_POINTER(param_name), 
			info);
		
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	info = NULL;

finish:
	
	/* Return the requested device information. */
	return info;

}

/**
 * @brief Get the OpenCL context object.
 * 
 * @param context The context wrapper object.
 * @return The OpenCL context object.
 * */
cl_context cl4_context_unwrap(CL4Context* ctx) {
	
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	
	/* Return the OpenCL context object. */
	return ctx->context;
}
 
/** 
 * @brief Get CL4 device wrapper at given index. 
 * 
 * @param context The context wrapper object.
 * @param index Index of device in context.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The CL4 device wrapper at given index or NULL if an error 
 * occurs.
 * */
CL4Device* cl4_context_get_device(
	CL4Context* ctx, guint index, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	
	/* Make sure device index is less than the number of devices. */
	g_return_val_if_fail(index < ctx->num_devices, NULL);
	
	/* The return value. */
	CL4Device* device_ret;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* Check if device list is already initialized. */
	if (ctx->devices != NULL) {
		
		/* Not initialized, initialize it. */
		cl4_context_init_devices(ctx, &err_internal);
		
		/* Check for errors. */
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
	}
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	device_ret = ctx->devices[index];
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	device_ret = NULL;
	
finish:		
	
	/* Return list of device wrappers. */
	return device_ret;

}

/**
 * @brief Return number of devices in context.
 * 
 * @param ctx The context wrapper object.
 * @return The number of devices in context or 0 if an error occurs or
 * is otherwise not possible to get any device.
 * */
guint cl4_context_device_count(CL4Context* ctx) {
	
	/* Make sure context is not NULL. */
	g_return_val_if_fail(ctx != NULL, 0);
	
	/* Return the number of devices in context. */
	return ctx->num_devices;
}

