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
 * Implementation of a wrapper class and its methods for OpenCL event
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "event_wrapper.h"
#include "queue_wrapper.h"

/**
 * Event wrapper class.
 *
 * @extends ccl_wrapper
 * */
struct ccl_event {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

	/**
	 * Event name, for profiling purposes only.
	 * @private
	 * */
	const char* name;

};

/**
 * @addtogroup EVENT_WRAPPER
 * @{
 */

/**
 * Get the event wrapper for the given OpenCL event.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to wrap the OpenCL event directly.
 *
 * @public @memberof ccl_event
 *
 * @param[in] event The OpenCL event to be wrapped.
 * @return The event wrapper for the given OpenCL event.
 * */
CCL_EXPORT
CCLEvent* ccl_event_new_wrap(cl_event event) {

	CCLEvent* evt = (CCLEvent*) ccl_wrapper_new(
		(void*) event, sizeof(CCLEvent));

	return evt;

}

/**
 * Decrements the reference count of the event wrapper object.
 * If it reaches 0, the event wrapper object is destroyed.
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt The event wrapper object.
 * */
CCL_EXPORT
void ccl_event_destroy(CCLEvent* evt) {

	ccl_wrapper_unref((CCLWrapper*) evt, sizeof(CCLEvent),
		NULL, (ccl_wrapper_release_cl_object) clReleaseEvent, NULL);

}

/**
 * Set event name for profiling purposes.
 *
 * This is used to distinguish from different event is profiling is to
 * be performed using the @ref PROFILER "profiler module".
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt The event wrapper object.
 * @param[in] name Name to associate with event.
 * */
CCL_EXPORT
void ccl_event_set_name(CCLEvent* evt, const char* name) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Set event name. */
	evt->name = name;

}

/**
 * Get the event name for profiling purposes. If not explicitly set
 * with ccl_event_set_name(), it will return NULL.
 *
 * This is used to distinguish from different event is profiling is to
 * be performed using the @ref PROFILER "profiler module".
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt The event wrapper object.
 * @return Name associated with event.
 * */
CCL_EXPORT
const char* ccl_event_get_name(CCLEvent* evt) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_val_if_fail(evt != NULL, NULL);

	/* Return event name. */
	return evt->name;

}

/**
 * Get the final event name for profiling purposes. If a name was not
 * explicitly set with ccl_event_set_name(), it will return a name
 * based on the type of command associated with the event.
 *
 * This is used to distinguish from different event is profiling is to
 * be performed using the @ref PROFILER "profiler module".
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt The event wrapper object.
 * @return Final name associated with event.
 * */
CCL_EXPORT
const char* ccl_event_get_final_name(CCLEvent* evt) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_val_if_fail(evt != NULL, NULL);

	/* Final name to return. */
	const char* final_name;

	/* Check if name is not NULL. */
	if (evt->name != NULL) {

		/* Name is not NULL, use it as final name. */
		final_name = evt->name;

	} else {

		/* Name is NULL, determine a final name based on type of
		 * command which produced the event. */

		GError* err_internal = NULL;

		cl_command_type ct =
			ccl_event_get_command_type(evt, &err_internal);

		if (err_internal != NULL) {
			g_warning("Unable to determine final event name due to" \
				"the following error: %s", err_internal->message);
			g_error_free(err_internal);
			return NULL;
		}

		switch (ct) {
			case CL_COMMAND_NDRANGE_KERNEL:
				final_name = "NDRANGE_KERNEL";
				break;
			case CL_COMMAND_NATIVE_KERNEL:
				final_name = "NATIVE_KERNEL";
				break;
			case CL_COMMAND_READ_BUFFER:
				final_name = "READ_BUFFER";
				break;
			case CL_COMMAND_WRITE_BUFFER:
				final_name = "WRITE_BUFFER";
				break;
			case CL_COMMAND_COPY_BUFFER:
				final_name = "COPY_BUFFER";
				break;
			case CL_COMMAND_READ_IMAGE:
				final_name = "READ_IMAGE";
				break;
			case CL_COMMAND_WRITE_IMAGE:
				final_name = "WRITE_IMAGE";
				break;
			case CL_COMMAND_COPY_IMAGE:
				final_name = "COPY_IMAGE";
				break;
			case CL_COMMAND_COPY_BUFFER_TO_IMAGE:
				final_name = "COPY_BUFFER_TO_IMAGE";
				break;
			case CL_COMMAND_COPY_IMAGE_TO_BUFFER:
				final_name = "COPY_IMAGE_TO_BUFFER";
				break;
			case CL_COMMAND_MAP_BUFFER:
				final_name = "MAP_BUFFER";
				break;
			case CL_COMMAND_MAP_IMAGE:
				final_name = "MAP_IMAGE";
				break;
			case CL_COMMAND_UNMAP_MEM_OBJECT:
				final_name = "UNMAP_MEM_OBJECT";
				break;
			case CL_COMMAND_MARKER:
				final_name = "MARKER";
				break;
			case CL_COMMAND_ACQUIRE_GL_OBJECTS:
				final_name = "ACQUIRE_GL_OBJECTS";
				break;
			case CL_COMMAND_RELEASE_GL_OBJECTS:
				final_name = "RELEASE_GL_OBJECTS";
				break;
			case CL_COMMAND_READ_BUFFER_RECT:
				final_name = "READ_BUFFER_RECT";
				break;
			case CL_COMMAND_WRITE_BUFFER_RECT:
				final_name = "WRITE_BUFFER_RECT";
				break;
			case CL_COMMAND_COPY_BUFFER_RECT:
				final_name = "COPY_BUFFER_RECT";
				break;
			case CL_COMMAND_USER:
				/* This is here just for completeness, as a user
				 * event can't be profiled. */
				final_name = "USER";
				break;
			case CL_COMMAND_BARRIER:
				final_name = "BARRIER";
				break;
			case CL_COMMAND_MIGRATE_MEM_OBJECTS:
				final_name = "MIGRATE_MEM_OBJECTS";
				break;
			case CL_COMMAND_FILL_BUFFER:
				final_name = "FILL_BUFFER";
				break;
			case CL_COMMAND_FILL_IMAGE:
				final_name = "FILL_IMAGE";
				break;
			case CL_COMMAND_SVM_FREE:
				final_name = "SVM_FREE";
				break;
			case CL_COMMAND_SVM_MEMCPY:
				final_name = "SVM_MEMCPY";
				break;
			case CL_COMMAND_SVM_MEMFILL:
				final_name = "SVM_MEMFILL";
				break;
			case CL_COMMAND_SVM_MAP:
				final_name = "SVM_MAP";
				break;
			case CL_COMMAND_SVM_UNMAP:
				final_name = "SVM_UNMAP";
				break;
			case CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR:
				final_name = "GL_FENCE_SYNC_OBJECT_KHR";
				break;
			case CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR:
				final_name = "ACQUIRE_D3D10_OBJECTS_KHR";
				break;
			case CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR:
				final_name = "RELEASE_D3D10_OBJECTS_KHR";
				break;
			case CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR:
				final_name = "ACQUIRE_DX9_MEDIA_SURFACES_KHR";
				break;
			case CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR:
				final_name = "RELEASE_DX9_MEDIA_SURFACES_KHR";
				break;
			case CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR:
				final_name = "ACQUIRE_D3D11_OBJECTS_KHR";
				break;
			case CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR:
				final_name = "RELEASE_D3D11_OBJECTS_KHR";
				break;
			case CL_COMMAND_EGL_FENCE_SYNC_OBJECT_KHR:
				final_name = "EGL_FENCE_SYNC_OBJECT_KHR";
				break;
			default:
				final_name = NULL;
				g_warning("Unknown event command type");
				break;
		}

	}

	/* Return final name. */
	return final_name;
}

/**
 * Get the command type which fired the given event.
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt Event wrapper.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The command type which fired the given event or 0 if an error
 * occurs.
 * */
CCL_EXPORT
cl_command_type ccl_event_get_command_type(
	CCLEvent* evt, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

	/* Make sure evt wrapper object is not NULL. */
	g_return_val_if_fail(evt != NULL, 0);

	/* The command type to return. */
	cl_command_type ct;

	/* Determine the command type. */
	CCLWrapperInfo* info =
		ccl_event_get_info(evt, CL_EVENT_COMMAND_TYPE, err);

	if (info == NULL) {
		/* Some error ocurred, return 0. */
		ct = 0;
	} else {
		/* Get the command type. */
		ct = *((cl_command_type*) info->value);
	}

	/* Return the command type. */
	return ct;
}

/**
 * Get the OpenCL version of the platform associated with this event
 * object.
 *
 * @public @memberof ccl_event
 *
 * @param[in] evt An event wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL version of the platform associated with this
 * event object as an integer. If an error occurs, 0 is returned.
 * */
CCL_EXPORT
cl_uint ccl_event_get_opencl_version(CCLEvent* evt, GError** err) {

	/* Make sure number evt is not NULL. */
	g_return_val_if_fail(evt != NULL, 0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

	/* OpenCL version. */
	cl_uint ocl_ver;

#ifdef CL_VERSION_1_1
	/* Get version from event if OpenCL >= 1.1 because CL_EVENT_CONTEXT
	 * isn't defined for version 1.0. */

	cl_context context;
	CCLContext* ctx;
	GError* err_internal = NULL;

	context = ccl_event_get_info_scalar(
		evt, CL_EVENT_CONTEXT, cl_context, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get context wrapper. */
	ctx = ccl_context_new_wrap(context);

	/* Get OpenCL version. */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Unref. the context wrapper. */
	ccl_context_unref(ctx);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	ocl_ver = 0;

finish:

#else

	/* It's version 1.0. */
	ocl_ver = 100;

#endif

	/* Return event wrapper. */
	return ocl_ver;

}

#ifdef CL_VERSION_1_1

/**
 * Wrapper for OpenCL clSetEventCallback() function.
 *
 * @public @memberof ccl_event
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] evt Event wrapper object.
 * @param[in] command_exec_callback_type The command execution status
 * for which the callback is registered (`CL_SUBMITTED`, `CL_RUNNING`,
 * or `CL_COMPLETE`).
 * @param[in] pfn_notify The event callback function that can be
 * registered by the application.
 * @param[in] user_data Will be passed as the user_data argument when
 * pfn_notify is called.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_event_set_callback(CCLEvent* evt,
	cl_int command_exec_callback_type, ccl_event_callback pfn_notify,
	void *user_data, GError** err) {

	/* Make sure evt is not NULL. */
	g_return_val_if_fail(evt != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* OpenCL function return status. */
	cl_int ocl_status;
	/* This function return status. */
	cl_bool ret_status;
	/* OpenCL version of the underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.1 */
	ocl_ver = ccl_event_get_opencl_version(evt, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.1, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: set event callback requires OpenCL version 1.1 or newer.",
		G_STRLOC);

	/* Set event callback.*/
	ocl_status = clSetEventCallback(ccl_event_unwrap(evt),
		command_exec_callback_type, pfn_notify, user_data);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to set event callback (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	ret_status = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	ret_status = CL_FALSE;

finish:

	/* Return status. */
	return ret_status;

}

/**
 * Create a new user event. Wraps the clCreateUserEvent() OpenCL
 * function.
 *
 * Returned event wrapper should be freed using ccl_event_destroy().
 *
 * @public @memberof ccl_event
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] ctx Context where to associate the user event.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new user event, which should be freed using
 * ccl_event_destroy().
 * */
CCL_EXPORT
CCLEvent* ccl_user_event_new(CCLContext* ctx, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);

	/* OpenCL status. */
	cl_int ocl_status;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	/* OpenCL event object. */
	cl_event event;
	/* OpenCL version of the underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.1 */
	ocl_ver = ccl_event_get_opencl_version(evt, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.1, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: User events require OpenCL version 1.1 or newer.",
		G_STRLOC);

	/* Create user event. */
	event = clCreateUserEvent(ccl_context_unwrap(ctx), &ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: error creating user event (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap event. */
	evt = ccl_event_new_wrap(event);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return event wrapper. */
	return evt;

}

/**
 * Sets the execution status of a user event object. Wraps the
 * clSetUserEventStatus() OpenCL function.
 *
 * @public @memberof ccl_event
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] evt Event wrapper object.
 * @param[in] execution_status The new execution status to be set, can
 * be `CL_COMPLETE` or a negative integer value to indicate an error.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_user_event_set_status(
	CCLEvent* evt, cl_int execution_status, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Make sure evt is not NULL. */
	g_return_val_if_fail(evt != NULL, CL_FALSE);

	/* OpenCL status. */
	cl_int ocl_status;
	/* Function return status. */
	cl_bool ret_status;
	/* OpenCL version of the underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.1 */
	ocl_ver = ccl_event_get_opencl_version(evt, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.1, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: User events require OpenCL version 1.1 or newer.",
		G_STRLOC);

	/* Set status. */
	ocl_status = clSetUserEventStatus(
		ccl_event_unwrap(evt), execution_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: error setting user event status (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	ret_status = CL_TRUE;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	ret_status = CL_FALSE;

finish:

	/* Return status. */
	return ret_status;

}

#endif

/**
 * Add event wrapper objects to an event wait list (variable argument
 * list version).
 *
 * @param[out] evt_wait_lst Event wait list.
 * @param[in] ... A `NULL`-terminated list of event wrapper objects.
 * */
CCL_EXPORT
void ccl_event_wait_list_add(
	CCLEventWaitList* evt_wait_lst, ...) {

	/* Check that evt_wait_lst is not NULL. */
	g_return_if_fail(evt_wait_lst != NULL);

	/* Variable argument list. */
	va_list al;

	/* Current event wrapper object. */
	CCLEvent* evt;

	/* Array of event wrapper objects. */
	GPtrArray* evts = g_ptr_array_new();

	/* Initialize variable argument list. */
	va_start(al, evt_wait_lst);

	/* Get arguments (i.e. event wrapper objects). */
	while ((evt = va_arg(al, CCLEvent*)) != NULL) {

		/* Add event wrapper to array. */
		g_ptr_array_add(evts, evt);

	}

	/* Add NULL to the end of array of event wrapper objects. */
	g_ptr_array_add(evts, NULL);

	/* Add to wait list using the array version of this function. */
	ccl_event_wait_list_add_v(evt_wait_lst, (CCLEvent**) evts->pdata);

	/* Destroy the array of event wrapper objects. */
	g_ptr_array_free(evts, TRUE);
}

/**
 * Add event wrapper objects to an event wait list (array version).
 *
 * @param[out] evt_wait_lst Event wait list.
 * @param[in] evts `NULL`-terminated array of event wrapper objects.
 * */
CCL_EXPORT
void ccl_event_wait_list_add_v(
	CCLEventWaitList* evt_wait_lst, CCLEvent** evts) {

	/* Check that evt_wait_lst is not NULL. */
	g_return_if_fail(evt_wait_lst != NULL);

	/* Check that events array is not NULL. */
	g_return_if_fail(evts != NULL);

	/* Check that events array contains events. */
	g_return_if_fail(evts[0] != NULL);

	/* Initialize list if required. */
	if (*evt_wait_lst == NULL)
		*evt_wait_lst = g_ptr_array_new();

	/* Cycle through array of event wrapper objects. */
	for (guint i = 0; evts[i] != NULL; ++i) {

		/* Add wrapped cl_event to array. */
		g_ptr_array_add(*evt_wait_lst, ccl_event_unwrap(evts[i]));

	}

}

/**
 * Clears an event wait list.
 *
 * This function will rarely be called from client code because event
 * wait lists are automatically cleared when passed to
 * `ccl_*_enqueue_*()` functions.
 *
 * @param[out] evt_wait_lst Event wait list.
 * */
CCL_EXPORT
void ccl_event_wait_list_clear(CCLEventWaitList* evt_wait_lst) {

	if ((evt_wait_lst != NULL) && (*evt_wait_lst != NULL)) {
		g_ptr_array_free(*evt_wait_lst, TRUE);
		*evt_wait_lst = NULL;
	}
}

/**
 * Waits on the host thread for commands identified by events
 * in the wait list to complete. This function is a wrapper for the
 * clWaitForEvents() OpenCL function.
 *
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_event_wait(CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* OpenCL status. */
	cl_int ocl_status;
	/* Function return status. */
	cl_bool ret_status;

	/* OpenCL wait for events. */
	ocl_status = clWaitForEvents(
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst));
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: error while waiting for events (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	ret_status = CL_TRUE;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	ret_status = CL_FALSE;

finish:

	/* Return status. */
	return ret_status;

}

/** @} */
