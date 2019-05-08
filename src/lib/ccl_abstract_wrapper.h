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
 * Definition of an abstract wrapper class and its methods for OpenCL
 * objects.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_ABSTRACT_WRAPPER_H_
#define _CCL_ABSTRACT_WRAPPER_H_

#include "ccl_oclversions.h"
#include "ccl_common.h"
#include "ccl_errors.h"

/**
 * Class which represents information about a wrapped OpenCL
 * object.
 * */
typedef struct ccl_wrapper_info {

    /**
     * Object information.
     * @public
     * */
    void * value;

    /**
     * Size in bytes of object information.
     * @public
     * */
    size_t size;

} CCLWrapperInfo;

/* Increase the reference count of the wrapper object. */
CCL_EXPORT
void ccl_wrapper_ref(CCLWrapper * wrapper);

/* Returns the wrapper object reference count. For debugging and
 * testing purposes only. */
CCL_EXPORT
int ccl_wrapper_ref_count(CCLWrapper * wrapper);

/* Get the wrapped OpenCL object. */
CCL_EXPORT
void * ccl_wrapper_unwrap(CCLWrapper * wrapper);

/* Get information about any wrapped OpenCL object. */
CCL_EXPORT
CCLWrapperInfo * ccl_wrapper_get_info(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err);

/* Get pointer to information value. */
CCL_EXPORT
void * ccl_wrapper_get_info_value(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err);

/* Get information size. */
CCL_EXPORT
size_t ccl_wrapper_get_info_size(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err);

/* Debug function which checks if memory allocated by wrappers
 * has been properly freed. */
CCL_EXPORT
cl_bool ccl_wrapper_memcheck();

/* Get wrapper class or type name. */
CCL_EXPORT
const char * ccl_wrapper_get_class_name(CCLWrapper * wrapper);

#endif

