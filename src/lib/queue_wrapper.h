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
 
#ifndef CL4_CQUEUE_WRAPPER_H
#define CL4_CQUEUE_WRAPPER_H 

#include <glib.h>
#include "oclversions.h"
#include "abstract_wrapper.h"
#include "device_wrapper.h"
#include "context_wrapper.h"
#include "event_wrapper.h"

/**
 * @defgroup QUEUE_WRAPPER Command queue wrapper
 *
 * @brief A wrapper object for OpenCL command queues and functions to 
 * manage them.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/** @brief Command queue wrapper object. */
typedef struct cl4_cqueue CL4CQueue;

CL4CQueue* cl4_cqueue_new_direct(cl_context context, 
	cl_device_id device, cl_command_queue_properties properties, 
	GError** err);

CL4CQueue* cl4_cqueue_new(CL4Context* ctx, CL4Device* dev, 
	cl_command_queue_properties properties, GError** err);

/** @brief Decrements the reference count of the command queue wrapper 
 * object. If it reaches 0, the command queue wrapper object is 
 * destroyed. */
void cl4_cqueue_destroy(CL4CQueue* cq);

CL4Context* cl4_cqueue_get_context(CL4CQueue* cq, GError** err);

CL4Device* cl4_cqueue_get_device(CL4CQueue* cq, GError** err);

CL4Event* cl4_cqueue_produce_event(CL4CQueue* cq, cl_event event);

void cl4_cqueue_iter_event_init(CL4CQueue* cq);

CL4Event* cl4_cqueue_iter_event_next(CL4CQueue* cq);

cl_int cl4_cqueue_flush(CL4CQueue* cq, GError** err);

cl_int cl4_cqueue_finish(CL4CQueue* cq, GError** err);

/**
 * @brief Get a ::CL4WrapperInfo command queue information object.
 * 
 * @param cq The command queue wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested command queue information object. This object will
 * be automatically freed when the command queue wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_cqueue_get_info(cq, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) cq, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetCommandQueueInfo, CL_TRUE, err)

/** 
 * @brief Macro which returns a scalar command queue information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param cq The command queue wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested command queue information value. This value will be 
 * automatically freed when the command queue wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define cl4_cqueue_get_scalar_info(cq, param_name, param_type, err) \
	*((param_type*) cl4_wrapper_get_info_value((CL4Wrapper*) cq, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetCommandQueueInfo, \
		CL_TRUE, err))

/** 
 * @brief Macro which returns an array command queue information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param cq The command queue wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested command queue information value. This value will be 
 * automatically freed when the command queue wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define cl4_cqueue_get_array_info(cq, param_name, param_type, err) \
	(param_type) cl4_wrapper_get_info_value((CL4Wrapper*) cq, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetCommandQueueInfo, \
		CL_TRUE, err)

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

/** @} */

/** @brief Get the command queue wrapper for the given OpenCL command 
 * queue. */
CL4CQueue* cl4_cqueue_new_wrap(cl_command_queue command_queue);

#endif
