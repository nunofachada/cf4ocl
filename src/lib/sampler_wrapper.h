
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
 * Definition of a wrapper class and its methods for OpenCL sampler
 * objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_SAMPLER_WRAPPER_H_
#define _CCL_SAMPLER_WRAPPER_H_

#include "abstract_wrapper.h"
#include "context_wrapper.h"

/**
 * @defgroup SAMPLER_WRAPPER Sampler wrapper
 *
 * A wrapper for handling samplers in OpenCL.
 * 
 * @todo Detailed description of module with code examples.
 * 
 * @{
 */

/** 
 * Sampler wrapper class
 * 
 * @extends ccl_wrapper
 * */
typedef struct ccl_sampler CCLSampler;

/* Get the sampler wrapper for the given OpenCL sampler. */
CCLSampler* ccl_sampler_new_wrap(cl_sampler sampler);

/* Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed. */
void ccl_sampler_destroy(CCLSampler* smplr);

/* Create a new sampler wrapper object by specifying a basic set of
 * sampler properties. */
CCLSampler* ccl_sampler_new(CCLContext* ctx, cl_bool normalized_coords,
	cl_addressing_mode addressing_mode, cl_filter_mode filter_mode, 
	GError** err);

/* Create a new sampler wrapper object using a list of properties. */
CCLSampler* ccl_sampler_new_full(CCLContext* ctx, 
	const cl_sampler_properties *sampler_properties, GError** err);
	
/**
 * Get a ::CCLWrapperInfo sampler information object.
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information object. This object will
 * be automatically freed when the sampler wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_sampler_get_info(smplr, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) smplr, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetSamplerInfo, CL_TRUE, err)

/** 
 * Macro which returns a scalar sampler information value. 
 * 
 * Use with care. In case an error occurs, zero is returned, which 
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information value. This value will be 
 * automatically freed when the sampler wrapper object is destroyed. 
 * If an error occurs, zero is returned.
 * */
#define ccl_sampler_get_scalar_info(smplr, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) smplr, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetSamplerInfo, \
		CL_TRUE, err))

/** 
 * Macro which returns an array sampler information value. 
 * 
 * @note As of OpenCL 2.0, there are no array sampler information
 * values. This macro is provided for completeness.
 * 
 * Use with care. In case an error occurs, NULL is returned, which 
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object. 
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr The sampler wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested sampler information value. This value will be 
 * automatically freed when the sampler wrapper object is destroyed. 
 * If an error occurs, NULL is returned.
 * */
#define ccl_sampler_get_array_info(smplr, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) smplr, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetSamplerInfo, \
		CL_TRUE, err)

/** 
 * Increase the reference count of the sampler wrapper object.
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] sampler The sampler wrapper object. 
 * */
#define ccl_sampler_ref(smplr) \
	ccl_wrapper_ref((CCLWrapper*) smplr)

/**
 * Alias to ccl_sampler_destroy().
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr sampler wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_sampler_unref(smplr) ccl_sampler_destroy(smplr)

/**
 * Get the OpenCL sampler object.
 * 
 * @public @memberof ccl_sampler
 * 
 * @param[in] smplr The sampler wrapper object.
 * @return The OpenCL sampler object.
 * */
#define ccl_sampler_unwrap(smplr) \
	((cl_sampler) ccl_wrapper_unwrap((CCLWrapper*) smplr))	

/** @} */

#endif




