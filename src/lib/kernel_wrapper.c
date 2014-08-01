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
 * @brief OpenCL kernel wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "kernel_wrapper.h"
#include "program_wrapper.h"

/**
 * @brief Kernel wrapper object.
 */
struct ccl_kernel {

	/** Parent wrapper object. */
	CCLWrapper base;
	
	/** Kernel arguments. */
	GHashTable* args;
	
};

/**
 * @brief Implementation of ccl_wrapper_release_fields() function for
 * ::CCLKernel wrapper objects.
 * 
 * @param krnl A ::CCLKernel wrapper object.
 * */
static void ccl_kernel_release_fields(CCLKernel* krnl) {

	/* Make sure krnl wrapper object is not NULL. */
	g_return_if_fail(krnl != NULL);

	/* Free kernel arguments. */
	if (krnl->args != NULL)
		g_hash_table_destroy(krnl->args);


}

/** 
 * @addtogroup KERNEL_WRAPPER
 * @{
 */

/**
 * @brief Get the kernel wrapper for the given OpenCL kernel.
 * 
 * If the wrapper doesn't exist, its created with a reference count 
 * of 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL kernel directly (using the
 * clCreateKernel() function) and then wrap the OpenCL kernel in a 
 * ::CCLKernel wrapper object.
 * 
 * @param kernel The OpenCL kernel to be wrapped.
 * @return The ::CCLKernel wrapper for the given OpenCL kernel.
 * */
CCLKernel* ccl_kernel_new_wrap(cl_kernel kernel) {
	
	return (CCLKernel*) ccl_wrapper_new(
		(void*) kernel, sizeof(CCLKernel));
		
}

CCLKernel* ccl_kernel_new(
	CCLProgram* prg, const char* kernel_name, GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure kernel_name is not NULL. */
	g_return_val_if_fail(kernel_name != NULL, NULL);

	/* Kernel wrapper object. */
	CCLKernel* krnl = NULL;

	/* OpenCL return status. */
	cl_int ocl_status;
		
	/* The OpenCL kernel object. */
	cl_kernel kernel = NULL;
		
	/* Create kernel. */
	kernel = clCreateKernel(ccl_program_unwrap(prg), 
		kernel_name, &ocl_status);
	gef_if_err_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler,
		"%s: unable to create kernel (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Create kernel wrapper. */
	krnl = ccl_kernel_new_wrap(kernel);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	krnl = NULL;
	
finish:

	/* Return kernel wrapper. */
	return krnl;
		
}

/** 
 * @brief Decrements the reference count of the kernel wrapper object. 
 * If it reaches 0, the kernel wrapper object is destroyed.
 *
 * @param krnl The kernel wrapper object.
 * */
void ccl_kernel_destroy(CCLKernel* krnl) {
	
	ccl_wrapper_unref((CCLWrapper*) krnl, sizeof(CCLKernel),
		(ccl_wrapper_release_fields) ccl_kernel_release_fields, 
		(ccl_wrapper_release_cl_object) clReleaseKernel, NULL); 

}

void ccl_kernel_set_arg(CCLKernel* krnl, cl_uint arg_index, 
	CCLArg* arg) {
		
	g_return_if_fail(krnl != NULL);
	
	if (krnl->args == NULL) {
		krnl->args = g_hash_table_new_full(g_direct_hash,
			g_direct_equal, NULL, (GDestroyNotify) ccl_arg_destroy);
	}
	
	g_hash_table_replace(krnl->args, GUINT_TO_POINTER(arg_index),
		(gpointer) arg);

}

void ccl_kernel_set_args(CCLKernel* krnl, ...) {
	
	va_list args;
	va_start(args, krnl);
	ccl_kernel_set_args_v(krnl, args);
	va_end(args);

}

void ccl_kernel_set_args_v(CCLKernel* krnl, va_list args) {
		
	for (cl_uint i = 0; ; i++) {
		CCLArg* arg = va_arg(args, CCLArg*);
		if (arg == NULL)
			break;
		ccl_kernel_set_arg(krnl, i, arg);
	}

}
CCLEvent* ccl_kernel_run(CCLKernel* krnl, CCLQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err) {
		
	/* Make sure number krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure number cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* OpenCL status flag. */
	cl_int ocl_status;
	
	/* OpenCL event. */
	cl_event event;
	/* Event wrapper. */
	CCLEvent* evt;
	
	/* Iterator for table of kernel arguments. */
	GHashTableIter iter;
	gpointer arg_index_ptr, arg_ptr;
	
	/* Set pending kernel arguments. */
	if (krnl->args != NULL) {
		g_hash_table_iter_init(&iter, krnl->args);
		while (g_hash_table_iter_next(&iter, &arg_index_ptr, &arg_ptr)) {
			cl_uint arg_index = GPOINTER_TO_UINT(arg_index_ptr);
			CCLArg* arg = (CCLArg*) arg_ptr;
			ocl_status = clSetKernelArg(ccl_kernel_unwrap(krnl), arg_index, 
				ccl_arg_size(arg), ccl_arg_value(arg));
			gef_if_err_create_goto(*err, CCL_ERROR, 
				CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
				"%s: unable to set kernel arg %d (OpenCL error %d: %s).",
				G_STRLOC, arg_index, ocl_status, ccl_err(ocl_status));
			g_hash_table_iter_remove(&iter);
		}
	}
		
	/* Run kernel. */
	ocl_status = clEnqueueNDRangeKernel(ccl_queue_unwrap(cq),
		ccl_kernel_unwrap(krnl), work_dim, global_work_offset,
		global_work_size, local_work_size, 
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_err_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to enqueue kernel (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:

	/* Return evt. */
	return evt;
	
}
	
/** 
 * @brief Set kernel arguments and run it. 
 * 
 * @warning This function is not thread-safe. For multi-threaded 
 * execution of the same kernel function, create multiple instances of 
 * a kernel wrapper for the given kernel function with ccl_kernel_new(),
 * one for each thread.
 * 
 * @param krnl
 * @param cq
 * @param work_dim
 * @param global_work_offset
 * @param global_work_size
 * @param local_work_size
 * @param evt_wait_lst
 * @param err
 * @param ...
 * @return
 * */
CCLEvent* ccl_kernel_set_args_and_run(CCLKernel* krnl, CCLQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, ...) {

	/* Event wrapper. */
	CCLEvent* evt;
	
	/* Variable argument list. */
	va_list args;
	
	/* Set kernel arguments and run it. */
	va_start(args, err);
	evt = ccl_kernel_set_args_and_run_v(krnl, cq, work_dim, 
		global_work_offset, global_work_size, local_work_size, 
		evt_wait_lst, err, args);
	va_end(args);
	
	/* Return event wrapper. */
	return evt;
	
}

CCLEvent* ccl_kernel_set_args_and_run_v(CCLKernel* krnl, CCLQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, va_list args) {

	/* Make sure number krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure number cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	GError* err_internal = NULL;
	
	CCLEvent* evt = NULL;
	
	/* Set kernel arguments. */
	ccl_kernel_set_args_v(krnl, args);
	
	/* Enqueue kernel. */
	evt = ccl_kernel_run(krnl, cq, work_dim, global_work_offset, 
		global_work_size, local_work_size, evt_wait_lst, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Return event wrapper. */
	return evt;
	
}

/** @} */

