/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 * Utilities used by the OpenCL testing stub.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#ifndef _CCL_OCL_STUB_UTILS_H_
#define _CCL_OCL_STUB_UTILS_H_

#include "ocl_impl.h"

guint veclen(void* vector, size_t elem_size);

void ocl_stub_create_event(
	cl_event* event, cl_command_queue queue, cl_command_type ctype);

#define seterrcode(errcode_ret, errcode) \
	if ((errcode_ret) != NULL) *(errcode_ret) = (errcode)

#define ccl_test_char_info(object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = strlen(object->info) + 1; \
		} \
	} else if (param_value_size < strlen(object->info) + 1) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, \
			strlen(object->info) + 1); \
	} \
	break;


#define ccl_test_vector_info(type, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = \
				sizeof(type) * veclen((void*) object->info, sizeof(type)); \
		} \
	} else if (param_value_size < sizeof(type) * veclen((void*) object->info, sizeof(type))) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, \
			sizeof(type) * veclen((void*) object->info, sizeof(type))); \
	} \
	break;

#define ccl_test_predefvector2d_info(type, qty, object, sizes, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = sizeof(type*) * qty; \
		} \
	} else if (param_value_size < sizeof(type*) * qty) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		for (cl_uint __i = 0; __i < qty; ++__i) \
			g_memmove(((type**) param_value)[__i], \
				object->info[__i], sizeof(type) * object->sizes[__i]); \
	} \
	break;

#define ccl_test_predefvector_info(type, qty, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = sizeof(type) * qty; \
		} \
	} else if (param_value_size < sizeof(type) * qty) { \
		status = CL_INVALID_VALUE; \
	} else if (object->info == NULL) { \
		status = CL_INVALID_VALUE; \
	} else { \
		g_memmove(param_value, object->info, sizeof(type) * qty); \
	} \
	break;

#define ccl_test_basic_info(type, object, info) \
	if (param_value == NULL) { \
		if (param_value_size_ret != NULL) { \
			*param_value_size_ret = sizeof(type); \
		} \
	} else if (param_value_size < sizeof(type)) { \
		status = CL_INVALID_VALUE; \
	} else { \
		*((type*) param_value) = object->info; \
	} \
	break;

#endif
