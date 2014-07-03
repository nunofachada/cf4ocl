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
 * @brief Tests for context, platforms and devices wrappers library.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "platforms.h"
#include "platform.h"
#include "device.h"
#include "devquery.h"
#include "context.h"
#include "common.h"
#include "program.h"
#include <glib/gstdio.h>

/* Max. length of information string. */
#define CL4_TEST_WRAPPERS_MAXINFOSTR 200

/* Test utility macro. Presents either the required information, or 
 * the error message, if it occurred. Also frees the error object if 
 * an error occurred. */
#define cl4_test_wrappers_msg(base_msg, format, ...) \
	if (err == NULL) { \
		g_snprintf(info_str, CL4_TEST_WRAPPERS_MAXINFOSTR, \
			format, ##__VA_ARGS__); \
	} else { \
		g_snprintf(info_str, CL4_TEST_WRAPPERS_MAXINFOSTR, \
			"%s", err->message); \
		g_clear_error(&err); \
	} \
	g_debug("%s %s", base_msg, info_str);
		
/**
 * @brief Tests creation, getting info from and destruction of 
 * platforms, platform and device wrapper objects.
 * */
static void platforms_create_info_destroy_test() {
	
	CL4Platforms* platfs = NULL;
	CL4Platform* p = NULL;
	CL4Device* d = NULL;
	CL4Device** ds = NULL;
	GError* err = NULL;
	CL4WrapperInfo* info;
	gchar* platf_info;
	guint num_devs;
	guint num_platfs;
	gchar info_str[CL4_TEST_WRAPPERS_MAXINFOSTR];
	
	char* info_check_array;
	cl_uint info_check_scalar;

	/* Get platforms. */
	platfs = cl4_platforms_new(&err);
	g_assert_no_error(err);
	
	/* Number of platforms. */
	num_platfs = cl4_platforms_count(platfs);
	g_debug("* Found %d OpenCL platforms", num_platfs);
		
	/* Cycle through platforms. */
	for (guint i = 0; i < num_platfs; i++) {
	
		/* Get current platform. */
		p = cl4_platforms_get_platform(platfs, i);
		g_debug(">> Platform %d:", i);

		/* Get platform profile. */
		platf_info = cl4_platform_info(p, CL_PLATFORM_PROFILE, &err);
		cl4_test_wrappers_msg("==== Profile :", "%s", platf_info);

		/* Get platform version. */
		platf_info = cl4_platform_info(p, CL_PLATFORM_VERSION, &err);
		cl4_test_wrappers_msg("==== Version :", "%s", platf_info);
		
		/* Get platform name. */
		platf_info = cl4_platform_info(p, CL_PLATFORM_NAME, &err);
		cl4_test_wrappers_msg("==== Name    :", "%s", platf_info);

		/* Get platform vendor. */
		platf_info = cl4_platform_info(p, CL_PLATFORM_VENDOR, &err);
		cl4_test_wrappers_msg("==== Vendor  :", "%s", platf_info);

		/* Get platform extensions. */
		platf_info = cl4_platform_info(p, CL_PLATFORM_EXTENSIONS, &err);
		cl4_test_wrappers_msg("==== Extens. :", "%s", platf_info);
		
		/* Get number of devices. */
		num_devs = cl4_platform_num_devices(p, &err);
		
		/* Only test for device information if device count was 
		 * successfully obtained. */
		if (err != NULL) {
			g_test_message("Error obtaining number of devices for platform %d (%s).",
				i, err->message);
			g_clear_error(&err);
		} else {
			
			g_debug("==== # Devs  : %d", num_devs);

			/* Cycle through devices in platform. */
			for (guint j = 0; j < num_devs; j++) {
				
				/* Get current device. */
				d = cl4_platform_get_device(p, j, &err);
				g_assert_no_error(err);
				g_debug("====== Device #%d", j);

				info = cl4_device_info(d, CL_DEVICE_NAME, &err);
				cl4_test_wrappers_msg("...... Name :", "%s", (gchar*) info->value);
				
				g_debug("...... Memory location : %p", *((void**) cl4_device_unwrap(d)));
				
				info = cl4_device_info(d, CL_DEVICE_ADDRESS_BITS, &err);
				cl4_test_wrappers_msg("...... Address bits :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_AVAILABLE, &err);
				cl4_test_wrappers_msg("...... Device available :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_BUILT_IN_KERNELS, &err);
				cl4_test_wrappers_msg("...... Built-in kernels :", "%s", (gchar*) info->value);

				info = cl4_device_info(d, CL_DEVICE_COMPILER_AVAILABLE, &err);
				cl4_test_wrappers_msg("...... Compiler available :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_DOUBLE_FP_CONFIG, &err);
				cl4_test_wrappers_msg("...... FP config (Double) :", "%lx", (cl_device_fp_config) *((cl_device_fp_config*) info->value));

				info = cl4_device_info(d, CL_DEVICE_SINGLE_FP_CONFIG, &err);
				cl4_test_wrappers_msg("...... FP config (Single) :", "%lx", (cl_device_fp_config) *((cl_device_fp_config*) info->value));

				info = cl4_device_info(d, CL_DEVICE_HALF_FP_CONFIG, &err);
				cl4_test_wrappers_msg("...... FP config (Half) :", "%lx", (cl_device_fp_config) *((cl_device_fp_config*) info->value));

				info = cl4_device_info(d, CL_DEVICE_ENDIAN_LITTLE, &err);
				cl4_test_wrappers_msg("...... Endian little :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_ERROR_CORRECTION_SUPPORT, &err);
				cl4_test_wrappers_msg("...... Error correction support :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_EXECUTION_CAPABILITIES, &err);
				cl4_test_wrappers_msg("...... FP config (Single) :", "%lx", (cl_device_exec_capabilities) *((cl_device_exec_capabilities*) info->value));

				info = cl4_device_info(d, CL_DEVICE_EXTENSIONS, &err);
				cl4_test_wrappers_msg("...... Extensions :", "%s", (gchar*) info->value);

				info = cl4_device_info(d, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &err);
				cl4_test_wrappers_msg("...... Global mem. cache size :", "%ld", *((cl_ulong*) info->value));

				info = cl4_device_info(d, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, &err);
				cl4_test_wrappers_msg("...... Global mem. cache type :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &err);
				cl4_test_wrappers_msg("...... Global mem. cacheline size :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_GLOBAL_MEM_SIZE, &err);
				cl4_test_wrappers_msg("...... Global mem. size :", "%ld", *((cl_ulong*) info->value));

				info = cl4_device_info(d, CL_DEVICE_HOST_UNIFIED_MEMORY, &err);
				cl4_test_wrappers_msg("...... Host unified memory :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_IMAGE_SUPPORT, &err);
				cl4_test_wrappers_msg("...... Image support :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_IMAGE2D_MAX_HEIGHT, &err);
				cl4_test_wrappers_msg("...... Image2d max. height :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE2D_MAX_WIDTH, &err);
				cl4_test_wrappers_msg("...... Image2d max. width :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE3D_MAX_DEPTH, &err);
				cl4_test_wrappers_msg("...... Image3d max. depth :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE3D_MAX_HEIGHT, &err);
				cl4_test_wrappers_msg("...... Image3d max. height :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE3D_MAX_WIDTH, &err);
				cl4_test_wrappers_msg("...... Image3d max. width :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, &err);
				cl4_test_wrappers_msg("...... Image max. buffer size :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, &err);
				cl4_test_wrappers_msg("...... Image max. array size :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_LINKER_AVAILABLE, &err);
				cl4_test_wrappers_msg("...... Linker available :", "%s", *((cl_bool*) info->value) ? "Yes" : "No");

				info = cl4_device_info(d, CL_DEVICE_LOCAL_MEM_SIZE, &err);
				cl4_test_wrappers_msg("...... Local mem. size :", "%ld", *((cl_ulong*) info->value));

				info = cl4_device_info(d, CL_DEVICE_LOCAL_MEM_TYPE, &err);
				cl4_test_wrappers_msg("...... Local mem. type :", "%d", *((cl_device_local_mem_type*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_CLOCK_FREQUENCY, &err);
				cl4_test_wrappers_msg("...... Max. clock frequency :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_COMPUTE_UNITS, &err);
				cl4_test_wrappers_msg("...... Max. compute units :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_CONSTANT_ARGS, &err);
				cl4_test_wrappers_msg("...... Max. constant args. :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &err);
				cl4_test_wrappers_msg("...... Max. constant buffer size :", "%ld", *((cl_ulong*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_MEM_ALLOC_SIZE, &err);
				cl4_test_wrappers_msg("...... Max. mem. alloc. size :", "%ld", *((cl_ulong*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_PARAMETER_SIZE, &err);
				cl4_test_wrappers_msg("...... Max. parameter size :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_READ_IMAGE_ARGS, &err);
				cl4_test_wrappers_msg("...... Max. read image args. :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_SAMPLERS, &err);
				cl4_test_wrappers_msg("...... Max. samplers. :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_WORK_GROUP_SIZE, &err);
				cl4_test_wrappers_msg("...... Max. work-group size :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &err);
				cl4_test_wrappers_msg("...... Max. work-item dims. :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MAX_WORK_ITEM_SIZES, &err);
				cl4_test_wrappers_msg("...... Max wkitem sizes :", "%d, %d, %d", (int) ((size_t*) info->value)[0], (int) ((size_t*) info->value)[1], (int) ((size_t*) info->value)[2]);

				info = cl4_device_info(d, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, &err);
				cl4_test_wrappers_msg("...... Max. write image args. :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MEM_BASE_ADDR_ALIGN, &err);
				cl4_test_wrappers_msg("...... Mem. base address align :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, &err);
				cl4_test_wrappers_msg("...... Min. data type align size (deprecated in OpenCL 1.2) :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, &err);
				cl4_test_wrappers_msg("...... Native vector width char :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, &err);
				cl4_test_wrappers_msg("...... Native vector width short :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, &err);
				cl4_test_wrappers_msg("...... Native vector width int :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, &err);
				cl4_test_wrappers_msg("...... Native vector width long :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, &err);
				cl4_test_wrappers_msg("...... Native vector width float :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, &err);
				cl4_test_wrappers_msg("...... Native vector width double :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, &err);
				cl4_test_wrappers_msg("...... Native vector width half :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_OPENCL_C_VERSION, &err);
				cl4_test_wrappers_msg("...... OpenCL C version :", "%s", (gchar*) info->value);

				info = cl4_device_info(d, CL_DEVICE_PARENT_DEVICE, &err);
				cl4_test_wrappers_msg("...... Parent device :", "%p", *((void**) info->value));

				info = cl4_device_info(d, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, &err);
				cl4_test_wrappers_msg("...... Partition max. sub-devices :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PARTITION_PROPERTIES, &err);
				cl4_test_wrappers_msg("...... Partition properties (only first) :", "%x", info != NULL ? (int) ((intptr_t*) info->value)[0] : 0);

				info = cl4_device_info(d, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, &err);
				cl4_test_wrappers_msg("...... Partition affinity domain :", "%lx", (cl_device_affinity_domain) *((cl_device_affinity_domain*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PARTITION_TYPE, &err);
				cl4_test_wrappers_msg("...... Partition type (only first):", "%x", info != NULL ? (int) ((intptr_t*) info->value)[0] : 0);

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, &err);
				cl4_test_wrappers_msg("...... Preferred vector width char :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, &err);
				cl4_test_wrappers_msg("...... Preferred vector width short :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, &err);
				cl4_test_wrappers_msg("...... Preferred vector width int :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, &err);
				cl4_test_wrappers_msg("...... Preferred vector width long :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, &err);
				cl4_test_wrappers_msg("...... Preferred vector width float :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, &err);
				cl4_test_wrappers_msg("...... Preferred vector width double :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, &err);
				cl4_test_wrappers_msg("...... Preferred vector width half :", "%d", *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PRINTF_BUFFER_SIZE, &err);
				cl4_test_wrappers_msg("...... Printf buffer size :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, &err);
				cl4_test_wrappers_msg("...... Preferred interop. user sync. :", "%s", *((cl_bool*) info->value) ? "User sync." : "Device sync.");

				info = cl4_device_info(d, CL_DEVICE_PROFILE, &err);
				cl4_test_wrappers_msg("...... Profile :", "%s", (gchar*) info->value);

				info = cl4_device_info(d, CL_DEVICE_PROFILING_TIMER_RESOLUTION, &err);
				cl4_test_wrappers_msg("...... Profiling timer resolution :", "%ld", (cl_ulong) *((size_t*) info->value));

				info = cl4_device_info(d, CL_DEVICE_QUEUE_PROPERTIES, &err);
				cl4_test_wrappers_msg("...... Command queue properties :", "%s %s", ((cl_command_queue_properties) *((cl_command_queue_properties*) info->value) & CL_QUEUE_PROFILING_ENABLE) ? "Profiling" : "", ((cl_command_queue_properties) *((cl_command_queue_properties*) info->value) & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? "Out-Of-Order" : "");

				info = cl4_device_info(d, CL_DEVICE_TYPE, &err);
				cl4_test_wrappers_msg("...... Type :", "%s", cl4_devquery_type2str(*((cl_device_type*) info->value)));

				info = cl4_device_info(d, CL_DEVICE_VENDOR, &err);
				cl4_test_wrappers_msg("...... Vendor :", "%s", (gchar*) info->value);

				/* Special check for info_value_array macro. */
				info_check_array = cl4_device_info_value_array(d, CL_DEVICE_VENDOR, char*, &err);
				g_assert_cmpstr((char*) info->value, ==, info_check_array);

				info = cl4_device_info(d, CL_DEVICE_VENDOR_ID, &err);
				cl4_test_wrappers_msg("...... Vendor ID :", "%x", *((cl_uint*) info->value));

				/* Special check for info_value_scalar macro. */
				info_check_scalar = cl4_device_info_value_scalar(d, CL_DEVICE_VENDOR_ID, cl_uint, &err);
				g_assert_cmphex(info_check_scalar, ==, *((cl_uint*) info->value));

				info = cl4_device_info(d, CL_DEVICE_VERSION, &err);
				cl4_test_wrappers_msg("...... Device version :", "%s", (gchar*) info->value);

				info = cl4_device_info(d, CL_DRIVER_VERSION, &err);
				cl4_test_wrappers_msg("...... Driver version :", "%s", (gchar*) info->value);

			}
		}
	}
	
	/* Test get_all_devices method of platform module. */
	for (guint i = 0; i < cl4_platforms_count(platfs); i++) {
		p = cl4_platforms_get_platform(platfs, i);
		ds = cl4_platform_get_all_devices(p, &err);
		g_assert_no_error(err);
		for (guint j = 0; j < cl4_platform_num_devices(p, &err); j++) {
			g_assert_no_error(err);
			d = cl4_platform_get_device(p, j, &err);
			g_assert_no_error(err);
			g_assert(d == ds[j]);
		}
		g_assert_no_error(err);
	}

	/* Destroy list of platforms. */
	cl4_platforms_destroy(platfs);
		
}

/** 
 * @brief Test increasing reference count of objects which compose 
 * larger objects, then destroy the larger object and verify that 
 * composing object still exists and must be freed by the function
 * which increase its reference count.
 * 
 * This function tests the following modules: platforms, platform and
 * device.
 * */
static void platforms_ref_unref_test() {

	CL4Platforms* platfs = NULL;
	CL4Platform* p = NULL;
	CL4Device* d = NULL;
	GError* err = NULL;

	/* Get platforms. */
	platfs = cl4_platforms_new(&err);
	g_assert_no_error(err);		
	
	/* Use first device of first platform. */
	p = cl4_platforms_get_platform(platfs, 0);
	
	d = cl4_platform_get_device(p, 0, &err);
	g_assert_no_error(err);
	
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) p), ==, 1);
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) d), ==, 1);

	cl4_platform_ref(p);
	cl4_device_ref(d);

	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) p), ==, 2);
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) d), ==, 2);
		
	cl4_platforms_destroy(platfs);
		
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) p), ==, 1);
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) d), ==, 2);
		
	cl4_platform_destroy(p);
	
	g_assert_cmpint(cl4_wrapper_ref_count((CL4Wrapper*) d), ==, 1);
		
	cl4_device_destroy(d);

}

/*
 * Independent pass-all filter for testing.
 * */
static gboolean cl4_devsel_indep_test_true(
	CL4Device* device, void *data, GError **err) {

	device = device;
	data = data;
	err = err;
	return TRUE;

}

/**
 * @brief Tests creation, getting info from and destruction of 
 * context wrapper objects.
 * */
static void context_create_info_destroy_test() {
	
	CL4Context* ctx = NULL;
	GError* err = NULL;
	CL4Platforms* ps = NULL;
	CL4Platform* p = NULL;
	CL4Device* d = NULL;
	cl_device_id d_id = NULL;
	CL4DevSelFilters filters = NULL;
	CL4WrapperInfo* info = NULL;
	cl_context_properties* ctx_props = NULL;
	cl_platform_id platform, platf_ref;
	cl_context context;
	cl_device_type device_type;
	guint num_devices;
	cl_int ocl_status;
	gboolean any_device;
	
	/* 
	 * 1. Test context creation from cl_devices. 
	 * */
	 
	/* Get platforms object. */
	ps = cl4_platforms_new(&err);
	g_assert_no_error(err);

	/* Get first platform wrapper from platforms object. */
	p = cl4_platforms_get_platform(ps, 0);
	g_assert(p != NULL);
	
	/* Get first device wrapper from platform wrapper. */
	d = cl4_platform_get_device(p, 0, &err);
	g_assert_no_error(err);
	
	/* Unwrap cl_device_id from device wrapper object. */
	d_id = cl4_device_unwrap(d);
	
	/* Create a context from this cl_device_id. */
	ctx = cl4_context_new_from_cldevices(1, &d_id, &err);
	g_assert_no_error(err);
	
	/* Get number of devices from context wrapper, check that this
	 * number is 1. */
#ifdef CL_VERSION_1_1
	info = cl4_context_info(ctx, CL_CONTEXT_NUM_DEVICES, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(*((cl_uint*) info->value), ==, 1);
#endif
	
	/* Get the cl_device_id from context via context info and check
	 * that it corresponds to the cl_device_id with which the context
	 * was created. */
	info = cl4_context_info(ctx, CL_CONTEXT_DEVICES, &err);
	g_assert_no_error(err);
	g_assert(((cl_device_id*) info->value)[0] == d_id);

	/* Check again that the number of devices is 1, this time not using
	 * CL_CONTEXT_NUM_DEVICES, which is not available in OpenCL 1.0. */
	g_assert_cmpuint(info->size / sizeof(cl_device_id), ==, 1);

	/* Free context. */
	cl4_context_destroy(ctx);

	/* 
	 * 2. Test context creation by cl_context. 
	 * */
	
	/* Create some context properties. */
	ctx_props = g_new0(cl_context_properties, 3);
	platform = (cl_platform_id) cl4_wrapper_unwrap((CL4Wrapper*) p);
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
	ctx = cl4_context_new_from_clcontext(context, &err);
	g_assert_no_error(err);
	g_assert(cl4_context_unwrap(ctx) == context);
	
	/* Get the first device wrapper from the context wrapper, check that 
	 * the unwrapped cl_device_id corresponds to the cl_device_id with
	 * which the cl_context was created. */
	d = cl4_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);
	g_assert(cl4_device_unwrap(d) == d_id);

	/* Check that the context number of devices taken using context
	 * info is 1. */
#ifdef CL_VERSION_1_1
	info = cl4_context_info(ctx, CL_CONTEXT_NUM_DEVICES, &err);
	g_assert_cmpuint(*((cl_uint*) info->value), ==, 1);
#else
	info = cl4_context_info(ctx, CL_CONTEXT_DEVICES, &err);
	g_assert_cmpuint(info->size / sizeof(cl_device_id), ==, 1);
#endif

	/* Free context, platforms and context properties. */
	cl4_context_destroy(ctx);
	cl4_platforms_destroy(ps);
	g_free(ctx_props);

	/* Explicitly destroy cl_context because it was created outside
	 * the context wrapper. */
	ocl_status = clReleaseContext(context);
	g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
	
	/* 
	 * 3. Test context creation by device filtering 
	 * (using shortcut macros). 
	 * */
	 
	/* For the next device type filters, at least one device must be
	 * found in order the test to pass. */
	any_device = FALSE;
	 
	/* 3.1. GPU device type filter. */
		
	/* Check that either there was no error or that no GPU was found. */
	ctx = cl4_context_new_gpu(&err);
	g_assert((err == NULL) || (err->code == CL4_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else { 
		cl4_context_destroy(ctx);
	}
	filters = NULL;

	/* 3.2. CPU device type filter. */

	/* Check that either there was no error or that no CPU was found. */
	ctx = cl4_context_new_cpu(&err);
	g_assert((err == NULL) || (err->code == CL4_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else { 
		cl4_context_destroy(ctx);
	}
	filters = NULL;

	/* 3.3. Accel. device type filter. */
	
	/* Check that either there was no error or that no accelerator was 
	 * found. */
	ctx = cl4_context_new_accel(&err);
	g_assert((err == NULL) || (err->code == CL4_ERROR_DEVICE_NOT_FOUND));
	any_device |= (ctx != NULL);

	/* Free context if no error and set filters to NULL. */
	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else { 
		cl4_context_destroy(ctx);
	}
	filters = NULL;
	
	/* Check that at least one device type context was created. */
	g_assert(any_device);
	
	/* 3.4. Specific platform filter. */

	/* Check that a context wrapper was created. */
	ctx = cl4_context_new_from_indep_filter(
		cl4_devsel_indep_platform, (void*) platform, &err);
	g_assert_no_error(err);
	
	/* Check that context wrapper contains a device. */
	d = cl4_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);
	
	/* Check that the device platform corresponds to the expected 
	 * platform (the one used in the filter). */
	platf_ref = cl4_device_info_value_scalar(d, CL_DEVICE_PLATFORM,
		cl_platform_id, &err);
	g_assert_no_error(err);
	g_assert(platf_ref == platform);
	
	/* Free context and set filters to NULL. */
	cl4_context_destroy(ctx);
	filters = NULL;
	
	/* 
	 * 4. Test context creation by device filtering 
	 * (explicit dependent filters). 
	 * */
	 
	/* Same platform filter. */
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_platform, NULL);

	/* Check that a context wrapper was created. */
	ctx = cl4_context_new_from_filters(&filters, &err);
	g_assert_no_error(err);
	
	/* Check that context wrapper contains a device. */
	d = cl4_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);
	
	/* Check that the device platform corresponds to the expected 
	 * platform (the one which the first device belongs to). */
	platf_ref = cl4_device_info_value_scalar(d, CL_DEVICE_PLATFORM,
		cl_platform_id, &err);
	g_assert_no_error(err);
	
	/* Get number of devices. */
	num_devices = cl4_context_num_devices(ctx, &err);
	g_assert_no_error(err);
	
	/* Check that all devices belong to the same platform. */
	for (guint i = 1; i < num_devices; i++) {
		
		d = cl4_context_get_device(ctx, i, &err);
		g_assert_no_error(err);
		
		platform = cl4_device_info_value_scalar(d, CL_DEVICE_PLATFORM,
			cl_platform_id, &err);
		g_assert_no_error(err);
			
		g_assert(platf_ref == platform);
	}
	
	/* Free context and set filters to NULL. */
	cl4_context_destroy(ctx);
	filters = NULL;

	/* 
	 * 5. Test context creation by device filtering 
	 * (explicit independent and dependent filters). 
	 * */
	
	/* Add pass all independent filter for testing. */
	cl4_devsel_add_indep_filter(
		&filters, cl4_devsel_indep_test_true, NULL);
		
	/* Add another pass all independent filter by manipulating the
	 * cl4_devsel_indep_type() filter. */
	device_type = CL_DEVICE_TYPE_ALL;
	cl4_devsel_add_indep_filter(
		&filters, cl4_devsel_indep_type, &device_type);
	
	/* Add same platform dependent filter. */
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_platform, NULL);

	/* Create context wrapper, which must have at least one device. */
	ctx = cl4_context_new_from_filters(&filters, &err);
	g_assert_no_error(err);
	
	num_devices = cl4_context_num_devices(ctx, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(num_devices, >, 0);

	/* Free context and set filters to NULL. */
	cl4_context_destroy(ctx);
	filters = NULL;

}

/** 
 * @brief Test increasing reference count of objects which compose 
 * larger objects, then destroy the larger object and verify that 
 * composing object still exists and must be freed by the function
 * which increase its reference count.
 * 
 * This function tests the following modules: context, device and 
 * platform.
 * */
static void context_ref_unref_test() {

	CL4Context* ctx = NULL;
	GError* err = NULL;
	CL4Platforms* ps = NULL;
	CL4Platform* p = NULL;
	CL4Device* d = NULL;
	cl_device_id d_id = NULL;
	CL4DevSelFilters filters = NULL;
	
	/* Test context creating from cl_devices. */
	ps = cl4_platforms_new(&err);
	g_assert_no_error(err);

	p = cl4_platforms_get_platform(ps, 0);
	g_assert(p != NULL);
	
	d = cl4_platform_get_device(p, 0, &err);
	g_assert_no_error(err);
	
	d_id = cl4_device_unwrap(d);
		
	ctx = cl4_context_new_from_cldevices(1, &d_id, &err);
	g_assert_no_error(err);
	
	g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) d), ==, 1);
	g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) ctx), ==, 1);
	
	cl4_context_ref(ctx);
	g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) ctx), ==, 2);
	cl4_context_unref(ctx);
	g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) ctx), ==, 1);
	
	cl4_platforms_destroy(ps);
	cl4_context_destroy(ctx);

	/* Test context creating by device filtering. */
	cl4_devsel_add_indep_filter(&filters, cl4_devsel_indep_type_gpu, NULL);
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_platform, NULL);	
	
	ctx = cl4_context_new_from_filters(&filters, &err);
	g_assert((err == NULL) || (err->code == CL4_ERROR_DEVICE_NOT_FOUND));

	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) ctx), ==, 1);
		cl4_context_destroy(ctx);
	}
	filters = NULL;

	cl4_devsel_add_indep_filter(&filters, cl4_devsel_indep_type_cpu, NULL);
	cl4_devsel_add_dep_filter(&filters, cl4_devsel_dep_platform, NULL);
	
	ctx = cl4_context_new_from_filters(&filters, &err);
	g_assert((err == NULL) || (err->code == CL4_ERROR_DEVICE_NOT_FOUND));

	if (err != NULL) {
		g_test_message("%s", err->message);
		g_clear_error(&err);
	} else {
		g_assert_cmpuint(cl4_wrapper_ref_count((CL4Wrapper*) ctx), ==, 1);
		cl4_context_destroy(ctx);
	}
	filters = NULL;

	/// @todo Test context from CL4Platform, check that devices have ref=2 (kept by CL4Platform and CL4Context)
	/// @todo Test ref/unref program
	/// @todo Test ref/unref queue
	/// @todo Test ref/unref kernels

}

#define CL4_TEST_WRAPPERS_PROGRAM_SUM "sum"

#define CL4_TEST_WRAPPERS_PROGRAM_SUM_NAME CL4_TEST_WRAPPERS_PROGRAM_SUM ".cl"

#define CL4_TEST_WRAPPERS_PROGRAM_SUM_CONTENT \
	"__kernel void " CL4_TEST_WRAPPERS_PROGRAM_SUM "(" \
	"		__global const float *a," \
	"		__global const float *b," \
	"		__global float *c)" \
	"{" \
	"	int gid = get_global_id(0);" \
	"	c[gid] = a[gid] + b[gid];" \
	"}"

/**
 * @brief Tests creation, getting info from and destruction of 
 * program wrapper objects.
 * */
static void program_create_info_destroy_test() {

	CL4Context* ctx = NULL;
	CL4Program* prg = NULL;
	CL4Kernel* krnl = NULL;
	CL4WrapperInfo* info = NULL;
	GError* err = NULL;
	
	g_file_set_contents(CL4_TEST_WRAPPERS_PROGRAM_SUM_NAME, 
		CL4_TEST_WRAPPERS_PROGRAM_SUM_CONTENT, -1, &err);
	g_assert_no_error(err);

	ctx = cl4_context_new_any(&err);
	g_assert_no_error(err);

	prg = cl4_program_new(ctx, CL4_TEST_WRAPPERS_PROGRAM_SUM_NAME, &err);
	g_assert_no_error(err);
	
	info = cl4_program_info(prg, CL_PROGRAM_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == cl4_context_unwrap(ctx));

	info = cl4_program_info(prg, CL_PROGRAM_NUM_DEVICES, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(*((cl_uint*) info->value), 
		==, cl4_context_num_devices(ctx, &err));
	g_assert_no_error(err);

	info = cl4_program_info(prg, CL_PROGRAM_SOURCE, &err);
	g_assert_no_error(err);
	g_assert_cmpstr((char*) info->value, 
		==, CL4_TEST_WRAPPERS_PROGRAM_SUM_CONTENT);
	
	cl4_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	if (g_unlink(CL4_TEST_WRAPPERS_PROGRAM_SUM_NAME) < 0)
		g_message("Unable to delete temporary file '"
			CL4_TEST_WRAPPERS_PROGRAM_SUM_NAME "'");
			
	krnl = cl4_program_get_kernel(
		prg, CL4_TEST_WRAPPERS_PROGRAM_SUM, &err);
	g_assert_no_error(err);

	info = cl4_kernel_info(krnl, CL_KERNEL_FUNCTION_NAME, &err);
	g_assert_no_error(err);
	g_assert_cmpstr(
		(gchar*) info->value, ==, CL4_TEST_WRAPPERS_PROGRAM_SUM);

	info = cl4_kernel_info(krnl, CL_KERNEL_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == cl4_context_unwrap(ctx));

	info = cl4_kernel_info(krnl, CL_KERNEL_PROGRAM, &err);
	g_assert_no_error(err);
	g_assert(*((cl_program*) info->value) == cl4_program_unwrap(prg));

	cl4_program_destroy(prg);
	cl4_context_destroy(ctx);

}

/**
 * @brief Main function.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add_func(
		"/wrappers/platforms-create-info-destroy", 
		platforms_create_info_destroy_test);

	g_test_add_func(
		"/wrappers/platforms-ref-unref", 
		platforms_ref_unref_test);

	g_test_add_func(
		"/wrappers/context-create-info-destroy", 
		context_create_info_destroy_test);

	g_test_add_func(
		"/wrappers/context-ref-unref", 
		context_ref_unref_test);
		
	g_test_add_func(
		"/wrappers/program-create-info-destroy", 
		program_create_info_destroy_test);		

	return g_test_run();
}

