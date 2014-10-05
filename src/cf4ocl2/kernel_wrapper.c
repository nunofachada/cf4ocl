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
 * Implementation of a wrapper class and its methods for OpenCL kernel
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "kernel_wrapper.h"
#include "program_wrapper.h"

/**
 * Kernel wrapper class.
 *
 * @extends ccl_wrapper
 */
struct ccl_kernel {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

	/**
	 * Kernel arguments.
	 * @private
	 * */
	GHashTable* args;

};

/**
 * @internal
 * Implementation of ::ccl_wrapper_release_fields() function for
 * ::CCLKernel wrapper objects.
 *
 * @private @memberof ccl_kernel
 *
 * @param[in] krnl A ::CCLKernel wrapper object.
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
 * Get the kernel wrapper for the given OpenCL kernel.
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
 * @public @memberof ccl_kernel
 *
 * @param[in] kernel The OpenCL kernel to be wrapped.
 * @return The ::CCLKernel wrapper for the given OpenCL kernel.
 * */
CCL_EXPORT
CCLKernel* ccl_kernel_new_wrap(cl_kernel kernel) {

	return (CCLKernel*) ccl_wrapper_new(
		(void*) kernel, sizeof(CCLKernel));

}

/**
 * Create a new kernel wrapper object.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] prg A program wrapper object.
 * @param[in] kernel_name The kernel name.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new kernel wrapper object.
 * */
CCL_EXPORT
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
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create kernel (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Create kernel wrapper. */
	krnl = ccl_kernel_new_wrap(kernel);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	krnl = NULL;

finish:

	/* Return kernel wrapper. */
	return krnl;

}

/**
 * Decrements the reference count of the kernel wrapper object.
 * If it reaches 0, the kernel wrapper object is destroyed.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl The kernel wrapper object.
 * */
CCL_EXPORT
void ccl_kernel_destroy(CCLKernel* krnl) {

	ccl_wrapper_unref((CCLWrapper*) krnl, sizeof(CCLKernel),
		(ccl_wrapper_release_fields) ccl_kernel_release_fields,
		(ccl_wrapper_release_cl_object) clReleaseKernel, NULL);

}

/**
 * Set one kernel argument. The argument is not immediatly set with the
 * clSetKernelArg() OpenCL function, but is instead kept in an argument
 * table for this kernel. The clSetKernelArg() function is called only
 * before kernel execution for arguments which have not yet been set or
 * have not been updated meanwhile.
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with ccl_kernel_new(),
 * one for each thread.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] arg_index Argument index.
 * @param[in] arg Argument to set. Arguments must be of type ::CCLArg*,
 * ::CCLBuffer*, ::CCLImage* or ::CCLSampler*.
 * */
CCL_EXPORT
void ccl_kernel_set_arg(CCLKernel* krnl, cl_uint arg_index, void* arg) {

	/* Make sure krnl is not NULL. */
	g_return_if_fail(krnl != NULL);

	/* Initialize table of kernel arguments if necessary. */
	if (krnl->args == NULL) {
		krnl->args = g_hash_table_new_full(g_direct_hash,
			g_direct_equal, NULL, (GDestroyNotify) ccl_arg_destroy);
	}

	/* Keep argument in table. */
	g_hash_table_replace(krnl->args, GUINT_TO_POINTER(arg_index),
		(gpointer) arg);

}

/**
 * Set all kernel arguments. This function accepts a variable list of
 * arguments which must end with `NULL`. Each argument is individually
 * set using the ::ccl_kernel_set_arg() function.
 *
 * The ::ccl_kernel_set_args_v() function performs the same operation
 * but accepts an array of kernel arguments instead.
 *
 * @public @memberof ccl_kernel
 *
 * @attention The variable argument list must end with `NULL`.
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with ccl_kernel_new(),
 * one for each thread.
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] ... A `NULL`-terminated list of arguments to set.
 * Arguments must be of type ::CCLArg*, ::CCLBuffer*, ::CCLImage* or
 * ::CCLSampler*.
 * */
CCL_EXPORT
void ccl_kernel_set_args(CCLKernel* krnl, ...) {

	/* The va_list, which represents the variable argument list. */
	va_list args_va;
	/* Array of arguments, created from the va_list. */
	void** args_array = NULL;
	/* Number of arguments. */
	guint num_args = 0;
	/* Aux. arg. when cycling through the va_list. */
	void* aux_arg;

	/* Initialize the va_list. */
	va_start(args_va, krnl);

	/* Get first argument. */
	aux_arg = va_arg(args_va, void*);

	/* Check if any arguments are given, and if so, populate array
	 * of arguments. */
	if (aux_arg != NULL) {

		/* 1. Determine number of arguments. */

		while (aux_arg != NULL) {
			num_args++;
			aux_arg = va_arg(args_va, void*);
		}
		va_end(args_va);

		/* 2. Populate array of arguments. */

		args_array = g_slice_alloc((num_args + 1) * sizeof(void*));
		va_start(args_va, krnl);

		for (guint i = 0; i < num_args; ++i) {
			aux_arg = va_arg(args_va, void*);
			args_array[i] = aux_arg;
		}
		va_end(args_va);
		args_array[num_args] = NULL;

	}

	/* If any arguments are given... */
	if (num_args > 0) {

		/* Call the array version of this function.*/
		ccl_kernel_set_args_v(krnl, args_array);

		/* Free the array of arguments. */
		g_slice_free1((num_args + 1) * sizeof(void*), args_array);

	}

}

/**
 * Set all kernel arguments. This function accepts a `NULL`-terminated
 * array of kernel arguments. Each argument is individually set using
 * the ::ccl_kernel_set_arg() function.
 *
 * The ::ccl_kernel_set_args() function performs the same operation but
 * accepts a `NULL`-terminated variable list of arguments instead.
 *
 * @public @memberof ccl_kernel
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with
 * ::ccl_kernel_new(), one for each thread.
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] args A `NULL`-terminated array of arguments to set.
 * Arguments must be of type ::CCLArg*, ::CCLBuffer*, ::CCLImage* or
 * ::CCLSampler*.
 * */
CCL_EXPORT
void ccl_kernel_set_args_v(CCLKernel* krnl, void** args) {

	/* Make sure krnl is not NULL. */
	g_return_if_fail(krnl != NULL);
	/* Make sure args is not NULL. */
	g_return_if_fail(args != NULL);

	/* Cycle through the arguments. */
	for (guint i = 0; args[i] != NULL; ++i) {

		/* Get next argument. */
		CCLArg* arg = args[i];

		/* Set the i^th kernel argument. */
		ccl_kernel_set_arg(krnl, i, arg);

	}

}

/**
 * Enqueues a kernel for execution on a device.
 *
 * Internally, this function calls the clSetKernelArg() OpenCL function
 * for each argument defined with the ::ccl_kernel_set_arg() function,
 * and the executes the kernel using the clEnqueueNDRangeKernel() OpenCL
 * function.
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with
 * ::ccl_kernel_new(), one for each thread.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] cq A command queue wrapper object.
 * @param[in] work_dim The number of dimensions used to specify the
 * global work-items and work-items in the work-group.
 * @param[in] global_work_offset Can be used to specify an array of
 * `work_dim` unsigned values that describe the offset used to calculate
 * the global ID of a work-item.
 * @param[in] global_work_size An array of `work_dim` unsigned values
 * that describe the number of global work-items in `work_dim`
 * dimensions that will execute the kernel function.
 * @param[in] local_work_size An array of `work_dim` unsigned values
 * that describe the number of work-items that make up a work-group that
 * will execute the specified kernel.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this command.
 * */
CCL_EXPORT
CCLEvent* ccl_kernel_enqueue_ndrange(CCLKernel* krnl, CCLQueue* cq,
	cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, GError** err) {

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
			ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
				CL_SUCCESS != ocl_status, ocl_status, error_handler,
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
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to enqueue kernel (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

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

/**
 * Set kernel arguments and enqueue it for execution on a device.
 *
 * Internally this function sets kernel arguments by calling
 * ::ccl_kernel_set_args_v(), and enqueues the kernel for execution
 * by calling ::ccl_kernel_enqueue_ndrange().
 *
 * The ::ccl_kernel_set_args_and_enqueue_ndrange_v() function performs
 * the same operation but accepts an array of arguments instead.
 *
 * @public @memberof ccl_kernel
 *
 * @attention The variable argument list must end with `NULL`.
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with
 * ::ccl_kernel_new(), one for each thread.
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] cq A command queue wrapper object.
 * @param[in] work_dim The number of dimensions used to specify the
 * global work-items and work-items in the work-group.
 * @param[in] global_work_offset Can be used to specify an array of
 * `work_dim` unsigned values that describe the offset used to calculate
 * the global ID of a work-item.
 * @param[in] global_work_size An array of `work_dim` unsigned values
 * that describe the number of global work-items in `work_dim`
 * dimensions that will execute the kernel function.
 * @param[in] local_work_size An array of `work_dim` unsigned values
 * that describe the number of work-items that make up a work-group that
 * will execute the specified kernel.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @param[in] ... A `NULL`-terminated list of arguments to set.
 * @return Event wrapper object that identifies this command.
 * */
CCL_EXPORT
CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange(CCLKernel* krnl, CCLQueue* cq,
	cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, GError** err, ...) {

	/* Make sure krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Event wrapper. */
	CCLEvent* evt;
	/* The va_list, which represents the variable argument list. */
	va_list args_va;
	/* Array of arguments, to be created from the va_list. */
	void** args_array = NULL;
	/* Number of arguments. */
	guint num_args = 0;
	/* Aux. arg. when cycling through the va_list. */
	void* aux_arg;

	/* Initialize the va_list. */
	va_start(args_va, err);

	/* Get first argument. */
	aux_arg = va_arg(args_va, void*);

	/* Check if any arguments are given, and if so, populate array
	 * of arguments. */
	if (aux_arg != NULL) {

		/* 1. Determine number of arguments. */

		while (aux_arg != NULL) {
			num_args++;
			aux_arg = va_arg(args_va, void*);
		}
		va_end(args_va);

		/* 2. Populate array of arguments. */

		args_array = g_slice_alloc((num_args + 1) * sizeof(void*));
		va_start(args_va, err);

		for (guint i = 0; i < num_args; ++i) {
			aux_arg = va_arg(args_va, void*);
			args_array[i] = aux_arg;
		}
		va_end(args_va);
		args_array[num_args] = NULL;

	}

	/* Set kernel arguments and run it. */
	evt = ccl_kernel_set_args_and_enqueue_ndrange_v(krnl, cq, work_dim,
		global_work_offset, global_work_size, local_work_size,
		evt_wait_lst, args_array, err);

	/* If any arguments are given... */
	if (num_args > 0) {

		/* Free the array of arguments. */
		g_slice_free1((num_args + 1) * sizeof(void*), args_array);

	}

	/* Return event wrapper. */
	return evt;

}

/**
 * Set kernel arguments and enqueue it for execution on a device.
 *
 * Internally this function sets kernel arguments by calling
 * ::ccl_kernel_set_args_v(), and enqueues the kernel for execution
 * by calling ::ccl_kernel_enqueue_ndrange().
 *
 * The ::ccl_kernel_set_args_and_enqueue_ndrange() function performs the
 * same operation but accepts a `NULL`-terminated variable list of
 * arguments instead.
 *
 * @public @memberof ccl_kernel
 *
 * @warning This function is not thread-safe. For multi-threaded
 * access to the same kernel function, create multiple instances of
 * a kernel wrapper for the given kernel function with ccl_kernel_new(),
 * one for each thread.
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[in] cq A command queue wrapper object.
 * @param[in] work_dim The number of dimensions used to specify the
 * global work-items and work-items in the work-group.
 * @param[in] global_work_offset Can be used to specify an array of
 * `work_dim` unsigned values that describe the offset used to calculate
 * the global ID of a work-item.
 * @param[in] global_work_size An array of `work_dim` unsigned values
 * that describe the number of global work-items in `work_dim`
 * dimensions that will execute the kernel function.
 * @param[in] local_work_size An array of `work_dim` unsigned values
 * that describe the number of work-items that make up a work-group that
 * will execute the specified kernel.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[in] args A `NULL`-terminated list of arguments to set.
 * Arguments must be of type ::CCLArg*, ::CCLBuffer*, ::CCLImage* or
 * ::CCLSampler*.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this command.
 * */
CCL_EXPORT
CCLEvent* ccl_kernel_set_args_and_enqueue_ndrange_v(CCLKernel* krnl,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size,
	CCLEventWaitList* evt_wait_lst, void** args, GError** err) {

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
	evt = ccl_kernel_enqueue_ndrange(krnl, cq, work_dim, global_work_offset,
		global_work_size, local_work_size, evt_wait_lst, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

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
 * Get the OpenCL version of the platform associated with this kernel.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl A kernel wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL version of the platform associated with this
 * kernel as an integer. If an error occurs, 0 is returned.
 * */
CCL_EXPORT
cl_uint ccl_kernel_get_opencl_version(CCLKernel* krnl, GError** err) {

	/* Make sure number krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, 0.0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0.0);

	cl_context context;
	CCLContext* ctx;
	GError* err_internal = NULL;
	cl_uint ocl_ver;

	/* Get cl_context object for this kernel. */
	context = ccl_kernel_get_info_scalar(
		krnl, CL_KERNEL_CONTEXT, cl_context, &err_internal);
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
 * Suggest appropriate global and local worksizes for the given real
 * work size, based on device and kernel characteristics.
 *
 * The returned global worksize may be larger than the real work size
 * in order to better fit the kernel preferred multiple worksize. As
 * such, kernels enqueued with worksizes given by this function should
 * check if their global ID is within `real_worksize`.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl Kernel wrapper object.
 * @param[in] dev Device wrapper object.
 * @param[in] dims The number of dimensions used to specify the global
 * work-items and work-items in the work-group.
 * @param[in] real_worksize The real worksize.
 * @param[out] gws A "nice" global worksize for the given kernel and
 * device, which must be equal or larger than the `real_worksize` and a
 * multiple of `lws`. This memory location should be pre-allocated with
 * space for `dims` size_t values.
 * @param[out] lws A "nice" local worksize, which is based and respects
 * the limits of the given kernel and device. This memory location
 * should be pre-allocated with space for `dims` size_t values.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if function returns successfully, `CL_FALSE`
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_kernel_suggest_worksizes(CCLKernel* krnl, CCLDevice* dev,
	cl_uint dims, size_t* real_worksize, size_t* gws, size_t* lws,
	GError** err) {

	/* The preferred workgroup size. */
	size_t wg_size_mult = 0;
	size_t wg_size_max;
	size_t wg_size = 1;
	size_t* max_wi_sizes;
	cl_uint dev_dims;
	cl_bool ret_status;
	size_t real_ws = 1;

	/* Error handling object. */
	GError* err_internal = NULL;

	/* Check if device supports the requested dims. */
	dev_dims = ccl_device_get_info_scalar(
		dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	ccl_if_err_create_goto(*err, CCL_ERROR, dims > dev_dims,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: device only supports a max. of %d dimension, but %d where requested.",
		G_STRLOC, dev_dims, dims);

	/* Determine maximum workgroup size. */
	wg_size_max = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_WORK_GROUP_SIZE, size_t, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

#ifdef CL_VERSION_1_1
	/* Determine preferred workgroup size multiple (OpenCL >= 1.1). */
	cl_uint ocl_ver = ccl_kernel_get_opencl_version(krnl, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	if (ocl_ver >= 110) {
		wg_size_mult = ccl_kernel_get_workgroup_info_scalar(
			krnl, dev, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
			size_t, &err_internal);
	} else {
		wg_size_mult = wg_size_max;
	}
#else
	wg_size_mult = wg_size_max;
#endif

	/* Get max. work item sizes for device. */
	max_wi_sizes = ccl_device_get_info_array(
		dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, size_t*, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Try to find an appropriate local worksize. */
	for (cl_uint i = 0; i < dims; ++i) {
		/* Each lws component is at most the preferred workgroup
		 * multiple or the maximum size of that component in device. */
		lws[i] = MIN(wg_size_mult, max_wi_sizes[i]);
		/* Update total workgroup size. */
		wg_size *= lws[i];
		/* Update total real worksize. */
		real_ws *= real_worksize[i];
	}

	/* Don't let each component of the local worksize to be
	 * higher than the respective component of the real
	 * worksize. */
	for (cl_uint i = 0; i < dims; ++i) {
		while (lws[i] > real_worksize[i]) {
			lws[i] /= 2;
			wg_size /= 2;
		}
	}

	/* The total workgroup size can't be higher than the maximum
	 * supported by the device. */
	while (wg_size > wg_size_max) {
		for (int i = dims - 1; i >= 0; --i) {
			lws[i] /= 2;
			wg_size /= 2;
			if (wg_size <= wg_size_max) break;
		}
	}

	/* Now get a global worksize which is a multiple of the local
	 * worksize and is big enough to handle the real worksize. */
	for (cl_uint i = 0; i < dims; ++i) {
		gws[i] = ((real_worksize[i] / lws[i])
			+ (((real_worksize[i] % lws[i]) > 0) ? 1 : 0))
			* lws[i];
	}

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

#ifdef CL_VERSION_1_2

/**
 * Kernel argument information adapter between a
 * ccl_wrapper_info_fp() function and the clGetKernelArgInfo()
 * function.
 *
 * @private @memberof ccl_kernel
 * @see ccl_wrapper_info_fp()
 *
 * @param[in] kernel The kernel wrapper object.
 * @param[in] ptr_arg_indx The kernel argument index, stuffed in a
 * pointer.
 * @param[in] param_name Name of information/parameter to get.
 * @param[in] param_value_size Size in bytes of memory pointed to by p
 * aram_value.
 * @param[out] param_value A pointer to memory where the appropriate
 * result being queried is returned.
 * @param[out] param_value_size_ret Returns the actual size in bytes of
 * data copied to param_value.
 * @return CL_SUCCESS if the function is executed successfully, or an
 * OpenCL error code otherwise.
 * */
static cl_int ccl_kernel_get_arg_info_adapter(cl_kernel kernel,
	void* ptr_arg_indx, cl_kernel_arg_info param_name,
	size_t param_value_size, void *param_value,
	size_t* param_value_size_ret) {

	return clGetKernelArgInfo(kernel, GPOINTER_TO_UINT(ptr_arg_indx),
		param_name, param_value_size, param_value, param_value_size_ret);
}

/**
 * Get a ::CCLWrapperInfo kernel argument information object.
 *
 * @public @memberof ccl_kernel
 * @see ccl_wrapper_get_info()
 * @note Requires OpenCL >= 1.2
 *
 * @param[in] krnl The kernel wrapper object.
 * @param[in] idx Argument index.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested kernel argument information object. This
 * object will be automatically freed when the kernel wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
CCL_EXPORT
CCLWrapperInfo* ccl_kernel_get_arg_info(CCLKernel* krnl, cl_uint idx,
	cl_kernel_arg_info param_name, GError** err) {

	CCLWrapper fake_wrapper;
	CCLWrapperInfo* info;
	GError* err_internal = NULL;
	double ocl_ver;

	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_kernel_get_opencl_version(krnl, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: information about kernel arguments requires OpenCL" \
		" version 1.2 or newer.", G_STRLOC);

	/* Wrap argument index in a fake cl_object. */
	fake_wrapper.cl_object = GUINT_TO_POINTER(idx);

	/* Get kernel argument info. */
	info = ccl_wrapper_get_info(
		(CCLWrapper*) krnl, &fake_wrapper, param_name, 0,
		(ccl_wrapper_info_fp) ccl_kernel_get_arg_info_adapter,
		CL_FALSE, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	info = NULL;

finish:

	/* Return argument info. */
	return info;

}

#endif /* OpenCL >=1.2 */

/** @} */

