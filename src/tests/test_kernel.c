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
 * Tests the ::ccl_kernel_suggest_worksizes() function.
 *
 * @todo Perform tests with a non-`NULL` kernel argument.
 * */
static void suggest_worksizes_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	GError* err = NULL;
	size_t rws[3], gws[3], lws[3], lws_max[3];

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Perform test 20 times with different values. */
	for (cl_uint i = 0; i < 200; ++i) {

		/* ************************* */
		/* ******* 1-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a
		 * real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 1, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		check_dev_limits(dev, 1, lws);

		/* 2. Request a local work size, forcing the global work size to
		 * be equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 1, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		check_dev_limits(dev, 1, lws);

		/* 3. Request for global work size and local work sizes given a
		 * real work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, 0, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, 0, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 1, rws, gws, lws, &err);
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
		ccl_kernel_suggest_worksizes(NULL, dev, 1, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		check_dev_limits(dev, 1, lws);

		/* ************************* */
		/* ******* 2-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a real
		 * work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 2, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		check_dev_limits(dev, 2, lws);

		/* 2. Request a local work size, forcing the global work size to be
		 * equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 2, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		check_dev_limits(dev, 2, lws);

		/* 3. Request for global work size and local work sizes given a real
		 * work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 2, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		check_dev_limits(dev, 2, lws);

		/* 4. Request a local work size (but specifying a maximum), forcing
		 * the global work size to be equal to the real work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, 0);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, 0);
		ccl_kernel_suggest_worksizes(NULL, dev, 2, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		check_dev_limits(dev, 2, lws);

		/* ************************* */
		/* ******* 3-D tests ******* */
		/* ************************* */

		/* 1. Request for global work size and local work sizes given a real
		 * work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(NULL, dev, 3, rws, gws, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(gws[0], >=, rws[0]);
		g_assert_cmpuint(gws[1], >=, rws[1]);
		g_assert_cmpuint(gws[2], >=, rws[2]);
		g_assert_cmpuint(gws[0] % lws[0], ==, 0);
		g_assert_cmpuint(gws[1] % lws[1], ==, 0);
		g_assert_cmpuint(gws[2] % lws[2], ==, 0);
		check_dev_limits(dev, 3, lws);

		/* 2. Request a local work size, forcing the global work size to be
		 * equal to the real work size. */
		WS_INIT(lws, 0, 0, 0);
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(NULL, dev, 3, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(rws[2] % lws[2], ==, 0);
		check_dev_limits(dev, 3, lws);

		/* 3. Request for global work size and local work sizes given a real
		 * work size and a maximum local work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, RAND_LWS);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(NULL, dev, 3, rws, gws, lws, &err);
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

		/* 4. Request a local work size (but specifying a maximum), forcing
		 * the global work size to be equal to the real work size. */
		WS_INIT(lws_max, RAND_LWS, RAND_LWS, RAND_LWS);
		memcpy(lws, lws_max, 3 * sizeof(size_t));
		WS_INIT(rws, RAND_RWS, RAND_RWS, RAND_RWS);
		ccl_kernel_suggest_worksizes(NULL, dev, 3, rws, NULL, lws, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(rws[0] % lws[0], ==, 0);
		g_assert_cmpuint(rws[1] % lws[1], ==, 0);
		g_assert_cmpuint(rws[2] % lws[2], ==, 0);
		g_assert_cmpuint(lws[0], <=, lws_max[0]);
		g_assert_cmpuint(lws[1], <=, lws_max[1]);
		g_assert_cmpuint(lws[2], <=, lws_max[2]);
		check_dev_limits(dev, 3, lws);

	}

	/* Destroy stuff. */
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
		"/wrappers/kernel/suggest-worksizes",
		suggest_worksizes_test);

	return g_test_run();
}



