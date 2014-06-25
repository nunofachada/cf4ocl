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
 * @brief Functions for obtaining information about OpenCL entities
 * such as platforms, devices, contexts, queues, kernels, etc.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_INFO_H
#define CL4_INFO_H 

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

typedef cl_int (*cl4_info_function)(
	gpointer cl_object,
 	cl_uint param_name,
 	size_t param_value_size,
 	void* param_value,
 	size_t* param_value_size_ret);
 
typedef struct cl4_wrapper CL4Wrapper;

/**
 * @brief Information about a wrapped OpenCL entity.
 * */
typedef struct cl4_info {
	/** Device information. */
	gpointer value;
	/** Size in bytes of device information. */
	gsize size;
} CL4Info;

/** @brief Create a new CL4Info* object. */
CL4Info* cl4_info_new(gpointer value, gsize size);

/** @brief Destroy a CL4Info* object. */
void cl4_info_destroy(void* info_value);

/** @brief Get information about any wrapped OpenCL object. */
CL4Info* cl4_info_get(CL4Wrapper* wrapper, cl_uint param_name, 
	cl4_info_function info_fun, GError** err);
	
/** @brief Get pointer to information value. */
gpointer cl4_info_get_value(CL4Wrapper* wrapper, 
	cl_uint param_name, cl4_info_function info_fun, GError** err);

/** @brief Get information size. */
gsize cl4_info_get_size(CL4Wrapper* wrapper, 
	cl_uint param_name, cl4_info_function info_fun, GError** err);

#endif

