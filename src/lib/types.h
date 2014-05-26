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

#ifndef CF4_TYPES_H
#define CF4_TYPES_H

#include <glib.h>

typedef enum {
	CF4_CHAR = 0,
	CF4_UCHAR = 1,
	CF4_INT = 2,
	CF4_UINT = 3,
	CF4_LONG = 4,
	CF4_ULONG = 5,
	CF4_HALF = 6,
	CF4_FLOAT = 7,
	CF4_DOUBLE = 8
} CF4CLType;

typedef struct cl4_type_info CF4CLTypeInfo;

/** @brief Return OpenCL type name. */
const char* cl4_type_name_get(CF4CLType type);

/** @brief Return OpenCL type size in bytes. */
int cl4_type_sizeof(CF4CLType type);

#endif

