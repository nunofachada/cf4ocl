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
 * Implementation of a wrapper class and its methods for OpenCL queue
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "queue_wrapper.h"

/**
 * Command queue wrapper class.
 *
 * @extends ccl_wrapper
 */
struct ccl_queue {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

	/**
	 * Context wrapper to which the queue is associated with.
	 * @private
	 * */
	CCLContext* ctx;

	/**
	 * Device wrapper to which the queue is associated with.
	 * @private
	 * */
	CCLDevice* dev;

	/**
	 * Events associated with the command queue.
	 * @private
	 * */
	GHashTable* evts;

	/**
	 * Event iterator.
	 * @private
	 * */
	GHashTableIter evt_iter;

};

/**
 * @internal
 * Implementation of ccl_wrapper_release_fields() function for
 * ::CCLQueue wrapper objects.
 *
 * @private @memberof ccl_queue
 *
 * @param[in] cq A ::CCLQueue wrapper object.
 * */
static void ccl_queue_release_fields(CCLQueue* cq) {

	/* Make sure cq wrapper object is not NULL. */
	g_return_if_fail(cq != NULL);

	/* Decrease reference count of context and device wrappers, if
	 * they're set. */
	 if (cq->ctx != NULL)
		ccl_context_unref(cq->ctx);
	 if (cq->dev != NULL)
		ccl_device_unref(cq->dev);

	/* Destroy the events table. */
	if (cq->evts != NULL) {
		g_hash_table_destroy(cq->evts);
	}
}

/**
 * @addtogroup QUEUE_WRAPPER
 * @{
 */

/**
 * Get the command queue wrapper for the given OpenCL command
 * queue.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL command queue directly (using the
 * clCreateCommandQueue() function) and then wrap the OpenCL command
 * queue in a ::CCLQueue wrapper object.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] command_queue The OpenCL command queue to be wrapped.
 * @return The ::CCLQueue wrapper for the given OpenCL command queue.
 * */
CCLQueue* ccl_queue_new_wrap(cl_command_queue command_queue) {

	return (CCLQueue*) ccl_wrapper_new(
		(void*) command_queue, sizeof(CCLQueue));

}

/**
 * Create a new on-host command queue wrapper object.
 *
 * This function accepts a zero-terminated list of `cl_queue_properties`
 * (instead of the `cl_command_queue_properties` bitfield used in the
 * ::ccl_queue_new() constructor), following the behavior of the
 * clCreateCommandQueueWithProperties() function (OpenCL >= 2.0). The
 * exact OpenCL constructor used is automatically selected based on the
 * OpenCL version of the underlying platform (i.e.
 * clCreateCommandQueue() if OpenCL <= 1.2, or
 * clCreateCommandQueueWithProperties() for OpenCL >= 2.0). However, if
 * "OpenCL 2.0 only" features are specified and the underlying platform
 * is OpenCL <= 1.2, a warning will be logged, and the queue will be
 * created with OpenCL <= 1.2 properties only.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] ctx Context wrapper object.
 * @param[in] dev Device wrapper object, must be associated with `ctx`.
 * @param[in] prop_full A zero-terminated list of `cl_queue_properties`.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLQueue wrapper for the given device and context,
 * or `NULL` if an error occurs.
 * */
CCLQueue* ccl_queue_new_full(CCLContext* ctx, CCLDevice* dev,
	const cl_queue_properties* prop_full, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* The OpenCL status flag. */
	cl_int ocl_status;
	/* The OpenCL command queue object. */
	cl_command_queue queue = NULL;
	/* The command queue wrapper object. */
	CCLQueue* cq = NULL;
	/* Internal error object. */
	GError* err_internal = NULL;
	/* Old-school properties. */
	cl_command_queue_properties properties = 0;
	/* Any new-school properties? */
	cl_bool prop_other = CL_FALSE;

	/* Extract old-school properties and flag indicating if any
	 * new-school properties are passed. */
	if (prop_full != NULL) {
		for (cl_uint i = 0; prop_full[i] != 0; ++i) {
			if (prop_full[i] == CL_QUEUE_PROPERTIES)
				properties = prop_full[i + 1];
			else
				prop_other = CL_TRUE;
			++i;
		}
		if ((properties &
			~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
			CL_QUEUE_PROFILING_ENABLE)) != 0) {
			prop_other = CL_TRUE;
		}
	}

	/* If dev is NULL, get first device in context. */
	if (dev == NULL) {
		dev = ccl_context_get_device(ctx, 0, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

#ifdef CL_VERSION_2_0
	/* OpenCL platform version of the given context. */
	double platf_ver;

	/* Get context platform version. */
	platf_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Create and keep the OpenCL command queue object. */
	if (platf_ver >= 200) {
		queue = clCreateCommandQueueWithProperties(
			ccl_context_unwrap(ctx), ccl_device_unwrap(dev),
			prop_full, &ocl_status);
	} else {
		if (prop_other) {
			g_warning("OpenCL 2.0 queue properties are not supported by "\
				"the selected OpenCL platform and will be ignored.");
			properties = properties &
				(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
				CL_QUEUE_PROFILING_ENABLE);
		}
		CCL_BEGIN_IGNORE_DEPRECATIONS
		queue = clCreateCommandQueue(ccl_context_unwrap(ctx),
			ccl_device_unwrap(dev), properties, &ocl_status);
		CCL_END_IGNORE_DEPRECATIONS
	}
#else
	/* Create and keep the OpenCL command queue object. */
	if (prop_other) {
		g_warning("OpenCL 2.0 queue properties are not supported by "\
			"the selected OpenCL platform and will be ignored.");
		properties = properties &
			(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
			CL_QUEUE_PROFILING_ENABLE);
	}
	queue = clCreateCommandQueue(ccl_context_unwrap(ctx),
		ccl_device_unwrap(dev), properties, &ocl_status);
#endif

	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap the queue. */
	cq = ccl_queue_new_wrap(queue);

	/* Keep the context and device wrappers, update their reference
	 * count. */
	cq->ctx = ctx;
	ccl_context_ref(ctx);
	cq->dev = dev;
	ccl_device_ref(dev);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return the new command queue wrapper object. */
	return cq;

}

/**
 * Create a new on-host command queue wrapper object.
 *
 * This function accepts a `cl_command_queue_properties` bitfield  of
 * command queue properties, mimicking the behavior of the OpenCL
 * clCreateCommandQueue() constructor (deprecated in OpenCL 2.0). The
 * exact OpenCL constructor used is automatically selected based on
 * the OpenCL version of the underlying platform (i.e.
 * clCreateCommandQueue() if OpenCL <= 1.2, or
 * clCreateCommandQueueWithProperties() for OpenCL >= 2.0).
 *
 * To specify OpenCL 2.0 only features, such as on-device queue size,
 * use the ::ccl_queue_new_full() constructor.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] ctx Context wrapper object.
 * @param[in] dev Device wrapper object, must be associated with `ctx`.
 * @param[in] properties Bitfield of command queue properties.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLQueue wrapper for the given device and context,
 * or `NULL` if an error occurs.
 * */
CCLQueue* ccl_queue_new(CCLContext* ctx, CCLDevice* dev,
	cl_command_queue_properties properties, GError** err) {

	const cl_queue_properties prop_full[] =
		{ CL_QUEUE_PROPERTIES, properties, 0 };

	return ccl_queue_new_full(ctx, dev, prop_full, err);

}

/**
 * Decrements the reference count of the command queue wrapper
 * object. If it reaches 0, the command queue wrapper object is
 * destroyed.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * */
void ccl_queue_destroy(CCLQueue* cq) {

	ccl_wrapper_unref((CCLWrapper*) cq, sizeof(CCLQueue),
		(ccl_wrapper_release_fields) ccl_queue_release_fields,
		(ccl_wrapper_release_cl_object) clReleaseCommandQueue, NULL);

}

/**
 * Get the context associated with the given command queue wrapper
 * object.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The context associated with the given command queue wrapper
 * object, or `NULL` if an error occurs.
 * */
CCLContext* ccl_queue_get_context(CCLQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	CCLContext* ctx = NULL;

	/* Internal error object. */
	GError* err_internal = NULL;

	/* Check if context wrapper is already kept by the queue wrapper. */
	if (cq->ctx != NULL) {
		/* If so, return it. */
		ctx = cq->ctx;
	} else {
		/* Otherwise, get it using a query. */
		CCLWrapperInfo* info = NULL;
		info = ccl_queue_get_info(
			cq, CL_QUEUE_CONTEXT, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		ctx = ccl_context_new_wrap(*((cl_context*) info->value));
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		cq->ctx = ctx;
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return the command queue context wrapper. */
	return ctx;

}

/**
 * Get the device associated with the given command queue wrapper
 * object.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The device associated with the given command queue wrapper
 * object, or `NULL` if an error occurs.
 * */
CCLDevice* ccl_queue_get_device(CCLQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* The device wrapper object to return. */
	CCLDevice* dev = NULL;

	/* Internal error object. */
	GError* err_internal = NULL;

	/* Check if device wrapper is already kept by the queue wrapper. */
	if (cq->dev != NULL) {
		/* If so, return it. */
		dev = cq->dev;
	} else {
		/* Otherwise, get it using a query. */
		CCLWrapperInfo* info = NULL;
		info = ccl_queue_get_info(
			cq, CL_QUEUE_DEVICE, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		dev = ccl_device_new_wrap(*((cl_device_id*) info->value));
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		cq->dev = dev;
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return the command queue device wrapper object. */
	return dev;

}

/**
 * @internal
 * Create an event wrapper from a given OpenCL event object and
 * associate it with the command queue.
 *
 * This function is used by the `ccl_*_enqueue_*()` functions and will
 * rarely be called from client code.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @param[in] event The OpenCL event to wrap and associate with the
 * given command queue.
 * @return The event wrapper object for the given OpenCL event object.
 * */
CCLEvent* ccl_queue_produce_event(CCLQueue* cq, cl_event event) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure event is not NULL. */
	g_return_val_if_fail(event != NULL, NULL);

	/* Wrap the OpenCL event. */
	CCLEvent* evt = ccl_event_new_wrap(event);

	/* Initialize the list of events of this command queue. */
	if (cq->evts == NULL) {
		cq->evts = g_hash_table_new_full(g_direct_hash, g_direct_equal,
			(GDestroyNotify) ccl_event_destroy, NULL);
	}

	/* Add the wrapped event to the list of events of this command
	 * queue. */
	g_hash_table_add(cq->evts, (gpointer) evt);

	/* Return the wrapped event. */
	return evt;

}

/**
 * @internal
 * Initialize an iterator for this command queue's list of event
 * wrappers. The event wrappers can be iterated in a loop using the
 * ccl_queue_iter_event_next() function.
 *
 * This function is used by @ref PROFILER "profile module" functions and
 * will rarely be called from client code.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * */
void ccl_queue_iter_event_init(CCLQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);

	/* Initialize iterator. */
	g_hash_table_iter_init(&cq->evt_iter, cq->evts);

}

/**
 * @internal
 * Get the next event wrapper associated with this queue.
 *
 * This function is used by @ref PROFILER "profile module" functions and
 * will rarely be called from client code.
 *
 * @attention Calling this before ccl_queue_iter_event_init() is
 * undefined behavior.
 * @warning This function is not thread-safe.
 * @warning No events should be enqueued on this queue while the
 * iteration is ongoing.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @return The next event wrapper associated with this queue, or `NULL`
 * if no more event wrappers are available.
 * */
CCLEvent* ccl_queue_iter_event_next(CCLQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);

	gpointer evt;
	gboolean exists = g_hash_table_iter_next(&cq->evt_iter, &evt, NULL);
	return exists ? (CCLEvent*) evt : NULL;
}

/**
 * Issues all previously queued commands in a command queue to the
 * associated device. This function is a wrapper for the clFlush()
 * OpenCL function.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_queue_flush(CCLQueue* cq, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_INT_MAX);
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);

	/* OpenCL status flag. */
	cl_int ocl_status;

	/* Flush queue. */
	ocl_status = clFlush(ccl_queue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CCL_OCL_ERROR, ocl_status,
			"%s: unable to flush queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Return status. */
	return ocl_status == CL_SUCCESS ? CL_TRUE : CL_FALSE;
}

/**
 * Blocks until all previously queued OpenCL commands in a command-queue
 * are issued to the associated device and have completed. This function
 * is a wrapper for the clFinish() OpenCL function.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_queue_finish(CCLQueue* cq, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_INT_MAX);
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);

	/* OpenCL status flag. */
	cl_int ocl_status;

	/* Finish queue. */
	ocl_status = clFinish(ccl_queue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CCL_OCL_ERROR, ocl_status,
			"%s: unable to finish queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Return status. */
	return ocl_status == CL_SUCCESS ? CL_TRUE : CL_FALSE;

}

/**
 * Release all events associated with the command queue.
 *
 * _cf4ocl_ command queue wrappers internally keep events for profiling
 * purposes and simpler handling of event associated memory. However,
 * a very large number of events can have an impact on utilized memory.
 * In such cases, this function can be used to periodically release
 * these events.
 *
 * This function is also called by the ::ccl_prof_calc() function,
 * i.e., the queue events are released after the profiling analysis is
 * performed.
 *
 * @public @memberof ccl_queue
 *
 * @param[in] cq The command queue wrapper object.
 * */
void ccl_queue_gc(CCLQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);

	/* Release events. */
	if (cq->evts != NULL) {
		g_hash_table_remove_all(cq->evts);
	}

}

/**
 * For platforms which do not support clEnqueueBarrierWithWaitList()
 * (OpenCL <= 1.1), this function implements the same functionality by
 * using the deprecated clEnqueueBarrier(), clEnqueueWaitForEvents() and
 * clEnqueueMarker() OpenCL functions.
 *
 * If `evt_wait_lst` is `NULL`, clEnqueueBarrier() and clEnqueueMarker()
 * are called in sequence; otherwise (if there are events which must be
 * waited on), clEnqueueWaitForEvents() and clEnqueueMarker() are
 * called in sequence. The calls on clEnqueueMarker() allow to fire
 * a marker event (not produced by either clEnqueueBarrier() or
 * clEnqueueWaitForEvents()). This marker event can then be used to
 * queue a wait on.
 *
 * @internal
 * @see ccl_enqueue_barrier()
 *
 * @param[in] cq Command queue wrapper object.
 * @param[in,out] evt_wait_lst Event wait list.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An OpenCL marker event (will be wrapped by the calling
 * function).
 * */
static cl_event ccl_enqueue_barrier_deprecated(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* OpenCL status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;

	CCL_BEGIN_IGNORE_DEPRECATIONS

	/* Exact OpenCL function to use depends on whether evt_wait_lst
	 * is NULL or empty. */
	if ((evt_wait_lst == NULL) ||
		(ccl_event_wait_list_get_num_events(evt_wait_lst) == 0)) {

		/* If so, use clEnqueueBarrier() */
		ocl_status = clEnqueueBarrier(ccl_queue_unwrap(cq));
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: error in clEnqueueBarrier() (OpenCL error %d: %s).",
			G_STRLOC, ocl_status, ccl_err(ocl_status));

	} else {

		/* Otherwise use clEnqueueWaitForEvents(). */
		ocl_status = clEnqueueWaitForEvents(ccl_queue_unwrap(cq),
			ccl_event_wait_list_get_num_events(evt_wait_lst),
			ccl_event_wait_list_get_clevents(evt_wait_lst));
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: error in clEnqueueWaitForEvents() (OpenCL error %d: %s).",
			G_STRLOC, ocl_status, ccl_err(ocl_status));

	}

	/* Enqueue a marker so we get an OpenCL event object. */
	ocl_status = clEnqueueMarker(ccl_queue_unwrap(cq), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: error in clEnqueueMarker() (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	CCL_END_IGNORE_DEPRECATIONS

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* In case of error, return a NULL event. */
	event = NULL;

finish:

	/* Return OpenCL event. */
	return event;

}

/**
 * Enqueues a barrier command on the given command queue. The barrier
 * can wait on a given list of events, or wait until all previous
 * enqueued commands have completed if `evt_wait_lst` is `NULL`. A
 * marker event is returned, which can be used to identify this barrier
 * command later on. This function is a wrapper for the
 * clEnqueueBarrierWithWaitList() OpenCL function (OpenCL >= 1.2).
 *
 * @public @memberof ccl_queue
 * @copydoc ccl_enqueue_barrier_deprecated()
 *
 * @param[in] cq Command queue wrapper object.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An event wrapper object that identifies this particular
 * command.
 * */
CCLEvent* ccl_enqueue_barrier(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Event wrapper to return. */
	CCLEvent* evt;
	/* OpenCL event object. */
	cl_event event;
	/* Internal error handling object. */
	GError* err_internal = NULL;

#ifdef CL_VERSION_1_2

	/* If library is compiled with support for OpenCL >= 1.2, then use
	 * the platform's OpenCL version for selecting the desired
	 * functionality. */

	/* Context associated with event. */
	CCLContext* ctx;
	/* OpenCL version. */
	double platf_ver;
	/* OpenCL status. */
	cl_int ocl_status;

	/* Get platform version. */
	ctx = ccl_queue_get_context(cq, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	platf_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Proceed depending on platform version. */
	if (platf_ver >= 120) {

		/* Use "new" functions. */
		ocl_status = clEnqueueBarrierWithWaitList(ccl_queue_unwrap(cq),
			ccl_event_wait_list_get_num_events(evt_wait_lst),
			ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: error in clEnqueueBarrierWithWaitList() (OpenCL error %d: %s).",
			G_STRLOC, ocl_status, ccl_err(ocl_status));

	} else {

		/* Use "old" functions. */
		event = ccl_enqueue_barrier_deprecated(
			cq, evt_wait_lst,  &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

#else

	/* If library is compiled with support for OpenCL 1.0 and 1.1,
	 * then use those functions by default. */
	event = ccl_enqueue_barrier_deprecated(
		cq, evt_wait_lst,  &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

#endif

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

	/* In case of error, return NULL. */
	evt = NULL;

finish:

	/* Return event. */
	return evt;

}

/**
 * For platforms which do not support clEnqueueMarkerWithWaitList()
 * (OpenCL <= 1.1), this function uses the deprecated clEnqueueMarker()
 * OpenCL function. However, in this case `evt_wait_lst` must be `NULL`,
 * because clEnqueueMarker() does not support markers with wait lists.
 * If `evt_wait_lst` is not `NULL`, it will be ignored (i.e. the marker
 * will only fire an event after all commands queued before the marker
 * command are complete) and a warning will be generated.
 *
 * @internal
 * @see ccl_enqueue_marker()
 *
 * @param[in] cq Command queue wrapper object.
 * @param[in,out] evt_wait_lst Event wait list. Must be `NULL` or a
 * warning will be generated.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An OpenCL marker event (will be wrapped by the calling
 * function).
 * */
static cl_event ccl_enqueue_marker_deprecated(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* OpenCL status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;

	/* evt_wait_lst must be NULL or empty, because getting a marker to
	 * wait on some events is only supported in OpenCL >= 1.2. */
	if (evt_wait_lst != NULL) {
		g_warning("The OpenCL version of the selected platform " \
			"doesn't support markers on specific events. The marker " \
			"will only fire an event when all previous events have " \
			"been completed");
	}

	CCL_BEGIN_IGNORE_DEPRECATIONS

	/* Call clEnqueueMarker() once. */
	ocl_status = clEnqueueMarker(ccl_queue_unwrap(cq), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: error in clEnqueueMarker() (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	CCL_END_IGNORE_DEPRECATIONS

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* In case of error, return a NULL event. */
	event = NULL;

finish:

	/* Return OpenCL event. */
	return event;

}

/**
 * Enqueues a marker command on the given command queue. The marker can
 * wait on a given list of events, or wait until all previous enqueued
 * commands have completed if `evt_wait_lst` is `NULL`. This function
 * is a wrapper for the clEnqueueMarkerWithWaitList() OpenCL function
 * (OpenCL >= 1.2).
 *
 * @public @memberof ccl_queue
 * @copydoc ccl_enqueue_marker_deprecated()
 * @note Requires OpenCL >= 1.2 if `evt_wait_lst` is not `NULL`.
 *
 * @param[in] cq Command queue wrapper object.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code. Must be `NULL` if OpenCL platform
 * version is <= 1.1.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An event wrapper object that identifies this particular
 * command.
 * */
CCLEvent* ccl_enqueue_marker(CCLQueue* cq,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Event wrapper to return. */
	CCLEvent* evt;
	/* OpenCL event object. */
	cl_event event;
	/* Internal error handling object. */
	GError* err_internal = NULL;

#ifdef CL_VERSION_1_2

	/* If library is compiled with support for OpenCL >= 1.2, then use
	 * the platform's OpenCL version for selecting the desired
	 * functionality. */

	/* Context associated with event. */
	CCLContext* ctx;
	/* OpenCL version. */
	double platf_ver;
	/* OpenCL status. */
	cl_int ocl_status;

	/* Get platform version. */
	ctx = ccl_queue_get_context(cq, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	platf_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Proceed depending on platform version. */
	if (platf_ver >= 120) {

		/* Use "new" functions. */
		ocl_status = clEnqueueMarkerWithWaitList(ccl_queue_unwrap(cq),
			ccl_event_wait_list_get_num_events(evt_wait_lst),
			ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: error in clEnqueueMarkerWithWaitList() (OpenCL error %d: %s).",
			G_STRLOC, ocl_status, ccl_err(ocl_status));

	} else {

		/* Use "old" functions. */
		event = ccl_enqueue_marker_deprecated(
			cq, evt_wait_lst, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

#else

	/* If library is compiled with support for OpenCL 1.0 and 1.1,
	 * then use those functions by default. */
	event = ccl_enqueue_marker_deprecated(
		cq, evt_wait_lst, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

#endif

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

	/* In case of error, return NULL. */
	evt = NULL;

finish:

	/* Return event. */
	return evt;

}

/** @} */
