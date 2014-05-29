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

#ifndef CL4_TYPES_H
#define CL4_TYPES_H

#include <glib.h>
#include "common.h"

typedef enum {
	CL4_CHAR = 0,
	CL4_UCHAR = 1,
	CL4_INT = 2,
	CL4_UINT = 3,
	CL4_LONG = 4,
	CL4_ULONG = 5,
	CL4_HALF = 6,
	CL4_FLOAT = 7,
	CL4_DOUBLE = 8
} CL4Type;

typedef struct cl4_type_info CL4TypeInfo;

/** @brief Return OpenCL type name. */
const char* cl4_type_name_get(CL4Type type);

/** @brief Return OpenCL type size in bytes. */
int cl4_type_sizeof(CL4Type type);

#endif

