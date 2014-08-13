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
 * Definition of a wrapper class and its methods for OpenCL image
 * objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_IMAGE_WRAPPER_H_
#define _CCL_IMAGE_WRAPPER_H_

#include "memobj_wrapper.h"

/**
 * @defgroup IMAGE_WRAPPER Image wrapper
 *
 * A wrapper for handling images in OpenCL.
 * 
 * @todo Detailed description of module with code examples.
 * 
 * @{
 */

/** 
 * Image wrapper class
 * 
 * @extends ccl_memobj
 * */
typedef struct ccl_image CCLImage;

/**
 * Describes the type and dimensions of the image or image array
 * independently of the OpenCL version. Should be initialized with the
 * ccl_image_desc_init() macro.
 * */
typedef struct ccl_image_desc {
	
	/** 
	 * The type of image, must be supported by the underlying OpenCL
	 * version. Only `CL_MEM_OBJECT_IMAGE2D`and `CL_MEM_OBJECT_IMAGE3D`
	 * are supported in all OpenCL versions. 
	 * @public
	 * */
	cl_mem_object_type image_type;
	
	/** 
	 * Width of the image in pixels. Used for all image types.
	 * @public
	 * */
	size_t image_width;
	
	/** 
	 * Height of the image in pixels. Used for 2D and 3D images and
	 * 2D image arrays.
	 * @public
	 * */
	size_t image_height;
	
	/** 
	 * Depth of the image in pixels. Used only for 3D images.
	 * @public
	 * */
	size_t image_depth;
	
	/** 
	 * Number of images in the image array. Used for 1D and 2D image
	 * arrays.
	 * @public
	 * */
	size_t image_array_size;
	
	/** 
	 * Scan-line pitch in bytes. Must be 0 if `host_ptr` is `NULL`.
	 * @public
	 * */
	size_t image_row_pitch;
	
	/** 
	 * Size in bytes of each 2D slice in the 3D image or the size in 
	 * bytes of each image in a 1D or 2D image array. Must be 0 if 
	 * `host_ptr` is `NULL`.
	 * @public
	 * */
	size_t image_slice_pitch;
	
	/** 
	 * Unused up to OpenCL 2.0.
	 * @public
	 * */
	cl_uint num_mip_levels;
	
	/** 
	 * Unused up to OpenCL 2.0.
	 * @public
	 * */
	cl_uint num_samples;
	
	/** 
	 * A memory object wrapper. In OpenCL 1.2, it refers to a buffer
	 * wrapper and is used for 1D image buffers. In OpenCL 2.0 it can 
	 * also be used with 2D images to create a new 2D image from data 
	 * shared with the specified memory object (either a buffer or a
	 * 2D image). 
	 * @public
	 * */
	CCLMemObj* mo;

} CCLImageDesc;

/**
 * This macro initializes a ::CCLImageDesc object. Usage:
 * @code{.c}
 * CCLImageDesc img_dsc = ccl_image_desc_init();
 * @endcode
 * */
#define ccl_image_desc_init() {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL}

/* Get the image wrapper for the given OpenCL image. */
CCLImage* ccl_image_new_wrap(cl_mem mem_object);

/* Decrements the reference count of the wrapper object. If it 
 * reaches 0, the wrapper object is destroyed. */
void ccl_image_destroy(CCLImage* img);

/* Creates a new image wrapper object. */
CCLImage* ccl_image_new(CCLContext* ctx, cl_mem_flags flags,
	const cl_image_format* image_format, const CCLImageDesc* img_dsc,
	void* host_ptr, GError** err);

#ifdef CL_VERSION_1_1

/// @todo

#endif

#ifdef CL_VERSION_1_2

/// @todo

#endif

/// @todo We can emulate the OpenCL>1.0 functions if not available in
/// OpenCL

/** @} */

#endif




