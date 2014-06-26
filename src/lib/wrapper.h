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
 * @brief Generic OpenCL object wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_WRAPPER_H
#define CL4_WRAPPER_H 

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include "gerrorf.h"
#include "common.h"
#include "errors.h"

typedef struct cl4_wrapper {

	/** The wrapped OpenCL object. */
	gpointer cl_object;

	/** Information about the wrapped OpenCL object. */
	GHashTable* info;

	/** Reference count. */
	gint ref_count;    
	
} CL4Wrapper;

/** @brief Initialize wrapper fields. */
void cl4_wrapper_init(CL4Wrapper* wrapper);

/** @brief Increase the reference count of the wrapper object. */
void cl4_wrapper_ref(CL4Wrapper* wrapper);

/** @brief Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed. */
gpointer cl4_wrapper_unref(CL4Wrapper* device);

/** @brief Returns the wrapper object reference count. For debugging and 
 * testing purposes only. */
gint cl4_wrapper_ref_count(CL4Wrapper* wrapper);

/** @brief Get the wrapped OpenCL object. */
gpointer cl4_wrapper_unwrap(CL4Wrapper* wrapper);




typedef cl_int (*cl4_wrapper_info_function)(gpointer cl_object,
	cl_uint param_name, size_t param_value_size, void* param_value,
	size_t* param_value_size_ret);

/**
 * @brief Information about a wrapped OpenCL entity.
 * */
typedef struct cl4_info {
	/** Device information. */
	gpointer value;
	/** Size in bytes of device information. */
	gsize size;
} CL4WrapperInfo;

/** @brief Create a new CL4WrapperInfo* object with a given value size. */
CL4WrapperInfo* cl4_wrapper_info_new(gsize size);

/** @brief Destroy a CL4WrapperInfo* object. */
void cl4_wrapper_info_destroy(CL4WrapperInfo* info);

/** @brief Get information about any wrapped OpenCL object. */
CL4WrapperInfo* cl4_wrapper_get_info(CL4Wrapper* wrapper, 
	cl_uint param_name, cl4_wrapper_info_function info_fun, 
	GError** err);
	
/** @brief Get pointer to information value. */
gpointer cl4_wrapper_get_info_value(CL4Wrapper* wrapper, 
	cl_uint param_name, cl4_wrapper_info_function info_fun, 
	GError** err);

/** @brief Get information size. */
gsize cl4_wrapper_get_info_size(CL4Wrapper* wrapper, 
	cl_uint param_name, cl4_wrapper_info_function info_fun, 
	GError** err);



#endif

