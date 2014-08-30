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
 * Definition of a class which represents the list of OpenCL
 * platforms available in the system and respective methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_PLATFORMS_H_
#define _CCL_PLATFORMS_H_

#include "common.h"
#include "platform_wrapper.h"
#include "errors.h"

/**
 * @defgroup PLATFORMS Platforms
 *
 * The platforms module provides functionality for managing the OpenCL
 * platforms available in the system.
 *
 * The ::ccl_platforms_new() function returns the set of platforms
 * available in the system. The ::ccl_platforms_count() can be used to
 * get the number of platforms in that set, while the
 * ::ccl_platforms_get() will return the @f$i^{th}@f$ platform in the
 * system.
 *
 *  _Example:_
 *
 * @dontinclude list.c
 * @skipline cf4ocl objects
 * @until CCLPlatform* platf;
 *
 * @skipline Other variables
 * @until cl_uint num_platfs;
 * @skipline GError* err = NULL;
 *
 * @skipline Get all platforms in system
 * @until platf_set = ccl_platforms_new(&err);
 *
 * @skipline Get number of platforms in system
 * @until num_platfs = ccl_platforms_count(platf_set);
 *
 * @skipline Cycle through platforms
 * @until platf = ccl_platforms_get(platf_set, i);
 *
 * @skipline Cycle platforms
 * @until ccl_platforms_destroy
 * @{
 */

/**
 * Class which represents the OpenCL platforms available
 * in the system.
 */
typedef struct ccl_platforms CCLPlatforms;

/* Creates a new CCLPlatforms* object, which contains the list
 * of OpenCL platforms available in the system. */
CCLPlatforms* ccl_platforms_new(GError **err);

/* Destroy a CCLPlatforms* object, including all underlying
 * platforms, devices and data. */
void ccl_platforms_destroy(CCLPlatforms* platforms);

/* Return number of OpenCL platforms found in CCLPlatforms*
 * object. */
cl_uint ccl_platforms_count(CCLPlatforms* platforms);

/* Get CCL platform wrapper object at given index. */
CCLPlatform* ccl_platforms_get(
	CCLPlatforms* platforms, cl_uint index);

/** @} */

#endif
