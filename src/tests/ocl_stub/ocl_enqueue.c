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
	cl_event* event) {

	/* These are ignored. */
	(void)(command_queue);
	(void)(kernel);
	(void)(work_dim);
	(void)(global_work_offset);
	(void)(global_work_size);
	(void)(local_work_size);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_NDRANGE_KERNEL);

	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_read, size_t offset, size_t size, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

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
	(void)(blocking_read);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_READ_BUFFER);

	/* Read buffer. */
	g_memmove(ptr, ((cl_uchar*)buffer->mem) + offset, size);

	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_write, size_t offset, size_t size, const void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

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
	(void)(blocking_write);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_WRITE_BUFFER);

	/* Write to buffer. */
	g_memmove(((cl_uchar*) buffer->mem) + offset, ptr, size);

	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
	cl_mem src_buffer, cl_mem dst_image, size_t src_offset,
	const size_t* dst_origin, const size_t* region,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* These are ignored. */
	(void)(command_queue);
	(void)(src_buffer);
	(void)(dst_image);
	(void)(src_offset);
	(void)(dst_origin);
	(void)(region);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

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
	cl_int* errcode_ret) {

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
		map_ptr = ((cl_uchar*)buffer->mem) + offset;
		buffer->map_count++;
	}

	/* These are ignored. */
	(void)(blocking_map);
	(void)(map_flags);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Return the mapped pointer. */
	return map_ptr;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue command_queue, cl_mem src_buffer,
	cl_mem dst_buffer, size_t src_offset, size_t dst_offset,
	size_t size, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) {

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
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Perform copy. */
	g_memmove(((cl_uchar*) dst_buffer->mem) + dst_offset,
		((cl_uchar*) src_buffer->mem) + src_offset, size);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_BUFFER);

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj,
	void* mapped_ptr, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) {

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
		|| ((cl_uchar*)mapped_ptr >= ((cl_uchar*)memobj->mem) + memobj->size)
		|| (memobj->map_count == 0)) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_UNMAP_MEM_OBJECT);

	/* Decrement map count. */
	memobj->map_count--;

	/* All good. */
	return CL_SUCCESS;
}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue command_queue, cl_mem image,
	cl_bool blocking_read, const size_t* origin, const size_t* region,
	size_t row_pitch, size_t slice_pitch, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (image == NULL) { /* Not checking if memobject is image. */
		return CL_INVALID_MEM_OBJECT;
	} else if (image->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (ptr == NULL) {
		return CL_INVALID_VALUE;
	} else if ((origin[0] + region[0] > image->image_desc.image_width)
		|| (origin[1] + region[1] > image->image_desc.image_height)
		|| (origin[2] + region[2] > image->image_desc.image_depth)) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	(void)(blocking_read);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_READ_IMAGE);

	/* Determine effective row and slice pitches. */
	if (row_pitch == 0)
		row_pitch = image->image_elem_size * region[1];
	if (slice_pitch == 0)
		slice_pitch = row_pitch * region[2];

	/* Get image width and height to more readeable variables. */
	size_t w = image->image_desc.image_width;
	size_t h = image->image_desc.image_height;

	/* Read image. */
	for (size_t z = 0; z < region[2]; z++) {
		size_t slice_pitch_index = z * slice_pitch;
		for (size_t y = 0; y < region[1]; y++) {
			size_t row_pitch_index = y * row_pitch;
			g_memmove(
				((cl_uchar*)ptr) + slice_pitch_index + row_pitch_index,
				((cl_uchar*)image->mem) + ((z + origin[2]) * w * h + (y + origin[1]) * w + origin[0]) * image->image_elem_size,
				region[0] * image->image_elem_size);
		}
	}

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue command_queue, cl_mem image,
	cl_bool blocking_write, const size_t* origin, const size_t* region,
	size_t input_row_pitch, size_t input_slice_pitch, const void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (image == NULL) { /* Not checking if memobject is image. */
		return CL_INVALID_MEM_OBJECT;
	} else if (image->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (ptr == NULL) {
		return CL_INVALID_VALUE;
	} else if ((origin[0] + region[0] > image->image_desc.image_width)
		|| (origin[1] + region[1] > image->image_desc.image_height)
		|| (origin[2] + region[2] > image->image_desc.image_depth)) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	(void)(blocking_write);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_WRITE_IMAGE);

	/* Determine effective row and slice pitches. */
	if (input_row_pitch == 0)
		input_row_pitch = image->image_elem_size * region[1];
	if (input_slice_pitch == 0)
		input_slice_pitch = input_row_pitch * region[2];

	/* Get image width and height to more readeable variables. */
	size_t w = image->image_desc.image_width;
	size_t h = image->image_desc.image_height;

	/* Write image. */
	for (size_t z = 0; z < region[2]; z++) {
		size_t slice_pitch_index = z * input_slice_pitch;
		for (size_t y = 0; y < region[1]; y++) {
			size_t row_pitch_index = y * input_row_pitch;
			g_memmove(
				((cl_uchar*)image->mem) + ((z + origin[2]) * w * h + (y + origin[1]) * w + origin[0]) * image->image_elem_size,
				((cl_uchar*)ptr) + slice_pitch_index + row_pitch_index,
				region[0] * image->image_elem_size);
		}
	}

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image,
	cl_mem dst_image, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if ((src_image == NULL) || (dst_image == NULL)) { /* Not checking if memobject are images. */
		return CL_INVALID_MEM_OBJECT;
	} else if ((src_image->context != command_queue->context)
		|| (dst_image->context != command_queue->context)) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if ((src_image->image_format.image_channel_data_type != dst_image->image_format.image_channel_data_type)
		|| (src_image->image_format.image_channel_order != dst_image->image_format.image_channel_order)) {
		return CL_IMAGE_FORMAT_MISMATCH;
	} else if ((src_origin[0] + region[0] > src_image->image_desc.image_width)
		|| (src_origin[1] + region[1] > src_image->image_desc.image_height)
		|| (src_origin[2] + region[2] > src_image->image_desc.image_depth)
		|| (dst_origin[0] + region[0] > dst_image->image_desc.image_width)
		|| (dst_origin[1] + region[1] > dst_image->image_desc.image_height)
		|| (dst_origin[2] + region[2] > dst_image->image_desc.image_depth)) {
		return CL_INVALID_VALUE;
	}

	/* These are ignored. */
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_IMAGE);

	/* Get images width and height to more readeable variables. */
	size_t src_w = src_image->image_desc.image_width;
	size_t src_h = src_image->image_desc.image_height;
	size_t dst_w = dst_image->image_desc.image_width;
	size_t dst_h = dst_image->image_desc.image_height;

	/* Copy image. */
	for (size_t z = 0; z < region[2]; z++) {
		for (size_t y = 0; y < region[1]; y++) {
			g_memmove(
				((cl_uchar*)dst_image->mem) + ((z + dst_origin[2]) * dst_w * dst_h + (y + dst_origin[1]) * dst_w + dst_origin[0]) * dst_image->image_elem_size,
				((cl_uchar*)src_image->mem) + ((z + src_origin[2]) * src_w * src_h + (y + src_origin[1]) * src_w + src_origin[0]) * src_image->image_elem_size,
				region[0] * dst_image->image_elem_size);
		}
	}

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
	cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin,
	const size_t* region, size_t dst_offset,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* Unimplemented. */
	g_assert_not_reached();

	(void)(command_queue);
	(void)(src_image);
	(void)(dst_buffer);
	(void)(src_origin);
	(void)(region);
	(void)(dst_offset);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	ocl_stub_create_event(event, command_queue, CL_COMMAND_COPY_IMAGE_TO_BUFFER);
	return CL_SUCCESS;

}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue command_queue, cl_mem image,
	cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin,
	const size_t* region, size_t* image_row_pitch,
	size_t* image_slice_pitch, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event,
	cl_int* errcode_ret) {

	void* map_ptr = NULL;

	/* Unused. */
	(void)(blocking_map);
	(void)(map_flags);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Error check. */
	if (command_queue == NULL) {
		seterrcode(errcode_ret, CL_INVALID_COMMAND_QUEUE);
	} else if (image == NULL) {
		seterrcode(errcode_ret, CL_INVALID_MEM_OBJECT);
	} else if (image->context != command_queue->context) {
		seterrcode(errcode_ret, CL_INVALID_CONTEXT);
		/* Not testing if events in wait list belong to this context. */
	} else if ((image_row_pitch == NULL)
		|| (origin == NULL) || (region == NULL)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
	} else if ((image->image_desc.image_type == CL_MEM_OBJECT_IMAGE3D)
		&& (image_slice_pitch == NULL)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
	} else if ((origin[0] + region[0] > image->image_desc.image_width)
		|| (origin[1] + region[1] > image->image_desc.image_height)
		|| (origin[2] + region[2] > image->image_desc.image_depth)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
	} else {

		/* Set event. */
		ocl_stub_create_event(event, command_queue, CL_COMMAND_MAP_IMAGE);
		seterrcode(errcode_ret, CL_SUCCESS);

		/* Just return a pointer to the memory region. */
		map_ptr = image->mem + origin[0]
			+ origin[1] * image->image_desc.image_row_pitch
			+ origin[2] * image->image_desc.image_slice_pitch;
		/* Set the row pitch. */
		*image_row_pitch = image->image_desc.image_row_pitch;

		/* Set the slice pitch. */
		if (image_slice_pitch != NULL) {
			*image_slice_pitch = image->image_desc.image_slice_pitch;
		}

		/* Increase map count. */
		image->map_count++;
	}

	/* Return mapped pointer. */
	return map_ptr;

}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarker(cl_command_queue command_queue, cl_event *event) {

	ocl_stub_create_event(event, command_queue, CL_COMMAND_MARKER);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWaitForEvents(cl_command_queue command_queue,
	cl_uint num_events, const cl_event* event_list) {

	/* These are ignored. */
	(void)(command_queue);
	(void)(num_events);
	(void)(event_list);

	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrier(cl_command_queue command_queue) {

	(void)(command_queue);

	return CL_SUCCESS;
}

#ifdef CL_VERSION_1_1

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_read, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region,
	size_t buffer_row_pitch, size_t buffer_slice_pitch,
	size_t host_row_pitch, size_t host_slice_pitch, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

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
	}
	/* Many errors not checked... */

	/* These are ignored. */
	(void)(blocking_read);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(
		event, command_queue, CL_COMMAND_READ_BUFFER_RECT);

	/* Determine effective row and slice pitches. */
	if (buffer_row_pitch == 0)
		buffer_row_pitch = region[0];
	if (buffer_slice_pitch == 0)
		buffer_slice_pitch = region[1];
	if (host_row_pitch == 0)
		host_row_pitch = region[0];
	if (host_slice_pitch == 0)
		host_slice_pitch = region[1];

	/* Read buffer. */
	for (size_t z = 0; z < region[2]; ++z) {
		size_t z_buffer = buffer_slice_pitch * buffer_row_pitch * (z + buffer_origin[2]);
		size_t z_host = host_slice_pitch * host_row_pitch * (z + host_origin[2]);
		for (size_t y = 0; y < region[1]; ++y) {
			size_t y_buffer = buffer_row_pitch * (y + buffer_origin[1]);
			size_t y_host = host_row_pitch * (y + host_origin[1]);
			g_memmove(
				((cl_uchar*)ptr) + z_host + y_host,
				((cl_uchar*)buffer->mem) + z_buffer + y_buffer,
				region[0]);
		}
	}

	/* All good. */
	return CL_SUCCESS;
}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue command_queue, cl_mem buffer,
	cl_bool blocking_write, const size_t* buffer_origin,
	const size_t* host_origin, const size_t* region,
	size_t buffer_row_pitch, size_t buffer_slice_pitch,
	size_t host_row_pitch, size_t host_slice_pitch, const void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

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
	}
	/* Many errors not checked... */

	/* These are ignored. */
	(void)(blocking_write);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(
		event, command_queue, CL_COMMAND_WRITE_BUFFER_RECT);

	/* Determine effective row and slice pitches. */
	if (buffer_row_pitch == 0)
		buffer_row_pitch = region[0];
	if (buffer_slice_pitch == 0)
		buffer_slice_pitch = region[1];
	if (host_row_pitch == 0)
		host_row_pitch = region[0];
	if (host_slice_pitch == 0)
		host_slice_pitch = region[1];

	/* Write buffer. */
	for (size_t z = 0; z < region[2]; ++z) {
		size_t z_buffer = buffer_slice_pitch * buffer_row_pitch * (z + buffer_origin[2]);
		size_t z_host = host_slice_pitch * host_row_pitch * (z + host_origin[2]);
		for (size_t y = 0; y < region[1]; ++y) {
			size_t y_buffer = buffer_row_pitch * (y + buffer_origin[1]);
			size_t y_host = host_row_pitch * (y + host_origin[1]);
			g_memmove(
				((cl_uchar*)buffer->mem) + z_buffer + y_buffer,
				((cl_uchar*)ptr) + z_host + y_host,
				region[0]);
		}
	}

	/* All good. */
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue command_queue,
	cl_mem src_buffer, cl_mem dst_buffer, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region,
	size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
	size_t dst_slice_pitch, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if ((src_buffer == NULL) || (dst_buffer == NULL)) {
		return CL_INVALID_MEM_OBJECT;
	} else if (src_buffer->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
	} else if (dst_buffer->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	}
	/* Many errors not checked... */

	/* These are ignored. */
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(
		event, command_queue, CL_COMMAND_COPY_BUFFER_RECT);

	/* Determine effective row and slice pitches. */
	if (src_row_pitch == 0)
		src_row_pitch = region[0];
	if (src_slice_pitch == 0)
		src_slice_pitch = region[1];
	if (dst_row_pitch == 0)
		dst_row_pitch = region[0];
	if (dst_slice_pitch == 0)
		dst_slice_pitch = region[1];

	/* Copy buffer. */
	for (size_t z = 0; z < region[2]; ++z) {
		size_t z_src = src_slice_pitch * src_row_pitch * (z + src_origin[2]);
		size_t z_dst = dst_slice_pitch * dst_row_pitch * (z + dst_origin[2]);
		for (size_t y = 0; y < region[1]; ++y) {
			size_t y_src = src_row_pitch * (y + src_origin[1]);
			size_t y_dst = dst_row_pitch * (y + dst_origin[1]);
			g_memmove(
				((cl_uchar*)dst_buffer->mem) + z_dst + y_dst,
				((cl_uchar*)src_buffer->mem) + z_src + y_src,
				region[0]);
		}
	}

	/* All good. */
	return CL_SUCCESS;

}

#endif

#ifdef CL_VERSION_1_2

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue command_queue,
	cl_uint num_mem_objects, const cl_mem* mem_objects,
	cl_mem_migration_flags flags, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) {

	/* Unimplemented. */
	g_assert_not_reached();

	(void)(command_queue);
	(void)(num_mem_objects);
	(void)(mem_objects);
	(void)(flags);
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	ocl_stub_create_event(event, command_queue, CL_COMMAND_MIGRATE_MEM_OBJECTS);

	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue command_queue, cl_mem buffer,
	const void* pattern, size_t pattern_size, size_t offset,
	size_t size, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) {

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
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_FILL_BUFFER);

	/* Fill buffer. */
	for (guint i = 0; i < size; i += (guint) pattern_size) {
		g_memmove(((cl_uchar*)buffer->mem) + offset + i, pattern, pattern_size);
	}

	/* All good. */
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	ocl_stub_create_event(event, command_queue, CL_COMMAND_MARKER);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	ocl_stub_create_event(event, command_queue, CL_COMMAND_BARRIER);
	return CL_SUCCESS;

}


CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue command_queue, cl_mem image,
	const void* fill_color, const size_t* origin, const size_t* region,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list,
	cl_event* event) {

	/* Error check. */
	if (command_queue == NULL) {
		return CL_INVALID_COMMAND_QUEUE;
	} else if (image == NULL) { /* Not checking if memobject is image. */
		return CL_INVALID_MEM_OBJECT;
	} else if (image->context != command_queue->context) {
		return CL_INVALID_CONTEXT;
		/* Not testing if events in wait list belong to this context. */
	} else if (fill_color == NULL) {
		return CL_INVALID_VALUE;
	} else if ((origin[0] + region[0] > image->image_desc.image_width)
		|| (origin[1] + region[1] > image->image_desc.image_height)
		|| (origin[2] + region[2] > image->image_desc.image_depth)) {
		return CL_INVALID_VALUE;
	}

	/* Convert fill color to RGBA format, the only one supported in this
	 * stub. This will only work in little-endian. */
	void* final_color = g_malloc0(image->image_elem_size);
	if (image->image_elem_size == 16) {
		/* 16 bytes, 4 bytes (32 bits) per component. */
		g_memmove(final_color, fill_color, 16);
	} else if (image->image_elem_size == 8) {
		/* 8 bytes, 2 bytes (16 bits) per component. */
		g_memmove(((cl_uchar*)final_color), ((cl_uchar*)fill_color) + 0, 2);
		g_memmove(((cl_uchar*)final_color) + 2, ((cl_uchar*)fill_color) + 4, 2);
		g_memmove(((cl_uchar*)final_color) + 4, ((cl_uchar*)fill_color) + 8, 2);
		g_memmove(((cl_uchar*)final_color) + 6, ((cl_uchar*)fill_color) + 12, 2);
	} else if (image->image_elem_size == 4) {
		/* 4 bytes, 1 byte (8 bits) per component. */
		g_memmove(((cl_uchar*)final_color), ((cl_uchar*)fill_color) + 0, 1);
		g_memmove(((cl_uchar*)final_color) + 1, ((cl_uchar*)fill_color) + 4, 1);
		g_memmove(((cl_uchar*)final_color) + 2, ((cl_uchar*)fill_color) + 8, 1);
		g_memmove(((cl_uchar*)final_color) + 3, ((cl_uchar*)fill_color) + 12, 1);
	} else {
		/* Others are unsupported. */
		g_assert_not_reached();
	}

	/* These are ignored. */
	(void)(num_events_in_wait_list);
	(void)(event_wait_list);

	/* Set event. */
	ocl_stub_create_event(event, command_queue, CL_COMMAND_FILL_IMAGE);

	/* Get image width and height to more readeable variables. */
	size_t w = image->image_desc.image_width;
	size_t h = image->image_desc.image_height;

	/* Fill image region. */
	for (size_t z = 0; z < region[2]; z++) {
		for (size_t y = 0; y < region[1]; y++) {
			for (size_t x = 0; x < region[0]; x++) {
				g_memmove(
					((cl_uchar*)image->mem) + ((z + origin[2]) * w * h + (y + origin[1]) * w + (x + origin[0])) * image->image_elem_size,
					final_color,
					image->image_elem_size);
			}
		}
	}
	g_free(final_color);
	/* All good. */
	return CL_SUCCESS;

}

#endif
