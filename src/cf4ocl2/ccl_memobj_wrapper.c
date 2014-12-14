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

#include "ccl_memobj_wrapper.h"
#include "_ccl_memobj_wrapper.h"

 /**
 * @file
 *
 * Implementation of a wrapper class and its methods for OpenCL memory
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

/**
 * @internal
 * Implementation of ccl_wrapper_release_fields() function for
 * ::CCLMemObj wrapper objects.
 *
 * @protected @memberof ccl_memobj
 *
 * @param[in] mo A ::CCLMemObj wrapper object.
 * */
void ccl_memobj_release_fields(CCLMemObj* mo) {

	/* Make sure mo wrapper object is not NULL. */
	g_return_if_fail(mo != NULL);

	/* Reduce reference count of memory object context wrapper. */
	if (mo->ctx != NULL)
		ccl_context_unref(mo->ctx);

}

/**
 * @addtogroup CCL_MEMOBJ_WRAPPER
 * @{
 */

/**
 * Get the OpenCL version of the platform associated with this memory
 * object.
 *
 * @public @memberof ccl_memobj
 *
 * @param[in] mo A memory object wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL version of the platform associated with this
 * memory object as an integer. If an error occurs, 0 is returned.
 * */
CCL_EXPORT
cl_uint ccl_memobj_get_opencl_version(CCLMemObj* mo, GError** err) {

	/* Make sure number mo is not NULL. */
	g_return_val_if_fail(mo != NULL, 0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

	cl_context context;
	CCLContext* ctx;
	GError* err_internal = NULL;
	cl_uint ocl_ver;

	/* Get cl_context object for this memory object. */
	context = ccl_memobj_get_info_scalar(
		mo, CL_MEM_CONTEXT, cl_context, &err_internal);
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

	/* Return event wrapper. */
	return ocl_ver;

}

/**
 * Enqueues a command to unmap a previously mapped region of a memory
 * object. This function wraps the clEnqueueUnmapMemObject() OpenCL
 * function.
 *
 * @public @memberof ccl_memobj
 *
 * @param[in] mo A memory object wrapper object.
 * @param[in] cq A command queue wrapper object.
 * @param[in] mapped_ptr The host address returned by a previous call
 * to ::ccl_buffer_enqueue_map() or ::ccl_image_enqueue_map() for `mo`.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this command.
 * */
CCL_EXPORT
CCLEvent* ccl_memobj_enqueue_unmap(CCLMemObj* mo, CCLQueue* cq,
	void* mapped_ptr, CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure mo is not NULL. */
	g_return_val_if_fail(mo != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event. */
	cl_event event;
	/* Event wrapper. */
	CCLEvent* evt;

	/* Enqueue unmap command. */
	ocl_status = clEnqueueUnmapMemObject (ccl_queue_unwrap(cq),
		ccl_memobj_unwrap(mo), mapped_ptr,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to unmap memory object (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Wrap event and associate it with the respective command queue.
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);

	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;

finish:

	/* Return evt. */
	return evt;


}

#ifdef CL_VERSION_1_1

/**
 * Wrapper for OpenCL clSetMemObjectDestructorCallback() function.
 *
 * @public @memberof ccl_memobj
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] mo A memory object wrapper object.
 * @param[in] pfn_notify The callback function that can be registered
 * by the application.
 * @param[in] user_data A pointer to user supplied data.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device if operation completes successfully,
 * `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_memobj_set_destructor_callback(CCLMemObj* mo,
	ccl_memobj_destructor_callback pfn_notify,
	void *user_data, GError** err) {

	/* Make sure mo is not NULL. */
	g_return_val_if_fail(mo != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* This function return status. */
	cl_bool ret_status;
	/* OpenCL version. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.1 */
	ocl_ver = ccl_memobj_get_opencl_version(mo, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.1, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: memory object destructor callbacks require OpenCL " \
		"version 1.1 or newer.",
		CCL_STRD);

	/* Set destructor callback. */
	ocl_status = clSetMemObjectDestructorCallback(ccl_memobj_unwrap(mo),
		pfn_notify, user_data);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to set memory object destructor callback (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

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


#ifdef CL_VERSION_1_2

/**
 * Enqueues a command to indicate which device a set of memory objects
 * should be associated with. Wrapper for OpenCL
 * clEnqueueMigrateMemObjects() function.
 *
 * @public @memberof ccl_memobj
 * @note Requires OpenCL >= 1.2
 *
 * @param[in] mos A pointer to a list of memory object wrappers.
 * @param[in] num_mos The number of memory objects specified in `mos`.
 * @param[in] cq A command queue wrapper object.
 * @param[in] flags Migration options
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return
 * */
CCL_EXPORT
CCLEvent* ccl_memobj_enqueue_migrate(CCLMemObj** mos, cl_uint num_mos,
 	CCLQueue* cq, cl_mem_migration_flags flags,
 	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure mos is not NULL. */
	g_return_val_if_fail(mos != NULL, NULL);
	/* Make sure num_mos > 0 is not NULL. */
	g_return_val_if_fail(num_mos > 0, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event. */
	cl_event event;
	/* Event wrapper. */
	CCLEvent* evt;
	/* OpenCL version. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;
	/* Array of OpenCL memory objects. */
	cl_mem* mem_objects = NULL;

	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_memobj_get_opencl_version(mos[0], &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: memory object migration requires OpenCL version 1.2 or " \
		"newer.",
		CCL_STRD);

	/* Allocate mmemory for memory objects. */
	mem_objects = (cl_mem*)g_slice_alloc(sizeof(cl_mem) * num_mos);

	/* Gather OpenCL memory objects in a array. */
	for (cl_uint i = 0; i < num_mos; ++i) {
		mem_objects[i] = ccl_memobj_unwrap(mos[i]);
	}

	/* Migrate memory objects. */
	ocl_status = clEnqueueMigrateMemObjects(ccl_queue_unwrap(cq),
		num_mos, (const cl_mem*) mem_objects, flags,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to migrate memory objects (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Wrap event and associate it with the respective command queue.
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);

	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;

finish:

	/* Release stuff. */
	if (mem_objects) g_slice_free1(sizeof(cl_mem) * num_mos, mem_objects);

	/* Return evt. */
	return evt;

}

#endif


/** @} */
