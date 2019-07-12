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
 * Test the image wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"
#include "_ccl_defs.h"

#define CCL_TEST_IMAGE_WIDTH 64
#define CCL_TEST_IMAGE_HEIGHT 64

/**
 * @internal
 *
 * @brief Tests creation, getting info from and destruction of
 * image wrapper objects.
 * */
static void create_info_destroy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLImage * img = NULL;
    cl_mem image = NULL;
    CCLErr * err = NULL;
    cl_int ocl_status;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    CCLImageDesc img_dsc = CCL_IMAGE_DESC_BLANK;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Test three ways to create an image. */
    for (cl_uint i = 0; i < 3; ++i) {

        /* Create image wrapper. */
        switch (i) {
            case 0:
                /* The regular way. */
                img = ccl_image_new(
                    ctx, CL_MEM_READ_WRITE, &image_format, NULL, &err,
                    "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
                    "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
                    "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
                    NULL);
                g_assert_no_error(err);
                break;
            case 1:
                /* Using the struct constructor. */
                img_dsc.image_type = CL_MEM_OBJECT_IMAGE2D;
                img_dsc.image_width = CCL_TEST_IMAGE_WIDTH;
                img_dsc.image_height = CCL_TEST_IMAGE_HEIGHT;
                img = ccl_image_new_v(ctx, CL_MEM_READ_WRITE,
                    &image_format, &img_dsc, NULL, &err);
                g_assert_no_error(err);
                break;
            case 2:
                /* Using the "wrap" constructor. */
                CCL_BEGIN_IGNORE_DEPRECATIONS
                image = clCreateImage2D(ccl_context_unwrap(ctx),
                    CL_MEM_READ_WRITE, &image_format,
                    CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 0,
                    NULL, &ocl_status);
                CCL_END_IGNORE_DEPRECATIONS
                g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
                img = ccl_image_new_wrap(image);
                g_assert_cmphex(GPOINTER_TO_SIZE(image), ==,
                    GPOINTER_TO_SIZE(ccl_image_unwrap(img)));
                break;
        }

        /* Get some info and check if the return value is as expected. */

        /* Generic memory object queries. */
        cl_mem_object_type mot;
        mot = ccl_memobj_get_info_scalar(
            img, CL_MEM_TYPE, cl_mem_object_type, &err);
        g_assert_no_error(err);
        g_assert_cmphex(mot, ==, CL_MEM_OBJECT_IMAGE2D);

        cl_mem_flags flags;
        flags = ccl_memobj_get_info_scalar(
            img, CL_MEM_FLAGS, cl_mem_flags, &err);
        g_assert_no_error(err);
        g_assert_cmphex(flags, ==, CL_MEM_READ_WRITE);

        void * host_ptr;
        host_ptr = ccl_memobj_get_info_scalar(
            img, CL_MEM_HOST_PTR, void *, &err);
        g_assert(
            (err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
        g_assert_cmphex(GPOINTER_TO_SIZE(host_ptr), ==,
            GPOINTER_TO_SIZE(NULL));
        ccl_err_clear(&err);

        cl_context context;
        context = ccl_memobj_get_info_scalar(
            img, CL_MEM_CONTEXT, cl_context, &err);
        g_assert_no_error(err);
        g_assert_cmphex(GPOINTER_TO_SIZE(context), ==,
            GPOINTER_TO_SIZE(ccl_context_unwrap(ctx)));

        /* Specific image queries. */
        cl_image_format img_fmt;
        img_fmt = ccl_image_get_info_scalar(
            img, CL_IMAGE_FORMAT, cl_image_format, &err);
        g_assert_no_error(err);
        g_assert_cmphex(img_fmt.image_channel_order, ==,
            image_format.image_channel_order);
        g_assert_cmphex(img_fmt.image_channel_data_type, ==,
            image_format.image_channel_data_type);

        size_t elem_size;
        elem_size = ccl_image_get_info_scalar(
            img, CL_IMAGE_ELEMENT_SIZE, size_t, &err);
        g_assert_no_error(err);
        g_assert_cmpuint(elem_size, ==, 4); /* Four channels of 1 byte each. */

        size_t width;
        width = ccl_image_get_info_scalar(
            img, CL_IMAGE_WIDTH, size_t, &err);
        g_assert_no_error(err);
        g_assert_cmpuint(width, ==, CCL_TEST_IMAGE_WIDTH);

        size_t height;
        height = ccl_image_get_info_scalar(
            img, CL_IMAGE_HEIGHT, size_t, &err);
        g_assert_no_error(err);
        g_assert_cmpuint(height, ==, CCL_TEST_IMAGE_HEIGHT);

        /* Destroy image. */
        ccl_image_destroy(img);
    }
}

/**
 * @internal
 *
 * @brief Tests image wrapper class reference counting.
 * */
static void ref_unref_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLImage * img = NULL;
    CCLErr * err = NULL;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Create 2D image. */
    img = ccl_image_new(
        ctx, CL_MEM_READ_WRITE, &image_format, NULL, &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Increase image reference count. */
    ccl_memobj_ref(img);

    /* Check that image ref count is 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) img));

    /* Increase again, this time using the helper macro. */
    ccl_image_ref(img);

    /* Check that image ref count is 3. */
    g_assert_cmpuint(3, ==, ccl_wrapper_ref_count((CCLWrapper *) img));

    /* Unref image. */
    ccl_image_unref(img);

    /* Check that image ref count is 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) img));

    /* Unref image. */
    ccl_image_unref(img);

    /* Check that image ref count is 1. */
    g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper *) img));

    /* Destroy stuff. */
    ccl_image_unref(img);
}

/**
 * @internal
 *
 * @brief Tests basic read/write operations from/to image objects.
 * */
static void read_write_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLImage * img = NULL;
    CCLQueue * q;
    gint32 himg_in[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Create a random 4-channel 8-bit image (i.e. each pixel has 32
     * bits). */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        himg_in[i] = g_test_rand_int();

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create 2D image and copy data from the host memory. */
    img = ccl_image_new(
        ctx,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        &image_format,
        himg_in,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Read image data back to host. */
    ccl_image_enqueue_read(
        img, q, CL_TRUE, origin, region, 0, 0, himg_out, NULL, &err);
    g_assert_no_error(err);

    /* Check image data is OK. */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

    /* Create some other image data. */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        himg_in[i] = g_test_rand_int();

    /* Write it explicitly to device image. */
    ccl_image_enqueue_write(
        img, q, CL_TRUE, origin, region, 0, 0, himg_in, NULL, &err);
    g_assert_no_error(err);

    /* Read new image data to host. */
    ccl_image_enqueue_read(
        img, q, CL_TRUE, origin, region, 0, 0, himg_out, NULL, &err);
    g_assert_no_error(err);

    /* Check image data is OK. */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

    /* Free stuff. */
    ccl_image_destroy(img);
    ccl_queue_destroy(q);
}

/**
 * @internal
 *
 * @brief Tests copy operations from one image to another.
 * */
static void copy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLImage * img1 = NULL;
    CCLImage * img2 = NULL;
    CCLQueue * q;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    gint32 himg_in[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    size_t src_origin[3] = {0, 0, 0};
    size_t dst_origin[3] =
        {CCL_TEST_IMAGE_WIDTH / 2, CCL_TEST_IMAGE_WIDTH / 2, 0};
    size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Create a random 4-channel 8-bit image (i.e. each pixel has 32
     * bits). */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        himg_in[i] = g_test_rand_int();

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create 2D image and copy data from the host memory. */
    img1 = ccl_image_new(
        ctx,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        &image_format,
        himg_in,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Create another image, double the dimensions of the previous one. */
    img2 = ccl_image_new(
        ctx,
        CL_MEM_READ_WRITE,
        &image_format,
        NULL,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) (CCL_TEST_IMAGE_WIDTH * 2),
        "image_height", (size_t) (CCL_TEST_IMAGE_HEIGHT * 2),
        NULL);
    g_assert_no_error(err);

    /* Copy data from first image to second image, using an offset on
     * the second image. */
    ccl_image_enqueue_copy(
        img1, img2, q, src_origin, dst_origin, region, NULL, &err);
    g_assert_no_error(err);

    /* Read image data back to host. */
    ccl_image_enqueue_read(
        img2, q, CL_TRUE, dst_origin, region, 0, 0, himg_out, NULL, &err);
    g_assert_no_error(err);

    /* Check image data is OK. */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

    /* Free stuff. */
    ccl_image_destroy(img1);
    ccl_image_destroy(img2);
    ccl_queue_destroy(q);
}

/**
 * @internal
 *
 * @brief Tests map/unmap operations in image objects.
 * */
static void map_unmap_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLEvent * evt = NULL;
    CCLImage * img = NULL;
    CCLQueue * q = NULL;
    CCLEventWaitList ewl = NULL;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    gint32 himg[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    gint32 * himg_map;
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
    size_t image_row_pitch;
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Check that a context is set. */
    if (ctx == NULL) {
        /* If not, skip test. */
        g_test_fail();
        g_test_message("An appropriate device for this test was not found.");
        return;
    }

    /* Create a random 4-channel 8-bit image (i.e. each pixel has 32
     * bits). */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        himg[i] = g_test_rand_int();

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create 2D image, copy data from host. */
    img = ccl_image_new(
        ctx,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        &image_format,
        himg,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Map image onto host memory. */
    himg_map = ccl_image_enqueue_map(
        img, q, CL_FALSE, CL_MAP_READ, origin, region,
        &image_row_pitch, NULL, NULL, &evt, &err);
    g_assert_no_error(err);

    /* Wait before mapping is complete. */
    ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
    g_assert_no_error(err);

    /* Compare image in device with image in host. */
    for (cl_uint i = 0; i < CCL_TEST_IMAGE_HEIGHT; ++i) {
        for (cl_uint j = 0; j < image_row_pitch; ++j) {
            if (j < CCL_TEST_IMAGE_WIDTH) {
                g_assert_cmphex(
                    himg_map[i * CCL_TEST_IMAGE_WIDTH + j],
                    ==,
                    himg[i * CCL_TEST_IMAGE_WIDTH + j]);
            }
        }
    }

    /* Unmap image. */
    ccl_image_enqueue_unmap(img, q, himg_map, NULL, &err);
    g_assert_no_error(err);

    /* Free stuff. */
    ccl_image_destroy(img);
    ccl_queue_destroy(q);
}

/**
 * @internal
 *
 * @brief Tests copy image to buffer and buffer to image functions.
 * */
static void copy_buffer_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLEvent * evt = NULL;
    CCLImage * img1 = NULL;
    CCLImage * img2 = NULL;
    CCLBuffer * buf = NULL;
    CCLQueue * cq = NULL;
    CCLEventWaitList ewl = NULL;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    cl_uint himg_in[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    cl_uint himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(0, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Create a random 4-channel 8-bit image (i.e. each pixel has 32
     * bits). */
    for (cl_uint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        himg_in[i] = (cl_uint) g_test_rand_int();

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    cq = ccl_queue_new(ctx, dev, 0, &err);
    g_assert_no_error(err);

    /* Create 2D image, copy data from host. */
    img1 = ccl_image_new(
        ctx,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        &image_format,
        himg_in,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Create destination 2D image. */
    img2 = ccl_image_new(
        ctx,
        CL_MEM_WRITE_ONLY,
        &image_format,
        NULL,
        &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Create buffer with enough size to hold image. */
    buf = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE,
        CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT * sizeof(cl_uint),
        NULL, &err);
    g_assert_no_error(err);

    /* Copy image to buffer. */
    ccl_image_enqueue_copy_to_buffer(
        img1, buf, cq, origin, region, 0, NULL, &err);
    g_assert_no_error(err);

    /* Copy buffer to new image. */
    ccl_buffer_enqueue_copy_to_image(
        buf, img2, cq, 0, origin, region, NULL, &err);
    g_assert_no_error(err);

    /* Read image to host. */
    evt = ccl_image_enqueue_read(
        img2, cq, CL_FALSE, origin, region, 0, 0, himg_out, NULL, &err);
    g_assert_no_error(err);

    /* Wait for transfer. */
    ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
    g_assert_no_error(err);

    /* Check that the contents are the same as in the origin buffer. */
    for (cl_uint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
    {
        g_assert_cmpuint(himg_in[i], ==, himg_out[i]);
    }

    /* Try invalid buffer/image copies and check for errors. */
    region[0] = 4 * CCL_TEST_IMAGE_WIDTH;

    ccl_image_enqueue_copy_to_buffer(
        img1, buf, cq, origin, region, 0, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    ccl_err_clear(&err);

    ccl_buffer_enqueue_copy_to_image(
        buf, img2, cq, 0, origin, region, NULL, &err);
    g_assert_error(err, CCL_OCL_ERROR, CL_INVALID_VALUE);
    ccl_err_clear(&err);

    /* Free stuff. */
    ccl_image_destroy(img1);
    ccl_image_destroy(img2);
    ccl_buffer_destroy(buf);
    ccl_queue_destroy(cq);
}

/**
 * @internal
 *
 * @brief Tests image fill.
 * */
static void fill_test() {

#ifndef CL_VERSION_1_2

    g_test_skip(
        "Test skipped due to lack of OpenCL 1.2 support.");

#else

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLImage * img = NULL;
    CCLQueue * q;
    cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
    gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
    const size_t origin[3] = {0, 0, 0};
    const size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
    CCLErr * err = NULL;

    /* Create a random color 4-channel 8-bit color (i.e. color has 32 bits). */
    gint32 rc = g_test_rand_int();
    cl_uint4 color = {{
        rc & 0xFF,
        (rc >> 8) & 0xFF,
        (rc >> 16) & 0xFF,
        (rc >> 24) & 0xFF
    }};

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new_with_image_support(120, &err);
    g_assert_no_error(err);
    if (!ctx) return;

    /* Get first device in context. */
    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    q = ccl_queue_new(ctx, d, 0, &err);
    g_assert_no_error(err);

    /* Create 2D image. */
    img = ccl_image_new(
        ctx, CL_MEM_READ_WRITE, &image_format, NULL, &err,
        "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
        "image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
        "image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
        NULL);
    g_assert_no_error(err);

    /* Fill image with color. */
    ccl_image_enqueue_fill(
        img, q, &color, origin, region, NULL, &err);
    g_assert_no_error(err);

    /* Read image data back to host. */
    ccl_image_enqueue_read(
        img, q, CL_TRUE, origin, region, 0, 0, himg_out, NULL, &err);
    g_assert_no_error(err);

    /* Check if data is Ok. */
    for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
        g_assert_cmphex(rc, ==, himg_out[i]);

    /* Free stuff. */
    ccl_image_destroy(img);
    ccl_queue_destroy(q);

#endif /* CL_VERSION_1_2 */

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
        "/wrappers/image/create-info-destroy",
        create_info_destroy_test);

    g_test_add_func(
        "/wrappers/image/ref-unref",
        ref_unref_test);

    g_test_add_func(
        "/wrappers/image/read-write",
        read_write_test);

    g_test_add_func(
        "/wrappers/image/copy",
        copy_test);

    g_test_add_func(
        "/wrappers/image/map-unmap",
        map_unmap_test);

    g_test_add_func(
        "/wrappers/image/copy-buffer",
        copy_buffer_test);

    g_test_add_func(
        "/wrappers/image/fill",
        fill_test);

    return g_test_run();
}
