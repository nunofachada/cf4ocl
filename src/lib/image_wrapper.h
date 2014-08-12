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

/// @todo

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




