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

#include "ccl_kernel_wrapper.h"
#include "ccl_program_wrapper.h"
#include "_ccl_abstract_wrapper.h"

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
 * @addtogroup CCL_KERNEL_WRAPPER
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
		CCL_KERNEL, (void*) kernel, sizeof(CCLKernel));

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
		CCL_STRD, ocl_status, ccl_err(ocl_status));

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
 * arguments which must end with `NULL`. Internally, this method
 * sets each argument individually using the ::ccl_kernel_set_arg()
 * function.
 *
 * If the ::ccl_arg_skip constant is passed in place of a specific
 * argument, that argument will not be set by this function call. Any
 * previously set argument continues to be valid.
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
 * If the ::ccl_arg_skip constant is passed in place of a specific
 * argument, that argument will not be set by this function call. Any
 * previously set argument continues to be valid.
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

		/* Ignore "skip" arguments. */
		if (arg == ccl_arg_skip) continue;

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

	/* Make sure krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure cq is not NULL. */
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
				CCL_STRD, arg_index, ocl_status, ccl_err(ocl_status));
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
 * If the ::ccl_arg_skip constant is passed in place of a specific
 * argument, that argument will not be set by this function call. Any
 * previously set argument continues to be valid.
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
 * If the ::ccl_arg_skip constant is passed in place of a specific
 * argument, that argument will not be set by this function call. Any
 * previously set argument continues to be valid.
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

	/* Make sure krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, NULL);
	/* Make sure cq is not NULL. */
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
 * Enqueues a command to execute a native C/C++ function not compiled
 * using the OpenCL compiler. This function is a wrapper for the
 * clEnqueueNativeKernel() OpenCL function, the documentation of which
 * provides additional information.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] cq A command queue wrapper object.
 * @param[in] user_func A pointer to a host-callable user function.
 * @param[in] args A pointer to the args list that `user_func` should be
 * called with.
 * @param[in] cb_args The size in bytes of the args list that args
 * points to.
 * @param[in] num_mos The number of ::CCLMemObj* objects that are passed
 * in `mo_list`.
 * @param[in] mo_list A list of ::CCLMemObj* objects (or `NULL`
 * references), if num_mos > 0.
 * @param[in] args_mem_loc A pointer to appropriate locations that
 * `args` points to where `cl_mem` values (unwrapped from the respective
 * ::CCLMemObj* objects) are stored. Before the user function is
 * executed, the `cl_mem` values are replaced by pointers to global
 * memory.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this command.
 * */
CCL_EXPORT
CCLEvent* ccl_kernel_enqueue_native(CCLQueue* cq,
	void (CL_CALLBACK * user_func)(void*), void* args, size_t cb_args,
	cl_uint num_mos, CCLMemObj* const* mo_list,
	const void** args_mem_loc, CCLEventWaitList* evt_wait_lst,
	GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure that num_mos == 0 AND mo_list != NULL, OR, that
	 * num_mos > 0  AND mo_list != NULL */
	g_return_val_if_fail(((num_mos == 0) && (mo_list == NULL))
		|| ((num_mos > 0) && (mo_list != NULL)), NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL status flag. */
	cl_int ocl_status;
	/* OpenCL event. */
	cl_event event = NULL;
	/* Event wrapper. */
	CCLEvent* evt = NULL;
	/* List of cl_mem objects. */
	cl_mem* mem_list = NULL;

	/* Unwrap memory objects. */
	if (num_mos > 0) {
		mem_list = g_slice_alloc(sizeof(cl_mem) * num_mos);
		for (cl_uint i = 0; i < num_mos; ++i) {
			mem_list[i] = mo_list[i] != NULL
				? ccl_memobj_unwrap(mo_list[i])
				: NULL;
		}
	}

	/* Enqueue kernel. */
	ocl_status = clEnqueueNativeKernel(ccl_queue_unwrap(cq), user_func,
		args, cb_args, num_mos, (const cl_mem*) mem_list, args_mem_loc,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to enqueue native kernel (OpenCL error %d: %s).",
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

finish:

	/* Release temporary cl_mem list. */
	if (num_mos > 0)
		g_slice_free1(sizeof(cl_mem) * num_mos, mem_list);

	/* Return event wrapper. */
	return evt;

}

/**
 * Get the OpenCL version of the platform associated with this kernel.
 * The version is returned as an integer, in the following format:
 *
 * * 100 for OpenCL 1.0
 * * 110 for OpenCL 1.1
 * * 120 for OpenCL 1.2
 * * 200 for OpenCL 2.0
 * * etc.
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

	/* Make sure krnl is not NULL. */
	g_return_val_if_fail(krnl != NULL, 0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0);

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
 * Suggest appropriate local (and optionally global) work sizes for the
 * given real work size, based on device and kernel characteristics.
 *
 * If the `gws` parameter is not `NULL`, it will be populated with a
 * global worksize which may be larger than the real work size
 * in order to better fit the kernel preferred multiple work size. As
 * such, kernels enqueued with global work sizes suggested by this
 * function should check if their global ID is within `real_worksize`.
 *
 * @public @memberof ccl_kernel
 *
 * @param[in] krnl Kernel wrapper object. If `NULL`, use only device
 * information for determining global and local worksizes.
 * @param[in] dev Device wrapper object.
 * @param[in] dims The number of dimensions used to specify the global
 * work-items and work-items in the work-group.
 * @param[in] real_worksize The real worksize.
 * @param[out] gws Location where to place a "nice" global worksize for
 * the given kernel and device, which must be equal or larger than the `
 * real_worksize` and a multiple of `lws`. This memory location should
 * be pre-allocated with space for `dims` values of size `size_t`. If
 * `NULL` it is assumed that the global worksize must be equal to
 * `real_worksize`.
 * @param[in,out] lws This memory location, of size
 * `dims * sizeof(size_t)`, serves a dual purpose: 1) as an input,
 * containing the maximum allowed local work size for each dimension, or
 * zeros if these maximums are to be fetched from the given device
 * CL_DEVICE_MAX_WORK_ITEM_SIZES information (if the specified values
 * are larger than the device limits, the device limits are used
 * instead); 2) as an output, where to place a "nice" local worksize,
 * which is based and respects the limits of the given kernel and device
 * (and of the non-zero values given as input).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if function returns successfully, `CL_FALSE`
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_kernel_suggest_worksizes(CCLKernel* krnl, CCLDevice* dev,
	cl_uint dims, const size_t* real_worksize, size_t* gws, size_t* lws,
	GError** err) {

	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, CL_FALSE);
	/* Make sure dims not zero. */
	g_return_val_if_fail(dims > 0, CL_FALSE);
	/* Make sure real_worksize is not NULL. */
	g_return_val_if_fail(real_worksize != NULL, CL_FALSE);
	/* Make sure lws is not NULL. */
	g_return_val_if_fail(lws != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* The preferred workgroup size. */
	size_t wg_size_mult = 0;
	size_t wg_size_max;
	size_t wg_size = 1, wg_size_aux;
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
		"%s: device only supports a max. of %d dimension, but %d were requested.",
		CCL_STRD, dev_dims, dims);

	/* Get max. work item sizes for device. */
	max_wi_sizes = ccl_device_get_info_array(
		dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, size_t*, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* For each dimension, if the user specified a maximum local work
	 * size, the effective maximum local work size will be the minimum
	 * between the user value and the device value. */
	for (cl_uint i = 0; i < dims; ++i) {
		if (lws[i] != 0)
			max_wi_sizes[i] = MIN(max_wi_sizes[i], lws[i]);
	}

	/* If kernel is not NULL, query it about workgroup size preferences
	 * and capabilities. */
	if (krnl != NULL) {
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
	} else {

		/* Kernel is NULL, use values obtained from device. */
		wg_size_max = ccl_device_get_info_scalar(
			dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		wg_size_mult = wg_size_max;
	}

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
		wg_size_aux = wg_size;
		for (int i = dims - 1; i >= 0; --i) {
			if (lws[i] > 1) {
				/* Local work size can't be smaller than 1. */
				lws[i] /= 2;
				wg_size /= 2;
			}
			if (wg_size <= wg_size_max) break;
		}
		/* Avoid infinite loops and throw error if wg_size didn't
		 * change. */
		ccl_if_err_create_goto(*err, CCL_ERROR, wg_size == wg_size_aux,
			CCL_ERROR_OTHER, error_handler,
			"%s: Unable to determine a work size within the device limit (%d).",
			CCL_STRD, (int) wg_size_max);
	}

	/* If output variable gws is not NULL... */
	if (gws != NULL) {
		/* ...find a global worksize which is a multiple of the local
		 * worksize and is big enough to handle the real worksize. */
		for (cl_uint i = 0; i < dims; ++i) {
			gws[i] = ((real_worksize[i] / lws[i])
				+ (((real_worksize[i] % lws[i]) > 0) ? 1 : 0))
				* lws[i];
		}
	} else {
		/* ...otherwise check if found local worksizes are divisors of
		 * the respective real_worksize. If so keep them, otherwise find
		 * local worksizes which respect the maximum sizes allowed by
		 * the kernel and the device, and is a dimension-wise divisor of
		 * the real_worksize. */
		cl_bool lws_are_divisors = CL_TRUE;
		for (cl_uint i = 0; i < dims; ++i) {
			/* Check if lws[i] is divisor of real_worksize[i]. */
			if (real_worksize[i] % lws[i] != 0) {
				/* Ops... lws[i] is not divisor of real_worksize[i], so
				 * we'll have to try and find new lws ahead. */
				lws_are_divisors = CL_FALSE;
				break;
			}
		}
		/* Is lws divisor of real_worksize, dimension-wise? */
		if (!lws_are_divisors) {
			/* No, so we'll have to find new lws. */
			wg_size = 1;
			for (cl_uint i = 0; i < dims; ++i) {

				/* For each dimension, try to use the previously
				 * found lws[i]. */
				if ((real_worksize[i] % lws[i] != 0)
					|| (lws[i] * wg_size > wg_size_max))
				{
					/* Previoulsy found lws[i] not usable, find
					 * new one. Must be a divisor of real_worksize[i]
					 * and respect the kernel and device maximum lws.*/
					cl_uint best_lws_i = 1;
					for (cl_uint j = 2; j <= real_worksize[i] / 2; ++j) {
						/* If current value is higher than the kernel
						 * and device limits, stop searching and use
						 * the best one so far. */
						if ((wg_size * j > wg_size_max)
							|| (j > max_wi_sizes[i])) break;
						/* Otherwise check if current value is divisor
						 * of lws[i]. If so, keep it as the best so
						 * far. */
						if (real_worksize[i] % j == 0)
							best_lws_i = j;
					}
					/* Keep the best divisor for current dimension. */
					lws[i] = best_lws_i;
				}
				/* Update absolute workgroup size (all dimensions). */
				wg_size *= lws[i];
			}
		}
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
		" version 1.2 or newer.", CCL_STRD);

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

