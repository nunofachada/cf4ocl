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
 * @brief Use and pass abstract OpenCL types in C functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_TYPES_H_
#define _CCL_TYPES_H_

#include <glib.h>
#include "common.h"

/**
 * @defgroup TYPES Types
 *
 * @brief Use and pass abstract OpenCL types in C functions.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

typedef enum {
	CCL_CHAR = 0,
	CCL_UCHAR = 1,
	CCL_INT = 2,
	CCL_UINT = 3,
	CCL_LONG = 4,
	CCL_ULONG = 5,
	CCL_HALF = 6,
	CCL_FLOAT = 7,
	CCL_DOUBLE = 8
} CCLType;

typedef struct ccl_type_info CCLTypeInfo;

/** @brief Return OpenCL type name. */
const char* ccl_type_name_get(CCLType type);

/** @brief Return OpenCL type size in bytes. */
int ccl_type_sizeof(CCLType type);

/** @} */

#endif

