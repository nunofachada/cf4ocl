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
 *
 * Implementation of a wrapper class and its methods for OpenCL context objects.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_context_wrapper.h"
#include "_ccl_abstract_dev_container_wrapper.h"
#include "_ccl_defs.h"

/**
 * The context wrapper class.
 *
 * @extends ccl_dev_container
 * */
struct ccl_context {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLDevContainer base;

	/**
	 * Platform (can be lazy initialized).
	 * @private
	 * */
	CCLPlatform* platf;

};

/**
 * @internal
 * Implementation of ccl_wrapper_release_fields() function for ::CCLContext
 * wrapper objects.
 *
 * @private @memberof ccl_context
 *
 * @param[in] ctx A ::CCLContext wrapper object.
 * */
static void ccl_context_release_fields(CCLContext* ctx) {

	/* Make sure context wrapper object is not NULL. */
	g_return_if_fail(ctx != NULL);

	/* Release devices. */
	ccl_dev_container_release_devices((CCLDevContainer*) ctx);

	/* Release platform. */
	if (ctx->platf) {
		ccl_platform_unref(ctx->platf);
	}
}

/**
 * @internal
 * Free the default context properties if required.
 *
 * @private @memberof ccl_context
 *
 * @param[in] properties The original const properties, may be NULL, in
 * which case the ctx_props parameter will be freed.
 * @param[in] ctx_props Context properties to be freed, if different
 * than the original const properties parameter.
 * */
#define ccl_context_properties_default_free(properties, ctx_props) \
	if (properties == NULL) /* Could also be (properties != ctx_props) */ \
		g_slice_free1(3 * sizeof(cl_context_properties), ctx_props)

/**
 * @internal
 * Create a default context properties object, if required. The
 * only property set in the default properties object is the OpenCL
 * cl_platform_id object.
 *
 * @private @memberof ccl_context
 *
 * @param[in] properties Original const properties, which if NULL imply
 * that a new default properties object should be created.
 * @param[in] device Reference device to build the context properties
 * for.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The properties parameter if not NULL, or a default set of
 * context properties.
 * */
static cl_context_properties* ccl_context_properties_default(
	const cl_context_properties* properties,
	cl_device_id device, CCLErr** err) {

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
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: unable to get platform from device (OpenCL error %d: %s).",
			CCL_STRD, ocl_status, ccl_err(ocl_status));

		/* Set context properties using discovered platform. */
		ctx_props[0] = CL_CONTEXT_PLATFORM;
		ctx_props[1] = (cl_context_properties) platform;
		ctx_props[2] = 0;

	} else {

		/* If properties parameter is not NULL, use it instead. */
		ctx_props = (cl_context_properties*) properties;
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return properties. */
	return ctx_props;

}

/**
 * @internal
 * Implementation of ccl_dev_container_get_cldevices() for the
 * context wrapper.
 *
 * @private @memberof ccl_context
 *
 * @param[in] devcon A ::CCLContext wrapper, passed as a
 * ::CCLDevContainer .
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A list of cl_device_id objects inside a ::CCLWrapperInfo
 * object.
 * */
static CCLWrapperInfo* ccl_context_get_cldevices(
	CCLDevContainer* devcon, CCLErr** err) {

	return ccl_context_get_info(devcon, CL_CONTEXT_DEVICES, err);
}

/**
 * @addtogroup CCL_CONTEXT_WRAPPER
 * @{
 */

/**
 * Get the context wrapper for the given OpenCL context.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL context directly (using the
 * clCreateContext() function) and then wrap the OpenCL context in a
 * ::CCLContext wrapper object.
 *
 * @public @memberof ccl_context
 *
 * @param[in] context The OpenCL context to be wrapped.
 * @return The context wrapper for the given OpenCL context.
 * */
CCL_EXPORT
CCLContext* ccl_context_new_wrap(cl_context context) {

	return (CCLContext*) ccl_wrapper_new(
		CCL_CONTEXT, (void*) context, sizeof(CCLContext));

}

/**
 * Create a new context wrapper object selecting devices using
 * the given set of filters.
 *
 * This function accepts all the parameters required for the
 * clCreateContext() OpenCL function. For simple context creation use
 * the ccl_context_new_from_filters() macro instead.
 *
 * The client should provide the necessary filters such that the
 * filtered devices belong to the same platform. Adding the
 * ccl_devsel_dep_platform() filter last to the filters set will ensure
 * this.
 *
 * @public @memberof ccl_context
 *
 * @param[in] properties A set of OpenCL context properties.
 * @param[in] filters Filters for selecting device.
 * @param[in] pfn_notify A callback function used by the OpenCL
 * implementation to report information on errors during context
 * creation as well as errors that occur at runtime in this context.
 * Ignored if NULL.
 * @param[in] user_data Passed as argument to pfn_notify, can be NULL.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object.
 * */
CCL_EXPORT
CCLContext* ccl_context_new_from_filters_full(
	const cl_context_properties* properties, CCLDevSelFilters* filters,
	ccl_context_callback pfn_notify, void* user_data, CCLErr **err) {

	/* Make sure number ds is not NULL. */
	g_return_val_if_fail(filters != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Error reporting object. */
	CCLErr* err_internal = NULL;

	/* Array of selected/filtered CCL device wrappers. */
	GPtrArray* devices = NULL;

	/* Context wrapper to create. */
	CCLContext* ctx = NULL;

	/* Get selected/filtered devices. */
	devices = ccl_devsel_select(filters, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Check if any device was found. */
	ccl_if_err_create_goto(*err, CCL_ERROR, devices->len == 0,
		CCL_ERROR_DEVICE_NOT_FOUND, error_handler,
		"%s: no device found for selected filters.",
		CCL_STRD);

	/* Create context wrapper. */
	ctx = ccl_context_new_from_devices_full(properties, devices->len,
		(CCLDevice**) devices->pdata, pfn_notify, user_data,
		&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free array object containing device wrappers. */
	if (devices != NULL) g_ptr_array_free(devices, TRUE);

	/* Return ctx. */
	return ctx;
}

/**
 * Creates a context wrapper given an array of ::CCLDevice wrappers and the
 * remaining parameters required by the clCreateContext() function.
 *
 * If the properties parameter is NULL, this function obtains the cl_platform_id
 * object from the first device.
 *
 * @public @memberof ccl_context
 *
 * @param[in] properties Context properties, may be NULL.
 * @param[in] num_devices Number of ::CCLDevice wrappers in array.
 * @param[in] devices Array of ::CCLDevice wrappers.
 * @param[in] pfn_notify A callback function used by the OpenCL implementation
 * to report information on errors during context creation as well as errors
 * that occur at runtime in this context. Ignored if NULL.
 * @param[in] user_data Passed as argument to pfn_notify, can be NULL.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object.
 * */
CCL_EXPORT
CCLContext* ccl_context_new_from_devices_full(
	const cl_context_properties* properties, cl_uint num_devices,
	CCLDevice* const* devices, ccl_context_callback pfn_notify,
	void* user_data, CCLErr** err) {

	/* Make sure number of devices is not zero. */
	g_return_val_if_fail(num_devices > 0, NULL);
	/* Make sure device array is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Array of unwrapped devices. */
	cl_device_id* cl_devices = NULL;
	/* Context properties, in case the properties parameter is NULL. */
	cl_context_properties* ctx_props = NULL;
	/* Return status of OpenCL function calls. */
	cl_int ocl_status;
	/* OpenCL context. */
	cl_context context = NULL;
	/* New context wrapper. */
	CCLContext* ctx = NULL;
	/* Error reporting object. */
	CCLErr* err_internal = NULL;

	/* Allocate memory for devices. */
	cl_devices =
		(cl_device_id*) g_slice_alloc(sizeof(cl_device_id) * num_devices);

	/* Unwrap devices. */
	for (guint i = 0; i < num_devices; i++)
		cl_devices[i] = ccl_device_unwrap(devices[i]);

	/* Get a set of default context properties, if required. */
	ctx_props = ccl_context_properties_default(
			properties, cl_devices[0], &err_internal);

	/* Create OpenCL context. */
	context = clCreateContext(
		(const cl_context_properties*) ctx_props, num_devices,
		(const cl_device_id*) cl_devices, pfn_notify, user_data,
		&ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create cl_context (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Wrap OpenCL context. */
	ctx = ccl_context_new_wrap(context);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Destroy what was built for the context wrapper. */
	ccl_context_destroy(ctx);
	ctx = NULL;

finish:

	/* Free stuff. */
	ccl_context_properties_default_free(properties, ctx_props);
	g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);

	/* Return result of function call. */
	return ctx;

}

/**
 * Creates a context wrapper using one device filter specified in the function
 * parameters.
 *
 * The first device accepted by the given filter is used. More than one device
 * may be used if all devices belong to the same platform (and pass the given
 * filter).
 *
 * @public @memberof ccl_context
 *
 * @param[in] ftype Filter type, either ::CCL_DEVSEL_DEP or ::CCL_DEVSEL_INDEP.
 * @param[in] filter Filter used to select device(s). If `NULL`, no filter is
 * used, and the first found device(s) is selected.
 * @param[in] data Specific filter data.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLContext* ccl_context_new_from_filter(CCLDevSelFilterType ftype,
	void* filter, void* data, CCLErr** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Error reporting object. */
	CCLErr* err_internal = NULL;

	/* Context wrapper to create. */
	CCLContext* ctx = NULL;

	/* Set of device selection filters. */
	CCLDevSelFilters filters = NULL;

	/* Add filter, if any was specified. */
	if (filter != NULL) {

		/* What type of filter? */
		if (ftype == CCL_DEVSEL_DEP) {
			/* Dependent filter. */
			ccl_devsel_add_dep_filter(
				&filters, (ccl_devsel_dep) filter, data);
		} else if (ftype == CCL_DEVSEL_INDEP) {
			/* Independent filter. */
			ccl_devsel_add_indep_filter(
				&filters, (ccl_devsel_indep) filter, data);
		} else {
			/* Unknown filter type. */
			ccl_if_err_create_goto(*err, CCL_ERROR, TRUE, CCL_ERROR_ARGS,
				error_handler, "%s: Unknown filter type.", CCL_STRD);
		}

	}

	/* Found devices should belong to the same platform. */
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Create a context with selected device. */
	ctx = ccl_context_new_from_filters(&filters, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return new context wrapper. */
	return ctx;

}

/**
 * Decrements the reference count of the context wrapper object.
 * If it reaches 0, the context wrapper object is destroyed.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * */
CCL_EXPORT
void ccl_context_destroy(CCLContext* ctx) {

	ccl_wrapper_unref((CCLWrapper*) ctx, sizeof(CCLContext),
		(ccl_wrapper_release_fields) ccl_context_release_fields,
		(ccl_wrapper_release_cl_object) clReleaseContext, NULL);

}

/**
 * Get the OpenCL version of the platform associated with this context.
 * The version is returned as an integer, in the following format:
 *
 * * 100 for OpenCL 1.0
 * * 110 for OpenCL 1.1
 * * 120 for OpenCL 1.2
 * * 200 for OpenCL 2.0
 * * 210 for OpenCL 2.1
 * * etc.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL version of the platform associated with this
 * context as an integer. If an error occurs, 0 is returned.
 * */
CCL_EXPORT
cl_uint ccl_context_get_opencl_version(CCLContext* ctx, CCLErr** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, 0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

	CCLPlatform* platf;
	cl_uint ver;

	platf = ccl_context_get_platform(ctx, err);
	if (platf == NULL) {
		ver = 0;
	} else {
		ver = ccl_platform_get_opencl_version(platf, err);
	}
	return ver;
}

/**
 * Get the platform associated with the context devices.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLPlatform wrapper associated with the context
 * devices or `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLPlatform* ccl_context_get_platform(CCLContext* ctx, CCLErr** err) {

	/* Make sure context is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	CCLPlatform* platf = NULL;
	CCLDevice* dev = NULL;
	CCLErr* err_internal = NULL;

	/* Check if platform wrapper already in context object. */
	if (ctx->platf != NULL) {
		/* Yes, use it. */
		platf = ctx->platf;
	} else {
		/* Get platform using device. */
		dev = ccl_context_get_device(ctx, 0, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		platf = ccl_platform_new_from_device(dev, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		/* Keep platform. */
		ctx->platf = platf;
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return platform wrapper. */
	return platf;

}

/**
 * Get the list of image formats supported by a given context. This
 * function wraps the clGetSupportedImageFormats() OpenCL function.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx A context wrapper object.
 * @param[in] flags Allocation and usage information about the image
 * memory object being queried.
 * @param[in] image_type The image type. Acceptable values depend on the
 * OpenCL version.
 * @param[out] num_image_formats Return location for number of image
 * formats in list, which will be zero if an error occurs.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A list of supported image formats, or `NULL` if an error
 * occurs. Doesn't need to be freed.
 * */
CCL_EXPORT
const cl_image_format* ccl_context_get_supported_image_formats(
	CCLContext* ctx, cl_mem_flags flags, cl_mem_object_type image_type,
	cl_uint* num_image_formats, CCLErr** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure num_image_formats is not NULL. */
	g_return_val_if_fail(num_image_formats != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Information object. We use it to keep image format information
	 * in the information table, so that it can be automatically
	 * destroyed when the context is destroyed. */
	CCLWrapperInfo* info = NULL;

	/* Variable to return. */
	const cl_image_format* image_formats = NULL;

	/* Let's query OpenCL object.*/
	cl_int ocl_status;

	/* Get number of image formats. */
	ocl_status = clGetSupportedImageFormats(ccl_context_unwrap(ctx),
		flags, image_type, 0, NULL, num_image_formats);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: get number of supported image formats (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));
	ccl_if_err_create_goto(*err, CCL_ERROR,
		*num_image_formats == 0, CCL_ERROR_OTHER, error_handler,
		"%s: number of returned supported image formats is 0.",
		CCL_STRD);

	/* Allocate memory for number of image formats. */
	info = ccl_wrapper_info_new(
		(*num_image_formats) * sizeof(cl_image_format));

	/* Get image formats. */
	ocl_status = clGetSupportedImageFormats(ccl_context_unwrap(ctx),
		flags, image_type, *num_image_formats,
		(cl_image_format*) info->value, NULL);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: get supported image formats (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Keep information in information table for latter disposal. */
	ccl_wrapper_add_info((CCLWrapper*) ctx, CL_IMAGE_FORMAT, info);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	image_formats = (const cl_image_format*) info->value;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	*num_image_formats = 0;

finish:

	/* Return requested supported image formats. */
	return image_formats;
}


/**
 * Get ::CCLDevice wrapper at given index.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] index Index of device in context.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLDevice wrapper at given index or `NULL` if an error
 * occurs.
 * */
CCL_EXPORT
CCLDevice* ccl_context_get_device(
	CCLContext* ctx, cl_uint index, CCLErr** err) {

	return ccl_dev_container_get_device((CCLDevContainer*) ctx,
		ccl_context_get_cldevices, index, err);
}

/**
 * Return number of devices in context.
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The number of devices in context or 0 if an error occurs or
 * is otherwise not possible to get any device.
 * */
CCL_EXPORT
cl_uint ccl_context_get_num_devices(CCLContext* ctx, CCLErr** err) {

	return ccl_dev_container_get_num_devices((CCLDevContainer*) ctx,
		ccl_context_get_cldevices, err);

}

/**
 * Get all device wrappers in context.
 *
 * This function returns the internal array containing the context
 * device wrappers. As such, clients should not modify the returned
 * array (e.g. they should not free it directly).
 *
 * @public @memberof ccl_context
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] err Return location for a ::CCLErr object, or NULL if error
 * reporting is to be ignored.
 * @return An array containing the ::CCLDevice wrappers which belong to
 * the given context, or `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLDevice* const* ccl_context_get_all_devices(CCLContext* ctx,
	CCLErr** err) {

	return ccl_dev_container_get_all_devices((CCLDevContainer*) ctx,
		ccl_context_get_cldevices, err);

}

/** @}*/
