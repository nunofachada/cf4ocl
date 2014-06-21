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

//#include "devsel.h"
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

#define cl4_context_new_from_filters(num_filters, filters, err) \
	cl4_context_new_from_filters_full( \
		NULL, (num_filters), (filters), NULL, NULL, (err))

/** @brief Macro to create a context wrapper given a list of 
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

CL4Context* cl4_context_new_from_filters_full(
	const cl_context_properties* properties, 
	guint num_filters, 
	cl4_devsel* filters,
	void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
    void* user_data,
	GError **err);

/** @brief Creates a context wrapper using the exact parameters received
 * by the clCreateContext function. */
CL4Context* cl4_context_new_from_cldevices_full(
	const cl_context_properties* properties, 
	cl_uint num_devices,
	const cl_device_id* devices,
	void (CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*),
    void* user_data,
    GError** err);


/** @brief Creates a context wrapper from a cl_context object. */
CL4Context* cl4_context_new_from_clcontext(cl_context ctx, GError **err);

/** @brief Destroy a context wrapper object. */
 void cl4_context_destroy(CL4Context* ctx);

#endif
