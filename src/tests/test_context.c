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
 * Test the context wrapper class and its methods. Also tests
 * device selection filters, device wrappers and platform wrappers.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include <glib/gstdio.h>

static const char* ccl_test_channel_order_string(cl_uint co) {
	switch(co) {
		case 0x10B0: return "CL_R";
		case 0x10B1: return "CL_A";
		case 0x10B2: return "CL_RG";
		case 0x10B3: return "CL_RA";
		case 0x10B4: return "CL_RGB";
		case 0x10B5: return "CL_RGBA";
		case 0x10B6: return "CL_BGRA";
		case 0x10B7: return "CL_ARGB";
		case 0x10B8: return "CL_INTENSITY";
		case 0x10B9: return "CL_LUMINANCE";
		case 0x10BA: return "CL_Rx";
		case 0x10BB: return "CL_RGx";
		case 0x10BC: return "CL_RGBx";
		case 0x10BD: return "CL_DEPTH";
		case 0x10BE: return "CL_DEPTH_STENCIL";
		case 0x10BF: return "CL_sRGB";
		case 0x10C0: return "CL_sRGBx";
		case 0x10C1: return "CL_sRGBA";
		case 0x10C2: return "CL_sBGRA";
		case 0x10C3: return "CL_ABGR";
	}
	return "Unknown";

}

static const char* ccl_test_channel_data_type_string(cl_uint cdt) {

	switch (cdt) {
		case 0x10D0: return "CL_SNORM_INT8";
		case 0x10D1: return "CL_SNORM_INT16";
		case 0x10D2: return "CL_UNORM_INT8";
		case 0x10D3: return "CL_UNORM_INT16";
		case 0x10D4: return "CL_UNORM_SHORT_565";
		case 0x10D5: return "CL_UNORM_SHORT_555";
		case 0x10D6: return "CL_UNORM_INT_101010";
		case 0x10D7: return "CL_SIGNED_INT8";
		case 0x10D8: return "CL_SIGNED_INT16";
		case 0x10D9: return "CL_SIGNED_INT32";
		case 0x10DA: return "CL_UNSIGNED_INT8";
		case 0x10DB: return "CL_UNSIGNED_INT16";
		case 0x10DC: return "CL_UNSIGNED_INT32";
		case 0x10DD: return "CL_HALF_FLOAT";
		case 0x10DE: return "CL_FLOAT";
		case 0x10DF: return "CL_UNORM_INT24";
	}
	return "Unknown";
}

/*
 * Independent pass-all filter for testing.
 * */
static cl_bool ccl_devsel_indep_test_true(
	CCLDevice* device, void *data, GError **err) {

	device = device;
	data = data;
	err = err;
	return CL_TRUE;

}

/**
 * Tests creation, getting info from and destruction of
 * context wrapper objects.
 * */
static void context_create_info_destroy_test() {

	CCLContext* ctx = NULL;
	GError* err = NULL;
	CCLPlatforms* ps = NULL;
	CCLPlatform* p = NULL;
	CCLDevice* d = NULL;
	cl_device_id d_id = NULL;
	CCLDevSelFilters filters = NULL;
	CCLWrapperInfo* info = NULL;
	cl_context_properties* ctx_props = NULL;
	cl_platform_id platform, platf_ref;
	cl_context context;
	cl_device_type device_type;
	guint num_devices;
	cl_int ocl_status;
	gboolean any_device;

	/*
	 * 1. Test context creation from devices.
	 * */

	/* Get platforms object. */
	ps = ccl_platforms_new(&err);
	g_assert_no_error(err);

	/* Get first platform wrapper from platforms object. */
	p = ccl_platforms_get(ps, 0);
	g_assert(p != NULL);

	/* Get first device wrapper from platform wrapper. */
	d = ccl_platform_get_device(p, 0, &err);
	g_assert_no_error(err);

	/* Unwrap cl_device_id from device wrapper object. */
	d_id = ccl_device_unwrap(d);

	/* Create a context from the device. */
	ctx = ccl_context_new_from_devices(1, &d, &err);
	g_assert_no_error(err);

	/* Get number of devices from context wrapper, check that this
	 * number is 1. */
#ifdef CL_VERSION_1_1
	info = ccl_context_get_info(ctx, CL_CONTEXT_NUM_DEVICES, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(*((cl_uint*) info->value), ==, 1);
#endif

	/* Get the cl_device_id from context via context info and check
	 * that it corresponds to the cl_device_id with which the context
	 * was created. */
	info = ccl_context_get_info(ctx, CL_CONTEXT_DEVICES, &err);
	g_assert_no_error(err);
	g_assert(((cl_device_id*) info->value)[0] == d_id);

	/* Check again that the number of devices is 1, this time not using
	 * CL_CONTEXT_NUM_DEVICES, which is not available in OpenCL 1.0. */
	g_assert_cmpuint(info->size / sizeof(cl_device_id), ==, 1);

	/* Free context. */
	ccl_context_destroy(ctx);

	/*
	 * 2. Test context creation by cl_context.
	 * */

	/* Create some context properties. */
	ctx_props = g_new0(cl_context_properties, 3);
	platform = (cl_platform_id) ccl_wrapper_unwrap((CCLWrapper*) p);
	ctx_props[0] = CL_CONTEXT_PLATFORM;
	ctx_props[1] = (cl_context_properties) platform;
	ctx_props[2] = 0;

	/* Create a CL context. */
	context = clCreateContext(
		(const cl_context_properties*) ctx_props,
		1, &d_id, NULL, NULL, &ocl_status);
	g_assert_cmpint(ocl_status, ==, CL_SUCCESS);

	/* Create a context wrapper using the cl_context, check that the
	 * unwrapped cl_context corresponds to the cl_context with which
	 * the context wrapper was created.*/
	ctx = ccl_context_new_wrap(context);
	g_assert(ccl_context_unwrap(ctx) == context);

	/* Get the first device wrapper from the context wrapper, check that
	 * the unwrapped cl_device_id corresponds to the cl_device_id with
	 * which the cl_context was created. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);
	g_assert(ccl_device_unwrap(d) == d_id);

	/* Check that the context number of devices taken using context
	 * info is 1. */
#ifdef CL_VERSION_1_1
	info = ccl_context_get_info(ctx, CL_CONTEXT_NUM_DEVICES, &err);
	g_assert_cmpuint(*((cl_uint*) info->value), ==, 1);
#else
	info = ccl_context_get_info(ctx, CL_CONTEXT_DEVICES, &err);
	g_assert_cmpuint(info->size / sizeof(cl_device_id), ==, 1);
#endif

	/* Free context, platforms and context properties. */
	ccl_context_destroy(ctx);
	ccl_platforms_destroy(ps);
	g_free(ctx_props);

	/*
	 * 3. Test context creation by device filtering
	 * (using shortcut macros).
	 * */

	/* For the next device type filters, at least one device must be
	 * found in order the test to pass. */
	any_device = FALSE;

	/* 3.1. GPU device type filter. */

	/* Check that either there was no error or that no GPU was found. */
	ctx = ccl_context_new_gpu(&err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		ccl_context_destroy(ctx);
	}

	/* 3.2. CPU device type filter. */

	/* Check that either there was no error or that no CPU was found. */
	ctx = ccl_context_new_cpu(&err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		ccl_context_destroy(ctx);
	}

	/* 3.3. Accel. device type filter. */

	/* Check that either there was no error or that no accelerator was
	 * found. */
	ctx = ccl_context_new_accel(&err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		ccl_context_destroy(ctx);
	}

	/* Check that at least one device type context was created. */
	g_assert(any_device);

	/* 3.4. Specific platform filter. */

	/* Check that a context wrapper was created. */
	ctx = ccl_context_new_from_indep_filter(
		ccl_devsel_indep_platform, (void*) platform, &err);
	g_assert_no_error(err);

	/* Check that context wrapper contains a device. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Check that the device platform corresponds to the expected
	 * platform (the one used in the filter). */
	platf_ref = ccl_device_get_info_scalar(d, CL_DEVICE_PLATFORM,
		cl_platform_id, &err);
	g_assert_no_error(err);
	g_assert(platf_ref == platform);

	/* Free context and set filters to NULL. */
	ccl_context_destroy(ctx);

	/*
	 * 4. Test context creation by device filtering
	 * (explicit dependent filters).
	 * */

	/* Same platform filter. */
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Check that a context wrapper was created. */
	ctx = ccl_context_new_from_filters(&filters, &err);
	g_assert_no_error(err);

	/* Check that context wrapper contains a device. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Check that the device platform corresponds to the expected
	 * platform (the one which the first device belongs to). */
	platf_ref = ccl_device_get_info_scalar(d, CL_DEVICE_PLATFORM,
		cl_platform_id, &err);
	g_assert_no_error(err);

	/* Get number of devices. */
	num_devices = ccl_context_get_num_devices(ctx, &err);
	g_assert_no_error(err);

	/* Check that all devices belong to the same platform. */
	for (guint i = 1; i < num_devices; i++) {

		d = ccl_context_get_device(ctx, i, &err);
		g_assert_no_error(err);

		platform = ccl_device_get_info_scalar(d, CL_DEVICE_PLATFORM,
			cl_platform_id, &err);
		g_assert_no_error(err);

		g_assert(platf_ref == platform);
	}

	/* Free context and set filters to NULL. */
	ccl_context_destroy(ctx);

	/*
	 * 5. Test context creation by device filtering
	 * (explicit independent and dependent filters).
	 * */

	/* Add pass all independent filter for testing. */
	ccl_devsel_add_indep_filter(
		&filters, ccl_devsel_indep_test_true, NULL);

	/* Add another pass all independent filter by manipulating the
	 * ccl_devsel_indep_type() filter. */
	device_type = CL_DEVICE_TYPE_ALL;
	ccl_devsel_add_indep_filter(
		&filters, ccl_devsel_indep_type, &device_type);

	/* Add same platform dependent filter. */
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Create context wrapper, which must have at least one device. */
	ctx = ccl_context_new_from_filters(&filters, &err);
	g_assert_no_error(err);

	num_devices = ccl_context_get_num_devices(ctx, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(num_devices, >, 0);

	/* Free context and set filters to NULL. */
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Test increasing reference count of objects which compose
 * larger objects, then destroy the larger object and verify that
 * composing object still exists and must be freed by the function
 * which increase its reference count. This function tests the following
 * modules: context, device and platform wrappers.
 * */
static void context_ref_unref_test() {

	CCLContext* ctx = NULL;
	CCLContext* ctx_cmp = NULL;
	GError* err = NULL;
	CCLPlatforms* ps = NULL;
	CCLPlatform* p = NULL;
	CCLPlatform* p_1 = NULL;
	CCLPlatform* p_l = NULL;
	cl_platform_id cl_p_1 = NULL;
	cl_platform_id cl_p_l = NULL;
	CCLDevice* d = NULL;
	CCLDevice* d_1 = NULL;
	CCLDevice* d_l = NULL;
	CCLDevice* const* ds;
	CCLDevSelFilters filters = NULL;
	cl_uint num_devs;
	cl_device_type dev_type;

	/* ********************************************* */
	/* **** Test context creating from_devices. **** */
	/* ********************************************* */

	/* Get all platforms in system. */
	ps = ccl_platforms_new(&err);
	g_assert_no_error(err);

	/* Cycle through platforms available in system. */
	for (cl_uint i = 0; i < ccl_platforms_count(ps); ++i) {

		/* Get current platform. */
		p = ccl_platforms_get(ps, i);
		g_assert(p != NULL);

		/* Cycle through devices available in platform. */
		num_devs = ccl_platform_get_num_devices(p, &err);
		g_assert_no_error(err);
		for (cl_uint j = 0; j < num_devs; ++j) {

			/* Get current device (this increases ref. count of device
			 * in platform wrapper due to lazy initialization). */
			d = ccl_platform_get_device(p, j, &err);
			g_assert_no_error(err);

			/* Create a context using current device (does not increase
			 * ref count of device wrapper because context wrapper
			 * doesn't hold the device wrappers at this stage). */
			ctx = ccl_context_new_from_devices(1, &d, &err);
			g_assert_no_error(err);

			/* Check the reference count of platform, device and context*/
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) p), ==, 1);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 1);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);

			/* Increase ref. count of context, check that ref count is now 2. */
			ccl_context_ref(ctx);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 2);

			/* Decrease ref. count of context, check that ref count is now 1. */
			ccl_context_unref(ctx);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);

			/* If this is the first device in platform, keep a
			 * reference to it. */
			if (j == 0) {
				d_1 = d;
				ccl_device_ref(d_1);
				/* Device ref count should be 1 (1 from platform + 1 from d_1). */
				g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 2);
			}

			/* If this is the last device in platform, keep a
			 * reference to it. */
			if (j == num_devs - 1) {
				d_l = d;
				ccl_device_ref(d_l);
				/* If first and last device are the same... */
				if (num_devs == 1) {
					/* ...then the total ref count of current (last)
					 * device should be 3 (1 from platform + 1 d_1 + 1 d_l). */
					g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 3);
				} else {
					/* Otherwise it should be 2 (1 from platform + 1 from d_l). */
					g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 2);
				}
			}

			/* Destroy current context: does not decrease ref count of
			 * device because of lazy initialization (device was never
			 * requested from context). */
			ccl_context_destroy(ctx);
		}

		/* If first and last device are the same... */
		if (num_devs == 1) {
			/* ...then the ref count of this device should be 3 (from first, last and platform). */
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 3);
			/* ...and d_1 and d_l should be the same. */
			g_assert_cmphex(GPOINTER_TO_UINT(d_1), ==, GPOINTER_TO_UINT(d_l));
		} else {
			/* Otherwise it should be 2 for each device (from itself and from platform). */
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 2);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 2);
			/* ...and d_1 and d_l should be the different. */
			g_assert_cmphex(GPOINTER_TO_UINT(d_1), !=, GPOINTER_TO_UINT(d_l));
		}

		/* Check that the platform wrappers associated with the first
		 * and last devices in platform are the same object and that
		 * its ref. count is 3 (2 from new_wrap + 1 from reference in
		 * platforms set). */
		cl_p_1 = ccl_device_get_info_scalar(
			d_1, CL_DEVICE_PLATFORM, cl_platform_id, &err);
		g_assert_no_error(err);
		p_1 = ccl_platform_new_wrap(cl_p_1);
		cl_p_l = ccl_device_get_info_scalar(
			d_l, CL_DEVICE_PLATFORM, cl_platform_id, &err);
		g_assert_no_error(err);
		p_l = ccl_platform_new_wrap(cl_p_l);
		g_assert_cmphex(GPOINTER_TO_UINT(p_1), ==, GPOINTER_TO_UINT(p_l));
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) p_1), ==, 3);
		ccl_platform_unref(p_1); /* Could use p_l, its the same object. */
		ccl_platform_unref(p_1); /* Could use p_l, its the same object. */

		/* Destroy first and last device in platform (which may be the same). */
		if (d_1 == d_l) {
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 3);
		} else {
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 2);
			g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 2);
		}
		ccl_device_destroy(d_1);
		ccl_device_destroy(d_l);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 1);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 1);

		/* At this time there is still a reference to each device held
		 * in the platform wrapper. */

		/* Create a context with all devices in current platform. The
		 * devices ref. count should be the same because devices were
		 * already requested (thus lazy initialized) from this platform. */
		ds = ccl_platform_get_all_devices(p, &err);
		g_assert_no_error(err);
		ctx = ccl_context_new_from_devices(num_devs, (CCLDevice**) ds, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 1);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 1);

		/* Check that the context ref count is 1. */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);

		/* The next instruction lazy initializes device wrappers array
		 * in context, increasing the device wrappers ref count.*/
		d = ccl_context_get_device(ctx, 0, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 2);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 2);

		/* The first device in context should be the same as the first
		 * device in platform. */
		g_assert_cmphex(GPOINTER_TO_UINT(d), ==, GPOINTER_TO_UINT(d_1));

		/* The next instruction increases the ref count of the platform
		 * wrapper, which should now be 2. */
		p_1 = ccl_platform_new_from_device(d, &err);
		g_assert_no_error(err);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) p), ==, 2);

		/* Check that the platform wrapper of the first device is the same
		 * object as the current platform wrapper. */
		g_assert_cmphex(GPOINTER_TO_UINT(p_1), ==, GPOINTER_TO_UINT(p));

		/* Destroy the context. This will decrease the ref count of the
		 * associated device wrappers. */
		ccl_context_destroy(ctx);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_1), ==, 1);
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d_l), ==, 1);

		/* Destroy the platform wrapper */
		ccl_platform_destroy(p_1);

		/* There should be a reference to the platform object, due to it
		 * still being reference in the first device. */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) p), ==, 1);

	}

	/* Destroy the platforms object. This should destroy the enclosed
	 * platform objects, which in turn means that the device wrappers
	 * enclosed in these will also be destroyed. */
	ccl_platforms_destroy(ps);

	/* *************************************************** */
	/* **** Test context creating by device filtering. *** */
	/* *************************************************** */

	/* Create a filter to get all GPUs from the same platform. */
	ccl_devsel_add_indep_filter(&filters, ccl_devsel_indep_type_gpu, NULL);
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Create context from filter. */
	ctx = ccl_context_new_from_filters(&filters, &err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_DEVICE_NOT_FOUND));

	/* If an error occurred and where here its because a GPU wasn't
	 * found. */
	if (err != NULL) {
		/* Take note of error and clear it. */
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		/* No error, so check that the context ref. count is 1. */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);
		/* Get first device. */
		d = ccl_context_get_device(ctx, 0, &err);
		g_assert_no_error(err);
		/* Check that its a GPU and ref it. */
		dev_type = ccl_device_get_info_scalar(d, CL_DEVICE_TYPE, cl_device_type, &err);
		g_assert_no_error(err);
		g_assert_cmphex(dev_type & CL_DEVICE_TYPE_GPU, ==, CL_DEVICE_TYPE_GPU);
		ccl_device_ref(d);
		/* Destroy context. */
		ccl_context_destroy(ctx);
		/* Check that device ref count is 1 (because we ref'ed it before
		 * context destruction). */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 1);
		/* Destroy device. */
		ccl_device_destroy(d);
	}

	/* Create a filter to get all CPUs from the same platform. */
	ccl_devsel_add_indep_filter(&filters, ccl_devsel_indep_type_cpu, NULL);
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Create context from filter. */
	ctx = ccl_context_new_from_filters(&filters, &err);
	g_assert((err == NULL) || (err->code == CCL_ERROR_DEVICE_NOT_FOUND));

	/* If an error occurred and where here its because a CPU wasn't
	 * found. */
	if (err != NULL) {
		/* Take note of error and clear it. */
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		/* No error, so check that the context ref. count is 1. */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);
		/* Get first device. */
		d = ccl_context_get_device(ctx, 0, &err);
		g_assert_no_error(err);
		/* Check that its a CPU and ref it. */
		dev_type = ccl_device_get_info_scalar(d, CL_DEVICE_TYPE, cl_device_type, &err);
		g_assert_no_error(err);
		g_assert_cmphex(dev_type & CL_DEVICE_TYPE_CPU, ==, CL_DEVICE_TYPE_CPU);
		ccl_device_ref(d);
		/* Destroy context. */
		ccl_context_destroy(ctx);
		/* Check that device ref count is 1 (because we ref'ed it before
		 * context destruction). */
		g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 1);
		/* Destroy device. */
		ccl_device_destroy(d);
	}

	/* **************************************** */
	/* **** Test context creation by menu. **** */
	/* **************************************** */

	int data = 0; /* Select device with index 0 in menu. */
	ctx = ccl_context_new_from_menu_full(&data, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 1);

	d = ccl_context_get_device(ctx, 0, &err);
	ccl_device_ref(d);

	/* Create a new context wrapper from the wrapped OpenCL context
	 * object, and check that the wrapper is also the same, but that its
	 * reference count is now 2. */
	ctx_cmp = ccl_context_new_wrap(ccl_context_unwrap(ctx));

	g_assert_cmphex(GPOINTER_TO_UINT(ctx_cmp), ==, GPOINTER_TO_UINT(ctx));
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) ctx), ==, 2);

	/* Unref context. We must do it twice, so as to maintain the logic
	 * that for each _new function, a _destroy (or _unref) function
	 * must be called. */
	ccl_context_unref(ctx);
	ccl_context_unref(ctx);
	/* We could have done these two unrefs with the ctx_cmp variable,
	 * because it represents the same object. */

	/* We ref'ed device, so it ref count should be 1, though we
	 * destroyed the enclosing context. */
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) d), ==, 1);

	/* Unref device. */
	ccl_device_unref(d);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Tests the ccl_context_get_supported_image_formats() function.
 * */
static void context_get_supported_image_formats() {

	CCLPlatforms* ps;
	CCLPlatform* p;
	CCLContext* c;
	CCLDevice* const* ds;
	cl_uint num_devs;
	const cl_image_format* image_formats;
	cl_uint num_image_formats;
	char* p_name;
	GError* err = NULL;

	/* Get all platforms. */
	ps = ccl_platforms_new(&err);
	g_assert_no_error(err);

	/* Cycle through platforms. */
	for (guint i = 0; i < ccl_platforms_count(ps); ++i) {

		/* Get current platform. */
		p = ccl_platforms_get(ps, i);

		/* Get number of devices in platform. */
		num_devs = ccl_platform_get_num_devices(p, &err);
		g_assert_no_error(err);

		/* Get all devices in platform. */
		ds = ccl_platform_get_all_devices(p, &err);
		g_assert_no_error(err);

		/* Create a context with all devices in current platform. */
		c = ccl_context_new_from_devices(num_devs, ds, &err);
		g_assert_no_error(err);

		/* Test the ccl_context_get_supported_image_formats() function. */
		image_formats = ccl_context_get_supported_image_formats(c,
			CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D,
			&num_image_formats, &err);
		g_assert_no_error(err);

		/* Get platform name and print it to debug output. */
		p_name = ccl_platform_get_info_string(p, CL_PLATFORM_NAME, &err);
		g_assert_no_error(err);
		g_debug("Image formats for platform '%s':", p_name);

		/* Cycle through image formats and print them to debug output. */
		for (guint j = 0; j < num_image_formats; ++j) {

			g_debug("\t(chan_order, chan_type) = (%s, %s)",
				ccl_test_channel_order_string(
					image_formats->image_channel_order),
				ccl_test_channel_data_type_string(
					image_formats->image_channel_data_type));
			image_formats++;
		}

		/* Destroy context. */
		ccl_context_destroy(c);
	}

	/* Destroy platforms. */
	ccl_platforms_destroy(ps);

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
		"/wrappers/context/create-info-destroy",
		context_create_info_destroy_test);

	g_test_add_func(
		"/wrappers/context/ref-unref",
		context_ref_unref_test);

	g_test_add_func(
		"/wrappers/context/get-supported-image-formats",
		context_get_supported_image_formats);

	return g_test_run();
}
