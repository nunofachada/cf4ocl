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
 * @brief Tests for profiling framework.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "profiler.h"
#include "ocl_stub/ocl_impl.h"

/**
 * @brief Tests .
 * */
static void create_add_destroy_test() {
	
	CL4Prof* prof = cl4_prof_new();
	
	cl4_prof_destroy(prof);

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
		"/profiler/create-add-destroy", create_add_destroy_test);
	return g_test_run();
}

