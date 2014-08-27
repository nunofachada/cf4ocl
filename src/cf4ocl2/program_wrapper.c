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
 * Implementation of a wrapper class and its methods for OpenCL program
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "program_wrapper.h"

/**
 * Program wrapper class.
 *
 * @extends ccl_dev_container
 */
struct ccl_program {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLDevContainer base;

	/**
	 * Program binaries.
	 * @private
	 * */
	GHashTable* binaries;

	/**
	 * Program kernels.
	 * @private
	 * */
	GHashTable* krnls;

};

/**
 * Class which represents a binary object associated with a program
 * and a device
 * */
struct ccl_program_binary {

	/**
	 * Binary data.
	 * @private
	 * */
	unsigned char* data;

	/**
	 * Size of binary data.
	 * @private
	 * */
	size_t size;
};

/**
 * @internal
 * Implementation of ccl_wrapper_release_fields() function for
 * ::CCLProgram wrapper objects.
 *
 * @private @memberof ccl_program
 *
 * @param[in] prg A ::CCLProgram wrapper object.
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
 * @internal
 * Implementation of ccl_dev_container_get_cldevices() for the
 * program wrapper.
 *
 * @private @memberof ccl_program
 *
 * @param[in] devcon A ::CCLProgram wrapper, passed as a ::CCLDevContainer .
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A list of cl_device_id objects inside a ::CCLWrapperInfo
 * object.
 * */
static CCLWrapperInfo* ccl_program_get_cldevices(
	CCLDevContainer* devcon, GError** err) {

	return ccl_program_get_info(devcon, CL_PROGRAM_DEVICES, err);
}

/**
 * @internal
 * Create a new binary object.
 *
 * @private @memberof ccl_program
 *
 * @param[in] data Binary data.
 * @param[in] size Size of binary data.
 * @return A new binary object.
 * */
static CCLProgramBinary* ccl_program_binary_new(
	unsigned char* data, size_t size) {

	CCLProgramBinary* pbin = g_slice_new(CCLProgramBinary);

	pbin->data = data;
	pbin->size = size;

	return pbin;

}

/**
 * @internal
 * Destroy a binary object.
 *
 * @private @memberof ccl_program
 *
 * @param[in] pbin The binary object to destroy.
 * */
static void ccl_program_binary_destroy(CCLProgramBinary* pbin) {

	/* Make sure pbin is not NULL. */
	g_return_if_fail(pbin != NULL);

	if (pbin->size > 0)
		g_free(pbin->data);
	g_slice_free(CCLProgramBinary, pbin);

}

/**
 * @internal
 * Helper macro to create a new empty binary object.
 *
 * @return A new empty binary object.
 * */
#define ccl_program_binary_new_empty() ccl_program_binary_new(NULL, 0)


/**
 * @addtogroup PROGRAM_WRAPPER
 * @{
 */

/**
 * Get the program wrapper for the given OpenCL program.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to create the OpenCL program directly (using the
 * clCreateProgramWith*() functions) and then wrap the OpenCL program
 * in a ::CCLProgram wrapper object.
 *
 * @public @memberof ccl_program
 *
 * @param[in] program The OpenCL program to be wrapped.
 * @return The ::CCLProgram wrapper for the given OpenCL program.
 * */
CCLProgram* ccl_program_new_wrap(cl_program program) {

	return (CCLProgram*) ccl_wrapper_new(
		(void*) program, sizeof(CCLProgram));

}

/**
 * Decrements the reference count of the program wrapper object.
 * If it reaches 0, the program wrapper object is destroyed.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * */
void ccl_program_destroy(CCLProgram* prg) {

	ccl_wrapper_unref((CCLWrapper*) prg, sizeof(CCLProgram),
		(ccl_wrapper_release_fields) ccl_program_release_fields,
		(ccl_wrapper_release_cl_object) clReleaseProgram, NULL);

}

/**
 * Create a new program wrapper object from a source file. This is a
 * utility function which calls the ccl_program_new_from_source_files()
 * function with `count` equal to 1.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] filename Path to source file.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_source_file(CCLContext* ctx,
	const char* filename, GError** err) {

	return ccl_program_new_from_source_files(ctx, 1, &(filename), err);

}

/**
 * Create a new program wrapper object from several source files. This
 * function delegates the actual program creation to the
 * ccl_program_new_from_sources() function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] count Number of source files.
 * @param[in] filenames List of source file paths.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
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
	/* Program wrapper object to return. */
	CCLProgram* prg = NULL;
	/* Source files contents. */
	gchar** strings = NULL;

	/* Allocate space for the specified number of source file
	 * strings. */
	strings = g_slice_alloc0(count * sizeof(gchar*));

	/* Read source files contents. */
	for (cl_uint i = 0; i < count; ++i) {

		g_file_get_contents(
			filenames[i], &strings[i], NULL, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

	}

	/* Create program from sources. */
	prg = ccl_program_new_from_sources(ctx, count,
		(const char**) strings, NULL, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free stuff. */
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

/**
 * Create a new program wrapper object from a null-terminated source
 * string. This is a utility function which calls the
 * ccl_program_new_from_sources() function with `count` equal to 1 and
 * assumes the passed source string is null-terminated.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] string Null-terminated source string.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_source(CCLContext* ctx,
	const char* string, GError** err) {

	return ccl_program_new_from_sources(ctx, 1, &string, NULL, err);
}

/**
 * Create a new program wrapper object from several source code strings.
 * This function directly wraps the clCreateProgramWithSource() OpenCL
 * function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] count Number of source strings.
 * @param[in] strings Source strings.
 * @param[in] lengths An array with the number of chars in each string
 * (the string length). If an element in lengths is zero, its
 * accompanying string is null-terminated. If lengths is `NULL`, all
 * strings in the strings argument are considered null-terminated.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_sources(CCLContext* ctx,
	cl_uint count, const char** strings, const size_t* lengths,
	GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_program program = NULL;
	CCLProgram* prg = NULL;

	/* Build program from sources. */
	program = clCreateProgramWithSource(
		ccl_context_unwrap(ctx), count, strings, lengths, &ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create cl_program with source (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap OpenCL program object. */
	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return prg. */
	return prg;

}

/**
 * Create a new program wrapper object from a file containing binary
 * code executable on a specific device. This is a utility function
 * which calls the ccl_program_new_from_binary_files() function for the
 * specified device only.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] dev Device for which to create the program, and on which
 * the binary code can be executed.
 * @param[in] filename Path to binary file.
 * @param[out] binary_status Returns whether the program binary was
 * loaded successfully or not.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_binary_file(CCLContext* ctx,
	CCLDevice* dev, const char* filename, cl_int *binary_status,
	GError** err) {

	return ccl_program_new_from_binary_files(
		ctx, 1, &(dev), &(filename), binary_status, err);
}

/**
 * Create a new program wrapper object from files containing binary
 * code executable on the given device list, one file per device. This
 * function delegates the actual program creation to the
 * ccl_program_new_from_binaries() function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] num_devices Number of devices (and of files).
 * @param[in] devs List of devices for which to create the program, and
 * on which the binary code loaded from the files can be executed.
 * @param[in] filenames Path to binary files, one per device.
 * @param[out] binary_status Returns whether the program binary for each
 * device specified in `devs` was loaded successfully or not.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_binary_files(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, const char** filenames,
	cl_int *binary_status, GError** err) {

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
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

	/* Create program. */
	prg = ccl_program_new_from_binaries(
		ctx, num_devices, devs, bins, binary_status, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

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

/**
 * Create a new program wrapper object from binary code executable on a
 * specific device. This is a utility function which calls the
 * ccl_program_new_from_binaries() function for the specified device
 * only.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] dev Device for which to create the program, and on which
 * the binary code can be executed.
 * @param[in] binary Binary code.
 * @param[out] binary_status Returns whether the program binary was
 * loaded successfully or not.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_binary(CCLContext* ctx, CCLDevice* dev,
	CCLProgramBinary* binary, cl_int *binary_status, GError** err) {

	return ccl_program_new_from_binaries(ctx, 1, &dev, &binary,
		binary_status, err);
}

/**
 * Create a new program wrapper object from a list of binary code
 * strings executable on the given device list, one binary string per
 * device. This function is a wrapper for the
 * clCreateProgramWithBinary() OpenCL function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] num_devices Number of devices (and of binary strings).
 * @param[in] devs List of devices for which to create the program, and
 * on which the binary code can be executed.
 * @param[in] bins List of binary code strings, one per device.
 * @param[out] binary_status Returns whether the program binary for each
 * device specified in `devs` was loaded successfully or not.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_binaries(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, CCLProgramBinary** bins,
	cl_int *binary_status, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure devs is not NULL. */
	g_return_val_if_fail(devs != NULL, NULL);
	/* Make sure num_devices > 0. */
	g_return_val_if_fail(num_devices > 0, NULL);

	cl_int ocl_status;
	CCLProgram* prg = NULL;
	cl_program program = NULL;
	cl_device_id* device_list = NULL;
	size_t* lengths = NULL;
	unsigned char** bins_raw = NULL;

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
	program = clCreateProgramWithBinary(ccl_context_unwrap(ctx),
		num_devices, device_list, lengths,
		(const unsigned char**) bins_raw,
		binary_status, &ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create cl_program from binaries (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap OpenCL program object. */
	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

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

#ifdef CL_VERSION_1_2

/**
 * Create a new program wrapper object from device built-in kernels.
 * This function is a wrapper for the
 * clCreateProgramWithBuiltInKernels() OpenCL function.
 *
 * @public @memberof ccl_program
 * @note Requires OpenCL >= 1.2
 *
 * @param[in] ctx The context wrapper object.
 * @param[in] num_devices Number of devices.
 * @param[in] devs List of devices for which to create the program, and
 * on which all the built-in kernels can be executed.
 * @param[in] kernel_names A semi-colon separated list of built-in
 * kernel names.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_new_from_built_in_kernels(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, const char *kernel_names,
	GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure num_devices > 0. */
	g_return_val_if_fail(num_devices > 0, NULL);

	/* OpenCL function return status. */
	cl_int ocl_status;
	/* OpenCL program object. */
	cl_program program = NULL;
	/* Program wrapper object. */
	CCLProgram* prg = NULL;
	/* List of cl_device_id objects. */
	cl_device_id* device_list = NULL;
	/* OpenCL version of underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: Program creation with built-in kernels requires OpenCL " \
		"version 1.2 or newer.", G_STRLOC);

	/* Unwrap devices. */
	device_list = g_slice_alloc(num_devices * sizeof(cl_device_id));
	for (guint i = 0; i < num_devices; ++i) {
		device_list[i] = ccl_device_unwrap(devs[i]);
	}

	/* Create program. */
	program = clCreateProgramWithBuiltInKernels(
		ccl_context_unwrap(ctx), num_devices, device_list, kernel_names,
		&ocl_status);

	/* Create kernel from built-in kernels. */
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create cl_program from built-in kernels (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap OpenCL program object. */
	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free stuff if necessary. */
	if (device_list != NULL)
		g_slice_free1(num_devices * sizeof(cl_device_id), device_list);

	/* Return prg. */
	return prg;

}

#endif

/**
 * Utility function which builds (compiles and links) a program
 * executable from the program source or binary. This function calls the
 * ccl_program_build_full() function, passing `NULL` to the
 * following parameters: `devices`, `pfn_notify` and `user_data`. In
 * other words, the program executable is built for all devices
 * associated with the program and no callback function is registered.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] options A null-terminated string of characters that
 * describes the build options to be used for building the program
 * executable.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_program_build(
	CCLProgram* prg, const char* options, GError** err) {

	return ccl_program_build_full(
		prg, 0, NULL, options, NULL, NULL, err);
}

/**
 * Builds (compiles and links) a program executable from the program
 * source or binary. This function wraps the clBuildProgram() OpenCL
 * function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] num_devices The number of devices listed in `devs`.
 * @param[in] devs List of device wrappers associated with program.
 * If `NULL`, the program executable is built for all devices
 * associated with program for which a source or binary has been loaded.
 * @param[in] options A null-terminated string of characters that
 * describes the build options to be used for building the program
 * executable.
 * @param[in] pfn_notify A callback function that can be registered and
 * which will be called when the program executable has been built
 * (successfully or unsuccessfully).
 * @param[in] user_data User supplied data for the callback function.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_program_build_full(CCLProgram* prg,
	cl_uint num_devices, CCLDevice** devs, const char *options,
	ccl_program_callback pfn_notify, void *user_data, GError** err) {

	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Array of unwrapped devices. */
	cl_device_id* cl_devices = NULL;
	/* Status of OpenCL function call. */
	cl_int ocl_status;
	/* Result of function call. */
	cl_bool result;

	/* Check if its necessary to unwrap devices. */
	if ((devs != NULL) && (num_devices > 0)) {
		cl_devices = g_slice_alloc0(sizeof(cl_device_id) * num_devices);
		/* Unwrap devices. */
		for (guint i = 0; i < num_devices; ++i)
			cl_devices[i] = ccl_device_unwrap(devs[i]);
	}

	/* Build program. */
	ocl_status = clBuildProgram(ccl_program_unwrap(prg),
		num_devices, cl_devices, options, pfn_notify, user_data);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to build program (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	result = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Bad result. */
	result = CL_FALSE;

finish:

	/* Check if necessary to release array of unwrapped devices. */
	if (cl_devices != NULL) {
		g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);
	}

	/* Return result of function call. */
	return result;

}

#ifdef CL_VERSION_1_2

/**
 * Compile a program's source code. This function wraps the
 * clCompileProgram() OpenCL function.
 *
 * @public @memberof ccl_program
 * @note Requires OpenCL >= 1.2
 *
 * @param[in] prg The program wrapper that holds the program to be
 * compiled.
 * @param[in] num_devices Number of devices in `devs`.
 * @param[in] devs List of device wrappers associated with the program.
 * If `NULL`, the compile is performed for all devices associated with
 * the program.
 * @param[in] options A null-terminated string of characters that
 * describes the compilation options to be used for building the program
 * executable.
 * @param[in] num_input_headers The number of programs wrappers that
 * describe headers in the array referenced by `prg_input_headers`.
 * @param[in] prg_input_headers An array of program wrapper embedded
 * headers created with any of the `ccl_program_new_from_source*()`
 * functions.
 * @param[in] header_include_names An array that has a one to one
 * correspondence with `prg_input_headers`. Each entry specifies the
 * include name used by source in program that comes from an embedded
 * header.
 * @param[in] pfn_notify A function pointer to a callback function that
 * an application can register and which will be called when the program
 * executable has been built (successfully or unsuccessfully).
 * @param[in] user_data User supplied data for the callback function.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_program_compile(CCLProgram* prg, cl_uint num_devices,
	CCLDevice** devs, const char* options, cl_uint num_input_headers,
	CCLProgram** prg_input_headers, const char** header_include_names,
	ccl_program_callback pfn_notify, void* user_data, GError** err) {

	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Array of unwrapped devices. */
	cl_device_id* cl_devices = NULL;
	/* Array of unwrapped programs (input headers). */
	cl_program* input_headers = NULL;
	/* Status of OpenCL function call. */
	cl_int ocl_status;
	/* Result of function call. */
	cl_bool result;
	/* OpenCL version of underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_program_get_opencl_version(prg, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: Program compilation requires OpenCL version 1.2 or newer.",
		G_STRLOC);

	/* Check if its necessary to unwrap devices. */
	if ((devs != NULL) && (num_devices > 0)) {
		cl_devices = g_slice_alloc0(sizeof(cl_device_id) * num_devices);
		/* Unwrap devices. */
		for (guint i = 0; i < num_devices; ++i)
			cl_devices[i] = ccl_device_unwrap(devs[i]);
	}

	/* Check if its necessary to unwrap programs (input headers). */
	if ((prg_input_headers != NULL) && (num_input_headers > 0)) {
		input_headers =
			g_slice_alloc0(sizeof(cl_program) * num_input_headers);
		/* Unwrap program objects. */
		for (guint i = 0; i < num_input_headers; ++i)
			input_headers[i] = ccl_program_unwrap(prg_input_headers[i]);
	}

	/* Compile program. */
	ocl_status = clCompileProgram(ccl_program_unwrap(prg), num_devices,
		(const cl_device_id*) cl_devices, options, num_input_headers,
		(const cl_program*) input_headers, header_include_names,
		pfn_notify, user_data);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to compile program (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	result = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Bad result. */
	result = CL_FALSE;

finish:

	/* Check if necessary to release array of unwrapped devices. */
	if (cl_devices != NULL) {
		g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);
	}
	/* Check if necessary to release array of input headers (programs). */
	if (input_headers != NULL) {
		g_slice_free1(
			sizeof(cl_program) * num_input_headers, input_headers);
	}

	/* Return result of function call. */
	return result;

}

/**
 * Link a set of compiled programs and create an executable program
 * wrapper. The return program wrapper must be freed with
 * ::ccl_program_destroy(). This function wraps the clLinkProgram()
 * OpenCL function.
 *
 * @public @memberof ccl_program
 * @note Requires OpenCL >= 1.2
 *
 * @param[in] ctx A context wrapper object.
 * @param[in] num_devices Number of devices in `devs`.
 * @param[in] devs List of device wrappers associated with the context.
 * If `NULL`, the link is performed for all devices associated with
 * the context for which a compiled object is available.
 * @param[in] options A null-terminated string of characters that
 * describes the link options to be used for building the program
 * executable.
 * @param[in] num_input_programs The number of program wrapper objects
 * in `input_prgs`.
 * @param[in] input_prgs An array of program wrapper objects that
 * contain compiled binaries or libraries that are to be linked to
 * create the program executable.
 * @param[in] pfn_notify A function pointer to a callback function that
 * an application can register and which will be called when the program
 * executable has been built (successfully or unsuccessfully).
 * @param[in] user_data User supplied data for the callback function.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new program wrapper object, or `NULL` if an error occurs.
 * */
CCLProgram* ccl_program_link(CCLContext* ctx, cl_uint num_devices,
	CCLDevice** devs, const char* options, cl_uint num_input_programs,
	CCLProgram** input_prgs, ccl_program_callback pfn_notify,
	void* user_data, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Array of unwrapped devices. */
	cl_device_id* cl_devices = NULL;
	/* Array of unwrapped input programs. */
	cl_program* input_programs = NULL;
	/* Status of OpenCL function call. */
	cl_int ocl_status;
	/* Program wrapper object to create. */
	CCLProgram* prg = NULL;
	/* OpenCL program object to create and wrap. */
	cl_program program = NULL;
	/* OpenCL version of underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_program_get_opencl_version(prg, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
		CCL_ERROR_UNSUPPORTED_OCL, error_handler,
		"%s: Program linking requires OpenCL version 1.2 or newer.",
		G_STRLOC);

	/* Check if its necessary to unwrap devices. */
	if ((devs != NULL) && (num_devices > 0)) {
		cl_devices = g_slice_alloc0(sizeof(cl_device_id) * num_devices);
		/* Unwrap devices. */
		for (guint i = 0; i < num_devices; ++i)
			cl_devices[i] = ccl_device_unwrap(devs[i]);
	}

	/* Check if its necessary to unwrap input programs. */
	if ((input_prgs != NULL) && (num_input_programs > 0)) {
		input_programs =
			g_slice_alloc0(sizeof(cl_program) * num_input_programs);
		/* Unwrap program objects. */
		for (guint i = 0; i < num_input_programs; ++i)
			input_programs[i] = ccl_program_unwrap(input_prgs[i]);
	}

	/* Link program. */
	program = clLinkProgram(ccl_context_unwrap(ctx), num_devices,
		(const cl_device_id*) cl_devices, options, num_input_programs,
		(const cl_program*) input_programs, pfn_notify, user_data,
		&ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to link program (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

	/* Wrap OpenCL program object. */
	prg = ccl_program_new_wrap(program);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Return NULL to signal error. */
	prg = NULL;

finish:

	/* Check if necessary to release array of unwrapped devices. */
	if (cl_devices != NULL) {
		g_slice_free1(sizeof(cl_device_id) * num_devices, cl_devices);
	}
	/* Check if necessary to release array of input programs. */
	if (input_programs != NULL) {
		g_slice_free1(
			sizeof(cl_program) * num_input_programs, input_programs);
	}

	/* Return linked program wrapper. */
	return prg;

}

#endif

/**
 * Get the OpenCL version of the platform associated with this program.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg A program wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL version of the platform associated with this
 * program as an integer. If an error occurs, 0 is returned.
 * */
cl_uint ccl_program_get_opencl_version(CCLProgram* prg, GError** err) {

	/* Make sure number prg is not NULL. */
	g_return_val_if_fail(prg != NULL, 0.0);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, 0.0);

	cl_context context;
	CCLContext* ctx;
	GError* err_internal = NULL;
	cl_uint ocl_ver;

	/* Get cl_context object for this program. */
	context = ccl_program_get_scalar_info(
		prg, CL_PROGRAM_CONTEXT, cl_context, &err_internal);
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
 * Get the kernel wrapper object for the given program kernel function.
 * This is a utility function which returns the same kernel wrapper
 * instance for each kernel function name. The returned kernel wrapper
 * object is automatically released when the program wrapper object
 * which contains it is destroyed; as such, it must not be externally
 * destroyed with ccl_kernel_destroy().
 *
 * @warning For multi-threaded handling and execution of the same
 * kernel function, create different kernel wrapper instances with the
 * ccl_kernel_new() function.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] kernel_name Name of kernel function.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The kernel wrapper object for the given program kernel
 * function.
 * */
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
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Keep new kernel wrapper in table. */
		g_hash_table_insert(prg->krnls, (gpointer) kernel_name, krnl);

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return kernel wrapper. */
	return krnl;

}

/**
 * Enqueues a program kernel function for execution on a device. This
 * is a utility function which handles one kernel wrapper instance
 * for each kernel function name.
 *
 * The operations performed by this function are equivalent to getting
 * the program's internally kept kernel wrapper instance for the given
 * kernel name using ccl_program_get_kernel(), and then enqueing it for
 * execution with ccl_kernel_set_args_and_enqueue_ndrange().
 * that
 *
 * @attention The variable argument list must end with `NULL`.
 *
 * @warning For multi-threaded execution of the same kernel
 * function, create different kernel wrapper instances with the
 * ccl_kernel_new() function and use the @ref KERNEL_WRAPPER
 * "kernel module" API to enqueue kernel executions.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] kernel_name Name of kernel function.
 * @param[in] cq Command queue wrapper object where to enqueue kernel
 * execution.
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
CCLEvent* ccl_program_enqueue_kernel(CCLProgram* prg,
	const char* kernel_name, CCLQueue* cq, cl_uint work_dim,
	const size_t* global_work_offset, const size_t* global_work_size,
	const size_t* local_work_size, CCLEventWaitList* evt_wait_lst,
	GError** err, ...) {

	/* Event wrapper. */
	CCLEvent* evt;
	/* The va_list, which represents the variable argument list. */
	va_list args_va;
	/* Array of arguments, to be created from the va_list. */
	CCLArg** args_array = NULL;
	/* Number of arguments. */
	guint num_args = 0;
	/* Aux. arg. when cycling through the va_list. */
	CCLArg* aux_arg;

	/* Initialize the va_list. */
	va_start(args_va, err);

	/* Get first argument. */
	aux_arg = va_arg(args_va, CCLArg*);

	/* Check if any arguments are given, and if so, populate array
	 * of arguments. */
	if (aux_arg != NULL) {

		/* 1. Determine number of arguments. */

		while (aux_arg != NULL) {
			num_args++;
			aux_arg = va_arg(args_va, CCLArg*);
		}
		va_end(args_va);

		/* 2. Populate array of arguments. */

		args_array = g_slice_alloc((num_args + 1) * sizeof(CCLArg*));
		va_start(args_va, err);

		for (guint i = 0; i < num_args; ++i) {
			aux_arg = va_arg(args_va, CCLArg*);
			args_array[i] = aux_arg;
		}
		va_end(args_va);
		args_array[num_args] = NULL;

	}

	/* Enqueue kernel using the array of arguments version of this
	 * function. */
	evt = ccl_program_enqueue_kernel_v(prg, kernel_name, cq, work_dim,
		global_work_offset, global_work_size, local_work_size,
		evt_wait_lst, args_array, err);

	/* If any arguments are given... */
	if (num_args > 0) {

		/* Free the array of arguments. */
		g_slice_free1((num_args + 1) * sizeof(CCLArg*), args_array);

	}

	/* Return the event. */
	return evt;

}

/**
 * Enqueues a program kernel function for execution on a device. This
 * is a utility function which handles one kernel wrapper instance
 * for each kernel function name.
 *
 * This function gets the program's internally kept kernel wrapper
 * instance for the given kernel name using ccl_program_get_kernel(),
 * and then enqueues it for execution with
 * ccl_kernel_set_args_and_enqueue_ndrange_v().
 *
 * @warning For multi-threaded execution of the same kernel
 * function, create different kernel wrapper instances with the
 * ccl_kernel_new() function and use the @ref KERNEL_WRAPPER
 * "kernel module" API to enqueue kernel executions.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] kernel_name Name of kernel function.
 * @param[in] cq Command queue wrapper object where to enqueue kernel
 * execution.
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
 * @param[in] args A `NULL`-terminated array of arguments to set.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this command.
 * */
CCLEvent* ccl_program_enqueue_kernel_v(CCLProgram* prg,
	const char* kernel_name, CCLQueue* cq, cl_uint work_dim,
	const size_t* global_work_offset, const size_t* global_work_size,
	const size_t* local_work_size, CCLEventWaitList* evt_wait_lst,
	CCLArg** args, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);

	CCLEvent* evt;
	CCLKernel* krnl;

	krnl = ccl_program_get_kernel(prg, kernel_name, err);
	if (krnl == NULL) return NULL;

	evt = ccl_kernel_set_args_and_enqueue_ndrange_v(krnl, cq, work_dim,
		global_work_offset, global_work_size, local_work_size,
		evt_wait_lst, args, err);
	return evt;
}

/**
 * @internal
 * Load the program binaries into the binaries table of the program
 * wrapper object.
 *
 * @private @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * */
static void ccl_program_load_binaries(CCLProgram* prg, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail((err) == NULL || *(err) == NULL);

	/* Make sure prg is not NULL. */
	g_return_if_fail(prg != NULL);

	/* Make sure binaries table is initialized. */
	g_return_if_fail(prg->binaries != NULL);

	cl_uint num_devices;
	cl_device_id* devices;
	size_t* binary_sizes;
	CCLWrapperInfo* info;
	unsigned char** bins_raw;
	GError* err_internal = NULL;
	cl_int ocl_status;

	/* Get number of program devices. */
	info = ccl_program_get_info(prg, CL_PROGRAM_NUM_DEVICES, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	num_devices = *((cl_uint*) info->value);

	/* Get program devices. */
	info = ccl_program_get_info(prg, CL_PROGRAM_DEVICES, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	devices = (cl_device_id*) info->value;

	/* Get binary sizes. */
	info = ccl_program_get_info(prg, CL_PROGRAM_BINARY_SIZES, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
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
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to get binaries from program (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));

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
	g_assert(err == NULL || *err == NULL);
	return;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	return;

}

/**
 * Get the program's binary object for the the specified device.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The program's binary object for the the specified device.
 * The returned object will be freed when the associated program is
 * destroyed.
 * */
CCLProgramBinary* ccl_program_get_binary(
	CCLProgram* prg, CCLDevice* dev, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, NULL);
	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;
	/* Binary code object. */
	CCLProgramBinary* binary = NULL;

	/* Check if binaries table is initialized. */
	if (prg->binaries == NULL) {

		/* Initialize binaries table. */
		prg->binaries = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL,
			(GDestroyNotify) ccl_program_binary_destroy);

		/* Load binaries. */
		ccl_program_load_binaries(prg, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

	/* Check if given device exists in the list of program devices. */
	if (g_hash_table_contains(prg->binaries, ccl_device_unwrap(dev))) {

		/* It exists, get it. */
		binary = g_hash_table_lookup(
			prg->binaries, ccl_device_unwrap(dev));

		/* If NULL, then perform a new binary fetch on the CL program
		 * object... */
		ccl_program_load_binaries(prg, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* ...and get it again. If it's NULL it's because binary isn't
		 * compiled for given device. */
		binary = g_hash_table_lookup(
			prg->binaries, ccl_device_unwrap(dev));

	} else {

		/* Device does not exist in list of program devices. */
		ccl_if_err_create_goto(*err, CCL_ERROR, TRUE,
			CCL_ERROR_DEVICE_NOT_FOUND,
			error_handler, "%s: device is not part of program devices.",
			G_STRLOC);
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return kernel wrapper. */
	return binary;
}

/**
 * Save the program's binary code for a specified device to a file.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] filename Name of file where to save the program's binary
 * code.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_program_save_binary(CCLProgram* prg, CCLDevice* dev,
	const char* filename, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, CL_FALSE);
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, CL_FALSE);
	/* Make sure filename is not NULL. */
	g_return_val_if_fail(filename != NULL, CL_FALSE);

	/* OpenCL function status. */
	cl_bool status;
	/* Internal error handling object. */
	GError* err_internal = NULL;
	/* The binary code object. */
	CCLProgramBinary* binary = NULL;

	/* Get the binary code object for the specified device and check
	 * for associated errors. */
	binary = ccl_program_get_binary(prg, dev, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	ccl_if_err_create_goto(*err, CCL_ERROR, binary->size == 0,
		CCL_ERROR_INVALID_DATA, error_handler,
		"%s: binary for given device has size 0.", G_STRLOC);

	/* Save binary code to specified file. */
	g_file_set_contents(filename, (const gchar*) binary->data,
		binary->size, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	status = CL_FALSE;

finish:

	/* Return function status. */
	return status;
}

/**
 * Save the program binaries for all associated devices to files, one
 * file per device.
 *
 * Applications can specify the prefix and suffix of saved files. The
 * variable part of the filename is obtained via the device name and
 * the device index.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] file_prefix Prefix of files to save, can include full or
 * relative paths.
 * @param[in] file_suffix Suffix of files to save.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if operation is successful, or `CL_FALSE`
 * otherwise.
 * */
cl_bool ccl_program_save_all_binaries(CCLProgram* prg,
	const char* file_prefix, const char* file_suffix, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, CL_FALSE);
	/* Make sure prg is not NULL. */
	g_return_val_if_fail(prg != NULL, CL_FALSE);
	/* Make sure file prefix and suffix are not NULL. */
	g_return_val_if_fail(
		(file_prefix != NULL) && (file_suffix != NULL), CL_FALSE);

	/* Internal error handling object. */
	GError* err_internal = NULL;
	/* Number of devices. */
	guint num_devices;
	/* OpenCL function return status. */
	cl_bool status;

	/* Get number of devices. */
	num_devices = ccl_program_get_num_devices(prg, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Cycle through each device. */
	for (guint i = 0; i < num_devices; ++i) {

		/* Save binaries, one per device. */

		/* Device wrapper object. */
		CCLDevice* dev = NULL;
		/* Variable part of filename. */
		gchar* file_middle = NULL;
		/* Complete filename. */
		gchar* filename;

		/* Get next device associated with program. */
		dev = ccl_program_get_device(prg, i, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Determine the variable part of current filename. */
		file_middle = g_strdup(
			ccl_device_get_array_info(
				dev, CL_DEVICE_NAME, char*, &err_internal));
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		g_strcanon(file_middle, CCL_VALIDFILECHARS, '_');

		/* Determine complete filename of current file. */
		filename = g_strdup_printf("%s%s_%02d%s",
			file_prefix, file_middle, i, file_suffix);

		/* Save current binary to file. */
		ccl_program_save_binary(prg, dev, filename, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Free filename variables. */
		g_free(filename);
		g_free(file_middle);
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	status = CL_FALSE;

finish:

	/* Return function status. */
	return status;

}

/**
 * Get ::CCLDevice wrapper at given index.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[in] index Index of device in program.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The ::CCLDevice wrapper at given index or `NULL` if an error
 * occurs.
 * */
CCLDevice* ccl_program_get_device(
	CCLProgram* prg, cl_uint index, GError** err) {

	return ccl_dev_container_get_device(
		(CCLDevContainer*) prg, ccl_program_get_cldevices, index, err);
}

/**
 * Return number of devices in program.
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The number of devices in program or 0 if an error occurs or
 * is otherwise not possible to get any device.
 * */
cl_uint ccl_program_get_num_devices(CCLProgram* prg, GError** err) {

	return ccl_dev_container_get_num_devices(
		(CCLDevContainer*) prg, ccl_program_get_cldevices, err);

}

/**
 * Get all device wrappers in program.
 *
 * This function returns the internal array containing the program
 * device wrappers. As such, clients should not modify the returned
 * array (e.g. they should not free it directly).
 *
 * @public @memberof ccl_program
 *
 * @param[in] prg The program wrapper object.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return An array containing the ::CCLDevice wrappers which belong to
 * the given program, or `NULL` if an error occurs.
 * */
CCLDevice* const* ccl_program_get_all_devices(CCLProgram* prg,
	GError** err) {

	return ccl_dev_container_get_all_devices((CCLDevContainer*) prg,
		ccl_program_get_cldevices, err);

}

/** @}*/

