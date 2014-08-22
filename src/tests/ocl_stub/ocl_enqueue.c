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

	/* These are ignored. */
	command_queue = command_queue;
	kernel = kernel;
	work_dim = work_dim;
	global_work_offset = global_work_offset;
	global_work_size = global_work_size;
	local_work_size = local_work_size;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_NDRANGE_KERNEL);
	
	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_read, size_t offset, size_t size, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {
		
	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (buffer == NULL) {
		return CL_INVALID_MEM_OBJECT;
	} else if (buffer->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (ptr == NULL) {
		return CL_INVALID_VALUE;
	} else if (offset + size > buffer->size) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	blocking_read = blocking_read;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_READ_BUFFER);

	/* Read buffer. */
	g_memmove(ptr, buffer->mem + offset, size);

	/* All good. */
	return CL_SUCCESS;
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer, 
	cl_bool blocking_write, size_t offset, size_t size, const void* ptr, 
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, 
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (buffer == NULL) {
		return CL_INVALID_MEM_OBJECT;
	} else if (buffer->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (ptr == NULL) {
		return CL_INVALID_VALUE;
	} else if (offset + size > buffer->size) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	blocking_write = blocking_write;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_WRITE_BUFFER);

	/* Write to buffer. */
	g_memmove(buffer->mem + offset, ptr, size);

	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue, 
	cl_mem src_buffer, cl_mem dst_image, size_t src_offset,
	const size_t* dst_origin, const size_t* region, 
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_0 {
		
	/* These are ignored. */
	command_queue = command_queue;
	src_buffer = src_buffer;
	dst_image = dst_image;
	src_offset = src_offset;
	dst_origin = dst_origin;
	region = region;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER_TO_IMAGE);

	/* Unimplemented. */
	g_assert_not_reached();
	
	return CL_SUCCESS;

}

CL_API_ENTRY void* CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event,
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 {

	void* map_ptr = NULL;
	
	/* Error check. */
	if (command_queue == NULL) {
		seterrcode(errcode_ret, CL_INVALID_COMMAND_QUEUE);
	} else if (buffer == NULL) {
		seterrcode(errcode_ret, CL_INVALID_MEM_OBJECT);
	} else if (buffer->context != command_queue->context) {
		seterrcode(errcode_ret, CL_INVALID_CONTEXT);
		/* Not testing if events in wait list belong to this context. */
	} else if (offset + size > buffer->size) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
	} else {

		/* Set event. */
		ocl_stub_create_event(event, command_queue, CL_COMMAND_MAP_BUFFER);
		seterrcode(errcode_ret, CL_SUCCESS);
		
		/* Just return a pointer to the memory region. */
		map_ptr = buffer->mem + offset;
		buffer->map_count++;
	}

	/* These are ignored. */
	blocking_map = blocking_map;
	map_flags = map_flags;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	/* Return the mapped pointer. */
	return map_ptr;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer,
	cl_mem dst_buffer, size_t src_offset, size_t dst_offset, 
	size_t size, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_0 {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if ((src_buffer == NULL) || (dst_buffer == NULL)) {
		return CL_INVALID_MEM_OBJECT;
	} else if ((src_buffer->context != command_queue->context)
		|| (dst_buffer->context != command_queue->context)) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if ((src_offset + size > src_buffer->size)
		|| (dst_offset + size > dst_buffer->size)) {
		return CL_INVALID_VALUE;
	} else if (src_buffer == dst_buffer) {
		/* For now just don't allow copies within the same buffer,
		 * although OCL allows it if they don't overlap. */
		return CL_INVALID_VALUE;
	}
	
	/* These are ignored. */
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	/* Perform copy. */
	g_memmove(dst_buffer->mem + dst_offset, src_buffer->mem + src_offset, size);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER);

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj,
	void* mapped_ptr, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_0 {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (memobj == NULL) {
		return CL_INVALID_MEM_OBJECT;
	} else if (memobj->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (mapped_ptr == NULL) {
		return CL_INVALID_VALUE;
	} else if ((mapped_ptr < memobj->mem) 
		|| (mapped_ptr >= memobj->mem + memobj->size) 
		|| (memobj->map_count == 0)) {
		return CL_INVALID_VALUE;
	}
	
	/* These are ignored. */
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_UNMAP_MEM_OBJECT);
	
	/* Decrement map count. */
	memobj->map_count--;
	
	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarker(cl_command_queue command_queue, cl_event *event) 
	CL_API_SUFFIX__VERSION_1_0 {

	ocl_stub_create_event(event, command_queue, CL_COMMAND_MARKER);
	return CL_SUCCESS;

}
    
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWaitForEvents(cl_command_queue command_queue,
	cl_uint num_events, const cl_event* event_list) 
	CL_API_SUFFIX__VERSION_1_0 {

	/* These are ignored. */
	command_queue = command_queue;
	num_events = num_events;
	event_list = event_list;
	
	return CL_SUCCESS;

}
    
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrier(cl_command_queue command_queue) 
	CL_API_SUFFIX__VERSION_1_0 {
	
	command_queue = command_queue;
	
	return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_read, const size_t* buffer_offset, 
	const size_t* host_offset, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	size_t host_row_pitch, size_t host_slice_pitch, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_1 {

	/* Unimplemented. */
	g_assert_not_reached();
	
	command_queue = command_queue;
	buffer = buffer;
	blocking_read = blocking_read;
	buffer_offset = buffer_offset;
	host_offset = host_offset;
	region = region;
	buffer_row_pitch = buffer_row_pitch;
	buffer_slice_pitch = buffer_slice_pitch;
	host_row_pitch = host_row_pitch;
	host_slice_pitch = host_slice_pitch;
	ptr = ptr;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
		
	ocl_stub_create_event(event, command_queue, CL_COMMAND_READ_BUFFER_RECT);
	return CL_SUCCESS;
}

                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_write, const size_t* buffer_offset,
	const size_t* host_offset, const size_t* region, 
	size_t buffer_row_pitch, size_t buffer_slice_pitch,
	size_t host_row_pitch, size_t host_slice_pitch, const void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_1 {

	/* Unimplemented. */
	g_assert_not_reached();
	
	command_queue = command_queue;
	buffer = buffer;
	blocking_write = blocking_write;
	buffer_offset = buffer_offset;
	host_offset = host_offset;
	region = region;
	buffer_row_pitch = buffer_row_pitch;
	buffer_slice_pitch = buffer_slice_pitch;
	host_row_pitch = host_row_pitch;
	host_slice_pitch = host_slice_pitch;
	ptr = ptr;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	ocl_stub_create_event(event, command_queue, CL_COMMAND_WRITE_BUFFER_RECT);
	return CL_SUCCESS;
}
                            
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue command_queue, 
	cl_mem src_buffer, cl_mem dst_buffer, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region, 
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
	size_t dst_slice_pitch, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_1 {

	/* Unimplemented. */
	g_assert_not_reached();
	
	command_queue = command_queue;
	src_buffer = src_buffer;
	dst_buffer = dst_buffer;
	src_origin = src_origin;
	dst_origin = dst_origin;
	region = region;
	src_row_pitch = src_row_pitch;
	src_slice_pitch = src_slice_pitch;
	dst_row_pitch = dst_row_pitch;
	dst_slice_pitch = dst_slice_pitch;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER_RECT);
	return CL_SUCCESS;
	
}

#endif

#ifdef CL_VERSION_1_2

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue command_queue,
	cl_uint num_mem_objects, const cl_mem* mem_objects, 
	cl_mem_migration_flags flags, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_2 {

	/* Unimplemented. */
	g_assert_not_reached();
	
	command_queue = command_queue;
	num_mem_objects = num_mem_objects;
	mem_objects = mem_objects;
	flags = flags;
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;
	ocl_stub_create_event(event, command_queue, CL_COMMAND_MIGRATE_MEM_OBJECTS);
	
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue command_queue, cl_mem buffer, 
	const void* pattern, size_t pattern_size, size_t offset, 
	size_t size, cl_uint num_events_in_wait_list, 
	const cl_event* event_wait_list, cl_event* event) 
	CL_API_SUFFIX__VERSION_1_2 {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (buffer == NULL) {
		return CL_INVALID_MEM_OBJECT;
	} else if (buffer->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (offset + size > buffer->size) {
		return CL_INVALID_VALUE;
	} else if ((pattern == NULL) || (pattern_size == 0)
		|| ((pattern_size&(pattern_size-1)) != 0) || (pattern_size > 128)) {
		return CL_INVALID_VALUE;
	} else if ((offset % pattern_size != 0) || (size % pattern_size != 0)) {
		return CL_INVALID_VALUE;
	}
	
	/* These are ignored. */
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_FILL_BUFFER);
	
	/* Fill buffer. */
	for (guint i = 0; i < size; i += pattern_size) {
		g_memmove(buffer->mem + offset + i, pattern, pattern_size);
	}
	
	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_2 {

	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	ocl_stub_create_event(event, command_queue, CL_COMMAND_MARKER);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) CL_API_SUFFIX__VERSION_1_2 {
		
	num_events_in_wait_list = num_events_in_wait_list;
	event_wait_list = event_wait_list;

	ocl_stub_create_event(event, command_queue, CL_COMMAND_BARRIER);
	return CL_SUCCESS;

}

#endif
