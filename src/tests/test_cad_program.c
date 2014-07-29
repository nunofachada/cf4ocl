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

#define CCL_TEST_PROGRAM_SUM_NAME CCL_TEST_PROGRAM_SUM ".cl"

#define CCL_TEST_PROGRAM_SUM_CONTENT \
	"__kernel void " CCL_TEST_PROGRAM_SUM "(" \
	"		__global const uint *a," \
	"		__global const uint *b," \
	"		__global uint *c, uint d)" \
	"{" \
	"	int gid = get_global_id(0);" \
	"	c[gid] = a[gid] + b[gid] + d;" \
	"}"

/**
 * @brief Tests creation, getting info from and destruction of 
 * program wrapper objects.
 * */
static void program_create_info_destroy_test() {

	CCLContext* ctx = NULL;
	CCLProgram* prg = NULL;
	CCLKernel* krnl = NULL;
	CCLWrapperInfo* info = NULL;
	CCLDevice* d = NULL;
	CCLQueue* cq = NULL;
	GError* err = NULL;
	
	/* Create a temporary kernel file. */
	g_file_set_contents(CCL_TEST_PROGRAM_SUM_NAME, 
		CCL_TEST_PROGRAM_SUM_CONTENT, -1, &err);
	g_assert_no_error(err);

	/* Create a context with first available device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);
	
	/* Get the device being used. */
	d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a new program from kernel file. */
	prg = ccl_program_new_from_source_file(
		ctx, CCL_TEST_PROGRAM_SUM_NAME, &err);
	g_assert_no_error(err);
	
	/* Get some program info, compare it with expected info. */
	info = ccl_program_get_info(prg, CL_PROGRAM_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == ccl_context_unwrap(ctx));

	info = ccl_program_get_info(prg, CL_PROGRAM_NUM_DEVICES, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(*((cl_uint*) info->value), 
		==, ccl_context_get_num_devices(ctx, &err));
	g_assert_no_error(err);

	info = ccl_program_get_info(prg, CL_PROGRAM_SOURCE, &err);
	g_assert_no_error(err);
	g_assert_cmpstr((char*) info->value, 
		==, CCL_TEST_PROGRAM_SUM_CONTENT);
	
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

	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_LOG, &err);
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

	info = ccl_kernel_get_info(krnl, CL_KERNEL_CONTEXT, &err);
	g_assert_no_error(err);
	g_assert(*((cl_context*) info->value) == ccl_context_unwrap(ctx));

	info = ccl_kernel_get_info(krnl, CL_KERNEL_PROGRAM, &err);
	g_assert_no_error(err);
	g_assert(*((cl_program*) info->value) == ccl_program_unwrap(prg));

	/* Remove temporary kernel file. */
	if (g_unlink(CCL_TEST_PROGRAM_SUM_NAME) < 0)
		g_message("Unable to delete temporary file '"
			CCL_TEST_PROGRAM_SUM_NAME "'");
			
	/* Save binaries for all available devices. */
	ccl_program_save_all_binaries(prg, "test_", ".bin", &err);
	g_assert_no_error(err);
	
	/* Save binary for a specific device (which we will load into a new
	 * program later). */
	ccl_program_save_binary(prg, d, "test_prg.bin", &err);
	g_assert_no_error(err);
	
	/* Destroy program. */
	ccl_program_destroy(prg);

	/* Create a new program using the saved binary. */
	prg = ccl_program_new_from_binary_file(ctx, d, "test_prg.bin", &err);
	g_assert_no_error(err);
	
	/* **** BUILD PROGRAM **** */
	ccl_program_build(prg, NULL, &err);
	g_assert_no_error(err);
	
	/* Get some program build info, compare it with expected values. */
	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_STATUS, &err);
	g_assert_no_error(err);
	g_assert((*((cl_build_status*) info->value) == CL_BUILD_SUCCESS) 
		|| (*((cl_build_status*) info->value) == CL_BUILD_IN_PROGRESS));

	info = ccl_program_get_build_info(prg, d, CL_PROGRAM_BUILD_LOG, &err);
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
	
	/* Set args and execute kernel. */
	size_t gws = 16;
	size_t lws = 8;
	cl_uint a_h[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	cl_uint b_h[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	cl_uint c_h[16];
	cl_uint d_h = 4;
	
	CCLMemObj* a_w = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, 16 * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	CCLMemObj* b_w = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, 16 * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	CCLMemObj* c_w = ccl_buffer_new(ctx, CL_MEM_WRITE_ONLY, 16 * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);
	
	CCLEvent* evt_w1 = ccl_buffer_write(cq, a_w, CL_FALSE, 0, 16 * sizeof(cl_uint), a_h, NULL, &err);
	g_assert_no_error(err);
	CCLEvent* evt_w2 = ccl_buffer_write(cq, b_w, CL_FALSE, 0, 16 * sizeof(cl_uint), b_h, NULL, &err);
	g_assert_no_error(err);

	CCLEventWaitList ewl = ccl_event_wait_list_new();
	ccl_event_wait_list_add(ewl, evt_w1);
	ccl_event_wait_list_add(ewl, evt_w2);
	
	CCLEvent* evt_kr = ccl_kernel_set_args_and_run(krnl, cq, 1, NULL, &gws, &lws, ewl, 
		&err, a_w, b_w, c_w, ccl_arg_private(d_h, cl_uint), NULL);
	g_assert_no_error(err);
	
	ccl_event_wait_list_add(ewl, evt_kr);
	CCLEvent* evt_r1 = ccl_buffer_read(cq, c_w, CL_FALSE, 0, 16 * sizeof(cl_uint), c_h, ewl, &err);
	g_assert_no_error(err);
	
	ccl_event_wait_list_add(ewl, evt_r1);
	
	ccl_event_wait(ewl, &err);
	g_assert_no_error(err);
	
#ifndef OPENCL_STUB
	for (guint i = 0; i < 16; i++) {
		g_assert_cmpuint(c_h[i], ==, a_h[i] + b_h[i] + d_h);
		//printf("c_h[%d] = %d\n", i, c_h[i]);
	}
#endif

	ccl_event_wait_list_destroy(ewl);

	ccl_memobj_destroy(a_w);
	ccl_memobj_destroy(b_w);
	ccl_memobj_destroy(c_w);
	
	/* Destroy the command queue. */
	ccl_queue_destroy(cq);

	/* Delete all created binaries. */
	/// @todo
	
	/* Destroy stuff. */
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

	return g_test_run();
}



