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
 * Implementation of a wrapper class and its methods for OpenCL image
 * objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "image_wrapper.h"

/** 
 * @addtogroup IMAGE_WRAPPER
 * @{
 */

/** 
 * Image wrapper class.
 * 
 * @extends ccl_memobj
 * */
struct ccl_image {

	/** 
	 * Parent wrapper object. 
	 * @private
	 * */
	CCLMemObj mo;

};

/** 
 * @internal
 * Creates a new OpenCL image object using the deprecated OpenCL 1.0/1.1
 * API.
 * 
 * @private @memberof ccl_image
 * 
 * @param[in] ctx A context wrapper object on which the image wrapper
 * object is to be created.
 * @param[in] flags Specifies allocation and usage information about the
 * image wrapper object being created.
 * @param[in] image_format A pointer to the OpenCL cl_image_format
 * structure, which describes format properties of the image to be
 * allocated.
 * @param[in] img_dsc A pointer to a ::CCLImageDesc object which 
 * describes type and dimensions of the image to be allocated.
 * @param[in] host_ptr A pointer to the image data that may already be 
 * allocated by the application.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new OpenCL image object.
 * */
static cl_mem ccl_image_new_deprecated(CCLContext* ctx, cl_mem_flags flags,
	const cl_image_format* image_format, const CCLImageDesc* img_dsc,
	void* host_ptr, GError** err) {
	
	/* OpenCL image object. */
	cl_mem image = NULL;
	/* OpenCL function status. */
	cl_int ocl_status;
	
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	
	/* Check what type of image to create. */
	if (img_dsc->image_type == CL_MEM_OBJECT_IMAGE2D) {
		
		/* Create a 2D image. */
		image = clCreateImage2D(ccl_context_unwrap(ctx), flags,
			image_format, img_dsc->image_width, img_dsc->image_height, 
			img_dsc->image_row_pitch, host_ptr, &ocl_status);
			
	} else if (img_dsc->image_type == CL_MEM_OBJECT_IMAGE3D) {
		
		/* Create a 3D image. */
		image = clCreateImage3D(ccl_context_unwrap(ctx), flags,
			image_format, img_dsc->image_width, img_dsc->image_height, 
			img_dsc->image_depth, img_dsc->image_row_pitch,
			img_dsc->image_slice_pitch, host_ptr, &ocl_status);
			
	} else {
		
		/* Unknown or unsupported image type. */
		*err = g_error_new(CCL_ERROR, CCL_ERROR_UNSUPPORTED_OCL,
			"%s: unknown or unsuported image type (%x)", G_STRLOC, 
			img_dsc->image_type); 
	}
	
	G_GNUC_END_IGNORE_DEPRECATIONS
	
	/* Return OpenCL image object. */
	return image;

}

/**
 * Get the image wrapper for the given OpenCL image.
 * 
 * If the wrapper doesn't exist, its created with a reference count 
 * of 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to directly wrap an OpenCL image in a 
 * ::CCLImage wrapper object.
 * 
 * @protected @memberof ccl_image
 * 
 * @param[in] mem_object The OpenCL image to be wrapped.
 * @return The ::CCLImage wrapper for the given OpenCL image.
 * */
CCLImage* ccl_image_new_wrap(cl_mem mem_object) {
	
	return (CCLImage*) ccl_wrapper_new(
		(void*) mem_object, sizeof(CCLImage));
		
}

/** 
 * Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed.
 *
 * @public @memberof ccl_image
 * 
 * @param[in] img The image wrapper object.
 * */
void ccl_image_destroy(CCLImage* img) {
	
	ccl_wrapper_unref((CCLWrapper*) img, sizeof(CCLImage),
		(ccl_wrapper_release_fields) ccl_memobj_release_fields, 
		(ccl_wrapper_release_cl_object) clReleaseMemObject, NULL); 

}

/** 
 * Creates a new image wrapper object.
 * 
 * The underlying OpenCL image object is created using the
 * clCreateImage2D() and clCreateImage3D() if the platform's OpenCL 
 * version is 1.1 or lower, or the clCreateImage() function otherwise.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] ctx A context wrapper object on which the image wrapper
 * object is to be created.
 * @param[in] flags Specifies allocation and usage information about the
 * image wrapper object being created.
 * @param[in] image_format A pointer to the OpenCL cl_image_format
 * structure, which describes format properties of the image to be
 * allocated.
 * @param[in] img_dsc A pointer to a ::CCLImageDesc object which 
 * describes type and dimensions of the image to be allocated.
 * @param[in] host_ptr A pointer to the image data that may already be 
 * allocated by the application.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A new image wrapper object or `NULL` if an error occurs.
 * */
CCLImage* ccl_image_new(CCLContext* ctx, cl_mem_flags flags,
	const cl_image_format* image_format, const CCLImageDesc* img_dsc,
	void* host_ptr, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure img_dsc is not NULL. */
	g_return_val_if_fail(img_dsc != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Image wrapper object. */
	CCLImage* img = NULL;
	/* OpenCL image object. */
	cl_mem image;
	/* Internal error handling object. */
	GError* err_internal = NULL;
	
#ifdef CL_VERSION_1_2

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL platform version. */
	double ocl_ver;
	
	/* Get OpenCL platform version. */
	ocl_ver = ccl_context_get_opencl_version(ctx, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Create image depending on OpenCL platform version.*/
	if (ocl_ver >= 1.2) {
		/* OpenCL is >= 1.2, use "new" API. */

		/* OpenCL image descriptor. Initialize it with data from 
		 * img_dsc (CCLImageDesc), unwrapping the wrapped buffer, if
		 * any. */
		cl_mem memory_object = (img_dsc->mo != NULL) 
			? ccl_memobj_unwrap(img_dsc->mo) : NULL;
		const cl_image_desc image_desc = { 
			.image_type = img_dsc->image_type,
			.image_width = img_dsc->image_width, 
			.image_height = img_dsc->image_height, 
			.image_depth = img_dsc->image_depth,
			.image_array_size = img_dsc->image_array_size, 
			.image_row_pitch = img_dsc->image_row_pitch,
			.image_slice_pitch = img_dsc->image_slice_pitch, 
			.num_mip_levels = img_dsc->num_mip_levels, 
			.num_samples = img_dsc->num_samples,
#ifdef CL_VERSION_2_0
			.mem_object = memory_object 
#else
			.buffer = memory_object 
#endif
		};
		
		/* Create image. */
		image = clCreateImage(ccl_context_unwrap(ctx), flags, 
			image_format, &image_desc, host_ptr, &ocl_status);
		ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
			CL_SUCCESS != ocl_status, ocl_status, error_handler,
			"%s: unable to create image with clCreateImage() " \
			"(OpenCL error %d: %s).",
			G_STRLOC, ocl_status, ccl_err(ocl_status));

	} else {
		/* OpenCL is < 1.2, use "old" API. */
		
		image = ccl_image_new_deprecated(ctx, flags, image_format, 
			img_dsc, host_ptr, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
		
	}
	
#else

	/* Create image. */
	image = ccl_image_new_deprecated(ctx, flags, image_format, 
		img_dsc, host_ptr, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

#endif

	/* Wrap image. */
	img = ccl_image_new_wrap(image);
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
finish:

	/* Return image wrapper. */
	return img;

}


/** @} */

