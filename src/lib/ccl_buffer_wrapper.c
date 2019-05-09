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
 * Implementation of a wrapper class and its methods for OpenCL buffer objects.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_buffer_wrapper.h"
#include "ccl_image_wrapper.h"
#include "_ccl_memobj_wrapper.h"
#include "_ccl_defs.h"

/**
 * Buffer wrapper class
 *
 * @extends ccl_memobj
 * */
struct ccl_buffer {

    /**
     * Parent wrapper object.
     * @private
     * */
    CCLMemObj mo;
};

/**
 * @addtogroup CCL_BUFFER_WRAPPER
 * @{
 */

/**
 * Get the buffer wrapper for the given OpenCL buffer.
 *
 * If the wrapper doesn't exist, its created with a reference count
 * of 1. Otherwise, the existing wrapper is returned and its reference
 * count is incremented by 1.
 *
 * This function will rarely be called from client code, except when
 * clients wish to directly wrap an OpenCL buffer in a
 * ::CCLBuffer wrapper object.
 *
 * @protected @memberof ccl_buffer
 *
 * @param[in] mem_object The OpenCL buffer to be wrapped.
 * @return The ::CCLBuffer wrapper for the given OpenCL buffer.
 * */
CCL_EXPORT
CCLBuffer * ccl_buffer_new_wrap(cl_mem mem_object) {

    return (CCLBuffer *) ccl_wrapper_new(
        CCL_BUFFER, (void *) mem_object, sizeof(CCLBuffer));

}

/**
 * Decrements the reference count of the wrapper object. If it
 * reaches 0, the wrapper object is destroyed.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in] buf The buffer wrapper object.
 * */
CCL_EXPORT
void ccl_buffer_destroy(CCLBuffer * buf) {

    ccl_wrapper_unref((CCLWrapper *) buf, sizeof(CCLBuffer),
        (ccl_wrapper_release_fields) ccl_memobj_release_fields,
        (ccl_wrapper_release_cl_object) clReleaseMemObject, NULL);
}

/**
 * Create a ::CCLBuffer wrapper object.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in] ctx Context wrapper.
 * @param[in] flags OpenCL memory flags as used in clCreateBuffer().
 * @param[in] size The size in bytes of the buffer memory object to be
 * allocated.
 * @param[in] host_ptr A pointer to the buffer data that may already be
 * allocated by the application. The size of the buffer that `host_ptr`
 * points to must be >= size bytes.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A new wrapper object.
 * */
CCL_EXPORT
CCLBuffer * ccl_buffer_new(CCLContext * ctx, cl_mem_flags flags,
    size_t size, void * host_ptr, CCLErr ** err) {

    /* Make sure ctx is not NULL. */
    g_return_val_if_fail(ctx != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    cl_int ocl_status;
    cl_mem buffer;
    CCLBuffer * buf = NULL;

    /* Create OpenCL buffer. */
    buffer = clCreateBuffer(ccl_context_unwrap(ctx), flags, size,
        host_ptr, &ocl_status);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to create buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap OpenCL buffer. */
    buf = ccl_buffer_new_wrap(buffer);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

finish:

    /* Return new buffer wrapper. */
    return buf;
}

/**
 * Read from a buffer object to host memory. This function wraps the
 * clEnqueueReadBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in] buf Buffer wrapper object where to read from.
 * @param[in] cq Command-queue wrapper object in which the read command
 * will be queued.
 * @param[in] blocking_read Indicates if the read operations are
 * blocking or non-blocking.
 * @param[in] offset The offset in bytes in the buffer object to read
 * from.
 * @param[in] size The size in bytes of data being read.
 * @param[out] ptr The pointer to buffer in host memory where data is to
 * be read into.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this read command.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_read(CCLBuffer * buf, CCLQueue * cq,
    cl_bool blocking_read, size_t offset, size_t size, void * ptr,
    CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    cl_int ocl_status;
    cl_event event = NULL;
    CCLEvent * evt = NULL;

    ocl_status = clEnqueueReadBuffer(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), blocking_read, offset, size, ptr,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to read buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Write to a buffer object from host memory. This function wraps the
 * clEnqueueWriteBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 *
 * @param[out] buf Buffer wrapper object where to write to.
 * @param[in] cq Command-queue wrapper object in which the write command
 * will be queued.
 * @param[in] blocking_write Indicates if the write operations are
 * blocking or non-blocking.
 * @param[in] offset The offset in bytes in the buffer object to read
 * from.
 * @param[in] size The size in bytes of data being read.
 * @param[in] ptr The pointer to buffer in host memory where data is to
 * be written from.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this write command, or
 * `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_write(CCLBuffer * buf, CCLQueue * cq,
    cl_bool blocking_write, size_t offset, size_t size, void * ptr,
     CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    cl_int ocl_status;
    cl_event event = NULL;
    CCLEvent * evt = NULL;

    ocl_status = clEnqueueWriteBuffer(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), blocking_write, offset, size, ptr,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to write buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Map a region of a buffer into the host address space and return a
 * pointer to this mapped region. This function wraps the
 * clEnqueueMapBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in,out] buf Buffer wrapper object to be mapped.
 * @param[in] cq Command-queue wrapper object in which the map command
 * will be queued.
 * @param[in] blocking_map Indicates if the map operation is blocking
 * or non-blocking.
 * @param[in] map_flags Flags which specify the type of mapping to
 * perform.
 * @param[in] offset The offset in bytes in the buffer object that is
 * being mapped.
 * @param[in] size The size of the region in the buffer object that is
 * being mapped.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] evt An event wrapper object that identifies this particular map
 * command. If `NULL`, no event will be returned.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A pointer in the host address space for the mapped region.
 * */
CCL_EXPORT
void * ccl_buffer_enqueue_map(CCLBuffer * buf, CCLQueue * cq,
    cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
    size_t size, CCLEventWaitList * evt_wait_lst, CCLEvent ** evt,
    CCLErr ** err) {

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

    /* Perform buffer map. */
    ptr = clEnqueueMapBuffer(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), blocking_map, map_flags, offset, size,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst),
        &event, &ocl_status);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to map buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt_inner = ccl_queue_produce_event(cq, event);
    if (evt != NULL)
        *evt = evt_inner;

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    ptr = NULL;

finish:

    /* Return host pointer. */
    return ptr;
}

/**
 * Copy from one buffer object to another. This function wraps the
 * clEnqueueCopyBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in] src_buf Source buffer wrapper object where to read from.
 * @param[out] dst_buf Destination buffer wrapper object where to write
 * to.
 * @param[in] cq Command-queue wrapper object in which the copy command
 * will be queued.
 * @param[in] src_offset The offset where to begin copying data from
 * src_buffer.
 * @param[in] dst_offset The offset where to begin copying data into
 * dst_buffer.
 * @param[in] size Size in bytes to copy.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this copy command.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_copy(CCLBuffer * src_buf,
    CCLBuffer * dst_buf, CCLQueue * cq, size_t src_offset,
    size_t dst_offset, size_t size, CCLEventWaitList * evt_wait_lst,
    CCLErr ** err) {

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
    CCLEvent * evt = NULL;

    ocl_status = clEnqueueCopyBuffer(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(src_buf), ccl_memobj_unwrap(dst_buf),
        src_offset, dst_offset, size,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to write buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Copy a buffer object to an image object. This function wraps the
 * clEnqueueCopyBufferToImage() OpenCL function.
 *
 * @public @memberof ccl_buffer
 *
 * @param[in] src_buf Source buffer wrapper object where to read from.
 * @param[out] dst_img Destination image wrapper object where to write
 * to.
 * @param[in] cq Command-queue wrapper object in which the copy command
 * will be queued.
 * @param[in] src_offset The offset where to begin copying data from
 * `src_buffer`.
 * @param[in] dst_origin Defines the @f$(x, y, z)@f$ offset in pixels in
 * the 1D, 2D or 3D image, the @f$(x, y)@f$ offset and the image index
 * in the 2D image array or the @f$(x)@f$ offset and the image index in
 * the 1D image array.
 * @param[in] region The @f$(width, height, depth)@f$ in pixels of the
 * 1D, 2D or 3D rectangle, the @f$(width, height)@f$ in pixels of the 2D
 * rectangle and the number of images of a 2D image array or the
 * @f$(width)@f$ in pixels of the 1D rectangle and the number of images
 * of a 1D image array.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this copy command.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_copy_to_image(CCLBuffer * src_buf,
    CCLImage * dst_img, CCLQueue * cq, size_t src_offset,
    const size_t * dst_origin, const size_t * region,
    CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure src_buf is not NULL. */
    g_return_val_if_fail(src_buf != NULL, NULL);
    /* Make sure dst_img is not NULL. */
    g_return_val_if_fail(dst_img != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL event object. */
    cl_event event = NULL;
    /* Event wrapper object. */
    CCLEvent * evt = NULL;

    /* Copy buffer to image. */
    ocl_status = clEnqueueCopyBufferToImage(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(src_buf), ccl_memobj_unwrap(dst_img),
        src_offset, dst_origin, region,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to copy buffer to image (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Creates a sub-buffer that represents a specific region in the given
 * buffer. This function wraps the clCreateSubBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] buf A buffer wrapper object which cannot represent a
 * sub-buffer.
 * @param[in] flags Allocation and usage information about the
 * sub-buffer memory object.
 * @param[in] origin Offset relative to the parent buffer.
 * @param[in] size Sub-buffer size.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A new buffer wrapper object which represents a specific
 * region in the original buffer.
 * */
CCL_EXPORT
CCLBuffer * ccl_buffer_new_from_region(CCLBuffer * buf,
    cl_mem_flags flags, size_t origin, size_t size, CCLErr ** err) {

    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL sub-buffer object- */
    cl_mem buffer;
    /* Buffer wrapper. */
    CCLBuffer * subbuf;
    /* OpenCL version of the underlying platform. */
    double ocl_ver;
    /* Internal error handling object. */
    CCLErr* err_internal = NULL;

#ifndef CL_VERSION_1_1

    CCL_UNUSED(ocl_ver);
    CCL_UNUSED(buffer);
    CCL_UNUSED(ocl_status);
    CCL_UNUSED(flags);
    CCL_UNUSED(origin);
    CCL_UNUSED(size);
    CCL_UNUSED(err_internal);

    /* If cf4ocl was not compiled with support for OpenCL >= 1.1, always throw
     * error. */
    g_if_err_create_goto(*err, CCL_ERROR, TRUE,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Sub-buffers require cf4ocl to be deployed with support "
        "for OpenCL version 1.1 or newer.",
        CCL_STRD);
#else

    /* Set options. */
    const cl_buffer_region br = { .origin = origin, .size = size};

    /* Check that context platform is >= OpenCL 1.1 */
    ocl_ver = ccl_memobj_get_opencl_version(
        (CCLMemObj *) buf, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* If OpenCL version is not >= 1.1, throw error. */
    g_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: sub-buffers require OpenCL version 1.1 or newer.",
        CCL_STRD);

    /* Create the OpenCL sub-buffer. */
    buffer = clCreateSubBuffer(ccl_memobj_unwrap(buf), flags,
        CL_BUFFER_CREATE_TYPE_REGION, (const void *) &br, &ocl_status);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable create sub-buffer (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap the OpenCL sub-buffer. */
    subbuf = ccl_buffer_new_wrap(buffer);

#endif

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    subbuf = NULL;

finish:

    /* Return sub-buffer. */
    return subbuf;
}

/**
 * Read from a 2D or 3D rectangular region from a buffer object to host
 * memory. This function wraps the clEnqueueReadBufferRect() OpenCL
 * function.
 *
 * @public @memberof ccl_buffer
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] buf Buffer wrapper object where to read from.
 * @param[in] cq Command-queue wrapper object in which the read command
 * will be queued.
 * @param[in] blocking_read Indicates if the read operations are
 * blocking or non-blocking.
 * @param[in] buffer_origin The @f$(x, y, z)@f$ offset in the memory
 * region associated with buffer.
 * @param[in] host_origin The @f$(x, y, z)@f$ offset in the memory
 * region pointed to by ptr.
 * @param[in] region The (width in bytes, height in rows, depth in
 * slices) of the 2D or 3D rectangle being read or written.
 * @param[in] buffer_row_pitch The length of each row in bytes to be
 * used for the memory region associated with buffer.
 * @param[in] buffer_slice_pitch The length of each 2D slice in bytes
 * to be used for the memory region associated with buffer.
 * @param[in] host_row_pitch The length of each row in bytes to be used
 * for the memory region pointed to by ptr.
 * @param[in] host_slice_pitch The length of each 2D slice in bytes to
 * be used for the memory region pointed to by ptr.
 * @param[out] ptr The pointer to buffer in host memory where data is to
 * be read into.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this read command.
 * */
CCL_EXPORT
CCLEvent* ccl_buffer_enqueue_read_rect(CCLBuffer * buf, CCLQueue * cq,
    cl_bool blocking_read, const size_t * buffer_origin,
    const size_t * host_origin, const size_t * region,
    size_t buffer_row_pitch, size_t buffer_slice_pitch,
    size_t host_row_pitch, size_t host_slice_pitch, void * ptr,
    CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL event object. */
    cl_event event = NULL;
    /* Event wrapper object. */
    CCLEvent * evt = NULL;
    /* OpenCL version of the underlying platform. */
    double ocl_ver;
    /* Internal error handling object. */
    CCLErr * err_internal = NULL;

#ifndef CL_VERSION_1_1

    CCL_UNUSED(blocking_read);
    CCL_UNUSED(buffer_origin);
    CCL_UNUSED(host_origin);
    CCL_UNUSED(region);
    CCL_UNUSED(buffer_row_pitch);
    CCL_UNUSED(buffer_slice_pitch);
    CCL_UNUSED(host_row_pitch);
    CCL_UNUSED(host_slice_pitch);
    CCL_UNUSED(ptr);
    CCL_UNUSED(evt_wait_lst);
    CCL_UNUSED(ocl_status);
    CCL_UNUSED(event);
    CCL_UNUSED(evt);
    CCL_UNUSED(ocl_ver);
    CCL_UNUSED(err_internal);

    /* If cf4ocl was not compiled with support for OpenCL >= 1.1, always throw
     * error. */
    g_if_err_create_goto(*err, CCL_ERROR, TRUE,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Rectangular buffer read requires cf4ocl to be deployed with "
        "support for OpenCL version 1.1 or newer.",
        CCL_STRD);

#else

    /* Check that context platform is >= OpenCL 1.1 */
    ocl_ver = ccl_memobj_get_opencl_version(
        (CCLMemObj *) buf, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* If OpenCL version is not >= 1.1, throw error. */
    g_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: rect. buffer reads require OpenCL version 1.1 or newer.",
        CCL_STRD);

    /* Read rectangular region of buffer. */
    ocl_status = clEnqueueReadBufferRect(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), blocking_read, buffer_origin,
        host_origin, region, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to enqueue a rectangular buffer read (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

#endif

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Write a 2D or 3D rectangular region to a buffer object from host
 * memory. This function wraps the clEnqueueWriteBufferRect() OpenCL
 * function.
 *
 * @public @memberof ccl_buffer
 * @note Requires OpenCL >= 1.1
 *
 * @param[out] buf Buffer wrapper object where to write to.
 * @param[in] cq Command-queue wrapper object in which the write command
 * will be queued.
 * @param[in] blocking_write Indicates if the write operations are
 * blocking or non-blocking.
 * @param[in] buffer_origin The @f$(x, y, z)@f$ offset in the memory
 * region associated with buffer.
 * @param[in] host_origin The @f$(x, y, z)@f$ offset in the memory
 * region pointed to by `ptr`.
 * @param[in] region The (width in bytes, height in rows, depth in
 * slices) of the 2D or 3D rectangle being read or written.
 * @param[in] buffer_row_pitch The length of each row in bytes to be
 * used for the memory region associated with buffer.
 * @param[in] buffer_slice_pitch The length of each 2D slice in bytes
 * to be used for the memory region associated with buffer.
 * @param[in] host_row_pitch The length of each row in bytes to be used
 * for the memory region pointed to by `ptr`.
 * @param[in] host_slice_pitch The length of each 2D slice in bytes to
 * be used for the memory region pointed to by ptr.
 * @param[in] ptr The pointer to buffer in host memory where data is to
 * be written from.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this write command, or
 * `NULL` if an error occurs.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_write_rect(CCLBuffer * buf, CCLQueue * cq,
    cl_bool blocking_write, const size_t * buffer_origin,
    const size_t * host_origin, const size_t * region,
    size_t buffer_row_pitch, size_t buffer_slice_pitch,
    size_t host_row_pitch, size_t host_slice_pitch, void * ptr,
    CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL event object. */
    cl_event event = NULL;
    /* Event wrapper object. */
    CCLEvent * evt = NULL;
    /* OpenCL version of the underlying platform. */
    double ocl_ver;
    /* Internal error handling object. */
    CCLErr * err_internal = NULL;

#ifndef CL_VERSION_1_1

    CCL_UNUSED(blocking_write);
    CCL_UNUSED(buffer_origin);
    CCL_UNUSED(host_origin);
    CCL_UNUSED(region);
    CCL_UNUSED(buffer_row_pitch);
    CCL_UNUSED(buffer_slice_pitch);
    CCL_UNUSED(host_row_pitch);
    CCL_UNUSED(host_slice_pitch);
    CCL_UNUSED(ptr);
    CCL_UNUSED(evt_wait_lst);
    CCL_UNUSED(ocl_status);
    CCL_UNUSED(event);
    CCL_UNUSED(evt);
    CCL_UNUSED(ocl_ver);
    CCL_UNUSED(err_internal);

    /* If cf4ocl was not compiled with support for OpenCL >= 1.1, always throw
     * error. */
    g_if_err_create_goto(*err, CCL_ERROR, TRUE,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Rectangular buffer write requires cf4ocl to be deployed with "
        "support for OpenCL version 1.1 or newer.",
        CCL_STRD);
#else

    /* Check that context platform is >= OpenCL 1.1 */
    ocl_ver = ccl_memobj_get_opencl_version(
        (CCLMemObj *) buf, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* If OpenCL version is not >= 1.1, throw error. */
    g_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: rect. buffer writes require OpenCL version 1.1 or newer.",
        CCL_STRD);

    /* Write rectangular region of buffer. */
    ocl_status = clEnqueueWriteBufferRect(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), blocking_write, buffer_origin,
        host_origin, region, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to enqueue a rectangular buffer write (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

#endif

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Copy a 2D or 3D rectangular region from a buffer object to another
 * buffer object. This function wraps the clEnqueueCopyBufferRect()
 * OpenCL function.
 *
 * @public @memberof ccl_buffer
 * @note Requires OpenCL >= 1.1
 *
 * @param[in] src_buf Source buffer wrapper object where to read from.
 * @param[out] dst_buf Destination buffer wrapper object where to write
 * to.
 * @param[in] cq Command-queue wrapper object in which the copy command
 * will be queued.
 * @param[in] src_origin The @f$(x, y, z)@f$ offset in memory associated
 * with `src_buf`.
 * @param[in] dst_origin The @f$(x, y, z)@f$ offset in memory associated
 * with `dst_buf`.
 * @param[in] region The (width in bytes, height in rows, depth in
 * slices) of the 2D or 3D rectangle being copied.
 * @param[in] src_row_pitch The length of each row in bytes to be
 * used for the memory region associated with `src_buf`.
 * @param[in] src_slice_pitch The length of each 2D slice in bytes
 * to be used for the memory region associated with `src_buf`.
 * @param[in] dst_row_pitch The length of each row in bytes to be
 * used for the memory region associated with `dst_buf`.
 * @param[in] dst_slice_pitch The length of each 2D slice in bytes
 * to be used for the memory region associated with `dst_buf`.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this copy command.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_copy_rect(CCLBuffer * src_buf,
    CCLBuffer * dst_buf, CCLQueue * cq, const size_t * src_origin,
    const size_t * dst_origin, const size_t * region,
    size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch,
    size_t dst_slice_pitch, CCLEventWaitList * evt_wait_lst,
    CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure src_buf is not NULL. */
    g_return_val_if_fail(src_buf != NULL, NULL);
    /* Make sure dst_buf is not NULL. */
    g_return_val_if_fail(dst_buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL event object. */
    cl_event event = NULL;
    /* Event wrapper object. */
    CCLEvent * evt = NULL;
    /* OpenCL version of the underlying platform. */
    double ocl_ver;
    /* Internal error handling object. */
    CCLErr * err_internal = NULL;

#ifndef CL_VERSION_1_1

    CCL_UNUSED(src_origin);
    CCL_UNUSED(dst_origin);
    CCL_UNUSED(region);
    CCL_UNUSED(src_row_pitch);
    CCL_UNUSED(src_slice_pitch);
    CCL_UNUSED(dst_row_pitch);
    CCL_UNUSED(dst_slice_pitch);
    CCL_UNUSED(evt_wait_lst);
    CCL_UNUSED(ocl_status);
    CCL_UNUSED(event);
    CCL_UNUSED(evt);
    CCL_UNUSED(ocl_ver);
    CCL_UNUSED(err_internal);

    /* If cf4ocl was not compiled with support for OpenCL >= 1.1, always throw
     * error. */
    g_if_err_create_goto(*err, CCL_ERROR, TRUE,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Rectangular buffer copy requires cf4ocl to be deployed with "
        "support for OpenCL version 1.1 or newer.",
        CCL_STRD);

#else

    /* Check that context platform is >= OpenCL 1.1 */
    ocl_ver = ccl_memobj_get_opencl_version(
        (CCLMemObj *) src_buf, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* If OpenCL version is not >= 1.1, throw error. */
    g_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 110,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: rect. buffer copy requires OpenCL version 1.1 or newer.",
        CCL_STRD);

    /* Copy rectangular region between buffers. */
    ocl_status = clEnqueueCopyBufferRect(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(src_buf), ccl_memobj_unwrap(dst_buf),
        src_origin, dst_origin, region, src_row_pitch, src_slice_pitch,
        dst_row_pitch, dst_slice_pitch,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to enqueue a rectangular buffer copy (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);
#endif

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/**
 * Fill a buffer object with a pattern of a given pattern size. This
 * function wraps the clEnqueueFillBuffer() OpenCL function.
 *
 * @public @memberof ccl_buffer
 * @note Requires OpenCL >= 1.2
 *
 * @param[out] buf Buffer wrapper object to fill.
 * @param[in] cq Command-queue wrapper object in which the fill command
 * will be queued.
 * @param[in] pattern A pointer to the data pattern.
 * @param[in] pattern_size Size of data pattern in bytes.
 * @param[in] offset The location in bytes of the region being filled in
 * buffer and must be a multiple of pattern_size.
 * @param[in] size The size in bytes of region being filled in buffer.
 * Must be a multiple of pattern_size.
 * @param[in,out] evt_wait_lst List of events that need to complete
 * before this command can be executed. The list will be cleared and
 * can be reused by client code.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return Event wrapper object that identifies this fill command.
 * */
CCL_EXPORT
CCLEvent * ccl_buffer_enqueue_fill(CCLBuffer * buf, CCLQueue * cq,
    const void * pattern, size_t pattern_size, size_t offset,
    size_t size, CCLEventWaitList * evt_wait_lst, CCLErr ** err) {

    /* Make sure cq is not NULL. */
    g_return_val_if_fail(cq != NULL, NULL);
    /* Make sure buf is not NULL. */
    g_return_val_if_fail(buf != NULL, NULL);
    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* OpenCL function status. */
    cl_int ocl_status;
    /* OpenCL event object. */
    cl_event event = NULL;
    /* Event wrapper object. */
    CCLEvent * evt = NULL;
    /* OpenCL version of the underlying platform. */
    double ocl_ver;
    /* Internal error handling object. */
    CCLErr * err_internal = NULL;

#ifndef CL_VERSION_1_2

    CCL_UNUSED(pattern);
    CCL_UNUSED(pattern_size);
    CCL_UNUSED(offset);
    CCL_UNUSED(size);
    CCL_UNUSED(evt_wait_lst);
    CCL_UNUSED(ocl_status);
    CCL_UNUSED(event);
    CCL_UNUSED(evt);
    CCL_UNUSED(ocl_ver);
    CCL_UNUSED(err_internal);

    /* If cf4ocl was not compiled with support for OpenCL >= 1.2, always throw
     * error. */
    g_if_err_create_goto(*err, CCL_ERROR, TRUE,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Buffer fill requires cf4ocl to be deployed with "
        "support for OpenCL version 1.1 or newer.",
        CCL_STRD);

#else

    /* Check that context platform is >= OpenCL 1.2 */
    ocl_ver = ccl_memobj_get_opencl_version(
        (CCLMemObj *) buf, &err_internal);
    g_if_err_propagate_goto(err, err_internal, error_handler);

    /* If OpenCL version is not >= 1.2, throw error. */
    g_if_err_create_goto(*err, CCL_ERROR, ocl_ver < 120,
        CCL_ERROR_UNSUPPORTED_OCL, error_handler,
        "%s: Buffer fill requires OpenCL version 1.2 or newer.",
        CCL_STRD);

    /* Fill buffer. */
    ocl_status = clEnqueueFillBuffer(ccl_queue_unwrap(cq),
        ccl_memobj_unwrap(buf), pattern, pattern_size, offset, size,
        ccl_event_wait_list_get_num_events(evt_wait_lst),
        ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
    g_if_err_create_goto(*err, CCL_OCL_ERROR,
        CL_SUCCESS != ocl_status, ocl_status, error_handler,
        "%s: unable to enqueue a fill buffer command (OpenCL error %d: %s).",
        CCL_STRD, ocl_status, ccl_err(ocl_status));

    /* Wrap event and associate it with the respective command queue.
     * The event object will be released automatically when the command
     * queue is released. */
    evt = ccl_queue_produce_event(cq, event);

    /* Clear event wait list. */
    ccl_event_wait_list_clear(evt_wait_lst);

#endif

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* An error occurred, return NULL to signal it. */
    evt = NULL;

finish:

    /* Return event. */
    return evt;
}

/** @} */
