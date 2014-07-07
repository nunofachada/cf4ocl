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

#include "common.h"

typedef struct cl4_arg {
	cl_bool dup;
	size_t size;
	void* value;
} CL4Arg;

#define cl4_arg_private(value, type) \
	cl4_arg_new(value, sizeof(type), CL_TRUE)

#define cl4_arg_local(count, type) \
	cl4_arg_new(NULL, count * sizeof(type), CL_FALSE)
	
#define cl4_arg_memobj(value) \
	cl4_arg_new(value, sizeof(cl_mem), CL_FALSE)
	
#define cl4_arg_sampler(value) \
	cl4_arg_new(value, sizeof(cl_sampler), CL_FALSE)
	
CL4Arg* cl4_arg_new(void* value, size_t size, cl_bool dup);

void cl4_arg_destroy(CL4Arg* arg);
