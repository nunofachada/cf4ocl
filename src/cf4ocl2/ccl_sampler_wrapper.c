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
 * Implementation of a wrapper class and its methods for OpenCL sampler
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_sampler_wrapper.h"
#include "priv_abstract_wrapper.h"

/**
 * Sampler wrapper class.
 *
 * @extends ccl_wrapper
 * */
struct ccl_sampler {

	/**
	 * Parent wrapper object.
	 * @private
	 * */
	CCLWrapper base;

};

/**
 * @addtogroup CCL_SAMPLER_WRAPPER
 * @{
 */

/**
 * @internal
 * Basic sampler properties accepted by the "old" clCreateSampler()
 * constructor.
 * */
struct ccl_sampler_basic_properties {

	/** Are the image coordinates normalized? */
	cl_bool normalized_coords;

	/** How to handle out-of-range coordinates. */
	cl_addressing_mode addressing_mode;

	/** Filter to apply when reading an image. */
	cl_filter_mode filter_mode;

};

/**
 * @internal
 * Gets a ::ccl_sampler_basic_properties containing the properties
 * accepted by the "old" clCreateSampler() constructor from a given
 * list of properties. The following default values are assumed for
 * non-specified properties:
 *
 * * `CL_SAMPLER_NORMALIZED_COORDS`: `CL_TRUE`
 * * `CL_SAMPLER_ADDRESSING_MODE` : `CL_ADDRESS_CLAMP`
 * * `CL_SAMPLER_FILTER_MODE` : `CL_FILTER_NEAREST`
 *
 * @private @memberof ccl_sampler
 *
 * @param[in] sampler_properties Zero-terminated list of sampler
 * properties.
 * @return A ::ccl_sampler_basic_properties object containing sampler
 * properties.
 * */
static struct ccl_sampler_basic_properties
	ccl_sampler_get_basic_properties(
		const cl_sampler_properties *sampler_properties) {

	/* Set defaults. */
	struct ccl_sampler_basic_properties sbp =
		{ CL_TRUE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST };

	/* Cycle through property list. */
	if (sampler_properties != NULL) {
		for (guint i = 0; sampler_properties[i] != 0; ++i) {
			/* Check if property is a basic known property. */
			switch (sampler_properties[i]) {
				case CL_SAMPLER_NORMALIZED_COORDS:
					sbp.normalized_coords = (cl_bool) sampler_properties[i + 1];
					break;
				case CL_SAMPLER_ADDRESSING_MODE:
					sbp.addressing_mode = (cl_addressing_mode) sampler_properties[i + 1];
					break;
				case CL_SAMPLER_FILTER_MODE:
					sbp.filter_mode = (cl_filter_mode) sampler_properties[i + 1];
					break;
			}
			/* Skip to next property. */
			++i;
		}
	}

	/* Return properties. */
	return sbp;

}

/**
 * Get the sampler wrapper for the given OpenCL sampler.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to directly wrap an OpenCL sampler in a
 * ::CCLSampler wrapper object.
 *
 * @protected @memberof ccl_sampler
 *
 * @param[in] sampler The OpenCL sampler to be wrapped.
 * @return The ::CCLSampler wrapper for the given OpenCL sampler.
 * */
CCL_EXPORT
CCLSampler* ccl_sampler_new_wrap(cl_sampler sampler) {

	return (CCLSampler*) ccl_wrapper_new(
		(void*) sampler, sizeof(CCLSampler));

}

/**
 * Decrements the reference count of the wrapper object. If it
 * reaches 0, the wrapper object is destroyed.
 *
 * @public @memberof ccl_sampler
 *
 * @param[in] smplr The sampler wrapper object.
 * */
CCL_EXPORT
void ccl_sampler_destroy(CCLSampler* smplr) {

	ccl_wrapper_unref((CCLWrapper*) smplr, sizeof(CCLSampler),
		NULL, (ccl_wrapper_release_cl_object) clReleaseSampler, NULL);

}

/**
 * Create a new sampler wrapper object by specifying a basic set of
 * sampler properties.
 *
 * @public @memberof ccl_sampler
 *
 * This function mimicks the style of the classic sampler constructor,
 * clCreateSampler(), but can be used with any version of OpenCL. This
 * function calls the ccl_sampler_new_full() function for actual
 * sampler creation.
 *
 * @param[in] ctx A context wrapper object.
 * @param[in] normalized_coords Are the image coordinates normalized?
 * @param[in] addressing_mode How to handle out-of-range coordinates.
 * @param[in] filter_mode Filter to apply when reading an image.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new sampler wrapper object or `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLSampler* ccl_sampler_new(CCLContext* ctx, cl_bool normalized_coords,
	cl_addressing_mode addressing_mode, cl_filter_mode filter_mode,
	GError** err) {

	const cl_sampler_properties sp[] = {
		CL_SAMPLER_NORMALIZED_COORDS, normalized_coords,
		CL_SAMPLER_ADDRESSING_MODE, addressing_mode,
		CL_SAMPLER_FILTER_MODE, filter_mode,
		0
	};

	return ccl_sampler_new_full(ctx, sp, err);
}


/**
 * Create a new sampler wrapper object using a list of properties.
 *
 * If a supported property is not specified, a default value is used.
 * Some valid properties are `CL_SAMPLER_NORMALIZED_COORDS` (default
 * value is `CL_TRUE`), `CL_SAMPLER_ADDRESSING_MODE` (default value is
 * `CL_ADDRESS_CLAMP`) and `CL_SAMPLER_FILTER_MODE` (default value is
 * `CL_FILTER_NEAREST`).
 *
 * This function mimicks the style of the OpenCL 2.0 sampler
 * constructor, clCreateSamplerWithProperties(), but can be used with
 * any version of OpenCL. Thus, The underlying OpenCL sampler object is
 * created using:
 *
 * * clCreateSampler() - for platforms with OpenCL version <= 1.2
 * * clCreateSamplerWithProperties() - for platforms with OpenCL version
 * >= 2.0.
 *
 * @public @memberof ccl_sampler
 *
 * @param[in] ctx A context wrapper object.
 * @param[in] sampler_properties A list of sampler property names and
 * their corresponding values. Each sampler property name is immediately
 * followed by the corresponding desired value. The list is terminated
 * with 0. If a supported property is not specified, its default value
 * will be used. If `NULL`, default values for supported sampler
 * properties will be used.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new sampler wrapper object or `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLSampler* ccl_sampler_new_full(CCLContext* ctx,
	const cl_sampler_properties *sampler_properties, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);

	/* New sampler wrapper object to create. */
	CCLSampler* smplr = NULL;
	/* OpenCL sampler object to create and wrap. */
	cl_sampler sampler;
	/* OpenCL function status. */
	cl_int ocl_status;

#ifdef CL_VERSION_2_0

	/* OpenCL platform version. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Get context platform version. */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Create the OpenCL sampler object. */
	if (ocl_ver >= 200) {
		/* Platform is OpenCL >= 2.0, use "new" API. */
		sampler = clCreateSamplerWithProperties(
			ccl_context_unwrap(ctx), sampler_properties, &ocl_status);
	} else {
		/* Platform is OpenCL <= 1.2, use "old" API. */
		struct ccl_sampler_basic_properties sbp =
			ccl_sampler_get_basic_properties(sampler_properties);
		CCL_BEGIN_IGNORE_DEPRECATIONS
		sampler = clCreateSampler(ccl_context_unwrap(ctx),
			sbp.normalized_coords, sbp.addressing_mode, sbp.filter_mode,
			&ocl_status);
		CCL_END_IGNORE_DEPRECATIONS
	}

#else

	/* Create OpenCL sampler object. */
	struct ccl_sampler_basic_properties sbp =
		ccl_sampler_get_basic_properties(sampler_properties);
	sampler = clCreateSampler(ccl_context_unwrap(ctx),
		sbp.normalized_coords, sbp.addressing_mode, sbp.filter_mode,
		&ocl_status);

#endif

	/* Check for errors. */
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler,
		"%s: unable to create sampler (OpenCL error %d: %s).",
		CCL_STRD, ocl_status, ccl_err(ocl_status));

	/* Create sampler wrapper. */
	smplr = ccl_sampler_new_wrap(sampler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return sampler wrapper. */
	return smplr;


}


/** @} */


