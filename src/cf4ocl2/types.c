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
 * Implementation of a class and methods to use and pass OpenCL types as
 * function arguments.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "types.h"

/**
 * Information about an OpenCL type.
 * */
struct ccl_type_info {

	/**
	 * Type name.
	 * @private
	 * */
	const char* name;

	/**
	 * Type size in bytes.
	 * @private
	 * */
	const int size;
};

/* Relation between OpenCL type names and sizes in bytes. */
static const CCLTypeInfo ccl_types[] = {

	{"char",   1}, /* CCL_CHAR   = 0 */
	{"uchar",  1}, /* CCL_UCHAR  = 1 */
	{"short",  2}, /* CCL_SHORT  = 2 */
	{"ushort", 2}, /* CCL_USHORT = 3 */
	{"int",    4}, /* CCL_INT    = 4 */
	{"uint",   4}, /* CCL_UINT   = 5 */
	{"long",   8}, /* CCL_LONG   = 6 */
	{"ulong",  8}, /* CCL_ULONG  = 7 */
	{"half",   2}, /* CCL_HALF   = 8 */
	{"float",  4}, /* CCL_FLOAT  = 9 */
	{"double", 8}  /* CCL_DOUBLE = 10 */
};

/**
 * @addtogroup TYPES
 * @{
 */

/**
 * Return OpenCL type name.
 *
 * @param[in] type Type constant.
 * @return A string containing the OpenCL type name.
 * */
const char* ccl_type_name_get(CCLType type) {
	g_assert_cmpint(type, <=, CCL_DOUBLE);
	g_assert_cmpint(type, >=, CCL_CHAR);
	return ccl_types[type].name;
}

/**
 * Return OpenCL type size in bytes.
 *
 * @param[in] type Type constant.
 * @return The size of the OpenCL type in bytes.
 * */
int ccl_type_sizeof(CCLType type) {
	g_assert_cmpint(type, <=, CCL_DOUBLE);
	g_assert_cmpint(type, >=, CCL_CHAR);
	return ccl_types[type].size;
}

/** @} */
