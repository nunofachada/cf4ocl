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
 * @brief OpenCL command queue wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_CQUEUE_H
#define CL4_CQUEUE_H 

#include <glib.h>
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include "wrapper.h"

/** @brief Command queue wrapper object. */
typedef struct cl4_cqueue CL4CQueue;

/** @brief Decrements the reference count of the command queue wrapper 
 * object. If it reaches 0, the command queue wrapper object is 
 * destroyed. */
void cl4_cqueue_destroy(CL4CQueue* cq);

/**
 * @brief Get command queue information object.
 * 
 * @param queue The command queue wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested information object. This object will be 
 * automatically freed when the wrapper object is destroyed. If an error 
 * occurs, NULL is returned.
 * */
#define cl4_cqueue_info(queue, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) queue, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetCommandQueueInfo, err)

/** 
 * @brief Increase the reference count of the command queue object.
 * 
 * @param cq The command queue wrapper object. 
 * */
#define cl4_cqueue_ref(cq) \
	cl4_wrapper_ref((CL4Wrapper*) cq)

/**
 * @brief Alias to cl4_queue_destroy().
 * 
 * @param cq Command queue wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define cl4_cqueue_unref(cq) cl4_cqueue_destroy(cq)

/**
 * @brief Get the OpenCL command queue object.
 * 
 * @param cq The command queue wrapper object.
 * @return The OpenCL command queue object.
 * */
#define cl4_cqueue_unwrap(cq) \
	((cl_command_queue) cl4_wrapper_unwrap((CL4Wrapper*) cq))		

#endif
