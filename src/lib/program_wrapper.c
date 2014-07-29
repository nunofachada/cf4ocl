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
 * @brief OpenCL program wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "program_wrapper.h"

/**
 * @brief Program wrapper object.
 */
struct ccl_program {

	/** Parent wrapper object. */
	CCLDevContainer base;

	/** Program binaries. */
	GHashTable* binaries;
	
	/** Program kernels. */
	GHashTable* krnls;
	
};

struct ccl_program_binary {
	
	/** Binary data. */
	unsigned char* data;
	
	/** Size of binary data. */
	size_t size;
};

/**
 * @brief Implementation of ccl_wrapper_release_fields() function for
 * ::CCLProgram wrapper objects.
 * 
 * @param prg A ::CCLProgram wrapper object.
 * */
static void ccl_program_release_fields(CCLProgram* prg) {

	/* Make sure prg wrapper object is not NULL. */
	g_return_if_fail(prg != NULL);
	
	/* Release devices. */
	ccl_dev_container_release_devices((CCLDevContainer*) prg);

	/* If the kernels table was created...*/
	if (prg->krnls != NULL) {
		
		/* ...free the kernel table and reduce reference count of 
		 * kernels in table (this is done automatically by the
		 * ccl_kernel_destroy() function passed as a destructor
		 * parameter during table creation). */
		g_hash_table_destroy(prg->krnls);
		
	}
		
	/* If the binaries table was created... */
	if (prg->binaries != NULL) {
		
		/*...free it and the included binaries. */
		g_hash_table_destroy(prg->binaries);
		
	}

}

/** 
 * @addtogroup PROGRAM_WRAPPER
 * @{
 */

/**
 * @brief Get the program wrapper for the given OpenCL program.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL program directly (using the
 * clCreateProgramWith*() functions) and then wrap the OpenCL program
 * in a ::CCLProgram wrapper object.
 * 
 * @param program The OpenCL program to be wrapped.
 * @return The ::CCLProgram wrapper for the given OpenCL program.
 * */
CCLProgram* ccl_program_new_wrap(cl_program program) {
	
	return (CCLProgram*) ccl_wrapper_new(
		(void*) program, sizeof(CCLProgram));
		
}

/** 
 * @brief Decrements the reference count of the program wrapper object. 
 * If it reaches 0, the program wrapper object is destroyed.
 *
 * @param prg The program wrapper object.
 * */
void ccl_program_destroy(CCLProgram* prg) {
	
	ccl_wrapper_unref((CCLWrapper*) prg, sizeof(CCLProgram),
		(ccl_wrapper_release_fields) ccl_program_release_fields, 
		(ccl_wrapper_release_cl_object) clReleaseProgram, NULL); 

}

CCLProgram* ccl_program_new_from_source_file(CCLContext* ctx, 
	const char* filename, GError** err) {

	return ccl_program_new_from_source_files(ctx, 1, &(filename), err);
	
}

CCLProgram* ccl_program_new_from_source_files(CCLContext* ctx, 
	cl_uint count, const char** filenames, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure filenames is not NULL. */
	g_return_val_if_fail(filenames != NULL, NULL);
	/* Make sure count > 0. */
	g_return_val_if_fail(count > 0, NULL);

	/* Error reporting object. */
	GError* err_internal = NULL;
	
	CCLProgram* prg = NULL;
	
	gchar** strings = NULL;
	
	strings = g_slice_alloc0(count * sizeof(gchar*));
	
	for (cl_uint i = 0; i < count; ++i) {

		g_file_get_contents(
			filenames[i], &strings[i], NULL, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	}
	
	prg = ccl_program_new_with_source(
		ccl_context_unwrap(ctx), count, (const char**)strings, NULL, 
		&err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:

	if (strings != NULL) {
		for (cl_uint i = 0; i < count; ++i) {
			if (strings[i] != NULL) {
				g_free(strings[i]);
			}
		}
		g_slice_free1(count * sizeof(gchar*), strings);
	}
		
	/* Return prg. */
	return prg;	

}

CCLProgram* ccl_program_new_with_source(cl_context context,
	cl_uint count, const char **strings, const size_t *lengths,
	GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	
	cl_program program = NULL;
	
	CCLProgram* prg = NULL;
		
	program = clCreateProgramWithSource(
		context, count, strings, lengths, &ocl_status);
	gef_if_error_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program with source (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	prg = ccl_program_new_wrap(program);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Return prg. */
	return prg;

}

CCLProgram* ccl_program_new_from_binary_file(CCLContext* ctx, 
	CCLDevice* dev, const char* filename, GError** err) {
	
	return ccl_program_new_from_binary_files(
		ctx, 1, &(dev), &(filename), err);
}

CCLProgram* ccl_program_new_from_binary_files(CCLContext* ctx, 
	cl_uint num_devices, CCLDevice** devs, const char** filenames, 
	GError** err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure devs is not NULL. */
	g_return_val_if_fail(devs != NULL, NULL);
	/* Make sure filenames is not NULL. */
	g_return_val_if_fail(filenames != NULL, NULL);
	/* Make sure num_devices > 0. */
	g_return_val_if_fail(num_devices > 0, NULL);
	
	GError* err_internal = NULL;
	CCLProgramBinary** bins = NULL;
	CCLProgram* prg = NULL;
	
	/* Open files and create binaries. */
	bins = g_slice_alloc0(num_devices * sizeof(CCLProgramBinary*));
	for (cl_uint i = 0; i < num_devices; ++i) {
		bins[i] = ccl_program_binary_new_empty();
		g_file_get_contents(filenames[i], (char**) &bins[i]->data, 
			&bins[i]->size, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}

	/* Create program. */
	prg = ccl_program_new_from_binaries(
		ctx, num_devices, devs, bins, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:

	/* Free stuff if necessary. */
	if (bins != NULL) {
		for (cl_uint i = 0; i < num_devices; ++i) {
			if (bins[i] != NULL) {
				ccl_program_binary_destroy(bins[i]);
			}
		}
		g_slice_free1(num_devices * sizeof(CCLProgramBinary*), bins);
	}
	
	/* Return prg. */
	return prg;
	
}

CCLProgram* ccl_program_new_from_binaries(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, CCLProgramBinary** bins,
	GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure devs is not NULL. */
	g_return_val_if_fail(devs != NULL, NULL);
	/* Make sure num_devices > 0. */
	g_return_val_if_fail(num_devices > 0, NULL);
	
	CCLProgram* prg = NULL;
	cl_device_id* device_list = NULL;
	size_t* lengths = NULL;
	unsigned char** bins_raw = NULL;
	GError* err_internal = NULL;
		
	/* Unwrap devices, binaries and lengths. */
	device_list = g_slice_alloc(num_devices * sizeof(cl_device_id));
	lengths = g_slice_alloc(num_devices * sizeof(size_t));
	bins_raw = g_slice_alloc(num_devices * sizeof(unsigned char*));
	for (guint i = 0; i < num_devices; ++i) {
		device_list[i] = ccl_device_unwrap(devs[i]);
		lengths[i] = bins[i]->size;
		bins_raw[i] = (unsigned char*) bins[i]->data;
	}
	
	/* Create program. */
	prg = ccl_program_new_with_binary(ccl_context_unwrap(ctx), 
		num_devices, device_list, lengths, 
		(const unsigned char**) bins_raw, NULL, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:

	/* Free stuff if necessary. */
	if (device_list != NULL)
		g_slice_free1(num_devices * sizeof(cl_device_id), device_list);
	if (lengths != NULL)
		g_slice_free1(num_devices * sizeof(size_t), lengths);
	if (bins_raw != NULL)
		g_slice_free1(num_devices * sizeof(unsigned char*), bins_raw);
	
	/* Return prg. */
	return prg;

}
	
CCLProgram* ccl_program_new_with_binary(cl_context context,
	cl_uint num_devices, const cl_device_id* device_list,
	const size_t *lengths, const unsigned char **binaries,
	cl_int *binary_status, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;
	cl_program program = NULL;
	CCLProgram* prg = NULL;
		
	/* Create program. */
	program = clCreateProgramWithBinary(context, 
		num_devices, device_list, lengths, binaries, binary_status, 
		&ocl_status);
	gef_if_error_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program from binaries (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:
	
	/* Return prg. */
	return prg;	


}

#ifdef CL_VERSION_1_2

CCLProgram* ccl_program_new_with_built_in_kernels(cl_context context,
	cl_uint num_devices, const cl_device_id *device_list,
	const char *kernel_names, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;
	cl_program program = NULL;
	CCLProgram* prg = NULL;

	/* Create program. */
	program = clCreateProgramWithBuiltInKernels(
		context, num_devices, device_list, kernel_names, &ocl_status);

	gef_if_error_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program from built-in kernels (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:
	
	/* Return prg. */
	return prg;	

}

#endif

cl_bool ccl_program_build_from_devices_full(CCLProgram* prg, 
	cl_uint num_devices, CCLDevice** devices, const char *options, 
	ccl_program_callback pfn_notify, void *user_data, GError** err) {
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);
	/* Make sure devices and num_devices are coherent. */
	g_return_val_if_fail(((num_devices == 0) && (devices == NULL)) 
		|| ((num_devices > 0) && (devices != NULL)), FALSE);

	/* Array of unwrapped devices. */
	cl_device_id* cl_devices = NULL;
	
	/* Result of function call. */
	gboolean result;
	
	/* Check if its necessary to unwrap devices. */
	if (devices != NULL) {
		cl_devices = g_slice_alloc0(sizeof(cl_device_id) * num_devices);
		/* Unwrap devices. */
		for (guint i = 0; i < num_devices; i++)
			cl_devices[i] = ccl_device_unwrap(devices[i]);
	}

	/* Build the program. */
	result = ccl_program_build_from_cldevices_full(
		prg, num_devices, cl_devices, options, pfn_notify, 
		user_data, err);
	
	/* Check if necessary to release array of unwrapped devices. */
	if (cl_devices != NULL) {
		g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);
	}
	
	/* Return result of function call. */
	return result;

}

cl_bool ccl_program_build_from_cldevices_full(CCLProgram* prg, 
	cl_uint num_devices, cl_device_id* device_list, const char *options, 
	ccl_program_callback pfn_notify, void *user_data, GError** err) {
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure devices and num_devices are coherent. */
	g_return_val_if_fail(((num_devices == 0) && (device_list == NULL)) 
		|| ((num_devices > 0) && (device_list != NULL)), FALSE);

	/* Status of OpenCL function call. */
	cl_int ocl_status;

	/* Result of function call. */
	gboolean result;

	/* Build program. */
	ocl_status = clBuildProgram(ccl_program_unwrap(prg),
		num_devices, device_list, options, pfn_notify, user_data);
	gef_if_error_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to build program (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, ccl_err(ocl_status));
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	result = CL_TRUE;
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* Bad result. */
	result = CL_FALSE;
	
finish:

	/* Return result of function call. */
	return result;

}

CCLKernel* ccl_program_get_kernel(
	CCLProgram* prg, const char* kernel_name, GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure kernel_name is not NULL. */
	g_return_val_if_fail(kernel_name != NULL, NULL);

	/* Internal error reporting object. */
	GError* err_internal = NULL;
	
	/* Kernel wrapper object. */
	CCLKernel* krnl = NULL;

	/* If kernels table is not yet initialized, then
	 * initialize it. */
	if (prg->krnls == NULL) {
		prg->krnls = g_hash_table_new_full(g_str_hash, g_str_equal,
			NULL, (GDestroyNotify) ccl_kernel_destroy);
	}
	
	/* Check if requested kernel is already present in the kernels 
	 * table. */
	if (g_hash_table_contains(
		prg->krnls, kernel_name)) {
		
		/* If so, retrieve it from there. */
		krnl = g_hash_table_lookup(
			prg->krnls, kernel_name);
		
	} else {
		
		/* Otherwise, get it from OpenCL program object.*/
		krnl = ccl_kernel_new(prg, kernel_name, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);	
		
		/* Keep new kernel wrapper in table. */
		g_hash_table_insert(prg->krnls, (gpointer) kernel_name, krnl);
		
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Return kernel wrapper. */
	return krnl;
		
}

CCLEvent* ccl_program_run(CCLProgram* prg, const char* kernel_name,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, ...) {

	CCLEvent* evt;
	va_list args;
	va_start(args, err);
	evt = ccl_program_run_v(prg, kernel_name, cq, work_dim, 
		global_work_offset, global_work_size, local_work_size, 
		evt_wait_lst, err, args);
	va_end(args);
	return evt;

}

CCLEvent* ccl_program_run_v(CCLProgram* prg, const char* kernel_name,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, va_list args) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);

	CCLEvent* evt;
	CCLKernel* krnl;
	
	krnl = ccl_program_get_kernel(prg, kernel_name, err);
	if (krnl == NULL) return NULL;
	
	evt = ccl_kernel_set_args_and_run_v(krnl, cq, work_dim, 
		global_work_offset, global_work_size, local_work_size, 
		evt_wait_lst, err, args);
	return evt;
}

static void ccl_program_load_binaries(CCLProgram* prg, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);

	/* Make sure binaries table is initialized. */
	g_return_val_if_fail(prg->binaries != NULL, NULL);

	cl_uint num_devices;
	cl_device_id* devices;
	size_t* binary_sizes;
	CCLWrapperInfo* info;
	unsigned char** bins_raw;
	GError* err_internal = NULL;
	cl_int ocl_status;
	
	/* Get number of program devices. */
	info = ccl_program_get_info(prg, CL_PROGRAM_NUM_DEVICES, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);	
	num_devices = *((cl_uint*) info->value);
	
	/* Get program devices. */
	info = ccl_program_get_info(prg, CL_PROGRAM_DEVICES, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	devices = (cl_device_id*) info->value;
		
	/* Get binary sizes. */
	info = ccl_program_get_info(prg, CL_PROGRAM_BINARY_SIZES, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	binary_sizes = (size_t*) info->value;

	/* Allocate memory for binaries. */
	bins_raw = g_slice_alloc0(num_devices * sizeof(unsigned char*));
	for (guint i = 0; i < num_devices; i++) {
		if (binary_sizes[i] > 0) {
			bins_raw[i] = g_malloc(binary_sizes[i]);
		}
	}

	/* Get binaries. */
	ocl_status = clGetProgramInfo(ccl_program_unwrap(prg),
		CL_PROGRAM_BINARIES, num_devices * sizeof(unsigned char*),
		bins_raw, NULL);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler,
		"%s: unable to get binaries from program.",
		G_STRLOC);

	/* Fill binaries table, associating each device with a 
	 * CCLProgramBinary* object containing the binary and its size. */
	for (guint i = 0; i < num_devices; ++i) {
		
		CCLProgramBinary* bin = ccl_program_binary_new_empty();
		bin->size = binary_sizes[i];
		bin->data = bins_raw[i];
		
		g_hash_table_replace(prg->binaries, devices[i], bin);
	}

	/* Free memory allocated for binary array. */
	g_slice_free1(num_devices * sizeof(unsigned char*), bins_raw);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	return;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	return;

}

CCLProgramBinary* ccl_program_get_binary(CCLProgram* prg, CCLDevice* dev,
	GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, NULL);
	
	GError* err_internal = NULL;
	
	CCLProgramBinary* binary = NULL;
	
	/* Check if binaries table is initialized. */
	if (prg->binaries == NULL) {
		
		/* Initialize binaries table. */
		prg->binaries = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, 
			(GDestroyNotify) ccl_program_binary_destroy);
		
		/* Load binaries. */
		ccl_program_load_binaries(prg, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}
	
	/* Check if given device exists in the list of program devices. */
	if (g_hash_table_contains(prg->binaries, ccl_device_unwrap(dev))) {
		
		/* It exists, get it. */
		binary = g_hash_table_lookup(
			prg->binaries, ccl_device_unwrap(dev));
		
		/* If NULL, then perform a new binary fetch on the CL program 
		 * object... */
		ccl_program_load_binaries(prg, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		/* ...and get it again. If it's NULL it's because binary isn't
		 * compiled for given device. */
		binary = g_hash_table_lookup(
			prg->binaries, ccl_device_unwrap(dev));

	} else {
		
		/* Device does not exist in list of program devices. */
		gef_if_error_create_goto(*err, CCL_ERROR, TRUE, CCL_ERROR_OCL, 
			error_handler,
			"%s: device is not part of program devices.",
			G_STRLOC);
	}
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
finish:

	/* Return kernel wrapper. */
	return binary;	
}

cl_bool ccl_program_save_binary(CCLProgram* prg, CCLDevice* dev,
	const char* filename, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure filename is not NULL. */
	g_return_val_if_fail(filename != NULL, NULL);
	
	GError* err_internal = NULL;
	
	CCLProgramBinary* binary = NULL;

	binary = ccl_program_get_binary(prg, dev, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	gef_if_error_create_goto(*err, CCL_ERROR, binary->size == 0, 
		CCL_ERROR_OCL, error_handler,
		"%s: binary for given device has size 0.",
		G_STRLOC);

	g_file_set_contents(filename, (const gchar*) binary->data,
		binary->size, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	cl_bool status = CL_TRUE;
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	status = CL_FALSE;

finish:

	/* Return function status. */
	return status;			
}

cl_bool ccl_program_save_all_binaries(CCLProgram* prg, 
	const char* file_prefix, const char* file_suffix, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure file prefix and suffix are not NULL. */
	g_return_val_if_fail(
		(file_prefix != NULL) && (file_suffix != NULL), NULL);
	
	GError* err_internal = NULL;
	guint num_devices;
	cl_bool status;
	
	/* Save binaries, one per device. */
	num_devices = ccl_program_get_num_devices(prg, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	for (guint i = 0; i < num_devices; ++i) {
		
		CCLDevice* dev = NULL;
		CCLWrapperInfo* file_middle = NULL;
		gchar* filename;
		
		dev = ccl_program_get_device(prg, i, &err_internal);	
		gef_if_err_propagate_goto(err, err_internal, error_handler);

		file_middle = ccl_device_get_info(dev, CL_DEVICE_NAME, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		filename = g_strdup_printf("%s%s_%2d%s", 
			file_prefix, (gchar*) file_middle->value, i, file_suffix);
		
		g_strcanon(filename, CCL_COMMON_VALIDFILECHARS, '_');
		
		ccl_program_save_binary(prg, dev, filename, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		g_free(filename);
	}


	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	status = CL_FALSE;

finish:

	/* Return function status. */
	return status;			


}

/** @}*/

CCLProgramBinary* ccl_program_binary_new(
	unsigned char* data, size_t size) {
		
	CCLProgramBinary* pbin = g_slice_new(CCLProgramBinary);
	
	pbin->data = data;
	pbin->size = size;
	
	return pbin;
		
}

void ccl_program_binary_destroy(CCLProgramBinary* pbin) {

	/* Make sure pbin is not NULL. */
	g_return_if_fail(pbin != NULL);

	if (pbin->size > 0)
		g_free(pbin->data);
	g_slice_free(CCLProgramBinary, pbin);

}

/** 
 * @brief Implementation of ccl_dev_container_get_cldevices() for the
 * program wrapper. 
 * 
 * @param devcon A ::CCLProgram wrapper, passed as a ::CCLDevContainer .
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A list of cl_device_id objects inside a ::CCLWrapperInfo
 * object.
 * */
CCLWrapperInfo* ccl_program_get_cldevices(
	CCLDevContainer* devcon, GError** err) {

	return ccl_program_get_info(devcon, CL_PROGRAM_DEVICES, err);
}

