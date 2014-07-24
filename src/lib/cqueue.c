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
 * @brief OpenCL Command Queue wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "cqueue.h"

/**
 * @brief Command queue wrapper object.
 */
struct cl4_cqueue {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
	/** Context wrapper to which the queue is associated with. */
	CL4Context* ctx;
	
	/** Device wrapper to which the queue is associated with. */
	CL4Device* dev;
	
	/** Events associated with the command queue. */
	GHashTable* evts;
	
	/** Event iterator. */
	GHashTableIter evt_iter;
	
};

/**
 * @brief Implementation of cl4_wrapper_release_fields() function for
 * ::CL4CQueue wrapper objects.
 * 
 * @param cq A ::CL4CQueue wrapper object.
 * */
static void cl4_cqueue_release_fields(CL4CQueue* cq) {

	/* Make sure cq wrapper object is not NULL. */
	g_return_if_fail(cq != NULL);

	/* Decrease reference count of context and device wrappers, if
	 * they're set. */
	 if (cq->ctx != NULL)
		cl4_context_unref(cq->ctx);
	 if (cq->dev != NULL)
		cl4_device_unref(cq->dev);

	/* Destroy the events table. */
	if (cq->evts != NULL) {
		g_hash_table_destroy(cq->evts);
	}

}

/**
 * @brief Get the command queue wrapper for the given OpenCL command 
 * queue.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL command queue directly (using the
 * clCreateCommandQueue() function) and then wrap the OpenCL command 
 * queue in a ::CL4CQueue wrapper object.
 * 
 * @param command_queue The OpenCL command queue to be wrapped.
 * @return The ::CL4CQueue wrapper for the given OpenCL command queue.
 * */
CL4CQueue* cl4_cqueue_new_wrap(cl_command_queue command_queue) {
	
	return (CL4CQueue*) cl4_wrapper_new(
		(void*) command_queue, sizeof(CL4CQueue));
		
}

CL4CQueue* cl4_cqueue_new_direct(cl_context context, 
	cl_device_id device, cl_command_queue_properties properties, 
	GError** err) {

	/* Make sure context is not NULL. */
	g_return_val_if_fail(context != NULL, NULL);
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
		
	/* The command queue wrapper object. */
	CL4CQueue* cq = NULL;
	
	/* The OpenCL status flag. */
	cl_int ocl_status;
	
	/* The OpenCL command queue object. */
	cl_command_queue queue = NULL;
	
	/* Create and keep the OpenCL command queue object. */
	queue = clCreateCommandQueue(context, device,
		properties, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, 
		CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
		"%s: unable to create queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, cl4_err(ocl_status));

	/* Wrap the queue. */
	cq = cl4_cqueue_new_wrap(queue);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		

	/* Return the new command queue wrapper object. */
	return cq;	
	
}

CL4CQueue* cl4_cqueue_new(CL4Context* ctx, CL4Device* dev, 
	cl_command_queue_properties properties, GError** err) {
		
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* The command queue wrapper object. */
	CL4CQueue* cq = NULL;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	/* If dev is NULL, get first device in context. */
	if (dev == NULL) {
		dev = cl4_context_get_device(ctx, 0, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}
	
	/* Create the command queue. */
	cq = cl4_cqueue_new_direct(cl4_context_unwrap(ctx), 
		cl4_device_unwrap(dev), properties, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Keep the context and device wrappers, update their reference
	 * count. */
	cq->ctx = ctx;
	cl4_context_ref(ctx);
	cq->dev = dev;
	cl4_device_ref(dev);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		

	/* Return the new command queue wrapper object. */
	return cq;	
	
}

/** 
 * @brief Decrements the reference count of the command queue wrapper 
 * object. If it reaches 0, the command queue wrapper object is 
 * destroyed.
 *
 * @param cq The command queue wrapper object.
 * */
void cl4_cqueue_destroy(CL4CQueue* cq) {
	
	cl4_wrapper_unref((CL4Wrapper*) cq, sizeof(CL4CQueue),
		(cl4_wrapper_release_fields) cl4_cqueue_release_fields, 
		(cl4_wrapper_release_cl_object) clReleaseCommandQueue, NULL); 

}

CL4Context* cl4_cqueue_get_context(CL4CQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	CL4Context* ctx = NULL;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	if (cq->ctx != NULL) {
		ctx = cq->ctx;
	} else {
		CL4WrapperInfo* info = NULL;
		info = cl4_cqueue_info(
			cq, CL_QUEUE_CONTEXT, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		ctx = cl4_context_new_wrap(*((cl_context*) info->value));
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		cq->ctx = ctx;
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		

	/* Return the command queue context wrapper. */
	return ctx;	

}

CL4Device* cl4_cqueue_get_device(CL4CQueue* cq, GError** err) {
	
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	CL4Device* dev = NULL;
	
	/* Internal error object. */
	GError* err_internal = NULL;
	
	if (cq->dev != NULL) {
		dev = cq->dev;
	} else {
		CL4WrapperInfo* info = NULL;
		info = cl4_cqueue_info(
			cq, CL_QUEUE_DEVICE, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		dev = cl4_device_new_wrap(*((cl_device_id*) info->value));
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		cq->dev = dev;
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:		

	/* Return the command queue device wrapper object. */
	return dev;	
	
}

CL4Event* cl4_cqueue_produce_event(CL4CQueue* cq, cl_event event) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure evt is not NULL. */
	g_return_val_if_fail(event != NULL, NULL);

	CL4Event* evt = cl4_event_new_wrap(event);
	
	if (cq->evts == NULL) {
		cq->evts = g_hash_table_new_full(g_direct_hash, g_direct_equal,
			(GDestroyNotify) cl4_event_destroy, NULL);
	}
	
	g_hash_table_add(cq->evts, (gpointer) evt);
	
	return evt;
	
}

void cl4_cqueue_iter_event_init(CL4CQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);

	g_hash_table_iter_init(&cq->evt_iter, cq->evts);

}

/* Calling this before the previous function is undefined behavior. 
 * Also warn this is not thread-safe. */
CL4Event* cl4_cqueue_iter_event_next(CL4CQueue* cq) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);

	gpointer evt;
	gboolean exists = g_hash_table_iter_next(&cq->evt_iter, &evt, NULL);
	return exists ? (CL4Event*) evt : NULL; 
}

cl_int cl4_cqueue_flush(CL4CQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);
	
	/* OpenCL status flag. */
	cl_int ocl_status;

	ocl_status = clFlush(cl4_cqueue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CL4_ERROR, ocl_status, 
			"%s: unable to flush queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, cl4_err(ocl_status));

	return ocl_status;
}

cl_int cl4_cqueue_finish(CL4CQueue* cq, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, CL_INVALID_COMMAND_QUEUE);
	
	/* OpenCL status flag. */
	cl_int ocl_status;

	ocl_status = clFinish(cl4_cqueue_unwrap(cq));
	if (ocl_status != CL_SUCCESS)
		g_set_error(err, CL4_ERROR, ocl_status, 
			"%s: unable to finish queue (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, cl4_err(ocl_status));

	return ocl_status;

}
