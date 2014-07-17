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

#include "program.h"

/**
 * @brief Program wrapper object.
 */
struct cl4_program {

	/** Parent wrapper object. */
	CL4DevContainer base;

	/** Program binaries. */
	GHashTable* binaries;
	
	/** Program kernels. */
	GHashTable* krnls;
	
};

struct cl4_program_binary {
	
	/** Binary data. */
	unsigned char* data;
	
	/** Size of binary data. */
	size_t size;
};

/**
 * @brief Implementation of cl4_wrapper_release_fields() function for
 * ::CL4Program wrapper objects.
 * 
 * @param prg A ::CL4Program wrapper object.
 * */
static void cl4_program_release_fields(CL4Program* prg) {

	/* Make sure prg wrapper object is not NULL. */
	g_return_if_fail(prg != NULL);
	
	/* Release devices. */
	cl4_dev_container_release_devices((CL4DevContainer*) prg);

	/* If the kernels table was created...*/
	if (prg->krnls != NULL) {
		
		/* ...free the kernel table and reduce reference count of 
		 * kernels in table (this is done automatically by the
		 * cl4_kernel_destroy() function passed as a destructor
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
 * @addtogroup PROGRAM
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
 * in a ::CL4Program wrapper object.
 * 
 * @param program The OpenCL program to be wrapped.
 * @return The ::CL4Program wrapper for the given OpenCL program.
 * */
CL4Program* cl4_program_new_wrap(cl_program program) {
	
	return (CL4Program*) cl4_wrapper_new(
		(void*) program, sizeof(CL4Program));
		
}

/** 
 * @brief Decrements the reference count of the program wrapper object. 
 * If it reaches 0, the program wrapper object is destroyed.
 *
 * @param prg The program wrapper object.
 * */
void cl4_program_destroy(CL4Program* prg) {
	
	cl4_wrapper_unref((CL4Wrapper*) prg, sizeof(CL4Program),
		(cl4_wrapper_release_fields) cl4_program_release_fields, 
		(cl4_wrapper_release_cl_object) clReleaseProgram, NULL); 

}

CL4Program* cl4_program_new_from_source_file(CL4Context* ctx, 
	const char* filename, GError** err) {

	return cl4_program_new_from_source_files(ctx, 1, &(filename), err);
	
}

CL4Program* cl4_program_new_from_source_files(CL4Context* ctx, 
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
	
	CL4Program* prg = NULL;
	
	gchar** strings = NULL;
	
	strings = g_slice_alloc0(count * sizeof(gchar*));
	
	for (cl_uint i = 0; i < count; ++i) {

		g_file_get_contents(
			filenames[i], &strings[i], NULL, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	}
	
	prg = cl4_program_new_with_source(
		cl4_context_unwrap(ctx), count, (const char**)strings, NULL, 
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

CL4Program* cl4_program_new_with_source(cl_context context,
	cl_uint count, const char **strings, const size_t *lengths,
	GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	
	cl_program program = NULL;
	
	CL4Program* prg = NULL;
		
	program = clCreateProgramWithSource(
		context, count, strings, lengths, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program with source (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, cl4_err(ocl_status));
	
	prg = cl4_program_new_wrap(program);
	
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

CL4Program* cl4_program_new_from_binary_file(CL4Context* ctx, 
	CL4Device* dev, const char* filename, GError** err) {
	
	return cl4_program_new_from_binary_files(
		ctx, 1, &(dev), &(filename), err);
}

CL4Program* cl4_program_new_from_binary_files(CL4Context* ctx, 
	cl_uint num_devices, CL4Device** devs, const char** filenames, 
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
	CL4ProgramBinary** bins = NULL;
	CL4Program* prg = NULL;
	
	/* Open files and create binaries. */
	bins = g_slice_alloc0(num_devices * sizeof(CL4ProgramBinary*));
	for (cl_uint i = 0; i < num_devices; ++i) {
		bins[i] = cl4_program_binary_new_empty();
		g_file_get_contents(filenames[i], (char**) &bins[i]->data, 
			&bins[i]->size, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}

	/* Create program. */
	prg = cl4_program_new_from_binaries(
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
				cl4_program_binary_destroy(bins[i]);
			}
		}
		g_slice_free1(num_devices * sizeof(CL4ProgramBinary*), bins);
	}
	
	/* Return prg. */
	return prg;
	
}

CL4Program* cl4_program_new_from_binaries(CL4Context* ctx,
	cl_uint num_devices, CL4Device** devs, CL4ProgramBinary** bins,
	GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure devs is not NULL. */
	g_return_val_if_fail(devs != NULL, NULL);
	/* Make sure num_devices > 0. */
	g_return_val_if_fail(num_devices > 0, NULL);
	
	CL4Program* prg = NULL;
	cl_device_id* device_list = NULL;
	size_t* lengths = NULL;
	unsigned char** bins_raw = NULL;
	GError* err_internal = NULL;
		
	/* Unwrap devices, binaries and lengths. */
	device_list = g_slice_alloc(num_devices * sizeof(cl_device_id));
	lengths = g_slice_alloc(num_devices * sizeof(size_t));
	bins_raw = g_slice_alloc(num_devices * sizeof(unsigned char*));
	for (guint i = 0; i < num_devices; ++i) {
		device_list[i] = cl4_device_unwrap(devs[i]);
		lengths[i] = bins[i]->size;
		bins_raw[i] = (unsigned char*) bins[i]->data;
	}
	
	/* Create program. */
	prg = cl4_program_new_with_binary(cl4_context_unwrap(ctx), 
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
	
CL4Program* cl4_program_new_with_binary(cl_context context,
	cl_uint num_devices, const cl_device_id* device_list,
	const size_t *lengths, const unsigned char **binaries,
	cl_int *binary_status, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;
	cl_program program = NULL;
	CL4Program* prg = NULL;
		
	/* Create program. */
	program = clCreateProgramWithBinary(context, 
		num_devices, device_list, lengths, binaries, binary_status, 
		&ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program from binaries (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, cl4_err(ocl_status));

	prg = cl4_program_new_wrap(program);

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

CL4Program* cl4_program_new_with_built_in_kernels(cl_context context,
	cl_uint num_devices, const cl_device_id *device_list,
	const char *kernel_names, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;
	cl_program program = NULL;
	CL4Program* prg = NULL;

	/* Create program. */
	program = clCreateProgramWithBuiltInKernels(
		context, num_devices, device_list, kernel_names, &ocl_status);

	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: unable to create cl_program from built-in kernels (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, cl4_err(ocl_status));

	prg = cl4_program_new_wrap(program);

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

cl_bool cl4_program_build_from_devices_full(CL4Program* prg, 
	cl_uint num_devices, CL4Device** devices, const char *options, 
	cl4_program_callback pfn_notify, void *user_data, GError** err) {
	
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
			cl_devices[i] = cl4_device_unwrap(devices[i]);
	}

	/* Build the program. */
	result = cl4_program_build_from_cldevices_full(
		prg, num_devices, cl_devices, options, pfn_notify, 
		user_data, err);
	
	/* Check if necessary to release array of unwrapped devices. */
	if (cl_devices != NULL) {
		g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);
	}
	
	/* Return result of function call. */
	return result;

}

cl_bool cl4_program_build_from_cldevices_full(CL4Program* prg, 
	cl_uint num_devices, cl_device_id* device_list, const char *options, 
	cl4_program_callback pfn_notify, void *user_data, GError** err) {
	
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
	ocl_status = clBuildProgram(cl4_program_unwrap(prg),
		num_devices, device_list, options, pfn_notify, user_data);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"%s: unable to build program (OpenCL error %d: %s).", 
		G_STRLOC, ocl_status, cl4_err(ocl_status));
		
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

CL4Kernel* cl4_program_get_kernel(
	CL4Program* prg, const char* kernel_name, GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure kernel_name is not NULL. */
	g_return_val_if_fail(kernel_name != NULL, NULL);

	/* Kernel wrapper object. */
	CL4Kernel* krnl = NULL;

	/* If kernels table is not yet initialized, then
	 * initialize it. */
	if (prg->krnls == NULL) {
		prg->krnls = g_hash_table_new_full(g_str_hash, g_str_equal,
			NULL, (GDestroyNotify) cl4_kernel_destroy);
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
		cl_int ocl_status;
		
		/* The OpenCL kernel object to get. */
		cl_kernel kernel = NULL;
		
		/* Get kernel. */
		kernel = clCreateKernel(
			cl4_program_unwrap(prg), kernel_name, &ocl_status);
		gef_if_error_create_goto(*err, CL4_ERROR,
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler,
			"%s: unable to create kernel (OpenCL error %d: %s).",
			G_STRLOC, ocl_status, cl4_err(ocl_status));
		
		/* Create kernel wrapper. */
		krnl = cl4_kernel_new_wrap(kernel);
		
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

static void cl4_program_load_binaries(CL4Program* prg, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);

	/* Make sure binaries table is initialized. */
	g_return_val_if_fail(prg->binaries != NULL, NULL);

	cl_uint num_devices;
	cl_device_id* devices;
	size_t* binary_sizes;
	CL4WrapperInfo* info;
	unsigned char** bins_raw;
	GError* err_internal = NULL;
	cl_int ocl_status;
	
	/* Get number of program devices. */
	info = cl4_program_info(prg, CL_PROGRAM_NUM_DEVICES, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);	
	num_devices = *((cl_uint*) info->value);
	
	/* Get program devices. */
	info = cl4_program_info(prg, CL_PROGRAM_DEVICES, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	devices = (cl_device_id*) info->value;
		
	/* Get binary sizes. */
	info = cl4_program_info(prg, CL_PROGRAM_BINARY_SIZES, &err_internal);
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
	ocl_status = clGetProgramInfo(cl4_program_unwrap(prg),
		CL_PROGRAM_BINARIES, num_devices * sizeof(unsigned char*),
		bins_raw, NULL);
	gef_if_error_create_goto(*err, CL4_ERROR, 
		CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler,
		"%s: unable to get binaries from program.",
		G_STRLOC);

	/* Fill binaries table, associating each device with a 
	 * CL4ProgramBinary* object containing the binary and its size. */
	for (guint i = 0; i < num_devices; ++i) {
		
		CL4ProgramBinary* bin = cl4_program_binary_new_empty();
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

CL4ProgramBinary* cl4_program_get_binary(CL4Program* prg, CL4Device* dev,
	GError** err) {
		
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, NULL);
	
	GError* err_internal = NULL;
	
	CL4ProgramBinary* binary = NULL;
	
	/* Check if binaries table is initialized. */
	if (prg->binaries == NULL) {
		
		/* Initialize binaries table. */
		prg->binaries = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, 
			(GDestroyNotify) cl4_program_binary_destroy);
		
		/* Load binaries. */
		cl4_program_load_binaries(prg, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
	}
	
	/* Check if given device exists in the list of program devices. */
	if (g_hash_table_contains(prg->binaries, cl4_device_unwrap(dev))) {
		
		/* It exists, get it. */
		binary = g_hash_table_lookup(
			prg->binaries, cl4_device_unwrap(dev));
		
		/* If NULL, then perform a new binary fetch on the CL program 
		 * object... */
		cl4_program_load_binaries(prg, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		/* ...and get it again. If it's NULL it's because binary isn't
		 * compiled for given device. */
		binary = g_hash_table_lookup(
			prg->binaries, cl4_device_unwrap(dev));

	} else {
		
		/* Device does not exist in list of program devices. */
		gef_if_error_create_goto(*err, CL4_ERROR, TRUE, CL4_ERROR_OCL, 
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

cl_bool cl4_program_save_binary(CL4Program* prg, CL4Device* dev,
	const char* filename, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	
	/* Make sure filename is not NULL. */
	g_return_val_if_fail(filename != NULL, NULL);
	
	GError* err_internal = NULL;
	
	CL4ProgramBinary* binary = NULL;

	binary = cl4_program_get_binary(prg, dev, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	gef_if_error_create_goto(*err, CL4_ERROR, binary->size == 0, 
		CL4_ERROR_OCL, error_handler,
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

cl_bool cl4_program_save_all_binaries(CL4Program* prg, 
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
	num_devices = cl4_program_get_num_devices(prg, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	for (guint i = 0; i < num_devices; ++i) {
		
		CL4Device* dev = NULL;
		CL4WrapperInfo* file_middle = NULL;
		gchar* filename;
		
		dev = cl4_program_get_device(prg, i, &err_internal);	
		gef_if_err_propagate_goto(err, err_internal, error_handler);

		file_middle = cl4_device_info(dev, CL_DEVICE_NAME, &err_internal);
		gef_if_err_propagate_goto(err, err_internal, error_handler);
		
		filename = g_strdup_printf("%s%s_%2d%s", 
			file_prefix, (gchar*) file_middle->value, i, file_suffix);
		
		g_strcanon(filename, CL4_COMMON_VALIDFILECHARS, '_');
		
		cl4_program_save_binary(prg, dev, filename, &err_internal);
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

CL4ProgramBinary* cl4_program_binary_new(
	unsigned char* data, size_t size) {
		
	CL4ProgramBinary* pbin = g_slice_new(CL4ProgramBinary);
	
	pbin->data = data;
	pbin->size = size;
	
	return pbin;
		
}

void cl4_program_binary_destroy(CL4ProgramBinary* pbin) {

	/* Make sure pbin is not NULL. */
	g_return_if_fail(pbin != NULL);

	if (pbin->size > 0)
		g_free(pbin->data);
	g_slice_free(CL4ProgramBinary, pbin);

}

/** 
 * @brief Implementation of cl4_dev_container_get_cldevices() for the
 * program wrapper. 
 * 
 * @param devcon A ::CL4Program wrapper, passed as a ::CL4DevContainer .
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return A list of cl_device_id objects inside a ::CL4WrapperInfo
 * object.
 * */
CL4WrapperInfo* cl4_program_get_cldevices(
	CL4DevContainer* devcon, GError** err) {

	return cl4_program_info(devcon, CL_PROGRAM_DEVICES, err);
}

