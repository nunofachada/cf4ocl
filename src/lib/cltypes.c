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
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "cltypes.h"

struct cf4_type_info {
	const char* name;
	const int size;
};

static const CF4CLTypeInfo cf4_types[] = {
	
	{"char",   1}, /* CF4_CHAR   = 0 */
	{"uchar",  1}, /* CF4_UCHAR  = 1 */
	{"short",  2}, /* CF4_SHORT  = 2 */
	{"ushort", 2}, /* CF4_USHORT = 3 */
	{"int",    4}, /* CF4_INT    = 4 */
	{"uint",   4}, /* CF4_UINT   = 5 */
	{"long",   8}, /* CF4_LONG   = 6 */
	{"ulong",  8}, /* CF4_ULONG  = 7 */
	{"half",   2}, /* CF4_HALF   = 8 */
	{"float",  4}, /* CF4_FLOAT  = 9 */
	{"double", 8}  /* CF4_DOUBLE = 10 */
};

/** 
 * @brief Return OpenCL type name.
 * 
 * @param type Type constant.
 * @return A string containing the OpenCL type name.
 * */
const char* cf4_type_name_get(CF4CLType type) {
	g_assert_cmpint(type, <=, CF4_DOUBLE);
	g_assert_cmpint(type, >=, CF4_CHAR);
	return cf4_types[type].name;
}

/** 
 * @brief Return OpenCL type size in bytes.
 * 
 * @param type Type constant.
 * @return The size of the OpenCL type in bytes.
 * */
int cf4_type_sizeof(CF4CLType type) {
	g_assert_cmpint(type, <=, CF4_DOUBLE);
	g_assert_cmpint(type, >=, CF4_CHAR);
	return cf4_types[type].size;
}
