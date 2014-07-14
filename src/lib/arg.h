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

#ifndef CL4_ARG_H
#define CL4_ARG_H 

#include "common.h"
#include "wrapper.h"

typedef CL4Wrapper CL4Arg;

#define cl4_arg_private(value, type) \
	cl4_arg_new(&value, sizeof(type))

#define cl4_arg_local(count, type) \
	cl4_arg_new(NULL, count * sizeof(type))
	
CL4Arg* cl4_arg_new(void* value, size_t size);

void cl4_arg_destroy(CL4Arg* arg);

size_t cl4_arg_size(CL4Arg* arg);

void* cl4_arg_value(CL4Arg* arg);

#endif
