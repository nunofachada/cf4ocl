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

CCLQueue* ccl_queue_new(CCLContext* ctx, CCLDevice* dev, 
	cl_command_queue_properties properties, GError** err) {
		
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
	if (platf_ver < 2.0) {
		queue = clCreateCommandQueueWithProperties(
			ccl_context_unwrap(ctx), ccl_device_unwrap(dev), 
			properties, &ocl_status);
	} else {
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		queue = clCreateCommandQueue(ccl_context_unwrap(ctx), 
			ccl_device_unwrap(dev), properties, &ocl_status);
		G_GNUC_END_IGNORE_DEPRECATIONS
	}
#else
	/* Create and keep the OpenCL command queue object. */
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

CCLContext* ccl_queue_get_context(CCLQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	CCLContext* ctx = NULL;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	if (cq->ctx != NULL) {
		ctx = cq->ctx;
	} else {
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

CCLDevice* ccl_queue_get_device(CCLQueue* cq, GError** err) {
	
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	CCLDevice* dev = NULL;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	if (cq->dev != NULL) {
		dev = cq->dev;
	} else {
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

CCLEvent* ccl_queue_produce_event(CCLQueue* cq, cl_event event) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure evt is not NULL. */
	g_return_val_if_fail(event != NULL, NULL);

	CCLEvent* evt = ccl_event_new_wrap(event);
	
	if (cq->evts == NULL) {
		cq->evts = g_hash_table_new_full(g_direct_hash, g_direct_equal,
			(GDestroyNotify) ccl_event_destroy, NULL);
	}
	
	g_hash_table_add(cq->evts, (gpointer) evt);
	
	return evt;
	
}

void ccl_queue_iter_event_init(CCLQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);

	g_hash_table_iter_init(&cq->evt_iter, cq->evts);

}

/* Calling this before the previous function is undefined behavior. 
 * Also warn this is not thread-safe. */
CCLEvent* ccl_queue_iter_event_next(CCLQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);

	gpointer evt;
	gboolean exists = g_hash_table_iter_next(&cq->evt_iter, &evt, NULL);
	return exists ? (CCLEvent*) evt : NULL; 
}

cl_int ccl_queue_flush(CCLQueue* cq, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_INT_MAX);
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);
	
	/* OpenCL status flag. */
	cl_int ocl_status;

	ocl_status = clFlush(ccl_queue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CCL_OCL_ERROR, ocl_status, 
			"%s: unable to flush queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	return ocl_status;
}

cl_int ccl_queue_finish(CCLQueue* cq, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_INT_MAX);
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);
	
	/* OpenCL status flag. */
	cl_int ocl_status;

	ocl_status = clFinish(ccl_queue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CCL_OCL_ERROR, ocl_status, 
			"%s: unable to finish queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	return ocl_status;

}

/** @} */
