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
 * Test the ccl_c utility.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <sys/wait.h>
#include <cf4ocl2.h>
#include "test.h"

/* Full path to the ccl_c binary and a string placeholder flag for command
 * line options. */
#define CCL_TEST_C_COM \
	CCL_TEST_BUILD_DIR G_DIR_SEPARATOR_S "src" G_DIR_SEPARATOR_S "utils" \
	G_DIR_SEPARATOR_S "ccl_c %s "

/* Full path to the ccl_c binary and a string placeholder flag for command line
 * options and an integer placeholder flag for command line device
 * specification. */
#define CCL_TEST_C_COM_DEV \
	CCL_TEST_C_COM "-d %u"

/* Full path to the example binaries and kernels. */
#define CCL_TEST_C_EXAMPLES \
	CCL_TEST_SRC_DIR G_DIR_SEPARATOR_S "src" G_DIR_SEPARATOR_S \
	"examples"

/* Full path to the canon.cl kernel */
#define CCL_TEST_C_K1_OK \
	CCL_TEST_C_EXAMPLES G_DIR_SEPARATOR_S "canon.cl"

/* Full path to the ca.cl kernel */
#define CCL_TEST_C_K2_OK \
	CCL_TEST_C_EXAMPLES G_DIR_SEPARATOR_S "ca.cl"

/* Full path to a file which will certainly not compile as an OpenCL kernel. */
#define CCL_TEST_C_K3_KO \
	CCL_TEST_C_EXAMPLES G_DIR_SEPARATOR_S "CMakeLists.txt"

/* Full path to a generated binary file. */
#define CCL_TEST_C_BIN \
	CCL_TEST_BUILD_DIR G_DIR_SEPARATOR_S "out.bin"

/**
 * Tests information requests made to the ccl_c utility.
 * */
static void info_test() {

	/* Test variables. */
	int status;
	GString* com;

	/* Initialize command string object. */
	com = g_string_sized_new(strlen(CCL_TEST_C_COM) + 10);

	/* Test help, which should return status 0. */
	g_string_printf(com, CCL_TEST_C_COM, "-?");
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	g_string_printf(com, CCL_TEST_C_COM, "--help");
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test list, which should return status 0. */
	g_string_printf(com, CCL_TEST_C_COM, "-l");
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	g_string_printf(com, CCL_TEST_C_COM, "--list");
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test version, which should return status 0. */
	g_string_printf(com, CCL_TEST_C_COM, "--version");
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Release command string object. */
	g_string_free(com, TRUE);

}


/**
 * Tests the build task of the ccl_c utility.
 * */
static void build_test() {

	/* Test variables. */
	int status;
	GString* com;

	/* Initialize command string object. */
	com = g_string_sized_new(strlen(CCL_TEST_C_COM_DEV) + 100);

	/* Test simple build with one source file. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K1_OK, ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test build with two source files. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K1_OK " -s " CCL_TEST_C_K2_OK,  ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test build with one source and create binary. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K1_OK " -o " CCL_TEST_C_BIN "1", ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test build with another source and create another binary. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K2_OK " -o " CCL_TEST_C_BIN "2", ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test build with one binary. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -b " CCL_TEST_C_BIN "1", ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_SUCCESS);

	/* Test build with two binaries. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -b " CCL_TEST_C_BIN "1 -b " CCL_TEST_C_BIN "2", ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Test build with one binary and one source file. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K1_OK " -b " CCL_TEST_C_BIN, ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Test build with source headers. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -h " CCL_TEST_C_K1_OK, ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Test build with erroneous kernel. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K3_KO,  ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Test build with non-existing device. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s " CCL_TEST_C_K1_OK " -s " CCL_TEST_C_K2_OK, 100000);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Test build with non-existing file. */
	g_string_printf(com, CCL_TEST_C_COM_DEV,
		" -s this_file_does_not_exist.cl", ccl_tests_devidx);
	g_debug("%s", com->str);
	status = system(com->str);
	g_assert_cmpint(WEXITSTATUS(status), ==, EXIT_FAILURE);

	/* Remove binaries. */
	g_unlink(CCL_TEST_C_BIN "1");
	g_unlink(CCL_TEST_C_BIN "2");

	/* Release command string object. */
	g_string_free(com, TRUE);

}


/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	ccl_test_init_device_index();
	g_test_init(&argc, &argv, NULL);

	g_test_add_func(
		"/utils/c/info",
		info_test);

	g_test_add_func(
		"/utils/c/build",
		build_test);

	/*g_test_add_func(
		"/utils/c/compile",
		compile_test);

	g_test_add_func(
		"/utils/c/link",
		link_test);*/

	return g_test_run();
}



