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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @internal
 *
 * @file
 * Test the buffer wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"
#include "_ccl_defs.h"

#define CCL_TEST_BUFFER_SIZE 512

/**
 * @internal
 *
 * @brief Tests creation, getting info from and destruction of
 * buffer wrapper objects.
 * */
static void create_info_destroy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * b = NULL;
    CCLErr * err = NULL;
    size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Create regular buffer. */
    b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Get some info and check if the return value is as expected. */
    cl_mem_object_type mot;
    mot = ccl_memobj_get_info_scalar(
        b, CL_MEM_TYPE, cl_mem_object_type, &err);
    g_assert_no_error(err);
    g_assert_cmphex(mot, ==, CL_MEM_OBJECT_BUFFER);

    cl_mem_flags flags;
    flags = ccl_memobj_get_info_scalar(
        b, CL_MEM_FLAGS, cl_mem_flags, &err);
    g_assert_no_error(err);
    g_assert_cmphex(flags, ==, CL_MEM_READ_WRITE);

    size_t mem_size;
    mem_size = ccl_memobj_get_info_scalar(b, CL_MEM_SIZE, size_t, &err);
    g_assert_no_error(err);
    g_assert_cmpuint(mem_size, ==, buf_size);

    void * host_ptr = NULL;
    host_ptr = ccl_memobj_get_info_scalar(
        b, CL_MEM_HOST_PTR, void*, &err);
    g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
    g_assert_cmphex(GPOINTER_TO_SIZE(host_ptr), ==, GPOINTER_TO_SIZE(NULL));
    g_clear_error(&err);

    cl_context context;
    context = ccl_memobj_get_info_scalar(
        b, CL_MEM_CONTEXT, cl_context, &err);
    g_assert_no_error(err);
    g_assert_cmphex(GPOINTER_TO_SIZE(context), ==,
        GPOINTER_TO_SIZE(ccl_context_unwrap(ctx)));

    /* Destroy buffer. */
    ccl_buffer_destroy(b);

    /* Try creating a buffer with invalid flags. */
    b = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, buf_size, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_HOST_PTR);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests buffer wrapper class reference counting.
 * */
static void ref_unref_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * b = NULL;
    CCLErr * err = NULL;
    size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Create regular buffer. */
    b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Increase buffer reference count. */
    ccl_memobj_ref(b);

    /* Check that buffer ref count is 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) b));

    /* Increase buffer reference count again, this time using helper
     * macro. */
    ccl_buffer_ref(b);

    /* Check that buffer ref count is 3. */
    g_assert_cmpuint(3, ==, ccl_wrapper_ref_count((CCLWrapper *) b));

    /* Unref buffer, twice. */
    ccl_buffer_unref(b);
    ccl_buffer_unref(b);

    /* Check that buffer ref count is 1. */
    g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper *) b));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy stuff. */
    ccl_buffer_unref(b);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests buffer wrapping and unwrapping.
 * */
static void wrap_unwrap_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * b = NULL;
    CCLBuffer * b_aux = NULL;
    cl_mem buffer = NULL;
    CCLErr * err = NULL;
    size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;
    cl_int status;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Create a buffer using OpenCL functions directly. */
    buffer = clCreateBuffer(
        ccl_context_unwrap(ctx), CL_MEM_READ_ONLY, buf_size, NULL, &status);
    g_assert_cmpint(status, ==, CL_SUCCESS);

    /* Wrap buffer. */
    b = ccl_buffer_new_wrap(buffer);

    /* If we now unwrap the wrapper, we must get the originally created
     * buffer. */
    g_assert(buffer == ccl_buffer_unwrap(b));

    /* If we again wrap the original buffer... */
    b_aux = ccl_buffer_new_wrap(buffer);

    /* ...we must get the same wrapper... */
    g_assert(b == b_aux);

    /* ... and the buffer wrapper ref count must be 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) b));

    /* Unref buffer, twice. */
    ccl_buffer_unref(b);

    /* Check that buffer ref count is 1. */
    g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper *) b));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy stuff. */
    ccl_buffer_destroy(b);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests basic read/write operations from/to buffer objects.
 * */
static void read_write_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b = NULL;
    CCLQueue * q;
    cl_uint h_in[CCL_TEST_BUFFER_SIZE];
    cl_uint h_out[CCL_TEST_BUFFER_SIZE];
    size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;

    /* Create a host array, put some stuff in it. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        h_in[i] = g_test_rand_int();

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create regular buffer and write data from the host buffer. */
    b = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, h_in, &err);
    g_assert_no_error(err);

    /* Read data back to host. */
    ccl_buffer_enqueue_read(
        b, q, CL_TRUE, 0, buf_size, (void *) h_out, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        g_assert_cmpuint(h_in[i], ==, h_out[i]);

    /* Set some other data in host array. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        h_in[i] = g_test_rand_int();

    /* Write it explicitly to buffer. */
    ccl_buffer_enqueue_write(
        b, q, CL_TRUE, 0, buf_size, (void *) h_in, NULL, &err);
    g_assert_no_error(err);

    /* Read new data to host. */
    ccl_buffer_enqueue_read(
        b, q, CL_TRUE, 0, buf_size, (void *) h_out, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        g_assert_cmpuint(h_in[i], ==, h_out[i]);

    /* Do an invalid read, check if error is thrown. */
    ccl_buffer_enqueue_read(
        b, q, CL_TRUE, 0, buf_size, NULL, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

    /* Do an invalid write, check if error is thrown. */
    ccl_buffer_enqueue_write(
        b, q, CL_TRUE, 0, buf_size, NULL, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b);
    ccl_queue_destroy(q);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests copy operations from one buffer to another.
 * */
static void copy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b1 = NULL;
    CCLBuffer * b2 = NULL;
    CCLQueue * q;
    cl_long h1[CCL_TEST_BUFFER_SIZE];
    cl_long h2[CCL_TEST_BUFFER_SIZE];
    size_t buf_size = sizeof(cl_long) * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;

    /* Create a host array, put some stuff in it. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        h1[i] = g_test_rand_int();

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create regular buffer and write data from the host buffer. */
    b1 = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, h1, &err);
    g_assert_no_error(err);

    /* Create another buffer, double the size. */
    b2 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, 2 * buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Copy data from first buffer to second buffer, using an offset on
     * the second buffer. */
    ccl_buffer_enqueue_copy(
        b1, b2, q, 0, buf_size / 2, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Read data back to host from the second buffer. */
    ccl_buffer_enqueue_read(
        b2, q, CL_TRUE, buf_size / 2, buf_size, h2, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        g_assert_cmpuint(h1[i], ==, h2[i]);

    /* Do an invalid copy, check if error is thrown. */
    ccl_buffer_enqueue_copy(
        b1, b1, q, 0, 0, buf_size, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_MEM_COPY_OVERLAP);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b1);
    ccl_buffer_destroy(b2);
    ccl_queue_destroy(q);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests map/unmap operations in buffer objects.
 * */
static void map_unmap_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b = NULL;
    CCLQueue * q;
    cl_uint h_in[CCL_TEST_BUFFER_SIZE];
    cl_uint * h_out;
    size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;

    /* Create a host array, put some stuff in it. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        h_in[i] = g_test_rand_int();

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create regular buffer and write data from the host buffer. */
    b = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, h_in, &err);
    g_assert_no_error(err);

    /* Map buffer onto host memory. */
    h_out = ccl_buffer_enqueue_map(
        b, q, CL_TRUE, CL_MAP_READ, 0, buf_size, NULL, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        g_assert_cmpuint(h_in[i], ==, h_out[i]);

    /* Unmap buffer. */
    ccl_memobj_enqueue_unmap(
        (CCLMemObj *) b, q, h_out, NULL, &err);
    g_assert_no_error(err);

    /* Do an invalid map, check if error is thrown. */
    ccl_buffer_enqueue_map(
        b, q, CL_TRUE, CL_MAP_READ, buf_size, buf_size, NULL, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b);
    ccl_queue_destroy(q);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());
}

#ifdef CL_VERSION_1_1

/**
 * @internal
 *
 * @brief Test callback function.
 *
 * @param memobj Memory object.
 * @param user_data User data.
 * */
static void CL_CALLBACK destructor_callback(
    cl_mem memobj, void * user_data) {

    /* The memory object cannot be NULL. */
    g_assert(memobj != NULL);

    /* Set userdata to CL_TRUE, thus providing evidence that the
     * callback was indeed called. */
    *((cl_bool *) user_data) = CL_TRUE;
}

#endif

/**
 * @internal
 *
 * @brief Test memory object destructor callbacks.
 * */
static void destructor_callback_test() {

#ifndef CL_VERSION_1_1

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.1 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLBuffer * b = NULL;
    CCLErr * err = NULL;
    GTimer * timer = NULL;
    cl_bool test_var = CL_FALSE;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(110, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Create a buffer. */
    b = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE, 128 * sizeof(cl_uint), NULL, &err);

    /* Add destructor callback. */
    ccl_memobj_set_destructor_callback(
        (CCLMemObj *) b, destructor_callback, &test_var, &err);
    g_assert_no_error(err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy buffer. */
    ccl_buffer_destroy(b);

    /* Destroy context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());

    /* Wait some more... */
    timer = g_timer_new();
    while (g_timer_elapsed(timer, NULL) < 2.0);
    g_timer_stop(timer);
    g_timer_destroy(timer);

    /* Confirm that test_var is CL_TRUE. */
    g_assert_cmpuint(test_var, ==, CL_TRUE);

#endif

}

/**
 * @internal
 *
 * @brief Tests rect buffer operations.
 * */
static void rect_read_write_copy_test() {

#ifndef CL_VERSION_1_1

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.1 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b1 = NULL;
    CCLBuffer * b2 = NULL;
    CCLQueue * cq;
    cl_uchar h1[CCL_TEST_BUFFER_SIZE * CCL_TEST_BUFFER_SIZE];
    cl_uchar h2[CCL_TEST_BUFFER_SIZE * CCL_TEST_BUFFER_SIZE];
    size_t buf_size = sizeof(cl_uchar) * sizeof(cl_uchar)
        * CCL_TEST_BUFFER_SIZE * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;
    const size_t origin[] = {0, 0, 0};
    const size_t region[] = {
        CCL_TEST_BUFFER_SIZE * sizeof(cl_uchar),
        CCL_TEST_BUFFER_SIZE * sizeof(cl_uchar),
        1 };
    const size_t invalid_region[] = {
        10 * CCL_TEST_BUFFER_SIZE * sizeof(cl_uchar),
        10 * CCL_TEST_BUFFER_SIZE * sizeof(cl_uchar),
        1 };

    /* Create a "2D" host array, put some stuff in it. */
    for (cl_uint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        for (cl_uint j = 0; j < CCL_TEST_BUFFER_SIZE; ++j)
            h1[i * CCL_TEST_BUFFER_SIZE + j] =
                (cl_uchar) (g_test_rand_int() % 0xFF);

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(110, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    cq = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create device buffers. */
    b1 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);
    b2 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Write "rect" data to first buffer in device. */
    ccl_buffer_enqueue_write_rect(
        b1, cq, CL_TRUE, origin, origin, region, 0, 0, 0, 0, h1, NULL, &err);
    g_assert_no_error(err);

    /* Copy "rect" data from first buffer to second buffer. */
    ccl_buffer_enqueue_copy_rect(
        b1, b2, cq, origin, origin, region, 0, 0, 0, 0, NULL, &err);
    g_assert_no_error(err);

    /* Read data "rect" back to host from the second buffer. */
    ccl_buffer_enqueue_read_rect(
        b2, cq, CL_TRUE, origin, origin, region, 0, 0, 0, 0, h2, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK doing a flat comparison. */
    for (cl_uint i = 0; i < CCL_TEST_BUFFER_SIZE * CCL_TEST_BUFFER_SIZE; ++i)
        g_assert_cmpuint(h1[i], ==, h2[i]);

    /* Invoke "rect" read with erroneous parameters. */
    ccl_buffer_enqueue_read_rect(
        b2, cq, CL_TRUE, origin, origin, invalid_region,
        0, 0, 0, 0, h2, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b1);
    ccl_buffer_destroy(b2);
    ccl_queue_destroy(cq);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());

#endif

}

/**
 * @internal
 *
 * @brief Tests the ccl_buffer_new_from_region() function.
 * */
static void create_from_region_test() {

#ifndef CL_VERSION_1_1

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.1 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * cq = NULL;
    CCLBuffer * buf = NULL;
    CCLBuffer * subbuf = NULL;
    CCLEvent * evt = NULL;
    CCLEventWaitList ewl = NULL;
    CCLErr * err = NULL;
    cl_ulong * hbuf;
    cl_ulong * hsubbuf;
    cl_uint min_align;
    size_t siz_buf;
    size_t siz_subbuf;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(110, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Get minimum alignment for sub-buffer in bits. */
    min_align = ccl_device_get_info_scalar(
        dev, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint, &err);
    g_assert_no_error(err);

    /* Determine buffer and sub-buffer sizes (divide by 64 because its
     * the number of bits in cl_ulong). */
    siz_subbuf = sizeof(cl_ulong) * min_align / 64;
    siz_buf = 4 * siz_subbuf;

    /* Allocate memory for host buffer and host sub-buffer. */
    hbuf = g_slice_alloc(siz_buf);
    hsubbuf = g_slice_alloc(siz_subbuf);

    /* Initialize initial host buffer. */
    for (cl_uint i = 0; i < siz_buf / sizeof(cl_ulong); ++i)
        hbuf[i] = g_test_rand_int();

    /* Create a command queue. */
    cq = ccl_queue_new(ctx, dev, 0, &err);
    g_assert_no_error(err);

    /* Create a regular buffer, put some data in it. */
    buf = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, siz_buf, hbuf, &err);
    g_assert_no_error(err);

    /* Create sub-buffer from indexes 16 to 31 (16 positions) of
     * original buffer. */
    subbuf = ccl_buffer_new_from_region(
        buf, 0, siz_subbuf, siz_subbuf, &err);
    g_assert_no_error(err);

    /* Get data in sub-buffer to a new host buffer. */
    evt = ccl_buffer_enqueue_read(
        subbuf, cq, CL_FALSE, 0, siz_subbuf, hsubbuf, NULL, &err);
    g_assert_no_error(err);

    /* Wait for read to be complete. */
    ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
    g_assert_no_error(err);

    /* Check that expected values were successfully read. */
    for (cl_uint i = 0; i < siz_subbuf / sizeof(cl_ulong); ++i)
        g_assert_cmpuint(
            hsubbuf[i], ==, hbuf[i + siz_subbuf / sizeof(cl_ulong)]);

    /* Destroy sub-buffer. */
    ccl_buffer_destroy(subbuf);

    /* Try to create an invalid sub-buffer and check for the appropriate
     * error. */
    subbuf = ccl_buffer_new_from_region(
        buf, 0, siz_buf * 2, siz_buf, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    g_clear_error(&err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy stuff. */
    ccl_buffer_destroy(buf);
    ccl_queue_destroy(cq);
    ccl_context_destroy(ctx);
    g_slice_free1(siz_buf, hbuf);
    g_slice_free1(siz_subbuf, hsubbuf);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());

#endif
}


/**
 * @internal
 *
 * @brief Tests buffer fill.
 * */
static void fill_test() {

#ifndef CL_VERSION_1_2

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.2 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b = NULL;
    CCLQueue * q;
    cl_char8 h[CCL_TEST_BUFFER_SIZE];
    cl_char8 pattern = {{ 1, -1, 5, 4, -12, 3, 7, -20 }};
    size_t buf_size = sizeof(cl_char8) * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(120, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create regular buffer. */
    b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Fill buffer with pattern. */
    ccl_buffer_enqueue_fill(
        b, q, &pattern, sizeof(cl_char8), 0, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Read data back to host. */
    ccl_buffer_enqueue_read(b, q, CL_TRUE, 0, buf_size, h, NULL, &err);
    g_assert_no_error(err);

    /* Check data is OK. */
    for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
        for (guint j = 0; j < 8; ++j)
            g_assert_cmpuint(h[i].s[j], ==, pattern.s[j]);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b);
    ccl_queue_destroy(q);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());

#endif

}

/**
 * @internal
 *
 * @brief Tests memory object migration.
 * */
static void migrate_test() {

#ifndef CL_VERSION_1_2

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.2 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLBuffer * b = NULL;
    CCLQueue * q;
    size_t buf_size = sizeof(cl_char8) * CCL_TEST_BUFFER_SIZE;
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(120, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue associated with first device in
     * context. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create regular buffer. */
    b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Assign buffer to first device in context (via the command
     * queue). */
    ccl_memobj_enqueue_migrate((CCLMemObj **) &b, 1, q, 0, NULL, &err);
    g_assert_no_error(err);

    /* Migrate buffer to host. */
    ccl_memobj_enqueue_migrate(
        (CCLMemObj **) &b, 1, q, CL_MIGRATE_MEM_OBJECT_HOST, NULL, &err);
    g_assert_no_error(err);

    /* Wait for queue to finish... */
    ccl_queue_finish(q, &err);
    g_assert_no_error(err);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free stuff. */
    ccl_buffer_destroy(b);
    ccl_queue_destroy(q);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly
     * freed. */
    g_assert(ccl_wrapper_memcheck());

#endif

}

/**
 * @internal
 *
 * @brief Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char ** argv) {

    g_test_init(&argc, &argv, NULL);

    g_test_add_func(
        "/wrappers/buffer/create-info-destroy",
        create_info_destroy_test);

    g_test_add_func(
        "/wrappers/buffer/ref-unref",
        ref_unref_test);

    g_test_add_func(
        "/wrappers/buffer/wrap-unwrap",
        wrap_unwrap_test);

    g_test_add_func(
        "/wrappers/buffer/read-write",
        read_write_test);

    g_test_add_func(
        "/wrappers/buffer/copy",
        copy_test);

    g_test_add_func(
        "/wrappers/buffer/map-unmap",
        map_unmap_test);

    g_test_add_func(
        "/wrappers/buffer/destruct_callback",
        destructor_callback_test);

    g_test_add_func(
        "/wrappers/buffer/rect-read-write-copy",
        rect_read_write_copy_test);

    g_test_add_func(
        "/wrappers/buffer/create-from-region",
        create_from_region_test);

    g_test_add_func(
        "/wrappers/buffer/fill",
        fill_test);

    g_test_add_func(
        "/wrappers/buffer/migrate",
        migrate_test);

    return g_test_run();
}
