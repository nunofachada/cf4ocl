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
 * Tests creation, addition, destruction (CAD) for profiling 
 * framework objects.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "profiler.h"

/**
 * Tests creation, getting info from and destruction of 
 * profiler objects, and their relationship with context, device and 
 * queue wrapper objects.
 * */
static void create_add_destroy_test() {
	
	/* Error reporting object. */
	GError* err = NULL;
	
	/* Create a new profile object. */
	CCLProf* prof = ccl_prof_new();
	
	/* Get a context and a device. */
	CCLContext* ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);
	
	CCLDevice* d = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create two command queue wrappers. */
	CCLQueue* cq1 = ccl_queue_new(
		ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	
	CCLQueue* cq2 = ccl_queue_new(
		ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);

	/* Add both queues to profile object. */
	ccl_prof_add_queue(prof, "A Queue", cq1);
	ccl_prof_add_queue(prof, "Another Queue", cq2);
	
	/* Unref cq1, which should not be destroyed because it is held
	 * by the profile object. */
	ccl_queue_destroy(cq1);

	/* Destroy the profile object, which will also destroy cq1. cq2 
	 * will me merely unrefed and must still be explicitly destroyed. */
	ccl_prof_destroy(prof);

	/* Destroy cq2. */
	ccl_queue_destroy(cq2);
	
	/* Destroy the context. */
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
		"/profiler/create-add-destroy", create_add_destroy_test);
	return g_test_run();
}

