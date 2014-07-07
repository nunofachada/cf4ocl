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
 * @brief Kernel argument wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "arg.h"

CL4Arg* cl4_arg_new(void* value, size_t size, cl_bool dup) {

	CL4Arg* arg = g_slice_new(CL4Arg);
	
	arg->dup = dup;
	arg->size = size;
	arg->value = dup ? g_memdup((const void*) value, size) : value;
	
	return arg;
	
}

void cl4_arg_destroy(CL4Arg* arg) {

	/* Make sure arg is not NULL. */
	g_return_if_fail(arg != NULL);

	if (arg->dup) g_free(arg->value);
	g_slice_free(CL4Arg, arg);
}
