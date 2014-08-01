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
 * @brief Generic OpenCL object wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_ABSTRACT_WRAPPER_H_
#define _CCL_ABSTRACT_WRAPPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "oclversions.h"
#include "gerrorf.h"
#include "common.h"
#include "errors.h"

typedef struct ccl_wrapper {

	/** The wrapped OpenCL object. */
	void* cl_object;

	/** Information about the wrapped OpenCL object. */
	GHashTable* info;

	/** Reference count. */
	int ref_count;    
	
} CCLWrapper;

typedef void (*ccl_wrapper_release_fields)(CCLWrapper* wrapper);

typedef cl_int (*ccl_wrapper_release_cl_object)(void* cl_object);

/** @brief Create a new wrapper object. This function is called by the
 * concrete wrapper constructors and should not be called by client
 * code. */
CCLWrapper* ccl_wrapper_new(void* cl_object, size_t size);

/** @brief Increase the reference count of the wrapper object. */
void ccl_wrapper_ref(CCLWrapper* wrapper);

/** @brief Decrements the reference count of the wrapper object.
 * If it reaches 0, the wrapper object is destroyed. */
cl_bool ccl_wrapper_unref(CCLWrapper* wrapper, size_t size,
	ccl_wrapper_release_fields rel_fields_fun,
	ccl_wrapper_release_cl_object rel_cl_fun, GError** err);

/** @brief Returns the wrapper object reference count. For debugging and 
 * testing purposes only. */
int ccl_wrapper_ref_count(CCLWrapper* wrapper);

/** @brief Get the wrapped OpenCL object. */
void* ccl_wrapper_unwrap(CCLWrapper* wrapper);

/**
 * @brief Generic type for OpenCL clget*Info() functions.
 * 
 * @param cl_object OpenCL object to be queried.
 * @param param_name Parameter to query.
 * @param param_value_size Used to specify the size in bytes of memory 
 * pointed to by param_value.
 * @param param_value A pointer to memory where the appropriate result 
 * being queried is returned
 * @param param_value_size_ret Returns the actual size in bytes of data 
 * copied to param_value. If param_value_size_ret is NULL, it is ignored.
 * @return Returns CL_SUCCESS if the function is executed successfully,
 * or an error code otherwise.
 * */
typedef cl_int (*ccl_wrapper_info_fp1)(void* cl_object,
	cl_uint param_name, size_t param_value_size, void* param_value,
	size_t* param_value_size_ret);

/**
 * @brief Generic type for OpenCL clget**Info() functions, in which two
 * OpenCL objects are involved.
 * 
 * @param cl_object1 OpenCL object to be queried.
 * @param cl_object2 OpenCL object required for query.
 * @param param_name Parameter to query.
 * @param param_value_size Used to specify the size in bytes of memory 
 * pointed to by param_value.
 * @param param_value A pointer to memory where the appropriate result 
 * being queried is returned
 * @param param_value_size_ret Returns the actual size in bytes of data 
 * copied to param_value. If param_value_size_ret is NULL, it is ignored.
 * @return Returns CL_SUCCESS if the function is executed successfully,
 * or an error code otherwise.
 * */
typedef cl_int (*ccl_wrapper_info_fp2)(void* cl_object1,
	void* cl_object2, cl_uint param_name, size_t param_value_size, 
	void* param_value, size_t* param_value_size_ret);

typedef cl_int (*ccl_wrapper_info_fp)(void);


/**
 * @defgroup WRAPPER_INFO Wrapper information
 *
 * @brief Wrapper information object and associated macros.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/**
 * @brief Information about a wrapped OpenCL entity.
 * */
typedef struct ccl_wrapper_info {
	/** Device information. */
	void* value;
	/** Size in bytes of device information. */
	size_t size;
} CCLWrapperInfo;

/**
 * @brief Helper macro which returns a scalar information value casted 
 * to specified scalar type.
 * 
 * @param info ::CCLWrapperInfo information object.
 * @param type Scalar type to which to cast value to.
 * @return The information value casted to the specified scalar type.
 * */
#define ccl_info_scalar(info, type) *((type*) (info)->value)

/**
 * @brief Helper macro which returns an array information value casted 
 * to specified array type.
 * 
 * @param info ::CCLWrapperInfo information object.
 * @param type Array (pointer) type to which to cast value to.
 * @return The information value casted to the specified array (pointer)
 * type.
 * */
#define ccl_info_array(info, type) ((type) (info)->value)

/** @brief Debug function which checks if memory allocated by wrappers
 * has been properly freed. */
cl_bool ccl_wrapper_memcheck();

/** @} */

/** @brief Create a new CCLWrapperInfo* object with a given value size. */
CCLWrapperInfo* ccl_wrapper_info_new(size_t size);

/** @brief Destroy a ::CCLWrapperInfo object. */
void ccl_wrapper_info_destroy(CCLWrapperInfo* info);

/** @brief Get information about any wrapped OpenCL object. */
CCLWrapperInfo* ccl_wrapper_get_info(CCLWrapper* wrapper1,
	CCLWrapper* wrapper2, cl_uint param_name, 
	ccl_wrapper_info_fp info_fun, cl_bool use_cache, GError** err);
	
/** @brief Get pointer to information value. */
void* ccl_wrapper_get_info_value(CCLWrapper* wrapper1,
	CCLWrapper* wrapper2, cl_uint param_name, 
	ccl_wrapper_info_fp info_fun, cl_bool use_cache, GError** err);

/** @brief Get information size. */
size_t ccl_wrapper_get_info_size(CCLWrapper* wrapper1,
	CCLWrapper* wrapper2, cl_uint param_name, 
	ccl_wrapper_info_fp info_fun, cl_bool use_cache, GError** err);

#endif

