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
 * Test the buffer wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

#define CCL_TEST_BUFFER_SIZE 512

/**
 * Tests creation, getting info from and destruction of
 * buffer wrapper objects.
 * */
static void buffer_create_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLBuffer* b = NULL;
	GError* err = NULL;
	size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create regular buffer. */
	b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
	g_assert_no_error(err);

	/* Get some info and check if the return value is as expected. */
	cl_mem_object_type mot;
	mot = ccl_memobj_get_info_scalar(
		b, CL_MEM_TYPE, cl_mem_object_type, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(mot, ==, CL_MEM_OBJECT_BUFFER);

	cl_mem_flags flags;
	flags = ccl_memobj_get_info_scalar(
		b, CL_MEM_FLAGS, cl_mem_flags, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(flags, ==, CL_MEM_READ_WRITE);

	size_t mem_size;
	mem_size = ccl_memobj_get_info_scalar(b, CL_MEM_SIZE, size_t, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(mem_size, ==, buf_size);

	void* host_ptr = NULL;
	host_ptr = ccl_memobj_get_info_scalar(
		b, CL_MEM_HOST_PTR, void*, &err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
	g_assert_cmphex((gulong) host_ptr, ==, (gulong) NULL);

	cl_context context;
	context = ccl_memobj_get_info_scalar(
		b, CL_MEM_CONTEXT, cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex((gulong) context, ==, (gulong) ccl_context_unwrap(ctx));

	/* Destroy stuff. */
	ccl_buffer_destroy(b);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests buffer wrapper class reference counting.
 * */
static void buffer_ref_unref_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLBuffer* b = NULL;
	GError* err = NULL;
	size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create regular buffer. */
	b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
	g_assert_no_error(err);

	/* Increase buffer reference count. */
	ccl_memobj_ref(b);

	/* Check that buffer ref count is 2. */
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) b));

	/* Unref buffer. */
	ccl_buffer_unref(b);

	/* Check that buffer ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) b));

	/* Destroy stuff. */
	ccl_buffer_unref(b);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Tests basic read/write operations from/to buffer objects.
 * */
static void buffer_read_write() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* d = NULL;
	CCLBuffer* b = NULL;
	CCLQueue* q;
	cl_uint h_in[CCL_TEST_BUFFER_SIZE];
	cl_uint h_out[CCL_TEST_BUFFER_SIZE];
	size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;
	GError* err = NULL;

	/* Create a host array, put some stuff in it. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		h_in[i] = g_test_rand_int();

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(ctx, d, 0, &err);
	g_assert_no_error(err);

	/* Create regular buffer and write data from the host buffer. */
	b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		buf_size, h_in, &err);
	g_assert_no_error(err);

	/* Read data back to host. */
	ccl_buffer_enqueue_read(b, q, CL_TRUE, 0, buf_size, (void*) h_out,
		NULL, &err);
	g_assert_no_error(err);

	/* Check data is OK. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		g_assert_cmpuint(h_in[i], ==, h_out[i]);

	/* Set some other data in host array. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		h_in[i] = g_test_rand_int();

	/* Write it explicitly to buffer. */
	ccl_buffer_enqueue_write(b, q, CL_TRUE, 0, buf_size, (void*) h_in,
		NULL, &err);
	g_assert_no_error(err);

	/* Read new data to host. */
	ccl_buffer_enqueue_read(b, q, CL_TRUE, 0, buf_size, (void*) h_out,
		NULL, &err);
	g_assert_no_error(err);

	/* Check data is OK. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		g_assert_cmpuint(h_in[i], ==, h_out[i]);

	/* Free stuff. */
	ccl_buffer_destroy(b);
	ccl_queue_destroy(q);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests copy operations from one buffer to another.
 * */
static void buffer_copy() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* d = NULL;
	CCLBuffer* b1 = NULL;
	CCLBuffer* b2 = NULL;
	CCLQueue* q;
	cl_long h1[CCL_TEST_BUFFER_SIZE];
	cl_long h2[CCL_TEST_BUFFER_SIZE];
	size_t buf_size = sizeof(cl_long) * CCL_TEST_BUFFER_SIZE;
	GError* err = NULL;

	/* Create a host array, put some stuff in it. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		h1[i] = g_test_rand_int();

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(ctx, d, 0, &err);
	g_assert_no_error(err);

	/* Create regular buffer and write data from the host buffer. */
	b1 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		buf_size, h1, &err);
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
	ccl_buffer_enqueue_read(b2, q, CL_TRUE, buf_size / 2, buf_size, h2,
		NULL, &err);
	g_assert_no_error(err);

	/* Check data is OK. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		g_assert_cmpuint(h1[i], ==, h2[i]);


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
 * Tests map/unmap operations in buffer objects.
 * */
static void buffer_map_unmap() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* d = NULL;
	CCLBuffer* b = NULL;
	CCLQueue* q;
	cl_uint h_in[CCL_TEST_BUFFER_SIZE];
	cl_uint* h_out;
	size_t buf_size = sizeof(cl_uint) * CCL_TEST_BUFFER_SIZE;
	GError* err = NULL;

	/* Create a host array, put some stuff in it. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		h_in[i] = g_test_rand_int();

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	q = ccl_queue_new(ctx, d, 0, &err);
	g_assert_no_error(err);

	/* Create regular buffer and write data from the host buffer. */
	b = ccl_buffer_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		buf_size, h_in, &err);
	g_assert_no_error(err);

	/* Map buffer onto host memory. */
	h_out = ccl_buffer_enqueue_map(b, q, CL_TRUE, CL_MAP_READ, 0,
		buf_size, NULL, NULL, &err);
	g_assert_no_error(err);

	/* Check data is OK. */
	for (guint i = 0; i < CCL_TEST_BUFFER_SIZE; ++i)
		g_assert_cmpuint(h_in[i], ==, h_out[i]);

	/* Unmap buffer. */
	ccl_memobj_enqueue_unmap((CCLMemObj*) b, q, h_out, NULL, &err);
	g_assert_no_error(err);

	/* Free stuff. */
	ccl_buffer_destroy(b);
	ccl_queue_destroy(q);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

#ifdef CL_VERSION_1_2

/**
 * Tests buffer fill.
 * */
static void buffer_fill() {

	/* Test variables. */
	CCLPlatforms* ps;
	CCLPlatform* p;
	CCLContext* ctx = NULL;
	CCLDevice* d = NULL;
	CCLBuffer* b = NULL;
	CCLQueue* q;
	cl_char8 h[CCL_TEST_BUFFER_SIZE];
	cl_char8 pattern = {{ 1, -1, 5, 4, -12, 3, 7, -20 }};
	size_t buf_size = sizeof(cl_char8) * CCL_TEST_BUFFER_SIZE;
	GError* err = NULL;

	/* Get a context which supports OpenCL 1.2, if possible. */
	ps = ccl_platforms_new(&err);
	g_assert_no_error(err);
	for (guint i = 0; i < ccl_platforms_count(ps); ++i) {
		p = ccl_platforms_get(ps, i);
		double ocl_ver = ccl_platform_get_opencl_version(p, &err);
		if (ocl_ver >= 120) {
			ctx = ccl_context_new_from_devices(
				ccl_platform_get_num_devices(p, NULL),
				ccl_platform_get_all_devices(p, NULL),
				&err);
			g_assert_no_error(err);
			break;
		}
	}

	/* If not possible to find a 1.2 or better context, finish this
	 * test. */
	if (ctx == NULL) {
		g_test_fail();
		g_test_message("'%s' test not performed because no platform " \
			"with OpenCL 1.2 support was found", __func__);
		ccl_platforms_destroy(ps);
		return;
	}

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

	/* Free stuff. */
	ccl_buffer_destroy(b);
	ccl_queue_destroy(q);
	ccl_context_destroy(ctx);
	ccl_platforms_destroy(ps);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

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

	g_test_add_func(
		"/wrappers/buffer/create-info-destroy",
		buffer_create_info_destroy_test);

	g_test_add_func(
		"/wrappers/buffer/ref-unref",
		buffer_ref_unref_test);

	g_test_add_func(
		"/wrappers/buffer/read-write",
		buffer_read_write);

	g_test_add_func(
		"/wrappers/buffer/copy",
		buffer_copy);

	g_test_add_func(
		"/wrappers/buffer/map-unmap",
		buffer_map_unmap);

#ifdef CL_VERSION_1_2
	g_test_add_func(
		"/wrappers/buffer/fill",
		buffer_fill);
#endif

	return g_test_run();
}



