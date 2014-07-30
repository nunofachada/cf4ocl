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
 * @brief A wrapper object for OpenCL programs and functions to manage 
 * them.
 * 
 * Todo: detailed description of module.
 * 
 * @{
 */
 
/** @brief Program wrapper object. */
typedef struct ccl_program CCLProgram;

/** @brief Represents a OpenCL binary object. */
typedef struct ccl_program_binary CCLProgramBinary;

typedef void (CL_CALLBACK* ccl_program_callback)(
	cl_program program, void* user_data);

/* WRAPPER API */
/** @brief Get the program wrapper for the given OpenCL program. */
CCLProgram* ccl_program_new_wrap(cl_program program);

/** @brief Decrements the reference count of the program wrapper object. 
 * If it reaches 0, the program wrapper object is destroyed. */
void ccl_program_destroy(CCLProgram* prg);

/* SOURCES */
CCLProgram* ccl_program_new_from_source_files(CCLContext* ctx, 
	cl_uint count, const char** filenames, GError** err);
	
CCLProgram* ccl_program_new_from_source_file(CCLContext* ctx, 
	const char* filename, GError** err);

CCLProgram* ccl_program_new_from_sources(CCLContext* ctx,
	cl_uint count, const char **strings, const size_t *lengths,
	GError** err);

#define ccl_program_new_from_source(ctx, src, err) \
	ccl_program_new_from_sources(ctx, 1, &src, NULL, err)

/* BINARIES */
CCLProgram* ccl_program_new_from_binary_files(CCLContext* ctx, 
	cl_uint num_devices, CCLDevice** devs, const char** filenames, 
	cl_int *binary_status, GError** err);

CCLProgram* ccl_program_new_from_binary_file(CCLContext* ctx, 
	CCLDevice* dev, const char* filename, cl_int *binary_status, 
	GError** err);

CCLProgram* ccl_program_new_from_binaries(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, CCLProgramBinary** bins,
	cl_int *binary_status, GError** err);

#define ccl_program_new_from_binary(ctx, dev, binary, bin_status, err) \
	ccl_program_new_from_binaries(ctx, 1, &dev, &binary, bin_status, err)

/* BUILT-IN KERNELS */
#ifdef CL_VERSION_1_2

CCLProgram* ccl_program_new_from_built_in_kernels(CCLContext* ctx,
	cl_uint num_devices, CCLDevice** devs, const char *kernel_names, 
	GError** err);

#endif
 
#define ccl_program_build(prg, options, err) \
	ccl_program_build_from_devices_full( \
		prg, 0, NULL, options, NULL, NULL, err)

cl_bool ccl_program_build_from_devices_full(CCLProgram* prg, 
	cl_uint num_devices, CCLDevice** devices, const char *options, 
	ccl_program_callback pfn_notify, void *user_data, GError** err);

CCLKernel* ccl_program_get_kernel(
	CCLProgram* prg, const char* kernel_name, GError** err);
	
CCLEvent* ccl_program_run(CCLProgram* prg, const char* kernel_name,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, ...)
	G_GNUC_NULL_TERMINATED;

CCLEvent* ccl_program_run_v(CCLProgram* prg, const char* kernel_name,
	CCLQueue* cq, cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size, 
	CCLEventWaitList evt_wait_lst, GError** err, va_list args);
	
CCLProgramBinary* ccl_program_get_binary(CCLProgram* prg, CCLDevice* dev,
	GError** err);

cl_bool ccl_program_save_binary(CCLProgram* prg, CCLDevice* dev,
	const char* filename, GError** err);

cl_bool ccl_program_save_all_binaries(CCLProgram* prg, 
	const char* file_prefix, const char* file_suffix, GError** err);

/**
 * @brief Get a ::CCLWrapperInfo program information object. To get the 
 * program binaries use the ::ccl_program_get_binary() function instead, 
 * as this macro will return NULL when the CL_PROGRAM_BINARIES parameter 
 * is requested.
 * 
 * @param prg The program wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested program information object. This object will
 * be automatically freed when the program wrapper object is 
 * destroyed.  If an error occurs or if the CL_PROGRAM_BINARIES 
 * parameter is requested, NULL is returned.
 * */
#define ccl_program_get_info(prg, param_name, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? NULL \
	: ccl_wrapper_get_info((CCLWrapper*) prg, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetProgramInfo, CL_TRUE, err)

/** 
 * @brief Macro which returns a scalar program information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param prg The program wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested program information value. This value will be 
 * automatically freed when the program wrapper object is destroyed. 
 * If an error occurs or if the CL_PROGRAM_BINARIES parameter is 
 * requested, zero is returned.
 * */
#define ccl_program_get_scalar_info(prg, param_name, param_type, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? (param_type) 0 \
	: *((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetProgramInfo, \
		CL_TRUE, err))

/** 
 * @brief Macro which returns an array program information value. To get 
 * the program binaries use the ::ccl_program_get_binary() function 
 * instead, as this macro will return NULL when the CL_PROGRAM_BINARIES 
 * parameter is requested.
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param prg The program wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested program information value. This value will be 
 * automatically freed when the program wrapper object is destroyed. 
 * If an error occurs or if the CL_PROGRAM_BINARIES parameter is 
 * requested, NULL is returned.
 * */
#define ccl_program_get_array_info(prg, param_name, param_type, err) \
	(param_name == CL_PROGRAM_BINARIES) \
	? NULL \
	: (param_type) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetProgramInfo, \
		CL_TRUE, err)

/**
 * @brief Get a ::CCLWrapperInfo program build information object.
 * 
 * @param prg The program wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The reque
 * sted program build information object. This object will be 
 * automatically freed when the program wrapper object is destroyed. If 
 * an error occurs, NULL is returned.
 * */
#define ccl_program_get_build_info(prg, dev, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) prg, (CCLWrapper*) dev, \
		param_name, (ccl_wrapper_info_fp) clGetProgramBuildInfo, \
		CL_FALSE, err)

/** 
 * @brief Macro which returns a scalar program build information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param prg The program wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested program build information value. This value 
 * will be automatically freed when the program wrapper object is 
 * destroyed. If an error occurs, zero is returned.
 * */
#define ccl_program_get_scalar_build_info(prg, dev, param_name, \
	param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		(CCLWrapper*) dev, param_name, \
		(ccl_wrapper_info_fp) clGetProgramBuildInfo, CL_FALSE, err))

/** 
 * @brief Macro which returns an array program build information value. 
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @param prg The program wrapper object.
 * @param dev The device wrapper object.
 * @param param_name Name of information/parameter to get value of.
 * @param param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested program build information value. This value 
 * will be automatically freed when the program wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_program_get_array_build_info(prg, dev, param_name, \
	param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) prg, \
		(CCLWrapper*) dev, param_name, \
		(ccl_wrapper_info_fp) clGetProgramBuildInfo, CL_FALSE, err)

/** 
 * @brief Increase the reference count of the program object.
 * 
 * @param prg The program wrapper object. 
 * */
#define ccl_program_ref(prg) \
	ccl_wrapper_ref((CCLWrapper*) prg)

/**
 * @brief Alias to ccl_program_destroy().
 * 
 * @param prg Program wrapper object to destroy if reference count 
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_program_unref(prg) ccl_program_destroy(prg)

/**
 * @brief Get the OpenCL program object.
 * 
 * @param prg The program wrapper object.
 * @return The OpenCL program object.
 * */
#define ccl_program_unwrap(prg) \
	((cl_program) ccl_wrapper_unwrap((CCLWrapper*) prg))		

/** 
 * @brief Get ::CCLDevice wrapper at given index. 
 * 
 * @param prg The program wrapper object.
 * @param index Index of device in program.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The ::CCLDevice wrapper at given index or NULL if an error 
 * occurs.
 * */
#define ccl_program_get_device(prg, index, err) \
	ccl_dev_container_get_device((CCLDevContainer*) prg, \
	ccl_program_get_cldevices, index, err)

/**
 * @brief Return number of devices in program.
 * 
 * @param prg The program wrapper object.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return The number of devices in program or 0 if an error occurs or 
 * is otherwise not possible to get any device.
 * */
#define ccl_program_get_num_devices(prg, err) \
	ccl_dev_container_get_num_devices((CCLDevContainer*) prg, \
	ccl_program_get_cldevices, err)

/** @} */

/** @brief Create a new ::CCLProgramBinary object with a given value 
 * size. */
CCLProgramBinary* ccl_program_binary_new(
	unsigned char* data, size_t size);

#define ccl_program_binary_new_empty() \
	ccl_program_binary_new(NULL, 0);

/** @brief Destroy a ::CCLProgramBinary object. */
void ccl_program_binary_destroy(CCLProgramBinary* bin);

/** @brief Implementation of ccl_dev_container_get_cldevices() for the
 * program wrapper. */
CCLWrapperInfo* ccl_program_get_cldevices(
	CCLDevContainer* devcon, GError** err);

#endif


