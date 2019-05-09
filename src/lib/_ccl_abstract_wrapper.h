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
 * Definition of an abstract wrapper class and some of its methods. This
 * file is only for building _cf4ocl_. Is is not part of its public API.
 *
 * @internal
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef __CCL_ABSTRACT_WRAPPER_H_
#define __CCL_ABSTRACT_WRAPPER_H_

#include "ccl_oclversions.h"
#include "ccl_common.h"
#include "ccl_errors.h"

/**
 * Information about wrapped OpenCL objects.
 * @internal
 * */
typedef struct ccl_wrapper_info_table CCLWrapperInfoTable;

/**
 * Base class for all OpenCL wrappers.
 * */
struct ccl_wrapper {

    /**
     * The class or type of wrapped OpenCL object.
     * @private
     * */
    CCLClass class;

    /**
     * The wrapped OpenCL object.
     * @private
     * */
    void* cl_object;

    /**
     * Information about the wrapped OpenCL object.
     * @private
     * */
    CCLWrapperInfoTable * info;

    /**
     * Reference count.
     * @private
     * */
    int ref_count;

};

/**
 * Release the fields of the concrete wrapper implementation.
 *
 * @internal @protected @memberof ccl_wrapper
 *
 * @param[in] wrapper A concrete wrapper implementation.
 * */
typedef void (*ccl_wrapper_release_fields)(CCLWrapper * wrapper);

/**
 * Release the OpenCL object wrapped by the concrete wrapper
 * implementation.
 *
 * Concrete implementations of this function are provided by the OpenCL
 * implementation.
 *
 * @internal @protected @memberof ccl_wrapper
 *
 * @param[in] cl_object The OpenCL object to release.
 * @return Returns `CL_SUCCESS` if the function is executed successfully
 * or an OpenCL error code otherwise.
 * */
typedef CL_API_ENTRY cl_int
    (CL_API_CALL * ccl_wrapper_release_cl_object)(void * cl_object);

/**
 * Generic type for OpenCL `clGet*Info()` functions.
 *
 * @internal @protected @memberof ccl_wrapper
 *
 * Concrete implementations of this function are provided by the OpenCL
 * implementation.
 *
 * @param[in] cl_object OpenCL object to be queried.
 * @param[in] param_name Parameter to query.
 * @param[in] param_value_size Used to specify the size in bytes of
 * memory pointed to by `param_value`.
 * @param[out] param_value A pointer to memory where the appropriate
 * result being queried is returned
 * @param[out] param_value_size_ret Returns the actual size in bytes of
 * data copied to `param_value`. If `param_value_size_ret` is `NULL`, it is
 * ignored.
 * @return Returns `CL_SUCCESS` if the function is executed successfully,
 * or an error code otherwise.
 * */
typedef CL_API_ENTRY cl_int (CL_API_CALL * ccl_wrapper_info_fp1)(
    void * cl_object, cl_uint param_name, size_t param_value_size,
    void * param_value, size_t * param_value_size_ret);

/**
 * Generic type for OpenCL `clGet**Info()` functions, in which two
 * OpenCL objects are involved.
 *
 * @internal @protected @memberof ccl_wrapper
 *
 * Concrete implementations of this function are provided by the OpenCL
 * implementation.
 *
 * @param[in] cl_object1 OpenCL object to be queried.
 * @param[in] cl_object2 OpenCL object required for query.
 * @param[in] param_name Parameter to query.
 * @param[in] param_value_size Used to specify the size in bytes of
 * memory pointed to by `param_value`.
 * @param[out] param_value A pointer to memory where the appropriate
 * result being queried is returned
 * @param[out] param_value_size_ret Returns the actual size in bytes of
 * data copied to `param_value`. If `param_value_size_ret` is` NULL`, it is
 * ignored.
 * @return Returns `CL_SUCCESS` if the function is executed successfully,
 * or an error code otherwise.
 * */
typedef CL_API_ENTRY cl_int (CL_API_CALL * ccl_wrapper_info_fp2)(
    void * cl_object1, void * cl_object2, cl_uint param_name,
    size_t param_value_size, void * param_value,
    size_t * param_value_size_ret);

/* Create a new wrapper object. This function is called by the
 * concrete wrapper constructors. */
CCLWrapper * ccl_wrapper_new(CCLClass class, void * cl_object, size_t size);

/* Decrements the reference count of the wrapper object.
 * If it reaches 0, the wrapper object is destroyed. */
cl_bool ccl_wrapper_unref(CCLWrapper * wrapper, size_t size,
    ccl_wrapper_release_fields rel_fields_fun,
    ccl_wrapper_release_cl_object rel_cl_fun, CCLErr ** err);

/* Add a ::CCLWrapperInfo object to the info table of the
 * given wrapper. */
void ccl_wrapper_add_info(CCLWrapper * wrapper, cl_uint param_name,
    CCLWrapperInfo * info);

/* Create a new CCLWrapperInfo* object with a given value size. */
CCLWrapperInfo * ccl_wrapper_info_new(size_t size);

/* Destroy a ::CCLWrapperInfo object. */
void ccl_wrapper_info_destroy(CCLWrapperInfo * info);

#endif
