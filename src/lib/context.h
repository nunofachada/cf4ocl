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

#ifndef CL4_CONTEXT_H
#define CL4_CONTEXT_H

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#include "devsel.h"
#include "gerrorf.h"
#include "common.h"
#include "errors.h"
#include "device.h"
#include "platform.h"

/**
 * @brief The context wrapper object encompasses the OpenCL context and
 * associated wrappers: devices, programs, queues and kernels. Should be 
 * accessed using the cl4_context_* group of functions.
 * */
typedef struct cl4_context CL4Context;

/** 
 * @brief A callback function used by the OpenCL 
 * implementation to report information on errors during context 
 * creation as well as errors that occur at runtime in this context.
 * Ignored if NULL.
 * 
 * @param errinfo Pointer to an error string.
 * @param private_info Pointer to binary data returned OpenCL, used to 
 * log additional debugging information.
 * @param cb Size of private_info data.
 * @param user_data Passed as the user_data argument when pfn_notify is 
 * called. user_data can be NULL.
 * */
typedef void (CL_CALLBACK* cl4_context_callback)(
	const char* errinfo, const void* private_info, size_t cb, 
	void* user_data);

/**
 * @defgroup CL4_CONTEXT_CONSTRUCTORS Context wrapper constructors,
 * i.e. cl4_context_new_* functions.
 *
 * @{
 */
 
/** 
 * @brief Create a new context wrapper object selecting devices using
 * the given set of filters. 
 *  
 * @param filters Filters for selecting device.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new context wrapper object.
 * */
#define cl4_context_new_from_filters(filters, err) \
	cl4_context_new_from_filters_full( \
		NULL, (filters), NULL, NULL, (err))

/** 
 * @brief Macro to create a context wrapper given a list of 
 * cl_device_id's.
 * 
 * This macro simply calls cl4_context_new_from_cldevices_full()
 * setting properties, callback and user data to NULL.
 * 
 * @param num_devices Number of cl_devices_id's in list.
 * @param devices List of cl_device_id's.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A new context wrapper object.
 * */
#define cl4_context_new_from_cldevices(num_devices, devices, err) \
	cl4_context_new_from_cldevices_full( \
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
#define cl4_context_new_cpu(err) \
	cl4_context_new_from_indep_filter(cl4_devsel_indep_type_cpu, err)
	
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
#define cl4_context_new_gpu(err) \
	cl4_context_new_from_indep_filter(cl4_devsel_indep_type_gpu, err)
	
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
#define cl4_context_new_accel(err) \
	cl4_context_new_from_indep_filter(cl4_devsel_indep_type_accel, err)
	
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
#define cl4_context_new_any(err) \
	cl4_context_new_from_indep_filter(NULL, err)	

/** @brief Create a new context wrapper object selecting devices using
 * the given set of filters. */
CL4Context* cl4_context_new_from_filters_full(
	const cl_context_properties* properties, 
	CL4DevSelFilters* filters,
	cl4_context_callback pfn_notify,
    void* user_data,
	GError **err);

/** @brief Creates a context wrapper using the exact parameters received
 * by the clCreateContext function. */
CL4Context* cl4_context_new_from_cldevices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	const cl_device_id* devices,
	cl4_context_callback pfn_notify,
    void* user_data,
    GError** err);

/** @brief Creates a context wrapper from a cl_context object. */
CL4Context* cl4_context_new_from_clcontext(
	cl_context context, GError** err);

/** @brief Creates a context wrapper using one independent device filter
 * specified in the function parameters. */
CL4Context* cl4_context_new_from_indep_filter(
	cl4_devsel_indep type_filter, GError** err);

/* @todo Future work */
// CL4Context* cl4_context_new_from_clplatform(cl_platform_id platform, GError** err);
// CL4Context* cl4_context_new_from_platform(CL4Platform* platform, GError** err);
// CL4Context* cl4_context_new_from_devices(CL4Devices** devices, GError** err);

/** @} */

/** @brief Decrements the reference count of the context wrapper object.
 * If it reaches 0, the context wrapper object is destroyed. */
void cl4_context_destroy(CL4Context* ctx);

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
#define cl4_context_info(ctx, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) ctx, param_name, \
		(cl4_wrapper_info_function) clGetContextInfo, err)

/** 
 * @brief Increase the reference count of the context wrapper object.
 * 
 * @param ctx The context wrapper object. 
 * */
#define cl4_context_ref(ctx) \
	cl4_wrapper_ref((CL4Wrapper*) ctx)

/**
 * @brief Alias to cl4_context_destroy().
 * 
 * @param ctx Context wrapper object to destroy if reference count is 1,
 * otherwise just decrement the reference count.
 * */
#define cl4_context_unref(ctx) cl4_context_destroy(ctx)

/**
 * @brief Get the OpenCL context object.
 * 
 * @param ctx The context wrapper object.
 * @return The OpenCL context object.
 * */
#define cl4_context_unwrap(ctx) \
	((cl_context) cl4_wrapper_unwrap((CL4Wrapper*) ctx))
 
/** @brief Get CL4 device wrapper at given index. */
CL4Device* cl4_context_get_device(
	CL4Context* ctx, guint index, GError** err);

/** @brief Return number of devices in context. */
guint cl4_context_num_devices(CL4Context* ctx, GError** err);

#endif
