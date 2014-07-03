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
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */
 
 /** 
 * @file
 * @brief OpenCL command queue stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

cl_command_queue clCreateCommandQueue(cl_context context,
	cl_device_id device, cl_command_queue_properties properties,
	cl_int* errcode_ret) {
		
	context = context; device = device; properties = properties;
	
	seterrcode(errcode_ret, CL_SUCCESS);
		
	cl_command_queue queue = 
		(cl_command_queue) malloc(sizeof(struct _cl_command_queue));
		
	queue->filler = 0;
	
	return queue;
		
}

cl_int clReleaseCommandQueue(cl_command_queue command_queue) {
	free(command_queue);
	return CL_SUCCESS;
}




