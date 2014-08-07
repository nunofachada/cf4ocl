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
 * @brief OpenCL event wrapper.
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

/**
 * @defgroup EVENT_WRAPPER Event wrapper
 *
 * @brief A wrapper object for OpenCL events and functions to manage 
 * them.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/** 
 * @brief Event wrapper class. 
 * 
 * @extends ccl_wrapper
 * */
typedef struct ccl_event CCLEvent;

/** @brief Get the event wrapper for the given OpenCL event. */
CCLEvent* ccl_event_new_wrap(cl_event event);

/** @brief Decrements the reference count of the event wrapper object. 
 * If it reaches 0, the event wrapper object is destroyed. */
void ccl_event_destroy(CCLEvent* evt);

void ccl_event_set_name(CCLEvent* evt, const char* name);

const char* ccl_event_get_name(CCLEvent* evt);

const char* ccl_event_get_final_name(CCLEvent* evt);

/** @brief Get the command type which fired the given event. */
cl_command_type ccl_event_get_command_type(
	CCLEvent* evt, GError** err);

/**
 * @brief Get a ::CCLWrapperInfo event information object.
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested event information object. This object will
 * be automatically freed when the event wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_event_get_info(evt, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetEventInfo, CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar event information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
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
 * @brief Macro which returns an array event information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
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
 * @brief Get a ::CCLWrapperInfo event profiling information object.
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The requested event profiling information object. This object
 * will be automatically freed when the event wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_event_get_profiling_info(evt, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) evt, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetEventProfilingInfo, CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar event profiling information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or NULL if error
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
 * @brief Macro which returns an array event profiling information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or NULL if error
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
 * @brief Increase the reference count of the event object.
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object. 
 * */
#define ccl_event_ref(evt) \
	ccl_wrapper_ref((CCLWrapper*) evt)

/**
 * @brief Alias to ccl_event_destroy().
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt Event wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_event_unref(evt) ccl_event_destroy(evt)

/**
 * @brief Get the OpenCL event object.
 * 
 * @public @memberof ccl_event
 * 
 * @param[in] evt The event wrapper object.
 * @return The OpenCL event object.
 * */
#define ccl_event_unwrap(evt) \
	((cl_event) ccl_wrapper_unwrap((CCLWrapper*) evt))

/**
 * @defgroup EVENT_WAIT_LIST Event wait lists
 *
 * @brief Simple management of event wait lists.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

typedef GPtrArray* CCLEventWaitList;

void ccl_event_wait_list_add(
	CCLEventWaitList* evt_wait_lst, CCLEvent* evt);
	
void ccl_event_wait_list_clear(CCLEventWaitList* evt_wait_lst);

#define ccl_event_wait_list_get_num_events(evt_wait_lst) \
	((((evt_wait_lst) != NULL) && (*(evt_wait_lst) != NULL)) \
	? (*(evt_wait_lst))->len \
	: 0)
	
#define ccl_event_wait_list_get_clevents(evt_wait_lst) \
	((((evt_wait_lst) != NULL) && (*(evt_wait_lst) != NULL)) \
		? (const cl_event*) (*(evt_wait_lst))->pdata \
		: NULL)


/** @} */

/** @brief Waits on the host thread for commands identified by events 
 * in the wait list to complete. This function is a wrapper for the
 * clWaitForEvents() OpenCL function. */ 
cl_int ccl_event_wait(CCLEventWaitList* evt_wait_lst, GError** err);

/** @} */

#endif


