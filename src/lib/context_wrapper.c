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
 
#include "context_wrapper.h"

/**
 * Context wrapper object.
 */
struct cl4_context {
	
	/** Parent wrapper object. */
	CL4DevContainer base;
	
	/** Platform (can be lazy initialized). */
	CL4Platform* platform;
		
};

/**
 * @brief Implementation of cl4_wrapper_release_fields() function for
 * ::CL4Context wrapper objects.
 * 
 * @param ctx A ::CL4Context wrapper object.
 * */
static void cl4_context_release_fields(CL4Context* ctx) {

	/* Make sure context wrapper object is not NULL. */
	g_return_if_fail(ctx != NULL);
	
	/* Release devices. */
	cl4_dev_container_release_devices((CL4DevContainer*) ctx);

	/* Release platform. */
	if (ctx->platform) {
		cl4_platform_unref(ctx->platform);
	}
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
	cl_device_id device, GError** err) {
	
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Context properties. */
	cl_context_properties* ctx_props = NULL;
	
	/* The OpenCL platform object. */
	cl_platform_id platform;
	
	/* OpenCL function call status. */
	cl_int ocl_status;
	
	/* Check if the original const properties object is NULL... */
	if (properties == NULL) {
		
		/* ...if so, create a default set of context properties. */
		
		/* Allocate memory for default properties. */
		ctx_props = g_slice_alloc(3 * sizeof(cl_context_properties));

		/* Get context platform using first device. */
		ocl_status = clGetDeviceInfo(device, CL_DEVICE_PLATFORM, 
			sizeof(cl_platform_id), &platform, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
			"%s: unable to get platform from device (OpenCL error %d: %s).", 
			G_STRLOC, ocl_status, cl4_err(ocl_status));

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
 * @addtogroup CONTEXT
 * @{
 */

/**
 * @brief Get the context wrapper for the given OpenCL context.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL context directly (using the
 * clCreateContext() function) and then wrap the OpenCL context in a
 * ::CL4Context wrapper object.
 * 
 * @param context The OpenCL context to be wrapped.
 * @return The context wrapper for the given OpenCL context.
 * */
CL4Context* cl4_context_new_wrap(cl_context context) {
	
	return (CL4Context*) cl4_wrapper_new(
		(void*) context, sizeof(CL4Context));
		
}

/**
 * @brief Create a new context wrapper object selecting devices using 
 * the given set of filters. 
 * 
 * This function accepts all the parameters required for the 
 * clCreateContext() OpenCL function. For simple context creation use 
 * the cl4_context_new_from_filters() macro instead. 
 * 
 * The client should provide the necessary filters such that the 
 * filtered devices belong to the same platform. Adding the 
 * cl4_devsel_dep_platform() filter last to the filters set will ensure 
 * this.
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
	cl4_context_callback pfn_notify,
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

	/* Context wrapper to create. */
	CL4Context* ctx = NULL;

	/* Get selected/filtered devices. */
	devices = cl4_devsel_select(filters, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Check if any device was found. */
	gef_if_error_create_goto(*err, CL4_ERROR, devices->len == 0, 
		CL4_ERROR_DEVICE_NOT_FOUND, error_handler, 
		"%s: no device found for selected filters.", 
		G_STRLOC);

	/* Create context wrapper. */
	ctx = cl4_context_new_from_devices_full(properties, devices->len,
		(CL4Device**) devices->pdata, pfn_notify, user_data, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Free array object containing device wrappers. */
	g_ptr_array_free(devices, TRUE);

	/* Return ctx. */
	return ctx;
}

/** 
 * @brief Creates a context wrapper given an array of ::CL4Device 
 * wrappers and the remaining parameters required by the 
 * clCreateContext function. 
 * 
 * If the properties parameter is NULL, this function obtains 
 * the cl_platform_id object from the first device.
 * 
 * @param properties Context properties, may be NULL.
 * @param num_devices Number of ::CL4Device wrappers in array.
 * @param devices Array of ::CL4Device wrappers.
 * @param pfn_notify A callback function used by the OpenCL 
 * implementation to report information on errors during context 
 * creation as well as errors that occur at runtime in this context. 
 * Ignored if NULL.
 * @param user_data Passed as argument to pfn_notify, can be NULL.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object.
 * */
CL4Context* cl4_context_new_from_devices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	CL4Device** devices,
	cl4_context_callback pfn_notify,
    void* user_data,
    GError** err) {

	/* Make sure number of devices is not zero. */
	g_return_val_if_fail(num_devices > 0, NULL);
	/* Make sure device array is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Array of unwrapped devices. */
	cl_device_id cl_devices[num_devices];
	
	/* New context wrapper. */
	CL4Context* ctx = NULL;
	
	/* Unwrap devices. */
	for (guint i = 0; i < num_devices; i++)
		cl_devices[i] = cl4_device_unwrap(devices[i]);

	/* Create context wrapper. */
	ctx = cl4_context_new_from_cldevices_full(properties, num_devices,
		cl_devices, pfn_notify, user_data, err);
	
	/* Return result of function call. */
	return ctx;

}

/**
 * @brief Creates a context wrapper using the exact parameters received 
 * by the clCreateContext function. For simpler context wrapper creation 
 * use the cl4_context_new_from_cldevices() macro.
 * 
 * If the properties parameter is NULL, this function obtains 
 * the cl_platform_id object from the first device.
 * 
 * @param properties Context properties, may be NULL.
 * @param num_devices Number of cl_devices_id's in devices array.
 * @param devices Array of cl_device_id's.
 * @param pfn_notify A callback function used by the OpenCL 
 * implementation to report information on errors during context 
 * creation as well as errors that occur at runtime in this context. 
 * Ignored if NULL.
 * @param user_data Passed as argument to pfn_notify, can be NULL.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object.
 * */
CL4Context* cl4_context_new_from_cldevices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	const cl_device_id* devices,
	cl4_context_callback pfn_notify,
    void* user_data,
    GError** err) {
		
	/* Make sure number of devices is not zero. */
	g_return_val_if_fail(num_devices > 0, NULL);
	/* Make sure device array is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Error reporting object. */
	GError* err_internal = NULL;
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	/* OpenCL context. */
	cl_context context = NULL;
	/* Context wrapper to create. */
	CL4Context* ctx = NULL;
	
	/* Get a set of default context properties, if required. */
	ctx_props = cl4_context_properties_default(
			properties, devices[0], &err_internal);
	
	/* Create OpenCL context. */
	context = clCreateContext(
		(const cl_context_properties*) ctx_props, num_devices, devices, 
		pfn_notify, user_data, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: unable to create cl_context (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, cl4_err(ocl_status));
		
	/* Wrap OpenCL context. */
	ctx = cl4_context_new_wrap(context);

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

/** 
 * @brief Creates a context wrapper using one independent device filter 
 * specified in the function parameters.
 * 
 * The first device accepted by the given filter is used. More 
 * than one device may be used if all devices belong to the same 
 * platform (and pass the given filter).
 * 
 * @param filter An independent device filter. If NULL, no independent 
 * filter is used, and the first found device(s) is used.
 * @param data Specific filter data.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
CL4Context* cl4_context_new_from_indep_filter(
	cl4_devsel_indep filter, void* data, GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Error reporting object. */
	GError* err_internal = NULL;
	
	/* Context wrapper to create. */
	CL4Context* ctx;
	
	/* Set of device selection filters. */
	CL4DevSelFilters filters = NULL;

	/* Add specific independent filter. */
	if (filter != NULL)
		cl4_devsel_add_indep_filter(&filters, filter, data);

	/* Found devices should belong to the same platform. */
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_platform, NULL);
	
	/* Create a context with selected device(s). */
	ctx = cl4_context_new_from_filters(&filters, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	

	/* Return new context wrapper. */
	return ctx;
		
}

/** 
 * @brief Creates a context wrapper using a device which the user 
 * selects from a menu.
 * 
 * @param data If not NULL, can point to a device index, such that the
 * device is automatically selected.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
CL4Context* cl4_context_new_from_menu_full(void* data, GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Error reporting object. */
	GError* err_internal = NULL;
	
	/* Context wrapper to create. */
	CL4Context* ctx;
	
	/* Set of device selection filters. */
	CL4DevSelFilters filters = NULL;

	/* Add menu dependent filter. */
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_menu, data);
	
	/* Create a context with selected device. */
	ctx = cl4_context_new_from_filters(&filters, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	

	/* Return new context wrapper. */
	return ctx;
		
}

/** 
 * @brief Decrements the reference count of the context wrapper object. 
 * If it reaches 0, the context wrapper object is destroyed.
 *
 * @param ctx The context wrapper object.
 * */
void cl4_context_destroy(CL4Context* ctx) {
	
	cl4_wrapper_unref((CL4Wrapper*) ctx, sizeof(CL4Context),
		(cl4_wrapper_release_fields) cl4_context_release_fields, 
		(cl4_wrapper_release_cl_object) clReleaseContext, NULL); 

}

/** @}*/

/** 
 * @brief Implementation of cl4_dev_container_get_cldevices() for the
 * context wrapper. 
 * 
 * @param devcon A ::CL4Context wrapper, passed as a ::CL4DevContainer .
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A list of cl_device_id objects inside a ::CL4WrapperInfo
 * object.
 * */
CL4WrapperInfo* cl4_context_get_cldevices(
	CL4DevContainer* devcon, GError** err) {

	return cl4_context_get_info(devcon, CL_CONTEXT_DEVICES, err);
}
