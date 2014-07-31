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
 * @brief Create, add, destroy tests for the program wrapper. Also 
 * tests the kernel and queue wrapper, kernel execution, and so on.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "platforms.h"
#include "platform_wrapper.h"
#include "device_wrapper.h"
#include "device_query.h"
#include "context_wrapper.h"
#include "common.h"
#include "program_wrapper.h"
#include "memobj_wrapper.h"
#include "buffer_wrapper.h"
#include <glib/gstdio.h>

#define CCL_TEST_PROGRAM_SUM "sum"

#define CCL_TEST_PROGRAM_SUM_FILENAME CCL_TEST_PROGRAM_SUM ".cl"

#define CCL_TEST_PROGRAM_SUM_CONTENT \
	"__kernel void " CCL_TEST_PROGRAM_SUM "(" \
	"		__global const uint *a," \
	"		__global const uint *b," \
	"		__global uint *c, uint d)" \
	"{" \
	"	int gid = get_global_id(0);" \
	"	c[gid] = a[gid] + b[gid] + d;" \
	"}"

#define CCL_TEST_PROGRAM_BUF_SIZE 16
#define CCL_TEST_PROGRAM_LWS 8 /* Must be a divisor of CCL_TEST_PROGRAM_BUF_SIZE */
#define CCL_TEST_PROGRAM_CONST 4
G_STATIC_ASSERT(CCL_TEST_PROGRAM_BUF_SIZE % CCL_TEST_PROGRAM_LWS == 0);

/**
 * @brief Tests creation, getting info from and destruction of 
 * program wrapper objects.
 * */
static void program_create_info_destroy_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLProgram* prg = NULL;
	CCLKernel* krnl = NULL;
	CCLWrapperInfo* info = NULL;
	CCLDevice* d = NULL;
	CCLQueue* cq = NULL;
	size_t gws;
	size_t lws;
	cl_uint a_h[CCL_TEST_PROGRAM_BUF_SIZE];
	cl_uint b_h[CCL_TEST_PROGRAM_BUF_SIZE];
	cl_uint c_h[CCL_TEST_PROGRAM_BUF_SIZE];
	cl_uint d_h ;
	CCLMemObj* a_w;
	CCLMemObj* b_w;
	CCLMemObj* c_w;
	CCLEvent* evt_w1;
	CCLEvent* evt_w2;
	CCLEvent* evt_kr;
	CCLEvent* evt_r1;
	CCLEventWaitList ewl;
	GError* err = NULL;
	gchar* tmp_dir_name;
	gchar* tmp_file_prefix;
	
	/* Get a temp. dir. */
	tmp_dir_name = g_dir_make_tmp("test_cad_program_XXXXXX", &err);
	g_assert_no_error(err);
	
	/* Get a temp file prefix. */
	tmp_file_prefix = g_strdup_printf("%s%c%s", 
		tmp_dir_name, G_DIR_SEPARATOR, CCL_TEST_PROGRAM_SUM_FILENAME);
	
	/* Create a temporary kernel file. */
	g_file_set_contents(
		tmp_file_prefix, CCL_TEST_PROGRAM_SUM_CONTENT, -1, &err);
	g_assert_no_error(err);

	/* Create a context with first available device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);
	
	/* Get the device being used. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a new program from kernel file. */
	prg = ccl_program_new_from_source_file(
		ctx, tmp_file_prefix, &err);
	g_assert_no_error(err);
	
	g_free(tmp_file_prefix);

	/* Get some program info, compare it with expected info. */
	info = ccl_program_get_info(prg, CL_PROGRAM_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == ccl_context_unwrap(ctx));

	/* Get number of devices from program info, check that this is the
	 * same value as the number of devices in context. */
	info = ccl_program_get_info(prg, CL_PROGRAM_NUM_DEVICES, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(*((cl_uint*) info->value), 
		==, ccl_context_get_num_devices(ctx, &err));
	g_assert_no_error(err);

	/* Get program source from program info, check that it is the 
	 * same as the passed source. */
	info = ccl_program_get_info(prg, CL_PROGRAM_SOURCE, &err);
	g_assert_no_error(err);
	g_assert_cmpstr((char*) info->value, 
		==, CCL_TEST_PROGRAM_SUM_CONTENT);
	
	/* Check that no build was performed yet. */
	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_STATUS, &err);
	g_assert_no_error(err);
	g_assert_cmpint(*((cl_build_status*) info->value), ==, CL_BUILD_NONE);
	
	/* **** BUILD PROGRAM **** */
	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);
	
	/* Get some program build info, compare it with expected values. */
	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_STATUS, &err);
	g_assert_no_error(err);
	g_assert((*((cl_build_status*) info->value) == CL_BUILD_SUCCESS) 
		|| (*((cl_build_status*) info->value) == CL_BUILD_IN_PROGRESS));

	/* Get the build log, check that no error occurs. */
	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_LOG, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper object. */
	krnl = ccl_program_get_kernel(
		prg, CCL_TEST_PROGRAM_SUM, &err);
	g_assert_no_error(err);

	/* Get some kernel info, compare it with expected info. */
	
	/* Get kernel function name from kernel info, compare it with the
	 * expected value. */
	info = ccl_kernel_get_info(krnl, CL_KERNEL_FUNCTION_NAME, &err);
	g_assert_no_error(err);
	g_assert_cmpstr(
		(gchar*) info->value, ==, CCL_TEST_PROGRAM_SUM);

	/* Check if the kernel context is the same as the initial context 
	 * and the program context. */
	info = ccl_kernel_get_info(krnl, CL_KERNEL_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == ccl_context_unwrap(ctx));

	info = ccl_kernel_get_info(krnl, CL_KERNEL_PROGRAM, &err);
	g_assert_no_error(err);
	g_assert(*((cl_program*) info->value) == ccl_program_unwrap(prg));
			
	/* Save binaries for all available devices. */
	tmp_file_prefix = g_strdup_printf(
		"%s%ctest_", tmp_dir_name, G_DIR_SEPARATOR);
	
	ccl_program_save_all_binaries(prg, tmp_file_prefix, ".bin", &err);
	g_assert_no_error(err);
	
	g_free(tmp_file_prefix);
	
	/* Save binary for a specific device (which we will load into a new
	 * program later). */
	tmp_file_prefix = g_strdup_printf(
		"%s%ctest_prg.bin", tmp_dir_name, G_DIR_SEPARATOR);

	ccl_program_save_binary(prg, d, tmp_file_prefix, &err);
	g_assert_no_error(err);
	
	/* Destroy program. */
	ccl_program_destroy(prg);

	/* Create a new program using the saved binary. */
	prg = ccl_program_new_from_binary_file(
		ctx, d, tmp_file_prefix, NULL, &err);
	g_assert_no_error(err);
	
	g_free(tmp_file_prefix);

	/* **** BUILD PROGRAM **** */
	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);
	
	/* Get some program build info, compare it with expected values. */
	info = ccl_program_get_build_info(
		prg, d, CL_PROGRAM_BUILD_STATUS, &err);
	g_assert_no_error(err);
	g_assert((*((cl_build_status*) info->value) == CL_BUILD_SUCCESS) 
		|| (*((cl_build_status*) info->value) == CL_BUILD_IN_PROGRESS));

	info = ccl_program_get_build_info(
		prg, d, CL_PROGRAM_BUILD_LOG, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper object. */
	krnl = ccl_program_get_kernel(
		prg, CCL_TEST_PROGRAM_SUM, &err);
	g_assert_no_error(err);

	/* Get some kernel info, compare it with expected info. */
	info = ccl_kernel_get_info(krnl, CL_KERNEL_FUNCTION_NAME, &err);
	g_assert_no_error(err);
	g_assert_cmpstr(
		(gchar*) info->value, ==, CCL_TEST_PROGRAM_SUM);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	
	/* Set kernel enqueue properties and initialize host data. */
	gws = CCL_TEST_PROGRAM_BUF_SIZE;
	lws = CCL_TEST_PROGRAM_LWS;
	
	for (cl_uint i = 0; i < CCL_TEST_PROGRAM_BUF_SIZE; ++i) {
		a_h[i] = i + 1;
		b_h[i] = i + 1;
	}
	d_h = CCL_TEST_PROGRAM_CONST;
	
	/* Create host buffers. */
	a_w = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	b_w = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	c_w = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	
	/* Copy host data to device buffers without waiting for transfer
	 * to terminate before continuing host program. */
	evt_w1 = ccl_buffer_write(cq, a_w, CL_FALSE, 0, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), a_h, NULL, &err);
	g_assert_no_error(err);
	evt_w2 = ccl_buffer_write(cq, b_w, CL_FALSE, 0, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), b_h, NULL, &err);
	g_assert_no_error(err);

	/* Initialize event wait list and add the two transfer events. */
	ewl = ccl_event_wait_list_new();
	ccl_event_wait_list_add(ewl, evt_w1);
	ccl_event_wait_list_add(ewl, evt_w2);
	
	/* Set args and execute kernel, waiting for the two transfer events
	 * to terminate (this will empty the event wait list). */
	evt_kr = ccl_kernel_set_args_and_run(krnl, cq, 1, NULL, &gws, &lws, 
		ewl, &err, a_w, b_w, c_w, ccl_arg_priv(d_h, cl_uint), NULL);
	g_assert_no_error(err);
	
	/* Add the kernel termination event to the wait list. */
	ccl_event_wait_list_add(ewl, evt_kr);
	
	/* Read back results from host, waiting for the kernel termination
	 * event (this will empty the event wait list) without waiting for 
	 * transfer to terminate before continuing host program.. */
	evt_r1 = ccl_buffer_read(cq, c_w, CL_FALSE, 0, 
		CCL_TEST_PROGRAM_BUF_SIZE * sizeof(cl_uint), c_h, ewl, &err);
	g_assert_no_error(err);
	
	/* Add read back results event to wait list. */
	ccl_event_wait_list_add(ewl, evt_r1);
	
	/* Wait for all events in wait list to terminate (this will empty
	 * the wait list). */
	ccl_event_wait(ewl, &err);
	g_assert_no_error(err);
	
#ifndef OPENCL_STUB
	/* Check results are as expected (not available with OpenCL stub). */
	for (guint i = 0; i < CCL_TEST_PROGRAM_BUF_SIZE; i++) {
		g_assert_cmpuint(c_h[i], ==, a_h[i] + b_h[i] + d_h);
		g_debug("c_h[%d] = %d\n", i, c_h[i]);
	}
#endif

	/* Destroy the event wait list. */
	ccl_event_wait_list_destroy(ewl);

	/* Destroy the memory objects. */
	ccl_memobj_destroy(a_w);
	ccl_memobj_destroy(b_w);
	ccl_memobj_destroy(c_w);
	
	/* Destroy the command queue. */
	ccl_queue_destroy(cq);

	/* Destroy stuff. */
	ccl_program_destroy(prg);
	ccl_context_destroy(ctx);

	/* Free strings. */
	g_free(tmp_dir_name);

}

/** 
 * @brief Test increasing reference count of objects which compose 
 * larger objects, then destroy the larger object and verify that 
 * composing object still exists and must be freed by the function
 * which increase its reference count.
 * 
 * This function tests the following modules: program, queue, kernel,
 * event.
 * */
static void program_ref_unref_test() {

	CCLContext* ctx = NULL;
	GError* err = NULL;
	//~ CCLDevice* d = NULL;
	CCLProgram* prg = NULL;
	CCLKernel* krnl1 = NULL;
	CCLKernel* krnl2 = NULL;
	CCLQueue* cq = NULL;
	//~ CCLEvent* evt = NULL;
	
	const char* src = CCL_TEST_PROGRAM_SUM_CONTENT;
	
	/* Get some context. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);
	
	/* Create a program from source. */
	prg = ccl_program_new_from_source(ctx, src, &err);
	g_assert_no_error(err);

	/* Build program. */
	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);

	/* Get kernel wrapper from program (will be the instance kept in the 
	 * program wrapper). */
	krnl1 = ccl_program_get_kernel(prg, CCL_TEST_PROGRAM_SUM, &err);
	g_assert_no_error(err);
	
	/* Create another kernel wrapper for the same kernel. This should 
	 * yield a different object because we're not getting it from 
	 * the program wrapper. */
	krnl2 = ccl_kernel_new(prg, CCL_TEST_PROGRAM_SUM, &err);
	g_assert_no_error(err);

	/* Check that they're different. */
	g_assert_cmphex(GPOINTER_TO_UINT(krnl1), !=, GPOINTER_TO_UINT(krnl2));
	
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

	/* Create a command queue and check its ref count. */
	cq = ccl_queue_new(ctx, NULL, 0, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(ccl_wrapper_ref_count((CCLWrapper*) cq), ==, 1);
	
	/// @todo Do test for task and of native kernel
	
	/* Destroy remaining stuff. */
	ccl_queue_destroy(cq);
	ccl_program_destroy(prg);
	ccl_context_destroy(ctx);

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
		"/wrappers/program/create-info-destroy", 
		program_create_info_destroy_test);

	g_test_add_func(
		"/wrappers/program/ref-unref", 
		program_ref_unref_test);

	return g_test_run();
}



