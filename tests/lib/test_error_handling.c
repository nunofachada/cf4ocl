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
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 *
 * Tests error handling in _cf4ocl_.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "_ccl_defs.h"

/** Resolves to error category identifying string, in this case an error
 * in the CCLErrf tests. */
#define TEST_CCL_ERROR test_error_handling_error_quark()

/**
 * Test error codes.
 * */
enum test_error_handling_error_codes {
	TEST_CCL_SUCCESS = 0,
	TEST_CCL_ERROR_1 = -1,
	TEST_CCL_ERROR_2 = -2
};

/**
 * Resolves to error category identifying string, in this case an
 * error in the CCLErrf tests.
 *
 * @return A GQuark structure defined by category identifying string,
 * which identifies the error as a gerrof tests generated error.
 */
GQuark test_error_handling_error_quark() {
	return g_quark_from_static_string("test-error-handling-error-quark");
}

/* ************** */
/* Aux. functions */
/* ************** */

/* This function can create an error. */
static cl_bool error_l2_aux(
	int code, const char* xtramsg, CCLErr **err) {

	/* Return status variable. */
	cl_bool status;

	/* Check for error. */
	ccl_if_err_create_goto(*err, TEST_CCL_ERROR,
		code != TEST_CCL_SUCCESS, code, error_handler,
		"Big error in level %d function: %s", 2, xtramsg);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	status = CL_FALSE;

finish:

	/* Return status. */
	return status;

}

/* This function can propagate an error created by another function. */
static cl_bool error_l1_aux(int code, CCLErr **err) {

	/* Return status variable. */
	cl_bool status;

	/* Internal error handling variable. */
	CCLErr* err_internal = NULL;

	/* Call sub-function, check for error. */
	error_l2_aux(code, "called by error_l1_aux", &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	status = CL_FALSE;

finish:

	/* Return status. */
	return status;

}

/* ************** */
/* Test functions */
/* ************** */

/**
 * Test one level error handling.
 * */
static void error_one_level_test() {

	/* Error handling object. */
	CCLErr *err = NULL;

	/* Status variable. */
	cl_bool status;

	/* Call function which can create an error, and force it to create
	 * error. */
	status = error_l2_aux(TEST_CCL_ERROR_1,
		"called by error_one_level_test", &err);
	ccl_if_err_goto(err, error_handler);

	/* Function should throw error, so we shouldn't get here. */
	g_assert_not_reached();
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_1);
	g_assert_cmpint(status, ==, CL_FALSE);
	g_assert_cmpstr(err->message, ==,
		"Big error in level 2 function: called by error_one_level_test");
	g_error_free(err);

finish:

	/* Return. */
	return;

}

/**
 * Test two level error handling.
 * */
static void error_two_level_test() {

	/* Error handling object. */
	CCLErr *err = NULL;

	/* Status variable. */
	cl_bool status;

	/* Call function which can propagate an error thrown by a
	 * sub-function. */
	status = error_l1_aux(TEST_CCL_ERROR_2, &err);
	ccl_if_err_goto(err, error_handler);

	/* Function should throw error, so we shouldn't get here. */
	g_assert_not_reached();
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_2);
	g_assert_cmpint(status, ==, CL_FALSE);
	g_assert_cmpstr(err->message, ==,
		"Big error in level 2 function: called by error_l1_aux");
	g_error_free(err);

finish:

	/* Return. */
	return;

}

/**
 * Test no errors.
 * */
static void error_none_test() {

	/* Error handling object. */
	CCLErr *err = NULL;

	/* Status variable. */
	cl_bool status;

	/* Call a function which will not throw an error. */
	status = error_l2_aux(TEST_CCL_SUCCESS,
		"called by error_one_level_test", &err);
	ccl_if_err_goto(err, error_handler);

	/* If we got here, everything is OK. */
	goto finish;

error_handler:

	/* No error should be thrown. */
	g_assert_not_reached();

finish:

	/* Confirm no error was thrown. */
	g_assert_no_error(err);
	g_assert_cmpint(status, ==, CL_TRUE);

	/* Return. */
	return;
}

/**
 * Test an error without additional arguments.
 * */
static void error_no_vargs_test() {

	/* Error handling object. */
	CCLErr *err = NULL;

	/* Create an error without additional arguments. */
	ccl_if_err_create_goto(err, TEST_CCL_ERROR, 1,
		TEST_CCL_ERROR_1, error_handler,
		"I have no additional arguments");

	/* We shouldn't get here. */
	g_assert_not_reached();
	goto finish;

error_handler:

	/* Check error properties. */
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_1);
	g_assert_cmpstr(err->message, ==, "I have no additional arguments");
	g_error_free(err);

finish:

	/* Return. */
	return;
}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/utils/error-onelevel", error_one_level_test);
	g_test_add_func("/utils/error-twolevel", error_two_level_test);
	g_test_add_func("/utils/error-none", error_none_test);
	g_test_add_func("/utils/error-novargs", error_no_vargs_test);

	return g_test_run();
}
