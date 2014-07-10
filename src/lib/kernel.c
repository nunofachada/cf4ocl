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

#include "kernel.h"

/**
 * @brief Kernel wrapper object.
 */
struct cl4_kernel {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
	/** Kernel arguments. */
	GHashTable* args;
	
};

/**
 * @brief Implementation of cl4_wrapper_release_fields() function for
 * ::CL4Kernel wrapper objects.
 * 
 * @param krnl A ::CL4Kernel wrapper object.
 * */
static void cl4_kernel_release_fields(CL4Kernel* krnl) {

	/* Make sure krnl wrapper object is not NULL. */
	g_return_if_fail(krnl != NULL);

	/* Free kernel arguments. */
	if (krnl->args != NULL)
		g_hash_table_destroy(krnl->args);


}

/**
 * @brief Get the kernel wrapper for the given OpenCL kernel.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL kernel directly (using the
 * clCreateKernel() function) and then wrap the OpenCL kernel in a 
 * ::CL4Kernel wrapper object.
 * 
 * @param kernel The OpenCL kernel to be wrapped.
 * @return The ::CL4Kernel wrapper for the given OpenCL kernel.
 * */
CL4Kernel* cl4_kernel_new_wrap(cl_kernel kernel) {
	
	return (CL4Kernel*) cl4_wrapper_new(
		(void*) kernel, sizeof(CL4Kernel));
		
}

/** 
 * @brief Decrements the reference count of the kernel wrapper object. 
 * If it reaches 0, the kernel wrapper object is destroyed.
 *
 * @param krnl The kernel wrapper object.
 * */
void cl4_kernel_destroy(CL4Kernel* krnl) {
	
	cl4_wrapper_unref((CL4Wrapper*) krnl, sizeof(CL4Kernel),
		(cl4_wrapper_release_fields) cl4_kernel_release_fields, 
		(cl4_wrapper_release_cl_object) clReleaseKernel, NULL); 

}

void cl4_kernel_set_arg(CL4Kernel* krnl, cl_uint arg_index, 
	CL4Arg* arg) {
		
	g_return_if_fail(krnl != NULL);
	
	if (krnl->args == NULL) {
		krnl->args = g_hash_table_new_full(g_direct_hash,
			g_direct_equal, NULL, (GDestroyNotify) cl4_arg_destroy);
	}
	
	g_hash_table_replace(krnl->args, GUINT_TO_POINTER(arg_index),
		(gpointer) arg);

}

void cl4_kernel_set_args(CL4Kernel* krnl, ...) {
	
	va_list args;
	va_start(args, krnl);
	cl4_kernel_set_args_v(krnl, args);
	va_end(args);

}

void cl4_kernel_set_args_v(CL4Kernel* krnl, va_list args) {
		
	for (cl_uint i = 0; ; i++) {
		CL4Arg* arg = va_arg(args, CL4Arg*);
		if (arg == NULL)
			break;
		cl4_kernel_set_arg(krnl, i, arg);
	}

}
CL4Event* cl4_kernel_run(CL4Kernel* krnl, CL4CQueue* cq, 
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CL4EventWaitList evt_wait_lst, GError** err) {
		
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
	CL4Event* evt;
	
	/* Iterator for table of kernel arguments. */
	GHashTableIter iter;
	gpointer arg_index_ptr, arg_ptr;
	
	/* Set pending kernel arguments. */
	if (krnl->args != NULL) {
		g_hash_table_iter_init(&iter, krnl->args);
		while (g_hash_table_iter_next(&iter, &arg_index_ptr, &arg_ptr)) {
			cl_uint arg_index = GPOINTER_TO_UINT(arg_index_ptr);
			CL4Arg* arg = (CL4Arg*) arg_ptr;
			ocl_status = clSetKernelArg(cl4_kernel_unwrap(krnl), arg_index, 
				cl4_arg_size(arg), cl4_arg_value(arg));
			gef_if_error_create_goto(*err, CL4_ERROR, 
				CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler, 
				"Function '%s': unable to set kernel arg %d (OpenCL error %d: %s).",
				__func__, arg_index, ocl_status, cl4_err(ocl_status));
			g_hash_table_iter_remove(&iter);
		}
	}
		
	/* Run kernel. */
	ocl_status = clEnqueueNDRangeKernel(cl4_cqueue_unwrap(cq),
		cl4_kernel_unwrap(krnl), work_dim, global_work_offset,
		global_work_size, local_work_size, 
		cl4_event_wait_list_get_num_events(evt_wait_lst),
		cl4_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"Function '%s': unable to enqueue kernel (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = cl4_cqueue_produce_event(cq, event);
	
	/* Clear event wait list. */
	cl4_event_wait_list_clear(evt_wait_lst);
	
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
	
CL4Event* cl4_kernel_set_args_and_run(CL4Kernel* krnl, CL4CQueue* cq, 
	cl_uint work_dim, size_t* global_work_offset, 
	size_t* global_work_size, size_t* local_work_size, 
	CL4EventWaitList evt_wait_lst, GError** err, ...) {

	/* Make sure number krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure number cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Variable argument list. */
	va_list args;
	
	GError* err_internal = NULL;
	
	CL4Event* evt = NULL;
	
	/* Set kernel arguments. */
	va_start(args, err);
	cl4_kernel_set_args_v(krnl, args);
	va_end(args);
	
	/* Enqueue kernel. */
	evt = cl4_kernel_run(krnl, cq, work_dim, global_work_offset, 
		global_work_size, local_work_size, evt_wait_lst, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Return ctx. */
	return evt;
	
}
