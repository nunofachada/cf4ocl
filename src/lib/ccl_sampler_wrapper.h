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
 * Definition of a wrapper class and its methods for OpenCL sampler objects.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_SAMPLER_WRAPPER_H_
#define _CCL_SAMPLER_WRAPPER_H_

#include "ccl_abstract_wrapper.h"
#include "ccl_context_wrapper.h"

/**
 * @defgroup CCL_SAMPLER_WRAPPER Sampler wrapper
 *
 * The sampler wrapper module provides functionality for simple
 * handling of OpenCL sampler objects.
 *
 * Sampler wrapper objects can be instanced with two constructors:
 *
 * * ccl_sampler_new()
 * * ccl_sampler_new_full()
 *
 * The former follows the constructor format in OpenCL 1.0-1.2, where
 * three main sampler properties are directly given as constructor
 * arguments, namely the normalized coordinates flag, the addressing
 * mode and the filter mode. The later uses the OpenCL 2.0 constructor
 * format, in which sampler properties are given in a zero-terminated
 * array of `cl_sampler_properties`. Both formats can be used without
 * concern for the underlying platform's OpenCL version, because
 * _cf4ocl_ will automatically select the most adequate OpenCL
 * constructor.
 *
 * Sampler wrapper objects should be freed with the
 * ::ccl_sampler_destroy() function, in accordance with the
 * _cf4ocl_ @ref ug_new_destroy "new/destroy" rule.
 *
 * Sampler wrapper objects can be directly passed as kernel arguments to
 * functions such as ::ccl_kernel_set_args_and_enqueue_ndrange() or
 * ::ccl_kernel_set_args().
 *
 * Information about sampler objects can be fetched using the
 * sampler @ref ug_getinfo "info macros":
 *
 * * ::ccl_sampler_get_info_scalar()
 * * ::ccl_sampler_get_info_array()
 * * ::ccl_sampler_get_info()
 *
 * _Example:_
 *
 * @dontinclude image_filter.c
 * @skipline CCLContext*
 * @skipline CCLSampler*
 * @skipline CCLErr*
 *
 * @skipline smplr =
 * @until CL_FILTER_NEAREST,
 *
 * @skipline ccl_sampler_destroy
 *
 * @{
 */

/* Get the sampler wrapper for the given OpenCL sampler. */
CCL_EXPORT
CCLSampler* ccl_sampler_new_wrap(cl_sampler sampler);

/* Decrements the reference count of the wrapper object. If it
 * reaches 0, the wrapper object is destroyed. */
CCL_EXPORT
void ccl_sampler_destroy(CCLSampler* smplr);

/* Create a new sampler wrapper object by specifying a basic set of
 * sampler properties. */
CCL_EXPORT
CCLSampler* ccl_sampler_new(CCLContext* ctx, cl_bool normalized_coords,
    cl_addressing_mode addressing_mode, cl_filter_mode filter_mode,
    CCLErr** err);

/* Create a new sampler wrapper object using a list of properties. */
CCL_EXPORT
CCLSampler* ccl_sampler_new_full(CCLContext* ctx,
    const cl_sampler_properties *sampler_properties, CCLErr** err);

/**
 * Get a ::CCLWrapperInfo sampler information object.
 *
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information object. This object will
 * be automatically freed when the sampler wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_sampler_get_info(smplr, param_name, err) \
    ccl_wrapper_get_info((CCLWrapper*) smplr, NULL, param_name, 0, \
        CCL_INFO_SAMPLER, CL_FALSE, err)

/**
 * Macro which returns a scalar sampler information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information value. This value will be
 * automatically freed when the sampler wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_sampler_get_info_scalar(smplr, param_name, param_type, err) \
    *((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) smplr, \
        NULL, param_name, sizeof(param_type), CCL_INFO_SAMPLER, CL_FALSE, err))

/**
 * Macro which returns an array sampler information value.
 *
 * @note As of OpenCL 2.0, there are no array sampler information
 * values. This macro is provided for completeness.
 *
 * Use with care. In case an error occurs, `NULL` is returned, which
 * might be ambiguous if `NULL` is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter in array (e.g. `char`, `size_t`,
 * etc.).
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information value. This value will be
 * automatically freed when the sampler wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_sampler_get_info_array(smplr, param_name, param_type, err) \
    (param_type *) ccl_wrapper_get_info_value((CCLWrapper *) smplr, \
        NULL, param_name, sizeof(param_type), CCL_INFO_SAMPLER, CL_FALSE, err)

/**
 * Increase the reference count of the sampler wrapper object.
 *
 * @param[in] smplr The sampler wrapper object.
 * */
#define ccl_sampler_ref(smplr) \
    ccl_wrapper_ref((CCLWrapper*) smplr)

/**
 * Alias to ccl_sampler_destroy().
 *
 * @param[in] smplr sampler wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_sampler_unref(smplr) ccl_sampler_destroy(smplr)

/**
 * Get the OpenCL sampler object.
 *
 * @param[in] smplr The sampler wrapper object.
 * @return The OpenCL sampler object.
 * */
#define ccl_sampler_unwrap(smplr) \
    ((cl_sampler) ccl_wrapper_unwrap((CCLWrapper*) smplr))

/** @} */

#endif
