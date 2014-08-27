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
 * Definition of a wrapper class and its methods for OpenCL event
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_EVENT_WRAPPER_H_
#define _CCL_EVENT_WRAPPER_H_

#include <glib.h>
#include "oclversions.h"
#include "abstract_wrapper.h"
#include "context_wrapper.h"

/* Forward declaration of CLLQueue. */
typedef struct ccl_queue CCLQueue;

/**
 * @defgroup EVENT_WRAPPER Event wrapper
 *
 * A wrapper object for OpenCL events and functions to manage
 * them.
 *
 * @todo Detailed description of module with code examples.
 *
 * @{
 */

/**
 * Event wrapper class.
 *
 * @extends ccl_wrapper
 * */
typedef struct ccl_event CCLEvent;

/**
 * Prototype for user event callback functions.
 *
 * @public @memberof ccl_event
 *
 * @param[in] event The OpenCL event object for which the callback
 * function is invoked.
 * @param[in] event_command_exec_status Execution status of command for
 * which this callback function is invoked.
 * @param[in] user_data A pointer to user supplied data.
 * */
typedef void (CL_CALLBACK *ccl_event_callback)(cl_event event,
	cl_int event_command_exec_status, void *user_data);

/* Get the event wrapper for the given OpenCL event. */
CCLEvent* ccl_event_new_wrap(cl_event event);

/* Decrements the reference count of the event wrapper object. */
void ccl_event_destroy(CCLEvent* evt);

/* Set event name for profiling purposes. */
void ccl_event_set_name(CCLEvent* evt, const char* name);

/* Get the event name for profiling purposes. */
const char* ccl_event_get_name(CCLEvent* evt);

/* Get the final event name for profiling purposes. */
const char* ccl_event_get_final_name(CCLEvent* evt);

/* Get the command type which fired the given event. */
cl_command_type ccl_event_get_command_type(
	CCLEvent* evt, GError** err);

/* Get the OpenCL version of the platform associated with this event
 * object. */
cl_uint ccl_event_get_opencl_version(CCLEvent* evt, GError** err);

#ifdef CL_VERSION_1_1

/* Wrapper for OpenCL clSetEventCallback() function. */
cl_bool ccl_event_set_callback(CCLEvent* evt,
	cl_int command_exec_callback_type, ccl_event_callback pfn_notify,
	void *user_data, GError** err);

/* Create a new user event. */
CCLEvent* ccl_user_event_new(CCLContext* ctx, GError** err);

/* Sets the execution status of a user event object. */
cl_bool ccl_user_event_set_status(
	CCLEvent* evt, cl_int execution_status, GError** err);

#endif

/**
 * Get a ::CCLWrapperInfo event information object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event information object. This object will
 * be automatically freed when the event wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_event_get_info(evt, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetEventInfo, CL_FALSE, err)

/**
 * Macro which returns a scalar event information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event information value. This value will be
 * automatically freed when the event wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_event_get_scalar_info(evt, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetEventInfo, \
		CL_FALSE, err))

/**
 * Macro which returns an array event information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event information value. This value will be
 * automatically freed when the event wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_event_get_array_info(evt, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetEventInfo, \
		CL_FALSE, err)

/**
 * Get a ::CCLWrapperInfo event profiling information object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event profiling information object. This object
 * will be automatically freed when the event wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_event_get_profiling_info(evt, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetEventProfilingInfo, CL_FALSE, err)

/**
 * Macro which returns a scalar event profiling information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event profiling information value. This value
 * will be automatically freed when the event wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_event_get_scalar_profiling_info(evt, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetEventProfilingInfo, \
		CL_FALSE, err))

/**
 * Macro which returns an array event profiling information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested event profiling information value. This value
 * will be automatically freed when the event wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_event_get_array_profiling_info(evt, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetEventProfilingInfo, \
		CL_FALSE, err)

/**
 * Increase the reference count of the event object.
 *
 * @param[in] evt The event wrapper object.
 * */
#define ccl_event_ref(evt) \
	ccl_wrapper_ref((CCLWrapper*) evt)

/**
 * Alias to ccl_event_destroy().
 *
 * @param[in] evt Event wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_event_unref(evt) ccl_event_destroy(evt)

/**
 * Get the OpenCL event object.
 *
 * @param[in] evt The event wrapper object.
 * @return The OpenCL event object.
 * */
#define ccl_event_unwrap(evt) \
	((cl_event) ccl_wrapper_unwrap((CCLWrapper*) evt))

/**
 * @defgroup EVENT_WAIT_LIST Event wait lists
 *
 * Simple management of event wait lists.
 *
 * Client code must initialize #CCLEventWaitList variables to NULL,
 * and can reuse them between `ccl_*_enqueue_*()` function calls. No
 * allocation and deallocation of events and event wait list is
 * required.
 *
 * Example:
 *
 * @code{.c}
 * CCLEvent *evt1, *evt2, *evt3;
 * CCLEventWaitList evt_wait_lst = NULL;
 * ...
 * evt1 = ccl_buffer_enqueue_write(cq, a_dev, CL_FALSE, 0, size, a_host, NULL, NULL);
 * evt2 = ccl_buffer_enqueue_write(cq, b_dev, CL_FALSE, 0, size, b_host, NULL, NULL);
 * ...
 * ccl_event_wait_list_add(evt1, &evt_wait_lst);
 * ccl_event_wait_list_add(evt2, &evt_wait_lst);
 * evt3 = ccl_kernel_enqueue_ndrange(krnl, cq, dim, offset, gws, lws, &evt_wait_lst, NULL);
 * ccl_buffer_enqueue_read(cq, c_dev, CL_TRUE, 0, size, c_host, &evt_wait_lst, NULL);
 * ...
 * @endcode
 *
 * @{
 */

/** A list of event objects on which enqueued commands can wait. */
typedef GPtrArray* CCLEventWaitList;

/** Add an event wrapper object to an event wait list. */
void ccl_event_wait_list_add(
	CCLEventWaitList* evt_wait_lst, CCLEvent* evt);

/** Clears an event wait list. */
void ccl_event_wait_list_clear(CCLEventWaitList* evt_wait_lst);

/**
 * @internal
 * Get number of events in the event wait list.
 *
 * This macro is used by the `ccl_*_enqueue_*()` functions and will
 * rarely be called from client code.
 *
 * @param[in] evt_wait_lst Event wait list.
 * @return Number of event in the event wait list.
 * */
#define ccl_event_wait_list_get_num_events(evt_wait_lst) \
	((((evt_wait_lst) != NULL) && (*(evt_wait_lst) != NULL)) \
	? (*(evt_wait_lst))->len \
	: 0)

/**
 * @internal
 * Get an array of OpenCL cl_event objects in the event wait list.
 *
 * This macro is used by the `ccl_*_enqueue_*()` functions and will
 * rarely be called from client code.
 *
 * @param[in] evt_wait_lst Event wait list.
 * @return Array of OpenCL cl_event objects in the event wait list.
 * */
#define ccl_event_wait_list_get_clevents(evt_wait_lst) \
	((((evt_wait_lst) != NULL) && (*(evt_wait_lst) != NULL)) \
		? (const cl_event*) (*(evt_wait_lst))->pdata \
		: NULL)

/** @} */

/* Waits on the host thread for commands identified by events in the
 * wait list to complete. */
cl_bool ccl_event_wait(CCLEventWaitList* evt_wait_lst, GError** err);

/* Enqueues a barrier command on the given command queue. */
CCLEvent* ccl_enqueue_barrier(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Enqueues a marker command on the given command queue. */
CCLEvent* ccl_enqueue_marker(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err);

/** @} */

#endif


