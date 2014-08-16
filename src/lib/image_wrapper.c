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
#include "buffer_wrapper.h"

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

#ifdef USE_GDKPIXBUF

/* Creates a new image wrapper object from an image file. */
CCLImage* ccl_image_new_from_file(CCLContext* ctx, const char* filename, 
	cl_mem_flags flags, GError** err) {
	
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure CL_MEM_USE_HOST_PTR and CL_MEM_ALLOC_HOST_PTR are not
	 * set in flags. */
	g_return_val_if_fail(((flags & CL_MEM_ALLOC_HOST_PTR) == 0)
		&& ((flags & CL_MEM_USE_HOST_PTR) == 0), NULL);

	/* A GDK pixel buffer where to load the image to. In OpenCL 
	 * this will be represented*/
	GdkPixbuf* buf;
	/* Internal error handling object. */
	GError* err_internal = NULL;
	/* Image wrapper object to create. */
	CCLImage* img = NULL;
	/* Image format. */
	cl_image_format image_format;
	/* Image description. */
	CCLImageDesc img_dsc;

	/* Load file into pixbuf. */
	buf = gdk_pixbuf_new_from_file(filename, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Make sure image has alpha channel. */
	if (!gdk_pixbuf_get_has_alpha(buf)) {
		
		/* Aux. GDK pixel buffer. */
		GdkPixbuf* buf_aux;
		/* Add alpha channel. */
		buf_aux = gdk_pixbuf_add_alpha(buf, FALSE, 0, 0, 0);
		/* Destroy original pixbuf. */
		g_object_unref(buf);
		/* Point buf to new pixbuf. */
		buf = buf_aux;
	}
	
	/* Set image format. */
	image_format = { 
		.image_channel_order = CL_RGBA,
		.image_channel_data_type = CL_UNSIGNED_INT8
	};
	
	/* Set image description. */
	img_dsc = {
		.image_type = CL_MEM_OBJECT_IMAGE2D,
		.image_width = gdk_pixbuf_get_width(buf),
		.image_height = gdk_pixbuf_get_height(buf),
		.image_depth = 0,
		.image_array_size = 0,
		.image_row_pitch = gdk_pixbuf_get_rowstride(buf),
		.image_slice_pitch = 0,
		.num_mip_levels = 0,
		.num_samples = 0,
		.mo = NULL
	};
	
	/* Create image wrapper object. */
	img = ccl_image_new(ctx, flags | CL_MEM_COPY_HOST_PTR, 
		&image_format, &img_dsc, (void*) gdk_pixbuf_get_pixels(buf), 
		&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Destroy pixbuf. */
	g_object_unref(buf);
	
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

/* Saves the image represented by the wrapper object into a file. */
cl_bool ccl_image_save_to_file(CCLImage* img, const char* filename, 
	GError** err) {

	/* Make sure img is not NULL. */
	g_return_val_if_fail(img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	/* Function return status. */
	cl_bool status;
	/* */
	
	/* Map image for reading. */
	ccl_image_enqueue_map(
	
	
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	status = CL_FALSE;
	
finish:

	/* Return function status. */
	return status;
}

#endif

/**
 * Read from an image or image array object to host memory. This 
 * function wraps the clEnqueueReadImage() OpenCL function.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] cq Command-queue wrapper object in which the read command
 * will be queued.
 * @param[in] img Image wrapper object where to read from.
 * @param[in] blocking_read Indicates if the read operation is 
 * blocking or non-blocking.
 * @param[in] origin The @f$(x, y, z)@f$ offset in pixels in the 1D, 2D,
 * or 3D image, the @f$(x, y)@f$ offset and the image index in the image
 * array or the @f$(x)@f$ offset and the image index in the 1D image 
 * array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array. 
 * @param[in] row_pitch The length of each row in bytes.
 * @param[in] slice_pitch Size in bytes of the 2D slice of the 3D
 * region of a 3D image or each image of a 1D or 2D image array being 
 * read.
 * @param[out] ptr A pointer to a buffer in host memory where data is to
 * be read into.
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this read command.
 * */
CCLEvent* ccl_image_enqueue_read(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_read, const size_t* origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure img is not NULL. */
	g_return_val_if_fail(img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	
	/* Read image from device into host. */
	ocl_status = clEnqueueReadImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(img), blocking_read, origin, region, 
		row_pitch, slice_pitch, ptr,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to enqueue an image read (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

/**
 * Write to an image or image array object from host memory. This 
 * function wraps the clEnqueueWriteImage() OpenCL function.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] cq Command-queue wrapper object in which the write command
 * will be queued.
 * @param[out] img Image wrapper object where to write to.
 * @param[in] blocking_write Indicates if the write operation is
 * blocking or non-blocking.
 * @param[in] origin The @f$(x, y, z)@f$ offset in pixels in the 1D, 2D,
 * or 3D image, the @f$(x, y)@f$ offset and the image index in the image
 * array or the @f$(x)@f$ offset and the image index in the 1D image 
 * array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array. 
 * @param[in] input_row_pitch The length of each row in bytes.
 * @param[in] input_slice_pitch Size in bytes of the 2D slice of the 3D
 * region of a 3D image or each image of a 1D or 2D image array being 
 * written.
 * @param[in] ptr A pointer to a buffer in host memory where data is to
 * be written from.
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this write command.
 * */
CCLEvent* ccl_image_enqueue_write(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_write, const size_t* origin, const size_t* region, 
	size_t input_row_pitch, size_t input_slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure img is not NULL. */
	g_return_val_if_fail(img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	
	/* Write image to device from host. */
	ocl_status = clEnqueueWriteImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(img), blocking_write, origin, region, 
		input_row_pitch, input_slice_pitch, ptr,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to enqueue an image write (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

/**
 * Copy image objects. This function wraps the clEnqueueCopyImage() 
 * OpenCL function.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] cq Command-queue wrapper object in which the copy command
 * will be queued.
 * @param[in] src_img Source image wrapper object.
 * @param[out] dst_img Destination image wrapper object.
 * @param[in] src_origin The @f$(x, y, z)@f$ offset in pixels in the 1D,
 * 2D, or 3D image source image, the @f$(x, y)@f$ offset and the image 
 * index in the source image array or the @f$(x)@f$ offset and the image
 * index in the source 1D image array.
 * @param[in] dst_origin The @f$(x, y, z)@f$ offset in pixels in the 1D,
 * 2D, or 3D destination image, the @f$(x, y)@f$ offset and the image 
 * index in the destination image array or the @f$(x)@f$ offset and the 
 * image index in the 1D destination image array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array. 
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this copy command.
 * */
CCLEvent* ccl_image_enqueue_copy(CCLQueue* cq, CCLImage* src_img,
	CCLImage* dst_img, const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region, 
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure src_img is not NULL. */
	g_return_val_if_fail(src_img != NULL, NULL);
	/* Make sure dst_img is not NULL. */
	g_return_val_if_fail(dst_img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	
	/* Copy image. */
	ocl_status = clEnqueueCopyImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(src_img), ccl_memobj_unwrap(dst_img),
		src_origin, dst_origin, region, 
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to enqueue an image copy (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

/**
 * Copy an image object to a buffer object. This function wraps the 
 * clEnqueueCopyImageToBuffer() OpenCL function.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] cq Command-queue wrapper object in which the copy command
 * will be queued.
 * @param[in] src_img Source image wrapper object where to read from.
 * @param[out] dst_buf Destination buffer wrapper object where to write
 * to.
 * @param[in] src_origin The @f$(x, y, z)@f$ offset in pixels in
 * the 1D, 2D or 3D image, the @f$(x, y)@f$ offset and the image index 
 * in the 2D image array or the @f$(x)@f$ offset and the image index in
 * the 1D image array. 
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array.
 * @param[in] dst_offset The offset where to begin copying data into 
 * `dst_buf`.
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this copy command.
 * */
CCLEvent* ccl_image_enqueue_copy_to_buffer(CCLQueue* cq, 
	CCLImage* src_img, CCLBuffer* dst_buf, const size_t *src_origin, 
	const size_t *region, size_t dst_offset,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure src_img is not NULL. */
	g_return_val_if_fail(src_img != NULL, NULL);
	/* Make sure dst_buf is not NULL. */
	g_return_val_if_fail(dst_buf != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	
	/* Copy image to buffer. */
	ocl_status = clEnqueueCopyImageToBuffer(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(src_img), ccl_memobj_unwrap(dst_buf),
		src_origin, region, dst_offset, 
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to copy image to buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

/**
 * Map a region of the image into the host address space and return a 
 * pointer to this mapped region. This function wraps the 
 * clEnqueueMapImage() OpenCL function.
 * 
 * @public @memberof ccl_image
 * 
 * @param[in] cq Command-queue wrapper object in which the map command
 * will be queued.
 * @param[in,out] img Image wrapper object to be mapped.
 * @param[in] blocking_map Indicates if the map operation is blocking 
 * or non-blocking.
 * @param[in] map_flags Flags which specify the type of mapping to
 * perform.
 * @param[in] origin The @f$(x, y, z)@f$ offset in pixels in the 1D, 2D,
 * or 3D image, the @f$(x, y)@f$ offset and the image index in the image
 * array or the @f$(x)@f$ offset and the image index in the 1D image 
 * array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array.
 * @param[out] image_row_pitch Returns the scan-line pitch in bytes for
 * the mapped region. This must be a non-`NULL` value.
 * @param[out] image_slice_pitch Returns the size in bytes of each 2D 
 * slice of a 3D image or the size of each 1D or 2D image in a 1D or 2D
 * image array for the mapped region. For a 1D and 2D image, zero is 
 * returned if this argument is not `NULL`. For a 3D image, 1D, and 2D
 * image array, `image_slice_pitch` must be a non-`NULL` value.
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] evt An event wrapper object that identifies this 
 * particular map command. If NULL, no event will be returned.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return A pointer in the host address space for the mapped region.
 * */
void* ccl_image_enqueue_map(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin,
	const size_t* region, size_t *image_row_pitch, 
	size_t *image_slice_pitch, CCLEventWaitList* evt_wait_lst,
	CCLEvent** evt, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure img is not NULL. */
	g_return_val_if_fail(img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt_inner = NULL;
	void* ptr = NULL;
	
	/* Perform image map. */
	ptr = clEnqueueMapImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(img), blocking_map, map_flags, 
		origin, region, image_row_pitch, image_slice_pitch,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), 
		&event, &ocl_status);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR,
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to map image (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt_inner = ccl_queue_produce_event(cq, event);
	if (evt != NULL)
		*evt = evt_inner;
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	ptr = NULL;
	
finish:
	
	/* Return host pointer. */
	return ptr;

}

#ifdef CL_VERSION_1_2

/**
 * Fill an image object with a specified color. This function wraps the 
 * clEnqueueFillImage() OpenCL function.
 * 
 * @public @memberof ccl_image
 * @note Requires OpenCL >= 1.2
 * 
 * @param[in] cq Command-queue wrapper object in which the fill command
 * will be queued.
 * @param[out] img Image wrapper object to fill.
 * @param[in] fill_color The fill color.
 * @param[in] origin The @f$(x, y, z)@f$ offset in pixels in the 1D, 2D,
 * or 3D image, the @f$(x, y)@f$ offset and the image index in the image
 * array or the @f$(x)@f$ offset and the image index in the 1D image 
 * array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the 
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the 
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array.
 * @param[in,out] evt_wait_lst List of events that need to complete 
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a GError, or `NULL` if error 
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this fill command.
 * */
CCLEvent* ccl_image_enqueue_fill(CCLQueue* cq, CCLImage* img, 
	const void *fill_color, const size_t *origin, const size_t *region,
	CCLEventWaitList* evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure img is not NULL. */
	g_return_val_if_fail(img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* OpenCL function status. */
	cl_int ocl_status;
	/* OpenCL event object. */
	cl_event event = NULL;
	/* Event wrapper object. */
	CCLEvent* evt = NULL;
	/* OpenCL version of the underlying platform. */
	double ocl_ver;
	/* Internal error handling object. */
	GError* err_internal = NULL;
	
	/* Check that context platform is >= OpenCL 1.2 */
	ocl_ver = ccl_memobj_get_opencl_version(
		(CCLMemObj*) img, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If OpenCL version is not >= 1.2, throw error. */
	ccl_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 1.2, 
		CCL_ERROR_UNSUPPORTED_OCL, error_handler, 
		"%s: Image fill requires OpenCL version 1.2 or newer.", 
		G_STRLOC);
	
	/* Fill image. */
	ocl_status = clEnqueueFillImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(img), fill_color, origin, region,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	ccl_if_err_create_goto(*err, CCL_OCL_ERROR, 
		CL_SUCCESS != ocl_status, ocl_status, error_handler, 
		"%s: unable to enqueue a fill image command (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

#endif

/** @} */

