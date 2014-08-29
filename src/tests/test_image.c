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
 * @file
 * Test the image wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

#define CCL_TEST_IMAGE_WIDTH 64
#define CCL_TEST_IMAGE_HEIGHT 64

/**
 * Setup image tests by creating a context with an image-supporting
 * device. A minimum OpenCL version can be set in `user_data`.
 * */
static void context_with_image_support_setup(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	CCLPlatforms* ps;
	CCLPlatform* p;
	CCLDevice* d;
	GError* err = NULL;
	cl_uint num_devs;
	*ctx_fixt = NULL;
	cl_uint min_ocl_ver = 0;
	if (user_data != NULL)
		min_ocl_ver = *((cl_uint*) user_data);

	/* Get all OpenCL platforms in system. */
	ps = ccl_platforms_new(&err);
	g_assert_no_error(err);

	/* Cycle through platforms. */
	for (guint i = 0; i < ccl_platforms_count(ps); ++i) {

		/* Get next platform. */
		p = ccl_platforms_get(ps, i);

		/* Check if a minimum OpenCL version was set. */
		if (min_ocl_ver > 0) {
			/* If so make sure current platform has the required
			 * OpenCL version. */
			cl_uint p_ocl_ver = ccl_platform_get_opencl_version(p, &err);
			g_assert_no_error(err);
			if (p_ocl_ver < min_ocl_ver) {
				/* If not, go to next platform. */
				continue;
			}
		}

		/* Get number of devices in current platform. */
		num_devs = ccl_platform_get_num_devices(p, &err);
		g_assert_no_error(err);

		/* Cycle through devices. */
		for (guint j = 0; j < num_devs; ++j) {

			/* Get next device. */
			d = ccl_platform_get_device(p, j, &err);
			g_assert_no_error(err);
			/* Check if current device supports images. */
			cl_bool image_support = ccl_device_get_info_scalar(
				d, CL_DEVICE_IMAGE_SUPPORT, cl_bool, &err);
			g_assert_no_error(err);
			if (image_support) {
				/* If so, create a context with current device and
				 * return. */
				*ctx_fixt = ccl_context_new_from_devices(
					1, (CCLDevice* const*) &d, &err);
				g_assert_no_error(err);
				ccl_platforms_destroy(ps);
				return;
			}
		}
	}
	ccl_platforms_destroy(ps);
}

/**
 * Teardown image tests by releasing the created context.
 * */
static void context_with_image_support_teardown(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	user_data = user_data;

	/* If context was created, release it. */
	if (*ctx_fixt != NULL)
		ccl_context_destroy(*ctx_fixt);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Tests creation, getting info from and destruction of
 * image wrapper objects.
 * */
static void image_create_info_destroy_test(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	/* Test variables. */
	CCLImage* img = NULL;
	GError* err = NULL;
	user_data = user_data;
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };

	/* Check that a context is set. */
	if (*ctx_fixt == NULL) {
		/* If not, skip test. */
		g_test_fail();
		g_test_message("An appropriate device for this test was not found.");
		return;
	}

	/* Create 2D image. */
	img = ccl_image_new(
		*ctx_fixt, CL_MEM_READ_WRITE, &image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
		"image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
		NULL);
	g_assert_no_error(err);

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

	void* host_ptr;
	host_ptr = ccl_memobj_get_info_scalar(
		img, CL_MEM_HOST_PTR, void*, &err);
	g_assert_no_error(err);
	g_assert_cmphex((gulong) host_ptr, ==, (gulong) NULL);

	cl_context context;
	context = ccl_memobj_get_info_scalar(
		img, CL_MEM_CONTEXT, cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex((gulong) context, ==,
		(gulong) ccl_context_unwrap(*ctx_fixt));

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

	/* Destroy stuff. */
	ccl_image_destroy(img);

}

/**
 * Tests image wrapper class reference counting.
 * */
static void image_ref_unref_test(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	/* Test variables. */
	CCLImage* img = NULL;
	GError* err = NULL;
	user_data = user_data;
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };

	/* Check that a context is set. */
	if (*ctx_fixt == NULL) {
		/* If not, skip test. */
		g_test_fail();
		g_test_message("An appropriate device for this test was not found.");
		return;
	}

	/* Create 2D image. */
	img = ccl_image_new(
		*ctx_fixt, CL_MEM_READ_WRITE, &image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
		"image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
		NULL);
	g_assert_no_error(err);

	/* Increase image reference count. */
	ccl_memobj_ref(img);

	/* Check that image ref count is 2. */
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) img));

	/* Unref image. */
	ccl_image_unref(img);

	/* Check that image ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) img));

	/* Destroy stuff. */
	ccl_image_unref(img);

}

/**
 * Tests basic read/write operations from/to image objects.
 * */
static void image_read_write(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	/* Test variables. */
	CCLDevice* d = NULL;
	CCLImage* img = NULL;
	CCLQueue* q;
	gint32 himg_in[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
	gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
	GError* err = NULL;
	user_data = user_data;

	/* Check that a context is set. */
	if (*ctx_fixt == NULL) {
		/* If not, skip test. */
		g_test_fail();
		g_test_message("An appropriate device for this test was not found.");
		return;
	}

	/* Create a random 4-channel 8-bit image (i.e. each pixel has 32
	 * bits). */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		himg_in[i] = g_test_rand_int();

	/* Get first device in context. */
	d = ccl_context_get_device(*ctx_fixt, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(*ctx_fixt, d, 0, &err);
	g_assert_no_error(err);

	/* Create 2D image and copy data from the host memory. */
	img = ccl_image_new(*ctx_fixt, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		&image_format, himg_in, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
		"image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
		NULL);
	g_assert_no_error(err);

	/* Read image data back to host. */
	ccl_image_enqueue_read(q, img, CL_TRUE, origin, region, 0, 0,
		himg_out, NULL, &err);
	g_assert_no_error(err);

	/* Check image data is OK. */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

	/* Create some other image data. */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		himg_in[i] = g_test_rand_int();

	/* Write it explicitly to device image. */
	ccl_image_enqueue_write(q, img, CL_TRUE, origin, region, 0, 0,
		himg_in, NULL, &err);
	g_assert_no_error(err);

	/* Read new image data to host. */
	ccl_image_enqueue_read(q, img, CL_TRUE, origin, region, 0, 0,
		himg_out, NULL, &err);
	g_assert_no_error(err);

	/* Check image data is OK. */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

	/* Free stuff. */
	ccl_image_destroy(img);
	ccl_queue_destroy(q);

}

/**
 * Tests copy operations from one image to another.
 * */
static void image_copy(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	/* Test variables. */
	CCLDevice* d = NULL;
	CCLImage* img1 = NULL;
	CCLImage* img2 = NULL;
	CCLQueue* q;
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
	gint32 himg_in[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
	gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
	size_t src_origin[3] = {0, 0, 0};
	size_t dst_origin[3] =
		{CCL_TEST_IMAGE_WIDTH / 2, CCL_TEST_IMAGE_WIDTH / 2, 0};
	size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
	GError* err = NULL;
	user_data = user_data;

	/* Check that a context is set. */
	if (*ctx_fixt == NULL) {
		/* If not, skip test. */
		g_test_fail();
		g_test_message("An appropriate device for this test was not found.");
		return;
	}

	/* Create a random 4-channel 8-bit image (i.e. each pixel has 32
	 * bits). */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		himg_in[i] = g_test_rand_int();

	/* Get first device in context. */
	d = ccl_context_get_device(*ctx_fixt, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(*ctx_fixt, d, 0, &err);
	g_assert_no_error(err);

	/* Create 2D image and copy data from the host memory. */
	img1 = ccl_image_new(*ctx_fixt, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		&image_format, himg_in, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
		"image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
		NULL);
	g_assert_no_error(err);

	/* Create another image, double the dimensions of the previous one. */
	img2 = ccl_image_new(
		*ctx_fixt, CL_MEM_READ_WRITE, &image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) (CCL_TEST_IMAGE_WIDTH * 2),
		"image_height", (size_t) (CCL_TEST_IMAGE_HEIGHT * 2),
		NULL);
	g_assert_no_error(err);

	/* Copy data from first image to second image, using an offset on
	 * the second image. */
	ccl_image_enqueue_copy(
		q, img1, img2, src_origin, dst_origin, region, NULL, &err);
	g_assert_no_error(err);

	/* Read image data back to host. */
	ccl_image_enqueue_read(q, img2, CL_TRUE, dst_origin, region, 0, 0,
		himg_out, NULL, &err);
	g_assert_no_error(err);

	/* Check image data is OK. */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		g_assert_cmpuint(himg_in[i], ==, himg_out[i]);

	/* Free stuff. */
	ccl_image_destroy(img1);
	ccl_image_destroy(img2);
	ccl_queue_destroy(q);

}

#ifdef CL_VERSION_1_2

/**
 * Tests image fill.
 * */
static void image_fill(
	CCLContext** ctx_fixt, gconstpointer user_data) {

	/* Test variables. */
	CCLDevice* d = NULL;
	CCLImage* img = NULL;
	CCLQueue* q;
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
	gint32 himg_out[CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT];
	const size_t origin[3] = {0, 0, 0};
	const size_t region[3] = {CCL_TEST_IMAGE_WIDTH, CCL_TEST_IMAGE_HEIGHT, 1};
	GError* err = NULL;
	user_data = user_data;
	/* Create a random color 4-channel 8-bit color (i.e. color has 32
	 * bits). */
	gint32 rc = g_test_rand_int();
	cl_uint4 color = {{ rc & 0xFF,
			(rc >> 8) & 0xFF,
			(rc >> 16) & 0xFF,
			(rc >> 24) & 0xFF }};

	/* Check that a context is set. */
	if (*ctx_fixt == NULL) {
		/* If not, skip test. */
		g_test_fail();
		g_test_message("An appropriate device for this test was not found.");
		return;
	}

	/* Get first device in context. */
	d = ccl_context_get_device(*ctx_fixt, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(*ctx_fixt, d, 0, &err);
	g_assert_no_error(err);

	/* Create 2D image. */
	img = ccl_image_new(
		*ctx_fixt, CL_MEM_READ_WRITE, &image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_IMAGE_WIDTH,
		"image_height", (size_t) CCL_TEST_IMAGE_HEIGHT,
		NULL);
	g_assert_no_error(err);

	/* Fill image with color. */
	ccl_image_enqueue_fill(
		q, img, &color, origin, region, NULL, &err);
	g_assert_no_error(err);

	/* Read image data back to host. */
	ccl_image_enqueue_read(q, img, CL_TRUE, origin, region, 0, 0,
		himg_out, NULL, &err);
	g_assert_no_error(err);

	/* Check if data is Ok. */
	for (guint i = 0; i < CCL_TEST_IMAGE_WIDTH * CCL_TEST_IMAGE_HEIGHT; ++i)
		g_assert_cmphex(rc, ==, himg_out[i]);

	/* Free stuff. */
	ccl_image_destroy(img);
	ccl_queue_destroy(q);

}

#endif

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add(
		"/wrappers/image/create-info-destroy",
		CCLContext*, NULL, context_with_image_support_setup,
		image_create_info_destroy_test,
		context_with_image_support_teardown);

	g_test_add(
		"/wrappers/image/ref-unref",
		CCLContext*, NULL, context_with_image_support_setup,
		image_ref_unref_test,
		context_with_image_support_teardown);

	g_test_add(
		"/wrappers/image/read-write",
		CCLContext*, NULL, context_with_image_support_setup,
		image_read_write,
		context_with_image_support_teardown);

	g_test_add(
		"/wrappers/image/copy",
		CCLContext*, NULL, context_with_image_support_setup,
		image_copy,
		context_with_image_support_teardown);

#ifdef CL_VERSION_1_2
	cl_uint ocl_min_ver = 120;
	g_test_add(
		"/wrappers/image/fill",
		CCLContext*, &ocl_min_ver, context_with_image_support_setup,
		image_fill,
		context_with_image_support_teardown);
#endif

	return g_test_run();
}




