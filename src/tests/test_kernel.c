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
 * Test the kernel wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

#define CCL_TEST_KERNEL_NAME "test_krnl"

#define CCL_TEST_KERNEL_CONTENT \
	"__kernel void " CCL_TEST_KERNEL_NAME "(__global uint *buf)\n" \
	"{\n" \
	"	int gid = get_global_id(0);\n" \
	"	buf[gid] = buf[gid] + 1;\n" \
	"}\n"


#define CCL_TEST_KERNEL_BUF_SIZE 16
#define CCL_TEST_KERNEL_LWS 8 /* Must be a divisor of CCL_TEST_KERNEL_BUF_SIZE */
G_STATIC_ASSERT(CCL_TEST_KERNEL_BUF_SIZE % CCL_TEST_KERNEL_LWS == 0);

/**
 * Tests creation, getting info from and destruction of
 * kernel wrapper objects.
 * */
static void create_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	cl_context context = NULL;
	CCLProgram* prg = NULL;
	cl_program program = NULL;
	CCLKernel* krnl = NULL;
	cl_kernel kernel = NULL;
	CCLDevice* d = NULL;
	CCLQueue* cq = NULL;
	size_t gws;
	size_t lws;
	cl_uint host_buf[CCL_TEST_KERNEL_BUF_SIZE];
	cl_uint host_buf_aux[CCL_TEST_KERNEL_BUF_SIZE];
	CCLBuffer* buf;
	GError* err = NULL;
	CCLEvent* evt = NULL;
	CCLEventWaitList ewl = NULL;
	const char* krnl_name;
	void* args[] = { NULL, NULL };
	cl_bool release_krnl;
	cl_int ocl_status;

	/* Create a context with devices from first available platform. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Create a new program from source and build it. */
	prg = ccl_program_new_from_source(
		ctx, CCL_TEST_KERNEL_CONTENT, &err);
	g_assert_no_error(err);

	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);

	/* Test three ways to create a kernel wrapper. */
	for (cl_uint i = 0; i < 3; ++i) {

		/* Create kernel wrapper. */
		switch (i) {
			case 0:
				/* Instantiate kernel directly. */
				krnl = ccl_kernel_new(prg, CCL_TEST_KERNEL_NAME, &err);
				g_assert_no_error(err);
				release_krnl = CL_TRUE;
				break;
			case 1:
				/* Using the program utility function. No need to free
				 * kernel in this case, because it will be freed when
				 * program is destroyed. */
				krnl = ccl_program_get_kernel(
					prg, CCL_TEST_KERNEL_NAME, &err);
				g_assert_no_error(err);
				release_krnl = CL_FALSE;
				break;
			case 2:
				/* Using the "wrap" constructor. */
				kernel = clCreateKernel(ccl_program_unwrap(prg),
					CCL_TEST_KERNEL_NAME, &ocl_status);
				g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
				krnl = ccl_kernel_new_wrap(kernel);
				g_assert_cmphex(GPOINTER_TO_UINT(kernel), ==,
					GPOINTER_TO_UINT(ccl_kernel_unwrap(krnl)));
				release_krnl = CL_TRUE;
				break;
		}

		/* Get some kernel info, compare it with expected info. */

		/* Get kernel function name from kernel info, compare it with the
		 * expected value. */
		krnl_name = ccl_kernel_get_info_array(
			krnl, CL_KERNEL_FUNCTION_NAME, char*, &err);
		g_assert_no_error(err);
		g_assert_cmpstr(krnl_name, ==, CCL_TEST_KERNEL_NAME);

		/* Check if the kernel context is the same as the initial context
		 * and the program context. */
		context = ccl_kernel_get_info_scalar(
			krnl, CL_KERNEL_CONTEXT, cl_context, &err);
		g_assert_no_error(err);
		g_assert(context == ccl_context_unwrap(ctx));

		program = ccl_kernel_get_info_scalar(
			krnl, CL_KERNEL_PROGRAM, cl_program, &err);
		g_assert_no_error(err);
		g_assert(program == ccl_program_unwrap(prg));

#ifndef OPENCL_STUB

		cl_uint ocl_ver;

		/* Get OpenCL version of kernel's underlying platform. */
		ocl_ver = ccl_kernel_get_opencl_version(krnl, &err);
		g_assert_no_error(err);

#ifdef CL_VERSION_1_1

		size_t kwgz;
		size_t* kcwgs;
		CCLDevice* dev = NULL;

		/* If platform supports kernel work group queries, get kernel
		 * work group information and compare it with expected info. */
		if (ocl_ver >= 110) {

			dev = ccl_context_get_device(ctx, 0, &err);
			g_assert_no_error(err);

			kwgz = ccl_kernel_get_workgroup_info_scalar(
				krnl, dev, CL_KERNEL_WORK_GROUP_SIZE, size_t, &err);
			g_assert_no_error(err);
			(void)kwgz;

			kcwgs = ccl_kernel_get_workgroup_info_array(krnl, dev,
				CL_KERNEL_COMPILE_WORK_GROUP_SIZE, size_t*, &err);
			g_assert_no_error(err);
			(void)kcwgs;

		}

#endif /* ifdef CL_VERSION_1_1 */

#ifdef CL_VERSION_1_2

		cl_kernel_arg_address_qualifier kaaq;
		char* kernel_arg_type_name;
		char* kernel_arg_name;

		/* If platform supports kernel argument queries, get kernel argument
		 * information and compare it with expected info. */
		if (ocl_ver >= 120) {

			kaaq = ccl_kernel_get_arg_info_scalar(krnl, 0,
					CL_KERNEL_ARG_ADDRESS_QUALIFIER,
					cl_kernel_arg_address_qualifier, &err);
			g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
			if (err == NULL) {
				g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_GLOBAL);
			} else {
				g_clear_error(&err);
			}

			kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 0,
				CL_KERNEL_ARG_TYPE_NAME, char*, &err);
			g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
			if (err == NULL) {
				g_assert_cmpstr(kernel_arg_type_name, ==, "uint*");
			} else {
				g_clear_error(&err);
			}

			kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 0,
				CL_KERNEL_ARG_NAME, char*, &err);
			if (err == NULL) {
				g_assert_cmpstr(kernel_arg_name, ==, "buf");
			} else {
				g_clear_error(&err);
			}

		}

#endif /* ifdef CL_VERSION_1_2 */

#endif /* ifndef OCL_STUB */

		/* Set kernel enqueue properties and initialize host data. */
		gws = CCL_TEST_KERNEL_BUF_SIZE;
		lws = CCL_TEST_KERNEL_LWS;

		for (cl_uint i = 0; i < CCL_TEST_KERNEL_BUF_SIZE; ++i) {
			host_buf[i] = i + 1;
		}

		/* Create device buffer. */
		buf = ccl_buffer_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			CCL_TEST_KERNEL_BUF_SIZE * sizeof(cl_uint), host_buf, &err);
		g_assert_no_error(err);

		/* Set args and execute kernel. */
		args[0] = buf;
		ccl_kernel_set_args_v(krnl, args);
		ccl_kernel_enqueue_ndrange(krnl, cq, 1, NULL, &gws, &lws, NULL, &err);
		g_assert_no_error(err);

		/* Read back results to host. */
		evt = ccl_buffer_enqueue_read(buf, cq, CL_FALSE, 0,
			CCL_TEST_KERNEL_BUF_SIZE * sizeof(cl_uint), host_buf_aux,
			NULL, &err);
		g_assert_no_error(err);

		/* Wait for all events in wait list to terminate (this will empty
		 * the wait list). */
		ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
		g_assert_no_error(err);

#ifndef OPENCL_STUB
		/* Check results are as expected (not available with OpenCL stub). */
		for (cl_uint i = 0; i < CCL_TEST_KERNEL_BUF_SIZE; ++i) {
			g_assert_cmpuint(host_buf[i] + 1, ==, host_buf_aux[i]);
		}
#endif

		/* Destroy kernel, if required. */
		if (release_krnl) ccl_kernel_destroy(krnl);

		/* Destroy the memory objects. */
		ccl_buffer_destroy(buf);
	}

	/* Destroy the command queue. */
	ccl_queue_destroy(cq);

	/* Destroy stuff. */
	ccl_program_destroy(prg);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Test increasing reference count of kernel wrappers.
 * */
static void ref_unref_test() {

	CCLContext* ctx = NULL;
	GError* err = NULL;
	CCLProgram* prg = NULL;
	CCLKernel* krnl1 = NULL;
	CCLKernel* krnl2 = NULL;

	/* Get some context. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Create a program from source. */
	prg = ccl_program_new_from_source(
		ctx, CCL_TEST_KERNEL_CONTENT, &err);
	g_assert_no_error(err);

	/* Build program. */
	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper from program (will be the instance kept in the
	 * program wrapper). */
	krnl1 = ccl_program_get_kernel(prg, CCL_TEST_KERNEL_NAME, &err);
	g_assert_no_error(err);

	/* Create another kernel wrapper for the same kernel. This should
	 * yield a different object because we're not getting it from
	 * the program wrapper. */
	krnl2 = ccl_kernel_new(prg, CCL_TEST_KERNEL_NAME, &err);
	g_assert_no_error(err);

	/* Check that they're different. */
	g_assert_cmphex(
		GPOINTER_TO_UINT(krnl1), !=, GPOINTER_TO_UINT(krnl2));

	/* Check that each has a ref count of 1. */
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) krnl1), ==, 1);
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) krnl2), ==, 1);

	/* Increment the ref count of the directly created kernel. */
	ccl_kernel_ref(krnl2);
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) krnl1), ==, 1);
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) krnl2), ==, 2);

	/* Get rid of the directly created kernel. */
	ccl_kernel_unref(krnl2);
	ccl_kernel_unref(krnl2);

	/* Get kernel wrapper from program again (will be the instance kept
	 * in the program wrapper). */
	krnl2 = ccl_program_get_kernel(prg, CCL_TEST_KERNEL_NAME, &err);
	g_assert_no_error(err);

	/* Check that it's the same as krnl1. */
	g_assert_cmphex(
		GPOINTER_TO_UINT(krnl1), ==, GPOINTER_TO_UINT(krnl2));

	/* Destroy remaining stuff. */
	ccl_program_destroy(prg);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

#define WS_INIT(ws, a, b, c) \
	ws[0] = a; ws[1] = b; ws[2] = c;

#define RAND_LWS (size_t) (1 << g_test_rand_int_range(1, 8))
#define RAND_RWS (size_t) g_test_rand_int_range(1, G_MAXINT32)

/**
 * @internal
 * Helper function for ::suggest_worksizes_test() which checks if
 * suggested work sizes are within device limits.
 * */
static void check_dev_limits(CCLDevice* dev, cl_uint dims, size_t* lws) {

	/* Error handling object. */
	GError* err = NULL;

	/* Max device workgroup size. */
	size_t max_wgsize = ccl_device_get_info_scalar(
		dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, &err);
	g_assert_no_error(err);

	/* Max device workitem sizes. */
	size_t* max_wisizes = ccl_device_get_info_array(
		dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, size_t*, &err);
	g_assert_no_error(err);

	/* Suggested work group size. */
	size_t wgsize = 1;

	/* Check dimension by dimension. */
	for (cl_uint i = 0; i < dims; ++i) {

		/* Check workitem size. */
		g_assert_cmpuint(lws[i], <=, max_wisizes[i]);

		/* Update work group size with size in current dimension. */
		wgsize *= lws[i];
	}

	/* Check work group size. */
	g_assert_cmpuint(wgsize, <=, max_wgsize);

}

/**
 * @internal
 *
 * Aux. function for suggest_worksizes_test()-
 * */
static void suggest_worksizes_aux(CCLDevice* dev, CCLKernel* krnl) {

	/* Test variables. */
	GError* err = NULL;
	size_t rws[3], gws[3], lws[3], lws_max[3];

	/* Perform test 20 times with different values. */
	for (cl_uint i = 0; i < 200; ++i) {

		/* ************************* */
		/* ******* 1-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a
		 * real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 1, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		check_dev_limits(dev, 1, lws);

		/* 2. Request a local work size, forcing the global work size to
		 * be equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 1, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		check_dev_limits(dev, 1, lws);

		/* 3. Request for global work size and local work sizes given a
		 * real work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, 0, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 1, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		check_dev_limits(dev, 1, lws);

		/* 4. Request a local work size (but specifying a maximum),
		 * forcing the global work size to be equal to the real work
		 * size. */
		WS_INIT(lws_max, RAND_LWS, 0, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 1, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		check_dev_limits(dev, 1, lws);

		/* ************************* */
		/* ******* 2-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a
		 * real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 2, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		check_dev_limits(dev, 2, lws);

		/* 2. Request a local work size, forcing the global work size to
		 * be equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 2, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		check_dev_limits(dev, 2, lws);

		/* 3. Request for global work size and local work sizes given a
		 * real work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 2, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		check_dev_limits(dev, 2, lws);

		/* 4. Request a local work size (but specifying a maximum),
		 * forcing the global work size to be equal to the real work
		 * size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(krnl, dev, 2, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		check_dev_limits(dev, 2, lws);

		/* ************************* */
		/* ******* 3-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a
		 * real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(krnl, dev, 3, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[2], >=, rws[2]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		g_assert_cmpuint(gws[2] % lws[2], ==, 0);
		check_dev_limits(dev, 3, lws);

		/* 2. Request a local work size, forcing the global work size to
		 * be equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(krnl, dev, 3, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(rws[2] % lws[2], ==, 0);
		check_dev_limits(dev, 3, lws);

		/* 3. Request for global work size and local work sizes given a
		 * real work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, RAND_LWS);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(krnl, dev, 3, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[2], >=, rws[2]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		g_assert_cmpuint(gws[2] % lws[2], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		g_assert_cmpuint(lws[2], <=, lws_max[2]);
		check_dev_limits(dev, 3, lws);

		/* 4. Request a local work size (but specifying a maximum),
		 * forcing the global work size to be equal to the real work
		 * size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, RAND_LWS);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(krnl, dev, 3, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(rws[2] % lws[2], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		g_assert_cmpuint(lws[2], <=, lws_max[2]);
		check_dev_limits(dev, 3, lws);

	}

}

/**
 * Tests the ::ccl_kernel_suggest_worksizes() function.
 *
 * */
static void suggest_worksizes_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	GError* err = NULL;

	/* Get the test context with the pre-defined device. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Test with NULL kernel. */
	suggest_worksizes_aux(dev, NULL);

#ifndef OPENCL_STUB

	/* Kernel info is not functional with the OCL stub, so this
	 * test will only take place with a real OCL implementation. */

	CCLProgram* prg = NULL;
	CCLKernel* krnl = NULL;

	/* Create and build program. */
	prg = ccl_program_new_from_source(
		ctx, CCL_TEST_KERNEL_CONTENT, &err);
	g_assert_no_error(err);

	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper object. */
	krnl = ccl_program_get_kernel(prg, CCL_TEST_KERNEL_NAME, &err);
	g_assert_no_error(err);

	/* Test with non-NULL kernel. */
	suggest_worksizes_aux(dev, krnl);

	/* Destroy program. */
	ccl_program_destroy(prg);

#endif

	/* Destroy stuff. */
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}


/* ******************************************** */
/* ********* Test kernel arguments ************ */
/* ******************************************** */

#define CCL_TEST_KERNEL_ARGS_NAME "test_krnl_args"

#define CCL_TEST_KERNEL_ARGS_CONTENT \
	"__kernel void " CCL_TEST_KERNEL_ARGS_NAME "(" \
	"	__global uint *buf,\n" \
	"	__read_only image2d_t img,\n" \
	"	sampler_t sampler,\n" \
	"	__local uint* loc,\n" \
	"	uint x)\n" \
	"{\n" \
	"	uint gid = get_global_id(0);\n" \
	"	uint lid = get_local_id(0);\n" \
	"	int2 coord = (int2) (gid, 1);\n" \
	"	uint4 point = read_imageui(img, sampler, coord);\n" \
	"	loc[lid] = point.x + point.y + point.z + point.w;\n" \
	"	buf[gid] = loc[lid] + x;\n" \
	"}\n"


#define CCL_TEST_KERNEL_ARGS_BUF_SIZE 16
#define CCL_TEST_KERNEL_ARGS_LWS 8 /* Must be a divisor of CCL_TEST_ARGS_KERNEL_BUF_SIZE */
G_STATIC_ASSERT(CCL_TEST_KERNEL_ARGS_BUF_SIZE % CCL_TEST_KERNEL_ARGS_LWS == 0);

/**
 * Tests functions and macros related with kernel arguments.
 * */
static void args_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLProgram* prg = NULL;
	CCLKernel* krnl = NULL;
	CCLImage* img = NULL;
	CCLBuffer* buf = NULL;
	CCLQueue* cq = NULL;
	CCLEvent* evt = NULL;
	CCLSampler* smplr = NULL;
	CCLEventWaitList ewl = NULL;
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
	union {cl_uint u; cl_uchar c[4];} himg[CCL_TEST_KERNEL_ARGS_BUF_SIZE];
	cl_uint hbuf[CCL_TEST_KERNEL_ARGS_BUF_SIZE];
	GError* err = NULL;
	size_t gws = CCL_TEST_KERNEL_ARGS_BUF_SIZE;
	size_t lws = CCL_TEST_KERNEL_ARGS_LWS;
	void* args[6];
	cl_uint to_sum = 3;
	char* krnl_name;

	/* **************************************************** */
	/* 1 - Test different types of arguments with a kernel. */
	/* **************************************************** */

	/* Get the test context with the pre-defined device. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Create a random 4-channel 8-bit image (i.e. each pixel has 32
	 * bits). */
	for (cl_uint i = 0; i < CCL_TEST_KERNEL_ARGS_BUF_SIZE; ++i)
		himg[i].u = (cl_uint) g_test_rand_int();

	/* Create 2D image, copy data from host. */
	img = ccl_image_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, himg, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CCL_TEST_KERNEL_ARGS_BUF_SIZE,
		"image_height", (size_t) 1,
		NULL);
	g_assert_no_error(err);

	/* Create buffer. */
	buf = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY,
		sizeof(cl_uint) * CCL_TEST_KERNEL_ARGS_BUF_SIZE, NULL, &err);
	g_assert_no_error(err);

	/* Create sampler (this could also be created in-kernel). */
	smplr = ccl_sampler_new(ctx, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE,
		CL_FILTER_NEAREST, &err);

	/* Create and build program. */
	prg = ccl_program_new_from_source(
		ctx, CCL_TEST_KERNEL_ARGS_CONTENT, &err);
	g_assert_no_error(err);

	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper object. */
	krnl = ccl_program_get_kernel(prg, CCL_TEST_KERNEL_ARGS_NAME, &err);
	g_assert_no_error(err);

	/* Get some kernel info, compare it with expected info. */

	/* Get kernel function name from kernel info, compare it with the
	 * expected value. */
	krnl_name = ccl_kernel_get_info_array(
		krnl, CL_KERNEL_FUNCTION_NAME, char*, &err);
	g_assert_no_error(err);
	g_assert_cmpstr(krnl_name, ==, CCL_TEST_KERNEL_ARGS_NAME);

#ifndef OPENCL_STUB
#ifdef CL_VERSION_1_2

	cl_uint ocl_ver;
	cl_kernel_arg_address_qualifier kaaq;
	char* kernel_arg_type_name;
	char* kernel_arg_name;

	/* Get OpenCL version of kernel's underlying platform. */
	ocl_ver = ccl_kernel_get_opencl_version(krnl, &err);
	g_assert_no_error(err);

	/* If platform supports kernel argument queries, get kernel argument
	 * information and compare it with expected info. */
	if (ocl_ver >= 120) {

		/* First kernel argument. */
		kaaq = ccl_kernel_get_arg_info_scalar(krnl, 0,
				CL_KERNEL_ARG_ADDRESS_QUALIFIER,
				cl_kernel_arg_address_qualifier, &err);
		g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
		if (err == NULL) {
			g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_GLOBAL);
		} else {
			g_clear_error(&err);
		}

		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 0,
			CL_KERNEL_ARG_TYPE_NAME, char*, &err);
		g_assert((err == NULL) || (err->code == CCL_ERROR_INFO_UNAVAILABLE_OCL));
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_type_name, ==, "uint*");
		} else {
			g_clear_error(&err);
		}

		kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 0,
			CL_KERNEL_ARG_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_name, ==, "buf");
		} else {
			g_clear_error(&err);
		}

		/* Second kernel argument. */
		kaaq = ccl_kernel_get_arg_info_scalar(krnl, 1,
				CL_KERNEL_ARG_ADDRESS_QUALIFIER,
				cl_kernel_arg_address_qualifier, &err);
		if (err == NULL) {
			g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_GLOBAL);
		} else {
			g_clear_error(&err);
		}

		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 1,
			CL_KERNEL_ARG_TYPE_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_type_name, ==, "image2d_t");
		} else {
			g_clear_error(&err);
		}

		kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 1,
			CL_KERNEL_ARG_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_name, ==, "img");
		} else {
			g_clear_error(&err);
		}

		/* Third kernel argument. */
		kaaq = ccl_kernel_get_arg_info_scalar(krnl, 2,
				CL_KERNEL_ARG_ADDRESS_QUALIFIER,
				cl_kernel_arg_address_qualifier, &err);
		if (err == NULL) {
			g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_PRIVATE);
		} else {
			g_clear_error(&err);
		}

		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 2,
			CL_KERNEL_ARG_TYPE_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_type_name, ==, "sampler_t");
		} else {
			g_clear_error(&err);
		}

		kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 2,
			CL_KERNEL_ARG_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_name, ==, "sampler");
		} else {
			g_clear_error(&err);
		}

		/* Fourth kernel argument. */
		kaaq = ccl_kernel_get_arg_info_scalar(krnl, 3,
				CL_KERNEL_ARG_ADDRESS_QUALIFIER,
				cl_kernel_arg_address_qualifier, &err);
		if (err == NULL) {
			g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_LOCAL);
		} else {
			g_clear_error(&err);
		}

		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 3,
			CL_KERNEL_ARG_TYPE_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_type_name, ==, "uint*");
		} else {
			g_clear_error(&err);
		}

		kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 3,
			CL_KERNEL_ARG_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_name, ==, "loc");
		} else {
			g_clear_error(&err);
		}

		/* Fifth kernel argument. */
		kaaq = ccl_kernel_get_arg_info_scalar(krnl, 4,
				CL_KERNEL_ARG_ADDRESS_QUALIFIER,
				cl_kernel_arg_address_qualifier, &err);
		if (err == NULL) {
			g_assert_cmphex(kaaq, ==, CL_KERNEL_ARG_ADDRESS_PRIVATE);
		} else {
			g_clear_error(&err);
		}

		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 4,
			CL_KERNEL_ARG_TYPE_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_type_name, ==, "uint");
		} else {
			g_clear_error(&err);
		}

		kernel_arg_name = ccl_kernel_get_arg_info_array(krnl, 4,
			CL_KERNEL_ARG_NAME, char*, &err);
		if (err == NULL) {
			g_assert_cmpstr(kernel_arg_name, ==, "x");
		} else {
			g_clear_error(&err);
		}

		/* Bogus request, should return NULL and should raise an error. */
		kernel_arg_type_name = ccl_kernel_get_arg_info_array(krnl, 0,
			0 /* invalid value */, char*, &err);
		g_assert(kernel_arg_type_name == NULL);
		g_assert (err != NULL);
		g_clear_error(&err);

	}

#endif
#endif

	/* Set args array. */
	args[0] = buf;
	args[1] = img;
	args[2] = smplr;
	args[3] = ccl_arg_local(lws, cl_uint);
	args[4] = ccl_arg_priv(to_sum, cl_uint);
	args[5] = NULL;

	/* Set args and execute kernel. */
	ccl_kernel_set_args_and_enqueue_ndrange_v(krnl, cq, 1, NULL,
		&gws, &lws, NULL, args, &err);
	g_assert_no_error(err);

	/* Get results. */
	evt = ccl_buffer_enqueue_read(buf, cq, CL_FALSE, 0,
		sizeof(cl_uint) * CCL_TEST_KERNEL_ARGS_BUF_SIZE,
		hbuf, NULL, &err);
	g_assert_no_error(err);

	/* Wait for transfer. */
	ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
	g_assert_no_error(err);

#ifndef OPENCL_STUB

	/* Check that results are as expected. */
	for (cl_uint i = 0; i < CCL_TEST_KERNEL_ARGS_BUF_SIZE; ++i)
		g_assert_cmpuint(hbuf[i], ==, himg[i].c[0] + himg[i].c[1] +
			himg[i].c[2] + himg[i].c[3] + to_sum);
		//~ printf("%5d == %5d (%3d, %3d, %3d, %3d, %3d)\n", hbuf[i],
			//~ himg[i].c[0] + himg[i].c[1] + himg[i].c[2] + himg[i].c[3] + to_sum,
			//~ himg[i].c[0], himg[i].c[1], himg[i].c[2], himg[i].c[3], to_sum);

#endif

	/* Destroy stuff. */
	ccl_sampler_destroy(smplr);
	ccl_image_destroy(img);
	ccl_buffer_destroy(buf);
	ccl_program_destroy(prg);
	ccl_queue_destroy(cq);
	ccl_context_destroy(ctx);

	/* ********************************************************** */
	/* 2 - Test kernel argument functions directly (these are not */
	/*     commonly used by client code).                         */
	/* ********************************************************** */

	cl_float pi = 3.1415f;
	cl_char c = 12;
	CCLArg* arg_test = NULL;

	arg_test = ccl_arg_new(&pi, sizeof(cl_float));
	g_assert(arg_test != NULL);
	g_assert_cmpuint(ccl_arg_size(arg_test), ==, sizeof(cl_float));
	g_assert_cmpfloat(pi, ==, *((cl_float*) ccl_arg_value(arg_test)));
	ccl_arg_destroy(arg_test);

	arg_test = NULL;
	arg_test = ccl_arg_full(&c, sizeof(cl_char));
	g_assert(arg_test != NULL);
	g_assert_cmpuint(ccl_arg_size(arg_test), ==, sizeof(cl_char));
	g_assert_cmpfloat(c, ==, *((cl_char*) ccl_arg_value(arg_test)));
	ccl_arg_destroy(arg_test);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/* ******************************************** */
/* **** Test ccl_kernel_enqueue_native() ****** */
/* ******************************************** */

#define CCL_TEST_KERNEL_NATIVE_BUF_SIZE 32

/* Data structure used for for the native_test. */
struct nk_args {
	cl_int* buf;
	cl_uint numel;
};

/* Native function used for the native_test. */
static void native_kernel(void* args) {

	struct nk_args* nka = (struct nk_args*) args;

	/* Perform some simple operation. */
	for (cl_uint i = 0; i < nka->numel; ++i)
		nka->buf[i] = nka->buf[i] + 1;

}

/**
 * Tests the ccl_kernel_enqueue_native() function.
 * */
static void native_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLBuffer* buf = NULL;
	CCLQueue* cq = NULL;
	GError* err = NULL;
	cl_int hbuf[CCL_TEST_KERNEL_NATIVE_BUF_SIZE];
	cl_int hbuf_out[CCL_TEST_KERNEL_NATIVE_BUF_SIZE];
	size_t bs = CCL_TEST_KERNEL_NATIVE_BUF_SIZE * sizeof(cl_int);
	cl_device_exec_capabilities exec_cap;
	struct nk_args args;
	const void* args_mem_loc;
	cl_uint i;

	/* Get the test context with the pre-defined device. */
	ctx = ccl_test_context_new(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Check if the device supports the execution of native kernels. */
	exec_cap = ccl_device_get_info_scalar(
		dev, CL_DEVICE_EXECUTION_CAPABILITIES,
		cl_device_exec_capabilities, &err);
	g_assert_no_error(err);

	/* If not, return. */
	if (!(exec_cap & CL_EXEC_NATIVE_KERNEL)) {
		g_test_message("Test device doesn't support native kernels." \
			"Native kernels test will not be performed.");
		return;
	}

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Initialize host buffer. */
	for (i = 0; i < CCL_TEST_KERNEL_NATIVE_BUF_SIZE; ++i)
		hbuf[i] = g_test_rand_int();

	/* Create device buffer, copy contents from host buffer. */
	buf = ccl_buffer_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		bs, hbuf, &err);
	g_assert_no_error(err);

	/* Initialize arguments for native kernel. We only initialize
	 * 'args.numel' because 'args.buf' will be setup by the OpenCL
	 * implementation using the device buffer 'buf'. */
	args.numel = CCL_TEST_KERNEL_NATIVE_BUF_SIZE;

	/* Here we specify the location of 'args.buf', which the OpenCL
	 * implementation will setup using data in the device buffer
	 * 'buf'. */
	args_mem_loc = (const void*) &args.buf;

	/* Test the ccl_kernel_enqueue_native() function. */
	ccl_kernel_enqueue_native(cq, native_kernel, &args,
		sizeof(struct nk_args), 1, (CCLMemObj* const*) &buf,
		&args_mem_loc, NULL, &err);
	g_assert_no_error(err);

	/* Read device buffer, modified by native kernel. */
	ccl_buffer_enqueue_read(
		buf, cq, CL_FALSE, 0, bs, hbuf_out, NULL, &err);
	g_assert_no_error(err);

	/* Wait for queue operations to complete. */
	ccl_queue_finish(cq, &err);
	g_assert_no_error(err);

	/* Check that buffer was properly modified. */
	for (i = 0; i < CCL_TEST_KERNEL_NATIVE_BUF_SIZE; ++i)
		g_assert_cmpint(hbuf[i] + 1, ==, hbuf_out[i]);

	/* Destroy stuff. */
	ccl_buffer_destroy(buf);
	ccl_queue_destroy(cq);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add_func(
		"/wrappers/kernel/create-info-destroy",
		create_info_destroy_test);

	g_test_add_func(
		"/wrappers/kernel/ref-unref",
		ref_unref_test);

	g_test_add_func(
		"/wrappers/kernel/suggest-worksizes",
		suggest_worksizes_test);

	g_test_add_func(
		"/wrappers/kernel/args",
		args_test);

	g_test_add_func(
		"/wrappers/kernel/native",
		native_test);

	return g_test_run();
}



