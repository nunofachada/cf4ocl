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
 * Definition of a wrapper class and its methods for OpenCL memory
 * objects.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_MEMOBJ_WRAPPER_H_
#define _CCL_MEMOBJ_WRAPPER_H_

#include "ccl_oclversions.h"
#include "ccl_abstract_wrapper.h"
#include "ccl_event_wrapper.h"
#include "ccl_queue_wrapper.h"

/**
 * @defgroup CCL_MEMOBJ_WRAPPER Memory object wrapper
 *
 * The memory object wrapper module provides functionality for simple
 * handling of generic OpenCL memory objects.
 *
 * All the functions in this module are direct wrappers of the
 * respective OpenCL memory object functions, except for the
 * ::ccl_memobj_get_opencl_version() function, which returns the OpenCL
 * version of the platform to which the memory object is associated
 * with.
 *
 * For specific buffer and image handling, see the
 * @ref CCL_BUFFER_WRAPPER "buffer wrapper" and
 * @ref CCL_IMAGE_WRAPPER "image wrapper" modules.
 *
 * Information about memory objects can be fetched using the memory
 * object @ref ug_getinfo "info macros":
 *
 * * ::ccl_memobj_get_info_scalar()
 * * ::ccl_memobj_get_info_array()
 * * ::ccl_memobj_get_info()
 *
 * @{
 */

/**
 * Prototype for memory object destructor callback functions.
 *
 * @public @memberof ccl_memobj
 *
 * @param[in] memobj The OpenCL memory object for which the callback
 * function is associated.
 * @param[in] user_data A pointer to user supplied data.
 * */
typedef void (CL_CALLBACK *ccl_memobj_destructor_callback)(
	cl_mem memobj, void *user_data);

/* Get the OpenCL version of the platform associated with this memory
 * object. */
CCL_EXPORT
cl_uint ccl_memobj_get_opencl_version(CCLMemObj* mo, GError** err);

/* Enqueues a command to unmap a previously mapped region of a memory
 * object. */
CCL_EXPORT
CCLEvent* ccl_memobj_enqueue_unmap(CCLMemObj* mo, CCLQueue* cq,
	void* mapped_ptr, CCLEventWaitList* evt_wait_lst, GError** err);

/* Wrapper for OpenCL clSetMemObjectDestructorCallback() function. */
CCL_EXPORT
cl_bool ccl_memobj_set_destructor_callback(CCLMemObj* mo,
	ccl_memobj_destructor_callback pfn_notify,
	void *user_data, GError** err);

/* Enqueues a command to indicate which device a set of memory objects
 * should be associated with. */
CCL_EXPORT
CCLEvent* ccl_memobj_enqueue_migrate(CCLMemObj** mos, cl_uint num_mos,
 	CCLQueue* cq, cl_mem_migration_flags flags,
 	CCLEventWaitList* evt_wait_lst, GError** err);

/**
 * Get a ::CCLWrapperInfo memory object information object.
 *
 * @param[in] mo The memory object wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested memory object information object. This object will
 * be automatically freed when the memory object wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_memobj_get_info(mo, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) mo, NULL, param_name, 0, \
		(ccl_wrapper_info_fp) clGetMemObjectInfo, CL_FALSE, err)

/**
 * Macro which returns a scalar memory object information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] mo The memory object wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested memory object information value. This value will be
 * automatically freed when the memory object wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_memobj_get_info_scalar(mo, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) mo, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetMemObjectInfo, CL_FALSE, err))

/**
 * Macro which returns an array memory object information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] mo The memory object wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested memory object information value. This value will be
 * automatically freed when the memory object wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_memobj_get_info_array(mo, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) mo, \
		NULL, param_name, sizeof(param_type), \
		(ccl_wrapper_info_fp) clGetMemObjectInfo, CL_FALSE, err)

/**
 * Increase the reference count of the cl_mem wrapper object.
 *
 * @param[in] mo The cl_mem wrapper object.
 * */
#define ccl_memobj_ref(mo) \
	ccl_wrapper_ref((CCLWrapper*) mo)

/**
 * Get the OpenCL cl_mem object.
 *
 * @param[in] mo The cl_mem wrapper object.
 * @return The OpenCL cl_mem object.
 * */
#define ccl_memobj_unwrap(mo) \
	((cl_mem) ccl_wrapper_unwrap((CCLWrapper*) mo))

/** @} */

#endif
