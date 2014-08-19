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
 * Definition of a wrapper class and its methods for OpenCL context
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_CONTEXT_WRAPPER_H_
#define _CCL_CONTEXT_WRAPPER_H_

#include "oclversions.h"
#include "device_selector.h"
#include "common.h"
#include "errors.h"
#include "device_wrapper.h"
#include "platform_wrapper.h"
#include "abstract_dev_container_wrapper.h"

/**
 * @defgroup CONTEXT_WRAPPER Context wrapper
 *
 * A wrapper object for OpenCL contexts and functions to manage 
 * them.
 * 
 * @todo Detailed description of module with code examples.
 * 
 * @{
 */
 
/**
 * The context wrapper class.
 * 
 * @extends ccl_dev_container
 * */
typedef struct ccl_context CCLContext;

/** 
 * A callback function used by the OpenCL implementation to 
 * report information on errors during context creation as well as 
 * errors that occur at runtime in this context. Ignored if NULL.
 * 
 * @param[in] errinfo Pointer to an error string.
 * @param[out] private_info Pointer to binary data returned by OpenCL,
 * used to log additional debugging information.
 * @param[out] cb Size of private_info data.
 * @param[in] user_data Passed as the user_data argument when pfn_notify
 * is called. user_data can be NULL.
 * */
typedef void (CL_CALLBACK* ccl_context_callback)(
	const char* errinfo, const void* private_info, size_t cb, 
	void* user_data);
 
/* Get the context wrapper for the given OpenCL context. */
CCLContext* ccl_context_new_wrap(cl_context context);

/** 
 * Create a new context wrapper object selecting devices using 
 * the given set of filters.
 *  
 * @param[in] filters Filters for selecting device.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object.
 * */
#define ccl_context_new_from_filters(filters, err) \
	ccl_context_new_from_filters_full( \
		NULL, (filters), NULL, NULL, (err))

/** 
 * Creates a context wrapper given an array of ::CCLDevice 
 * wrappers.
 * 
 * This macro simply calls ccl_context_new_from_devices_full() 
 * setting properties, callback and user data to NULL.
 * 
 * @param[in] num_devices Number of cl_devices_id's in list.
 * @param[in] devices Array of ::CCLDevice wrappers.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return A new context wrapper object.
 * */
#define ccl_context_new_from_devices(num_devices, devices, err) \
	ccl_context_new_from_devices_full( \
		NULL, (num_devices), (devices), NULL, NULL, (err))

/** 
 * Creates a context wrapper for a CPU device.
 * 
 * The first found CPU device is used. More than one CPU might be used 
 * if all CPUs belong to the same platform.
 * 
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
#define ccl_context_new_cpu(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_cpu, NULL, err)
	
/** 
 * Creates a context wrapper for a GPU device.
 * 
 * The first found GPU device is used. More than one GPU might be used 
 * if all GPUs belong to the same platform.
 * 
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
#define ccl_context_new_gpu(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_gpu, NULL, err)
	
/** 
 * Creates a context wrapper for an Accelerator device.
 * 
 * The first found Accelerator device is used. More than one Accelerator 
 * might be used if all Accelerators belong to the same platform.
 * 
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
#define ccl_context_new_accel(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_accel, NULL, err)
	
/** 
 * Creates a context wrapper for the fist found device(s).
 * 
 * The first found device is used. More than one device might be used if 
 * all devices belong to the same platform.
 * 
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
#define ccl_context_new_any(err) \
	ccl_context_new_from_indep_filter(NULL, NULL, err)


/** 
 * Creates a context wrapper from a device selected by the user
 * from a menu.
 * 
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new context wrapper object or `NULL` if an error occurs.
 * */
#define ccl_context_new_from_menu(err) \
	ccl_context_new_from_menu_full(NULL, err)

/* Create a new context wrapper object selecting devices using 
 * the given set of filters. */
CCLContext* ccl_context_new_from_filters_full(
	const cl_context_properties* properties, CCLDevSelFilters* filters,
	ccl_context_callback pfn_notify, void* user_data, GError **err);

/* Creates a context wrapper given an array of ::CCLDevice 
 * wrappers and the remaining parameters required by the 
 * clCreateContext function. */
CCLContext* ccl_context_new_from_devices_full(
	const cl_context_properties* properties, cl_uint num_devices,
	CCLDevice* const* devices, ccl_context_callback pfn_notify,
	void* user_data, GError** err);

/* Creates a context wrapper using one independent device filter 
 * specified in the function parameters. */
CCLContext* ccl_context_new_from_indep_filter(
	ccl_devsel_indep filter, void* data, GError** err);
	
/* Creates a context wrapper using a device which the user 
 * selects from a menu. */
CCLContext* ccl_context_new_from_menu_full(void* data, GError** err);

/* Decrements the reference count of the context wrapper object. 
 * If it reaches 0, the context wrapper object is destroyed. */
void ccl_context_destroy(CCLContext* ctx);

/* Get the OpenCL version of the platform associated with this 
 * context. */
double ccl_context_get_opencl_version(
	CCLContext* ctx, GError** err);
	
/* Get the platform associated with the context devices. */
CCLPlatform* ccl_context_get_platform(CCLContext* ctx, GError** err);

/* Get the list of image formats supported by a given context. */
const cl_image_format* ccl_context_get_supported_image_formats(
	CCLContext* ctx, cl_mem_flags flags, cl_mem_object_type image_type, 
	cl_uint* num_image_formats, GError** err);

/* Get ::CCLDevice wrapper at given index. */
CCLDevice* ccl_context_get_device(
	CCLContext* ctx, cl_uint index, GError** err);

/* Return number of devices in context. */
cl_uint ccl_context_get_num_devices(CCLContext* ctx, GError** err);

/* Get all device wrappers in context. */
CCLDevice* const* ccl_context_get_all_devices(CCLContext* ctx, 
	GError** err);

/**
 * Get a ::CCLWrapperInfo context information object.
 * 
 * @param[in] ctx The context wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested context information object. This object will
 * be automatically freed when the context wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_context_get_info(ctx, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) ctx, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetContextInfo, CL_TRUE, err)

/** 
 * Macro which returns a scalar context information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param[in] ctx The context wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested context information value. This value will be 
 * automatically freed when the context wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define ccl_context_get_scalar_info(ctx, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) ctx, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetContextInfo, \
		CL_TRUE, err))

/** 
 * Macro which returns an array context information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param[in] ctx The context wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested context information value. This value will be 
 * automatically freed when the context wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define ccl_context_get_array_info(ctx, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) ctx, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetContextInfo, \
		CL_TRUE, err)
/** 
 * Increase the reference count of the context wrapper object.
 * 
 * @param[in] ctx The context wrapper object. 
 * */
#define ccl_context_ref(ctx) \
	ccl_wrapper_ref((CCLWrapper*) ctx)

/**
 * Alias to ccl_context_destroy().
 * 
 * @param[in] ctx Context wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_context_unref(ctx) \
	ccl_context_destroy(ctx)

/**
 * Get the OpenCL context object.
 * 
 * @param[in] ctx The context wrapper object.
 * @return The OpenCL context object.
 * */
#define ccl_context_unwrap(ctx) \
	((cl_context) ccl_wrapper_unwrap((CCLWrapper*) ctx))

/** @} */

#endif
