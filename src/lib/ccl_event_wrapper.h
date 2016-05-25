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
#include "ccl_oclversions.h"
#include "ccl_abstract_wrapper.h"
#include "ccl_context_wrapper.h"

/**
 * @defgroup CCL_EVENT_WRAPPER Event wrapper
 *
 * The event wrapper module provides functionality for simple
 * handling of OpenCL event objects.
 *
 * Typical event wrappers are not directly created by client code. They
 * are returned by event producing functions (such as
 * ::ccl_image_enqueue_write(), which wraps the clEnqueueWriteImage()
 * OpenCL function). As such, and in accordance with the
 * @ref ug_new_destroy "new/destroy" rule, regular event wrappers
 * objects should not be destroyed by client code. They are
 * automatically released when the command queue wrapper where the event
 * took place is destroyed. User events (OpenCL >= 1.1), created with
 * the ::ccl_user_event_new() constructor, are the only exception. These
 * are special events which allow applications to enqueue commands that
 * wait on user-controlled occurrences before the command is executed by
 * the device. These events should be destroyed with
 * ::ccl_event_destroy().
 *
 * The @ref CCL_EVENT_WAIT_LIST "event wait list section" provides
 * additional information on how to use events to synchronize the
 * execution of OpenCL commands. Events are also used by the
 * @ref CCL_PROFILER "profiler module", although indirectly via
 * ::CCLQueue* wrappers, to profile and benchmark applications.
 *
 * Information about event objects can be fetched using the respective
 * @ref ug_getinfo "info macros":
 *
 * * ::ccl_event_get_info_scalar()
 * * ::ccl_event_get_info_array()
 * * ::ccl_event_get_info()
 *
 * Three additional macros are provided for getting event profiling
 * info. These work in the same way as the regular info macros:
 *
 * * ::ccl_event_get_profiling_info_scalar()
 * * ::ccl_event_get_profiling_info_array()
 * * ::ccl_event_get_profiling_info()
 *
 * @{
 */

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
CCL_EXPORT
CCLEvent* ccl_event_new_wrap(cl_event event);

/* Decrements the reference count of the event wrapper object. */
CCL_EXPORT
void ccl_event_destroy(CCLEvent* evt);

/* Set event name for profiling purposes. */
CCL_EXPORT
void ccl_event_set_name(CCLEvent* evt, const char* name);

/* Get the event name for profiling purposes. */
CCL_EXPORT
const char* ccl_event_get_name(CCLEvent* evt);

/* Get the final event name for profiling purposes. */
CCL_EXPORT
const char* ccl_event_get_final_name(CCLEvent* evt);

/* Get the command type which fired the given event. */
CCL_EXPORT
cl_command_type ccl_event_get_command_type(
	CCLEvent* evt, GError** err);

/* Get the OpenCL version of the platform associated with this event
 * object. */
CCL_EXPORT
cl_uint ccl_event_get_opencl_version(CCLEvent* evt, GError** err);

#ifdef CL_VERSION_1_1

/* Wrapper for OpenCL clSetEventCallback() function. */
CCL_EXPORT
cl_bool ccl_event_set_callback(CCLEvent* evt,
	cl_int command_exec_callback_type, ccl_event_callback pfn_notify,
	void *user_data, GError** err);

/* Create a new user event. */
CCL_EXPORT
CCLEvent* ccl_user_event_new(CCLContext* ctx, GError** err);

/* Sets the execution status of a user event object. */
CCL_EXPORT
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
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, 0, \
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
#define ccl_event_get_info_scalar(evt, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetEventInfo, CL_FALSE, err))

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
#define ccl_event_get_info_array(evt, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetEventInfo, \
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
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, 0, \
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
#define ccl_event_get_profiling_info_scalar(evt, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetEventProfilingInfo, CL_FALSE, err))

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
#define ccl_event_get_profiling_info_array(evt, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) evt, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetEventProfilingInfo, CL_FALSE, err)

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
 * @defgroup CCL_EVENT_WAIT_LIST Event wait lists
 *
 * This module provides simple management of event wait lists.
 *
 * Client code must initialize ::CCLEventWaitList variables to `NULL`,
 * and can reuse them between `ccl_*_enqueue_*()` function calls. No
 * allocation and deallocation of events and event wait lists is
 * required if populated event wait lists are consumed by
 * `ccl_*_enqueue_*()` functions; otherwise, unused non-empty event
 * wait lists should be freed with the ::ccl_event_wait_list_clear()
 * function.
 *
 * _Example 1:_
 *
 * @code{.c}
 * CCLEvent *evt1, *evt2, *evt3;
 * CCLEventWaitList evt_wait_lst = NULL;
 * @endcode
 * @code{.c}
 * evt1 = ccl_buffer_enqueue_write(cq, a_dev, CL_FALSE, 0, size, a_host, NULL, NULL);
 * evt2 = ccl_buffer_enqueue_write(cq, b_dev, CL_FALSE, 0, size, b_host, NULL, NULL);
 * @endcode
 * @code{.c}
 * ccl_event_wait_list_add(&evt_wait_lst, evt1, evt2, NULL);
 * evt3 = ccl_kernel_enqueue_ndrange(krnl, cq, dim, offset, gws, lws, &evt_wait_lst, NULL);
 * ccl_event_wait_list_add(&evt_wait_lst, evt3, NULL);
 * ccl_buffer_enqueue_read(cq, c_dev, CL_TRUE, 0, size, c_host, &evt_wait_lst, NULL);
 * @endcode
 *
 * _Example 2:_
 *
 * @code{.c}
 * CCLEvent *evt = NULL;
 * CCLEventWaitList evt_wait_lst = NULL;
 * @endcode
 * @code{.c}
 * evt = ccl_buffer_enqueue_write(cq1, dev, CL_FALSE, 0, size, a_host, NULL, NULL);
 * @endcode
 * @code{.c}
 * ccl_kernel_enqueue_ndrange(krnl, cq2, dim, offset, gws, lws,
 *     ccl_ewl(&evt_wait_lst, evt, NULL), NULL);
 * @endcode
 * @{
 */

/** A list of event objects on which enqueued commands can wait. */
typedef GPtrArray* CCLEventWaitList;

/**
 * Alias the for the ::ccl_event_wait_list_add() function. Intended as
 * a shortcut to use ::ccl_event_wait_list_add() directly in
 * `ccl_*_enqueue_*()` functions.
 * */
#define ccl_ewl ccl_event_wait_list_add

/* Add event wrapper objects to an event wait list (variable argument
 * list version). */
CCL_EXPORT
CCLEventWaitList* ccl_event_wait_list_add(
	CCLEventWaitList* evt_wait_lst, ...) G_GNUC_NULL_TERMINATED;

/* Add event wrapper objects to an event wait list (array version). */
CCL_EXPORT
CCLEventWaitList* ccl_event_wait_list_add_v(
	CCLEventWaitList* evt_wait_lst, CCLEvent** evt);

/* Clears an event wait list. */
CCL_EXPORT
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
CCL_EXPORT
cl_bool ccl_event_wait(CCLEventWaitList* evt_wait_lst, GError** err);

/** @} */

#endif


