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
 * OpenCL enqueue API.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

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
	ocl_stub_create_event(event, command_queue, CL_COMMAND_NDRANGE_KERNEL);
	
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
	ocl_stub_create_event(event, command_queue, CL_COMMAND_READ_BUFFER);

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
	ocl_stub_create_event(event, command_queue, CL_COMMAND_WRITE_BUFFER);

	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue, 
	cl_mem src_buffer, cl_mem dst_image, size_t src_offset,
	const size_t* dst_origin, const size_t* region, 
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {
		
	command_queue = command_queue;
	src_buffer = src_buffer;
	dst_image = dst_image;
	src_offset = src_offset;
	dst_origin = dst_origin;
	region = region;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER_TO_IMAGE);

	return CL_SUCCESS;		

}

CL_API_ENTRY void* CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event,
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
		
	command_queue = command_queue;
	buffer = buffer;
	blocking_map = blocking_map;
	map_flags = map_flags;
	offset = offset;
	size = size;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_MAP_BUFFER);
	seterrcode(errcode_ret, CL_SUCCESS);
	return NULL;		

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer,
	cl_mem dst_buffer, size_t src_offset, size_t dst_offset, 
	size_t size, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_0 {

	command_queue = command_queue;
	src_buffer = src_buffer;
	dst_buffer = dst_buffer;
	src_offset = src_offset;
	dst_offset = dst_offset;
	size = size;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER);
	return CL_SUCCESS;		

}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj,
	void* mapped_ptr, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_0 {
		
	command_queue = command_queue;
	memobj = memobj;
	mapped_ptr = mapped_ptr;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_UNMAP_MEM_OBJECT);
	
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue command_queue,
	cl_uint num_mem_objects, const cl_mem* mem_objects, 
	cl_mem_migration_flags flags, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_2 {

	command_queue = command_queue;
	num_mem_objects = num_mem_objects;
	mem_objects = mem_objects;
	flags = flags;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_MIGRATE_MEM_OBJECTS);
	
	return CL_SUCCESS;
}
