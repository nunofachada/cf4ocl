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
 
#ifndef CL4_EVENT_WRAPPER_H
#define CL4_EVENT_WRAPPER_H 

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

/** @brief Event wrapper object. */
typedef struct cl4_event CL4Event;

/** @brief Decrements the reference count of the event wrapper object. 
 * If it reaches 0, the event wrapper object is destroyed. */
void cl4_event_destroy(CL4Event* evt);

void cl4_event_set_name(CL4Event* evt, const char* name);

const char* cl4_event_get_name(CL4Event* evt);

const char* cl4_event_get_final_name(CL4Event* evt);

cl_command_type cl4_event_get_command_type(
	CL4Event* evt, GError** err);

/**
 * @brief Get a ::CL4WrapperInfo event information object.
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event information object. This object will
 * be automatically freed when the event wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_event_get_info(evt, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) evt, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetEventInfo, CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar event information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event information value. This value will be 
 * automatically freed when the event wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define cl4_event_get_scalar_info(evt, param_name, param_type, err) \
	*((param_type*) cl4_wrapper_get_info_value((CL4Wrapper*) evt, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetEventInfo, \
		CL_FALSE, err))

/** 
 * @brief Macro which returns an array event information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event information value. This value will be 
 * automatically freed when the event wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define cl4_event_get_array_info(evt, param_name, param_type, err) \
	(param_type) cl4_wrapper_get_info_value((CL4Wrapper*) evt, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetEventInfo, \
		CL_FALSE, err)

/**
 * @brief Get a ::CL4WrapperInfo event profiling information object.
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event profiling information object. This object
 * will be automatically freed when the event wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_event_get_profiling_info(evt, param_name, err) \
	cl4_wrapper_get_info((CL4Wrapper*) evt, NULL, param_name, \
		(cl4_wrapper_info_fp) clGetEventProfilingInfo, CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar event profiling information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event profiling information value. This value 
 * will be automatically freed when the event wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define cl4_event_get_scalar_profiling_info(evt, param_name, param_type, err) \
	*((param_type*) cl4_wrapper_get_info_value((CL4Wrapper*) evt, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetEventProfilingInfo, \
		CL_FALSE, err))

/** 
 * @brief Macro which returns an array event profiling information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param evt The event wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested event profiling information value. This value 
 * will be automatically freed when the event wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define cl4_event_get_array_profiling_info(evt, param_name, param_type, err) \
	(param_type) cl4_wrapper_get_info_value((CL4Wrapper*) evt, \
		NULL, param_name, (cl4_wrapper_info_fp) clGetEventProfilingInfo, \
		CL_FALSE, err)

/** 
 * @brief Increase the reference count of the event object.
 * 
 * @param evt The event wrapper object. 
 * */
#define cl4_event_ref(evt) \
	cl4_wrapper_ref((CL4Wrapper*) evt)

/**
 * @brief Alias to cl4_event_destroy().
 * 
 * @param evt Event wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define cl4_event_unref(evt) cl4_event_destroy(evt)

/**
 * @brief Get the OpenCL event object.
 * 
 * @param evt The event wrapper object.
 * @return The OpenCL event object.
 * */
#define cl4_event_unwrap(evt) \
	((cl_event) cl4_wrapper_unwrap((CL4Wrapper*) evt))

/**
 * @defgroup EVENT_WAIT_LIST Event wait lists
 *
 * @brief Simple management of event wait lists.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

typedef GPtrArray* CL4EventWaitList;

cl_int cl4_event_wait(CL4EventWaitList evt_wait_lst, GError** err);

#define cl4_event_wait_list_new() \
	g_ptr_array_new()
	
#define cl4_event_wait_list_add(evt_wait_lst, evt) \
	g_ptr_array_add(evt_wait_lst, (void*) cl4_event_unwrap(evt))

#define cl4_event_wait_list_get_num_events(evt_wait_lst) \
	((evt_wait_lst != NULL) ? evt_wait_lst->len : 0)
	
#define cl4_event_wait_list_get_clevents(evt_wait_lst) \
	((evt_wait_lst != NULL) \
		? (const cl_event*) evt_wait_lst->pdata \
		: NULL)
	
#define cl4_event_wait_list_clear(evt_wait_lst) \
	if (evt_wait_lst != NULL) \
		g_ptr_array_remove_range( \
			evt_wait_lst, 0, evt_wait_lst->len)
	
#define cl4_event_wait_list_destroy(evt_wait_lst) \
	g_ptr_array_free(evt_wait_lst, TRUE)

/** @} */

/** @} */

/** @brief Get the event wrapper for the given OpenCL event. */
CL4Event* cl4_event_new_wrap(cl_event event);

#endif


