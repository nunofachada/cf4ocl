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
 * @brief OpenCL buffer wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "buffer_wrapper.h"

/** 
 * @addtogroup BUFFER_WRAPPER
 * @{
 */

/** @brief Create a ::CCLBuffer wrapper object. */
CCLBuffer* ccl_buffer_new(CCLContext* ctx, cl_mem_flags flags,
	size_t size, void *host_ptr, GError** err) {
		
	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;
	cl_mem buffer;
	CCLBuffer* buf = NULL;
	
	buffer = clCreateBuffer(ccl_context_unwrap(ctx), flags, size,
		host_ptr, &ocl_status);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to create buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	buf = ccl_memobj_new_wrap(buffer);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:
	
	/* Return new buffer wrapper. */
	return buf;

}

CCLEvent* ccl_buffer_read(CCLQueue* cq, CCLBuffer* buf,
	cl_bool blocking_read, size_t offset, size_t size, void *ptr,
	CCLEventWaitList evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure buf is not NULL. */
	g_return_val_if_fail(buf != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt = NULL;
	
	ocl_status = clEnqueueReadBuffer(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(buf), blocking_read, offset, size, ptr,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to read buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

CCLEvent* ccl_buffer_write(CCLQueue* cq, CCLBuffer* buf,
	cl_bool blocking_write, size_t offset, size_t size, void *ptr,
 	CCLEventWaitList evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure buf is not NULL. */
	g_return_val_if_fail(buf != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt = NULL;
	
	ocl_status = clEnqueueWriteBuffer(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(buf), blocking_write, offset, size, ptr,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to write buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;
	
}

void* ccl_buffer_map(CCLQueue* cq, CCLBuffer* buf,
	cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, CCLEventWaitList evt_wait_lst, CCLEvent** evt,
	GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure buf is not NULL. */
	g_return_val_if_fail(buf != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt_inner = NULL;
	void* ptr = NULL;
	
	ptr = clEnqueueMapBuffer(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(buf), blocking_map, map_flags, offset, size,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), 
		&event, &ocl_status);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to map buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt_inner = ccl_queue_produce_event(cq, event);
	if (evt != NULL)
		*evt = evt_inner;
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	ptr = NULL;
	*evt = NULL;
	
finish:
	
	/* Return host pointer. */
	return ptr;

}

CCLEvent* ccl_buffer_copy(CCLQueue* cq, CCLBuffer* src_buf,
	CCLBuffer* dst_buf, size_t src_offset, size_t dst_offset, 
	size_t size, CCLEventWaitList evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure src_buf is not NULL. */
	g_return_val_if_fail(src_buf != NULL, NULL);
	/* Make sure dst_buf is not NULL. */
	g_return_val_if_fail(dst_buf != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt = NULL;
	
	ocl_status = clEnqueueCopyBuffer(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(src_buf), ccl_memobj_unwrap(dst_buf),
		src_offset, dst_offset, size,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to write buffer (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}
	
CCLEvent* ccl_buffer_copy_to_image(CCLQueue* cq, CCLBuffer* src_buf,
	CCLBuffer* dst_img, size_t src_offset, const size_t *dst_origin,
	const size_t *region, CCLEventWaitList evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure src_buf is not NULL. */
	g_return_val_if_fail(src_buf != NULL, NULL);
	/* Make sure dst_img is not NULL. */
	g_return_val_if_fail(dst_img != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	cl_event event = NULL;
	CCLEvent* evt = NULL;
	
	ocl_status = clEnqueueCopyBufferToImage(ccl_queue_unwrap(cq), 
		ccl_memobj_unwrap(src_buf), ccl_memobj_unwrap(dst_img),
		src_offset, dst_origin, region,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: unable to copy buffer to image (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:
	
	/* Return event. */
	return evt;

}

//~ #ifdef CL_VERSION_1_1
//~ 
//~ CCLBuffer* ccl_buffer_new_from_region(CCLBuffer* buf, 
	//~ cl_mem_flags flags, size_t origin, size_t size, GError** err) {
//~ 
	//~ /* Make sure buf is not NULL. */
	//~ g_return_val_if_fail(buf != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
//~ 
//~ }
//~ 
//~ CCLEvent* ccl_buffer_read_rect(CCLQueue* cq, CCLBuffer* buf,
	//~ cl_bool blocking_read, const size_t* buffer_origin,
	//~ const size_t* host_origin, const size_t* region, 
	//~ size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	//~ size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	//~ CCLEventWaitList evt_wait_lst, GError** err) {
//~ 
	//~ /* Make sure cq is not NULL. */
	//~ g_return_val_if_fail(cq != NULL, NULL);
	//~ /* Make sure buf is not NULL. */
	//~ g_return_val_if_fail(buf != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
//~ }
//~ 
//~ CCLEvent* ccl_buffer_write_rect(CCLQueue* cq, CCLBuffer* buf,
	//~ cl_bool blocking_write, const size_t* buffer_origin,
	//~ const size_t* host_origin, const size_t* region, 
	//~ size_t buffer_row_pitch, size_t buffer_slice_pitch, 
	//~ size_t host_row_pitch, size_t host_slice_pitch, void *ptr,
	//~ CCLEventWaitList evt_wait_lst, GError** err) {
//~ 
	//~ /* Make sure cq is not NULL. */
	//~ g_return_val_if_fail(cq != NULL, NULL);
	//~ /* Make sure buf is not NULL. */
	//~ g_return_val_if_fail(buf != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
//~ 
//~ }
//~ 
//~ CCLEvent* ccl_buffer_copy_rect(CCLQueue* cq, CCLBuffer* src_buf,
	//~ CCLBuffer* dst_buf, const size_t *src_origin, 
	//~ const size_t *dst_origin, const size_t *region, 
	//~ size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
	//~ size_t dst_slice_pitch, CCLEventWaitList evt_wait_lst, 
	//~ GError** err) {
//~ 
	//~ /* Make sure cq is not NULL. */
	//~ g_return_val_if_fail(cq != NULL, NULL);
	//~ /* Make sure src_buf is not NULL. */
	//~ g_return_val_if_fail(src_buf != NULL, NULL);
	//~ /* Make sure dst_buf is not NULL. */
	//~ g_return_val_if_fail(dst_buf != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
//~ 
//~ }
//~ 
//~ #endif
//~ 
//~ #ifdef CL_VERSION_1_2
//~ 
//~ CCLEvent* ccl_buffer_fill(CCLQueue* cq, CCLBuffer* buf, 
	//~ const void *pattern, size_t pattern_size, size_t offset, 
	//~ size_t size, CCLEventWaitList evt_wait_lst, GError** err) {
//~ 
	//~ /* Make sure cq is not NULL. */
	//~ g_return_val_if_fail(cq != NULL, NULL);
	//~ /* Make sure buf is not NULL. */
	//~ g_return_val_if_fail(buf != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
//~ 
//~ 
//~ }
//~ 
//~ #endif

/** @} */
