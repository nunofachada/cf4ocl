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
 * @brief OpenCL enqueue API.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel,
	cl_uint work_dim, const size_t* global_work_offset, 
	const size_t* global_work_size, const size_t* local_work_size,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {

	command_queue = command_queue;
	kernel = kernel;
	work_dim = work_dim;
	global_work_offset = global_work_offset;
	global_work_size = global_work_size;
	local_work_size = local_work_size;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	event = event;
	
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_read, size_t offset, size_t size, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {

	command_queue = command_queue;
	buffer = buffer;
	blocking_read = blocking_read;
	offset = offset;
	size = size;
	ptr = ptr;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	event = event;
	return CL_SUCCESS;
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, 
	cl_bool blocking_write, size_t offset, size_t size, const void* ptr, 
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, 
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {

	command_queue = command_queue;
	buffer = buffer;
	blocking_write = blocking_write;
	offset = offset;
	size = size;
	ptr = ptr;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	event = event;
	return CL_SUCCESS;		
}
