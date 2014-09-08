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
 * Definition of a wrapper class and its methods for OpenCL program
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_PROGRAM_WRAPPER_H_
#define _CCL_PROGRAM_WRAPPER_H_

#include <glib.h>
#include "oclversions.h"
#include "context_wrapper.h"
#include "abstract_wrapper.h"
#include "event_wrapper.h"
#include "queue_wrapper.h"
#include "kernel_wrapper.h"

/**
 * @defgroup PROGRAM_WRAPPER Program wrapper
 *
 * The program wrapper module provides functionality for simple
 * handling of OpenCL program objects.
 *
 * OpenCL program objects can be created from source code, from binary
 * data or from built-in kernels using the clCreateProgramWithSource(),
 * clCreateProgramWithBinary() or clCreateProgramWithBuiltInKernels(),
 * respectively (the later requires OpenCL >= 1.2). _cf4ocl_ provides a
 * set of ::CCLProgram* program wrapper constructors which not only map
 * the native OpenCL program constructors, but also extend some of their
 * functionality.
 *
 * For creating programs from source code, _cf4ocl_ provides the
 * following constructors:
 *
 * * ::ccl_program_new_from_source_file() - Create a new program wrapper
 * object from a source file.
 * * ::ccl_program_new_from_source_files() - Create a new program
 * wrapper object from several source files.
 * * ::ccl_program_new_from_source() - Create a new program wrapper
 * object from a null-terminated source string.
 * * ::ccl_program_new_from_sources() - Create a new program wrapper
 * object from several source code strings.
 *
 * Program constructors which use binary data follow the same pattern as
 * their source code counterparts:
 *
 * * ::ccl_program_new_from_binary_file() - Create a new program wrapper
 * object from a file containing binary code executable on a specific
 * device.
 * * ::ccl_program_new_from_binary_files() - Create a new program
 * wrapper object from files containing binary code executable on the
 * given device list, one file per device.
 * * ::ccl_program_new_from_binary() - Create a new program wrapper
 * object from binary code executable on a specific device.
 * * ::ccl_program_new_from_binaries() - Create a new program wrapper
 * object from a list of binary code strings executable on the given
 * device list, one binary string per device.
 *
 * The ::ccl_program_new_from_built_in_kernels() constructor directly
 * wraps the native OpenCL clCreateProgramWithBuiltInKernels()
 * function, allowing to create programs from built-in kernels. This
 * method is only available for platforms which support OpenCL version
 * 1.2 or higher.
 *
 * Like most _cf4ocl_ wrapper objects, program wrapper objects follow
 * the @ref ug_new_destroy "new/destroy" rule, and should be released
 * with the ::ccl_program_destroy() destructor.
 *
 * The ::ccl_program_build() and ::ccl_program_build_full() methods
 * allow to build a program executable from the program source or
 * binary. While the later directly maps the native clBuildProgram()
 * OpenCL function, the former provides a simpler interface which will
 * be useful in many situations.
 *
 * Compilation and linking (which require OpenCL >= 1.2) are provided
 * by the ::ccl_program_compile() and ::ccl_program_link()
 * functions.
 *
 * Information about program objects can be obtained using the
 * program module @ref ug_getinfo "info macros":
 *
 * * ::ccl_program_get_info_scalar()
 * * ::ccl_program_get_info_array()
 * * ::ccl_program_get_info()
 *
 * However, program binaries cannot be retrieved using these macros.
 * Consequently, _cf4ocl_ provides a specific and straightforward API
 * for handling them:
 *
 * * ::ccl_program_get_binary() - Return the program binary object for
 * the specified device.
 * * ::ccl_program_save_binary() - Save the program binary code for a
 * specified device to a file.
 * * ::ccl_program_save_all_binaries() - Save the program binaries for
 * all associated devices to files, one file per device.
 *
 * Program build information can be obtained using a specific set of
 * @ref ug_getinfo "info macros":
 *
 * * ::ccl_program_get_build_info_scalar()
 * * ::ccl_program_get_build_info_array()
 * * ::ccl_program_get_build_info()
 *
 * For simple programs and kernels, the program wrapper module offers
 * three functions, which can be used onced a program is built:
 *
 * * ::ccl_program_get_kernel() - Get the kernel wrapper object for the
 * given program kernel function.
 * * ::ccl_program_enqueue_kernel() - Enqueues a program kernel function
 * for execution on a device, accepting kernel arguments as
 * `NULL`-terminated variable list of parameters.
 * * ::ccl_program_enqueue_kernel_v() - Enqueues a program kernel
 * function for execution on a device, accepting kernel arguments as
 * `NULL`-terminated array of parameters.
 *
 * Program wrapper objects only keep one kernel wrapper instance per
 * kernel function; as such, for a given kernel function, these
 * methods will always use the same  kernel wrapper instance (and
 * consequently, the same OpenCL kernel object). While this will work
 * for single-threaded host code, it will fail if the same kernel
 * wrapper is invoked from different threads. In such cases, use the
 * @ref KERNEL_WRAPPER "kernel wrapper module" API for handling kernel
 * wrapper objects.
 *
 * The ::CCLProgram* class extends the ::CCLDevContainer* class; as
 * such, it provides methods for handling a list of devices associated
 * with the program:
 *
 * * ::ccl_program_get_all_devices()
 * * ::ccl_program_get_device()
 * * ::ccl_program_get_num_devices()
 *
 * _Example:_
 *
 * @dontinclude canon.c
 * @skipline Wrappers.
 * @until c_dev
 * @skipline CCLEvent* evt_exec
 *
 * @skipline Global and local
 * @until lws
 *
 * @skipline Error reporting
 * @until GError
 *
 * @skipline Create a new program
 * @until prg =
 *
 * @skipline Build program
 * @until ccl_program_build
 *
 * @skipline evt_exec =
 * @until NULL);
 *
 * @skipline Destroy wrappers
 * @skipline ccl_program_destroy
 *
 * @example canon.c
 *
 * @{
 */

/**
 * Program wrapper class.
 *
 * @extends ccl_dev_container
 */
typedef struct ccl_program CCLProgram;

/** Class which represents a binary object associated with a program
 * and a device. */
typedef struct ccl_program_binary CCLProgramBinary;

/**
 * Prototype of callback functions for program build, compile and
 * link.
 *
 * @public @memberof ccl_program
 *
 * @param[in] program Program wrapper object.
 * @param[in] user_data A pointer to user supplied data.
 * */
typedef void (CL_CALLBACK* ccl_program_callback)(
	cl_program program, void* user_data);

/* *********** */
/* WRAPPER API */
/* *********** */

/* Get the program wrapper for the given OpenCL program. */
CCLProgram* ccl_program_new_wrap(cl_program program);

/* Decrements the reference count of the program wrapper object.
 * If it reaches 0, the program wrapper object is destroyed. */
void ccl_program_destroy(CCLProgram* prg);

/* *********************** */
/* CREATE FROM SOURCES API */
/* *********************** */

/* Create a new program wrapper object from a source file. */
CCLProgram* ccl_program_new_from_source_file(CCLContext* ctx,
	const char* filename, GError** err);

/* Create a new program wrapper object from several source files. */
CCLProgram* ccl_program_new_from_source_files(CCLContext* ctx,
	cl_uint count, const char** filenames, GError** err);

/* Create a new program wrapper object from a null-terminated source
 * string. */
CCLProgram* ccl_program_new_from_source(CCLContext* ctx,
	const char* string, GError** err);

/* Create a new program wrapper object from several source code
 * strings. */
CCLProgram* ccl_program_new_from_sources(CCLContext* ctx,
	cl_uint count, const char** strings, const size_t* lengths,
	GError** err);

/* ************************ */
/* CREATE FROM BINARIES API */
/* ************************ */

/* Create a new program wrapper object with binary code from a specific
 * device. */
CCLProgram* ccl_program_new_from_binary_file(CCLContext* ctx,
	CCLDevice* dev, const char* filename, cl_int *binary_status,
	GError** err);

/* Create a new program wrapper object from files containing binary
 * code executable on the given device list, one file per device. */
CCLProgram* ccl_program_new_from_binary_files(CCLContext* ctx,
	cl_uint num_devices, CCLDevice* const* devs, const char** filenames,
	cl_int *binary_status, GError** err);

/* Create a new program wrapper object from binary code executable on a
 * specific device. */
CCLProgram* ccl_program_new_from_binary(CCLContext* ctx, CCLDevice* dev,
	CCLProgramBinary* binary, cl_int *binary_status, GError** err);

/* Create a new program wrapper object from a list of binary code
 * strings executable on the given device list, one binary string per
 * device. */
CCLProgram* ccl_program_new_from_binaries(CCLContext* ctx,
	cl_uint num_devices, CCLDevice* const* devs, CCLProgramBinary** bins,
	cl_int *binary_status, GError** err);

/* ******************************** */
/* CREATE FROM BUILT-IN KERNELS API */
/* ******************************** */

#ifdef CL_VERSION_1_2

/* Create a new program wrapper object from device built-in kernels. */
CCLProgram* ccl_program_new_from_built_in_kernels(CCLContext* ctx,
	cl_uint num_devices, CCLDevice* const* devs, const char *kernel_names,
	GError** err);

#endif

/* ************************ */
/* BUILD, COMPILE, LINK API */
/* ************************ */

/* Utility function which builds (compiles and links) a program
 * executable from the program source or binary. */
cl_bool ccl_program_build(
	CCLProgram* prg, const char* options, GError** err);

/* Builds (compiles and links) a program executable from the program
 * source or binary. This function wraps the clBuildProgram() OpenCL
 * function. */
cl_bool ccl_program_build_full(CCLProgram* prg,
	cl_uint num_devices, CCLDevice* const* devs, const char* options,
	ccl_program_callback pfn_notify, void* user_data, GError** err);
	
/* Get build log for most recent build, compile or link. */
const char* ccl_program_get_build_log(CCLProgram* prg);

#ifdef CL_VERSION_1_2

/* Compile a program's source code. */
cl_bool ccl_program_compile(CCLProgram* prg, cl_uint num_devices,
	CCLDevice* const* devs, const char* options, cl_uint num_input_headers,
	CCLProgram** prg_input_headers, const char** header_include_names,
	ccl_program_callback pfn_notify, void* user_data, GError** err);

/* Link a set of compiled programs. */
CCLProgram* ccl_program_link(CCLContext* ctx, cl_uint num_devices,
	CCLDevice* const* devs, const char* options, cl_uint num_input_programs,
	CCLProgram** input_prgs, ccl_program_callback pfn_notify,
	void* user_data, GError** err);

#endif

/* ********************** */
/* PROGRAM OPENCL VERSION */
/* ********************** */

/* Get the OpenCL version of the platform associated with this
 * program. */
cl_uint ccl_program_get_opencl_version(CCLProgram* prg, GError** err);

/* ******************************* */
/* KERNEL RELATED HELPER FUNCTIONS */
/* ******************************* */

/* Get the kernel wrapper object for the given program kernel
 * function. */
CCLKernel* ccl_program_get_kernel(
	CCLProgram* prg, const char* kernel_name, GError** err);

/* Enqueues a program kernel function for execution on a device. */
CCLEvent* ccl_program_enqueue_kernel(CCLProgram* prg,
	const char* kernel_name, CCLQueue* cq, cl_uint work_dim,
	const size_t* global_work_offset, const size_t* global_work_size,
	const size_t* local_work_size, CCLEventWaitList* evt_wait_lst,
	GError** err, ...) G_GNUC_NULL_TERMINATED;

/* Enqueues a program kernel function for execution on a device. */
CCLEvent* ccl_program_enqueue_kernel_v(CCLProgram* prg,
	const char* kernel_name, CCLQueue* cq, cl_uint work_dim,
	const size_t* global_work_offset, const size_t* global_work_size,
	const size_t* local_work_size, CCLEventWaitList* evt_wait_lst,
	CCLArg** args, GError** err);

/* ************************* */
/* BINARY HANDLING FUNCTIONS */
/* ************************* */

/* Get the program's binary object for the the specified device. */
CCLProgramBinary* ccl_program_get_binary(
	CCLProgram* prg, CCLDevice* dev, GError** err);

/* Save the program's binary code for a specified device to a file. */
cl_bool ccl_program_save_binary(CCLProgram* prg, CCLDevice* dev,
	const char* filename, GError** err);

/* Save the program binaries for all associated devices to files, one
 * file per device. */
cl_bool ccl_program_save_all_binaries(CCLProgram* prg,
	const char* file_prefix, const char* file_suffix, GError** err);

/* ***************************************** */
/* DEVICE CONTAINER FUNCTION IMPLEMENTATIONS */
/* ***************************************** */

/* Get ::CCLDevice wrapper at given index. */
CCLDevice* ccl_program_get_device(
	CCLProgram* prg, cl_uint index, GError** err);

/* Return number of devices in program. */
cl_uint ccl_program_get_num_devices(CCLProgram* prg, GError** err);

/* Get all device wrappers in program. */
CCLDevice* const* ccl_program_get_all_devices(CCLProgram* prg,
	GError** err);

/* ************************************************* */
/* ABSTRACT WRAPPER MACROS (INFO, REF/UNREF, UNWRAP) */
/* ************************************************* */

/**
 * Get a ::CCLWrapperInfo program information object. To get the
 * program binaries use the ::ccl_program_get_binary() function instead,
 * as this macro will return NULL when the CL_PROGRAM_BINARIES parameter
 * is requested.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program information object. This object will
 * be automatically freed when the program wrapper object is
 * destroyed.  If an error occurs or if the CL_PROGRAM_BINARIES
 * parameter is requested, NULL is returned.
 * */
#define ccl_program_get_info(prg, param_name, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? NULL \
	: ccl_wrapper_get_info((CCLWrapper*) prg, NULL, param_name, 0, \
		(ccl_wrapper_info_fp) clGetProgramInfo, CL_TRUE, err)

/**
 * Macro which returns a scalar program information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program information value. This value will be
 * automatically freed when the program wrapper object is destroyed.
 * If an error occurs or if the CL_PROGRAM_BINARIES parameter is
 * requested, zero is returned.
 * */
#define ccl_program_get_info_scalar(prg, param_name, param_type, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? (param_type) 0 \
	: *((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetProgramInfo, CL_TRUE, err))

/**
 * Macro which returns an array program information value. To get
 * the program binaries use the ::ccl_program_get_binary() function
 * instead, as this macro will return NULL when the CL_PROGRAM_BINARIES
 * parameter is requested.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program information value. This value will be
 * automatically freed when the program wrapper object is destroyed.
 * If an error occurs or if the CL_PROGRAM_BINARIES parameter is
 * requested, NULL is returned.
 * */
#define ccl_program_get_info_array(prg, param_name, param_type, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? NULL \
	: (param_type) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetProgramInfo, CL_TRUE, err)

/**
 * Get a ::CCLWrapperInfo program build information object.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program build information object. This object
 * will be automatically freed when the program wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_program_get_build_info(prg, dev, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) prg, (CCLWrapper*) dev, \
		param_name, 0, (ccl_wrapper_info_fp) clGetProgramBuildInfo, \
		CL_FALSE, err)

/**
 * Macro which returns a scalar program build information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program build information value. This value
 * will be automatically freed when the program wrapper object is
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_program_get_build_info_scalar(prg, dev, param_name, \
	param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		(CCLWrapper*) dev, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetProgramBuildInfo, CL_FALSE, err))

/**
 * Macro which returns an array program build information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] prg The program wrapper object.
 * @param[in] dev The device wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested program build information value. This value
 * will be automatically freed when the program wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_program_get_build_info_array(prg, dev, param_name, \
	param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		(CCLWrapper*) dev, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetProgramBuildInfo, CL_FALSE, err)

/**
 * Increase the reference count of the program object.
 *
 * @param[in] prg The program wrapper object.
 * */
#define ccl_program_ref(prg) \
	ccl_wrapper_ref((CCLWrapper*) prg)

/**
 * Alias to ccl_program_destroy().
 *
 * @param[in] prg Program wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_program_unref(prg) ccl_program_destroy(prg)

/**
 * Get the OpenCL program object.
 *
 * @param[in] prg The program wrapper object.
 * @return The OpenCL program object.
 * */
#define ccl_program_unwrap(prg) \
	((cl_program) ccl_wrapper_unwrap((CCLWrapper*) prg))

/** @} */

#endif


