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
 * Definition of a wrapper class and its methods for OpenCL queue
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_QUEUE_WRAPPER_H_
#define _CCL_QUEUE_WRAPPER_H_

#include <glib.h>
#include "ccl_oclversions.h"
#include "ccl_abstract_wrapper.h"
#include "ccl_device_wrapper.h"
#include "ccl_context_wrapper.h"
#include "ccl_event_wrapper.h"

/**
 * @defgroup CCL_QUEUE_WRAPPER Command queue wrapper
 *
 * The command queue wrapper module provides functionality for simple
 * handling of OpenCL command queue objects.
 *
 * Queue wrappers can be instantiated with the ::ccl_queue_new() and
 * ::ccl_queue_new_full() constructors. While both constructors can be
 * used with any OpenCL version, the later is targeted for OpenCL 2.0,
 * exposing OpenCL 2.0 features, such as on-device queues, to client
 * code. If "OpenCL 2.0 only" features are requested for platforms with
 * OpenCL version <= 2.0, a warning will be logged and the queue will be
 * created without the unsupported features.
 *
 * Instantiation and destruction of queue wrappers follows the _cf4ocl_
 * @ref ug_new_destroy "new/destroy" rule; as such, queues should be
 * freed with the ::ccl_queue_destroy() destructor.
 *
 * Queue wrappers created with the `CL_QUEUE_PROFILING_ENABLE`
 * property can be automatically profiled with the
 * @ref CCL_PROFILER "profiler module".
 *
 * Information about queue objects can be fetched using the
 * @ref ug_getinfo "info macros":
 *
 * * ::ccl_queue_get_info_scalar()
 * * ::ccl_queue_get_info_array()
 * * ::ccl_queue_get_info()
 *
 * _Example:_
 *
 * @dontinclude image_filter.c
 * @skipline Wrappers for
 * @skipline CCLQueue*
 * @skipline Error handling
 * @skipline GError*
 *
 * @skipline Create a command queue
 * @skipline queue =
 *
 * @skipline Release wrappers
 * @skipline ccl_queue_destroy
 *
 * @{
 */

/* Get the command queue wrapper for the given OpenCL command
 * queue. */
CCL_EXPORT
CCLQueue* ccl_queue_new_wrap(cl_command_queue command_queue);

/* Create a new command queue wrapper object. */
CCL_EXPORT
CCLQueue* ccl_queue_new_full(CCLContext* ctx, CCLDevice* dev,
	const cl_queue_properties* prop_full, GError** err);

/* Create a new command queue wrapper object. */
CCL_EXPORT
CCLQueue* ccl_queue_new(CCLContext* ctx, CCLDevice* dev,
	cl_command_queue_properties properties, GError** err);

/* Decrements the reference count of the command queue wrapper
 * object. If it reaches 0, the command queue wrapper object is
 * destroyed. */
CCL_EXPORT
void ccl_queue_destroy(CCLQueue* cq);

/* Get the context associated with the given command queue wrapper
 * object. */
CCL_EXPORT
CCLContext* ccl_queue_get_context(CCLQueue* cq, GError** err);

/* Get the device associated with the given command queue wrapper
 * object. */
CCL_EXPORT
CCLDevice* ccl_queue_get_device(CCLQueue* cq, GError** err);

/* Create an event wrapper from a given OpenCL event object and
 * associate it with the command queue. */
CCL_EXPORT
CCLEvent* ccl_queue_produce_event(CCLQueue* cq, cl_event event);

/* Initialize an iterator for this command queue's list of event
 * wrappers. */
CCL_EXPORT
void ccl_queue_iter_event_init(CCLQueue* cq);

/* Get the next event wrapper associated with this queue. */
CCL_EXPORT
CCLEvent* ccl_queue_iter_event_next(CCLQueue* cq);

/* Issues all previously queued commands in a command queue to the
 * associated device. */
CCL_EXPORT
cl_bool ccl_queue_flush(CCLQueue* cq, GError** err);

/* Blocks until all previously queued OpenCL commands in a command-queue
 * are issued to the associated device and have completed. */
CCL_EXPORT
cl_bool ccl_queue_finish(CCLQueue* cq, GError** err);

/* Release all events associated with the command queue. */
CCL_EXPORT
void ccl_queue_gc(CCLQueue* cq);

/* Enqueues a barrier command on the given command queue. */
CCL_EXPORT
CCLEvent* ccl_enqueue_barrier(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Enqueues a marker command on the given command queue. */
CCL_EXPORT
CCLEvent* ccl_enqueue_marker(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err);

/**
 * Get a ::CCLWrapperInfo command queue information object.
 *
 * @param[in] cq The command queue wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested command queue information object. This object
 * will be automatically freed when the command queue wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_queue_get_info(cq, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) cq, NULL, param_name, 0, \
		(ccl_wrapper_info_fp) clGetCommandQueueInfo, CL_FALSE, err)

/**
 * Macro which returns a scalar command queue information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] cq The command queue wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested command queue information value. This value
 * will be automatically freed when the command queue wrapper object is
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_queue_get_info_scalar(cq, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) cq, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetCommandQueueInfo, CL_FALSE, err))

/**
 * Macro which returns an array command queue information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] cq The command queue wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested command queue information value. This value
 * will be automatically freed when the command queue wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_queue_get_info_array(cq, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) cq, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetCommandQueueInfo, CL_FALSE, err)

/**
 * Increase the reference count of the command queue object.
 *
 * @param[in] cq The command queue wrapper object.
 * */
#define ccl_queue_ref(cq) \
	ccl_wrapper_ref((CCLWrapper*) cq)

/**
 * Alias to ccl_queue_destroy().
 *
 * @param[in] cq Command queue wrapper object to destroy if reference
 * count is 1, otherwise just decrement the reference count.
 * */
#define ccl_queue_unref(cq) ccl_queue_destroy(cq)

/**
 * Get the OpenCL command queue object.
 *
 * @param[in] cq The command queue wrapper object.
 * @return The OpenCL command queue object.
 * */
#define ccl_queue_unwrap(cq) \
	((cl_command_queue) ccl_wrapper_unwrap((CCLWrapper*) cq))

/** @} */

#endif
