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

#define cl4_arg_is_local(arg) \
	 (arg->info == (void*) &arg_local_marker)
	 
static char arg_local_marker;

CL4Arg* cl4_arg_new(void* value, size_t size) {

	CL4Arg* arg = g_slice_new(CL4Arg);
	
	arg->cl_object = g_memdup((const void*) value, size);
	arg->info = (void*) &arg_local_marker;
	arg->ref_count = (gint) size;
	
	return arg;
	
}

void cl4_arg_destroy(CL4Arg* arg) {

	/* Make sure arg is not NULL. */
	g_return_if_fail(arg != NULL);

	if cl4_arg_is_local(arg)
		g_free(arg->cl_object);
		
	g_slice_free(CL4Arg, arg);
}

size_t cl4_arg_size(CL4Arg* arg) {

	/* Make sure arg is not NULL. */
	g_return_if_fail(arg != NULL);

	return cl4_arg_is_local(arg)
		? (size_t) arg->ref_count
		: sizeof(void*);
}

void* cl4_arg_value(CL4Arg* arg) {

	/* Make sure arg is not NULL. */
	g_return_if_fail(arg != NULL);

	return cl4_arg_is_local(arg)
		? arg->cl_object
		: &arg->cl_object;
}
