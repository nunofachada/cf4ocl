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

#ifndef _CCL_CONTEXT_WRAPPER_H_
#define _CCL_CONTEXT_WRAPPER_H_

#include "oclversions.h"
#include "device_selector.h"
#include "gerrorf.h"
#include "common.h"
#include "errors.h"
#include "device_wrapper.h"
#include "platform_wrapper.h"
#include "abstract_dev_container_wrapper.h"

/**
 * @defgroup CONTEXT_WRAPPER Context wrapper
 *
 * @brief A wrapper object for OpenCL contexts and functions to manage 
 * them.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */
 
/**
 * @brief The context wrapper object wraps the OpenCL context and 
 * associated devices. Should be managed with the 
 * @ref CONTEXT_WRAPPER "context wrapper module" group of functions and 
 * macros.
 * */
typedef struct ccl_context CCLContext;

/** 
 * @brief A callback function used by the OpenCL implementation to 
 * report information on errors during context creation as well as 
 * errors that occur at runtime in this context. Ignored if NULL.
 * 
 * @param errinfo Pointer to an error string.
 * @param private_info Pointer to binary data returned OpenCL, used to 
 * log additional debugging information.
 * @param cb Size of private_info data.
 * @param user_data Passed as the user_data argument when pfn_notify is 
 * called. user_data can be NULL.
 * */
typedef void (CL_CALLBACK* ccl_context_callback)(
	const char* errinfo, const void* private_info, size_t cb, 
	void* user_data);
 
/** 
 * @brief Create a new context wrapper object selecting devices using 
 * the given set of filters. 
 *  
 * @param filters Filters for selecting device.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object.
 * */
#define ccl_context_new_from_filters(filters, err) \
	ccl_context_new_from_filters_full( \
		NULL, (filters), NULL, NULL, (err))

/** 
 * @brief Creates a context wrapper given an array of ::CCLDevice 
 * wrappers.
 * 
 * This macro simply calls ccl_context_new_from_devices_full() 
 * setting properties, callback and user data to NULL.
 * 
 * @param num_devices Number of cl_devices_id's in list.
 * @param devices Array of ::CCLDevice wrappers.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object.
 * */
#define ccl_context_new_from_devices(num_devices, devices, err) \
	ccl_context_new_from_devices_full( \
		NULL, (num_devices), (devices), NULL, NULL, (err))

/** 
 * @brief Creates a context wrapper for a CPU device.
 * 
 * The first found CPU device is used. More than one CPU might be used 
 * if all CPUs belong to the same platform.
 * 
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
#define ccl_context_new_cpu(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_cpu, NULL, err)
	
/** 
 * @brief Creates a context wrapper for a GPU device.
 * 
 * The first found GPU device is used. More than one GPU might be used 
 * if all GPUs belong to the same platform.
 * 
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
#define ccl_context_new_gpu(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_gpu, NULL, err)
	
/** 
 * @brief Creates a context wrapper for an Accelerator device.
 * 
 * The first found Accelerator device is used. More than one Accelerator 
 * might be used if all Accelerators belong to the same platform.
 * 
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
#define ccl_context_new_accel(err) \
	ccl_context_new_from_indep_filter( \
		ccl_devsel_indep_type_accel, NULL, err)
	
/** 
 * @brief Creates a context wrapper for the fist found device(s).
 * 
 * The first found device is used. More than one device might be used if 
 * all devices belong to the same platform.
 * 
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A new context wrapper object or NULL if an error occurs.
 * */
#define ccl_context_new_any(err) \
	ccl_context_new_from_indep_filter(NULL, NULL, err)

/** @brief Create a new context wrapper object selecting devices using 
 * the given set of filters. */
CCLContext* ccl_context_new_from_filters_full(
	const cl_context_properties* properties, 
	CCLDevSelFilters* filters,
	ccl_context_callback pfn_notify,
    void* user_data,
	GError **err);

/** @brief Creates a context wrapper given an array of ::CCLDevice 
 * wrappers and the remaining parameters required by the 
 * clCreateContext function. */
CCLContext* ccl_context_new_from_devices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	CCLDevice** devices,
	ccl_context_callback pfn_notify,
    void* user_data,
    GError** err);

/** @brief Creates a context wrapper using one independent device filter 
 * specified in the function parameters. */
CCLContext* ccl_context_new_from_indep_filter(
	ccl_devsel_indep filter, void* data, GError** err);
	
/** @brief Creates a context wrapper using a device which the user 
 * selects from a menu. */
CCLContext* ccl_context_new_from_menu_full(void* data, GError** err);

#define ccl_context_new_from_menu(err) \
	ccl_context_new_from_menu_full(NULL, err)

/** @brief Decrements the reference count of the context wrapper object. 
 * If it reaches 0, the context wrapper object is destroyed. */
void ccl_context_destroy(CCLContext* ctx);

/**
 * @brief Get a ::CCLWrapperInfo context information object.
 * 
 * @param ctx The context wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested context information object. This object will
 * be automatically freed when the context wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_context_get_info(ctx, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) ctx, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetContextInfo, CL_TRUE, err)

/** 
 * @brief Macro which returns a scalar context information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param ctx The context wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested context information value. This value will be 
 * automatically freed when the context wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define ccl_context_get_scalar_info(ctx, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) ctx, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetContextInfo, \
		CL_TRUE, err))

/** 
 * @brief Macro which returns an array context information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param ctx The context wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested context information value. This value will be 
 * automatically freed when the context wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define ccl_context_get_array_info(ctx, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) ctx, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetContextInfo, \
		CL_TRUE, err)
/** 
 * @brief Increase the reference count of the context wrapper object.
 * 
 * @param ctx The context wrapper object. 
 * */
#define ccl_context_ref(ctx) \
	ccl_wrapper_ref((CCLWrapper*) ctx)

/**
 * @brief Alias to ccl_context_destroy().
 * 
 * @param ctx Context wrapper object to destroy if reference count is 1, 
 * otherwise just decrement the reference count.
 * */
#define ccl_context_unref(ctx) \
	ccl_context_destroy(ctx)

/**
 * @brief Get the OpenCL context object.
 * 
 * @param ctx The context wrapper object.
 * @return The OpenCL context object.
 * */
#define ccl_context_unwrap(ctx) \
	((cl_context) ccl_wrapper_unwrap((CCLWrapper*) ctx))
 
/** 
 * @brief Get ::CCLDevice wrapper at given index. 
 * 
 * @param ctx The context wrapper object.
 * @param index Index of device in context.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The ::CCLDevice wrapper at given index or NULL if an error 
 * occurs.
 * */
#define ccl_context_get_device(ctx, index, err) \
	ccl_dev_container_get_device((CCLDevContainer*) ctx, \
	ccl_context_get_cldevices, index, err)

/**
 * @brief Return number of devices in context.
 * 
 * @param ctx The context wrapper object.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The number of devices in context or 0 if an error occurs or 
 * is otherwise not possible to get any device.
 * */
#define ccl_context_get_num_devices(ctx, err) \
	ccl_dev_container_get_num_devices((CCLDevContainer*) ctx, \
	ccl_context_get_cldevices, err)

/** @} */

/** @brief Get the context wrapper for the given OpenCL context. */
CCLContext* ccl_context_new_wrap(cl_context context);

/** @brief Implementation of ccl_dev_container_get_cldevices() for the
 * context wrapper. */
CCLWrapperInfo* ccl_context_get_cldevices(
	CCLDevContainer* devcon, GError** err);

#endif
