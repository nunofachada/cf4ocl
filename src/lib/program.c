/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with cf4ocl. If not, see 
 * <http://www.gnu.org/licenses/>.
 * */
 
 /** 
 * @file
 * @brief OpenCL program wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "program.h"

/**
 * @brief Program wrapper object.
 */
struct cl4_program {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
};

CL4Program* cl4_program_new(
	CL4Context* ctx, const char* file, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure file is not NULL. */
	g_return_val_if_fail(file != NULL, NULL);

	/* Error reporting object. */
	GError* err_internal = NULL;
	
	CL4Program* prg = NULL;
	
	gchar* src = NULL;

	g_file_get_contents(file, &src, NULL, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	prg = cl4_program_new_with_source(ctx, 1, (const char**) &src, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:

	if (src != NULL)
		g_free(src);
		
	/* Return prg. */
	return prg;	

}

CL4Program* cl4_program_new_with_source(CL4Context* ctx, cl_uint count, 
	const char **strings, GError** err) {

	/* Make sure ctx is not NULL. */
	g_return_val_if_fail(ctx != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	/* Make sure count > 0. */
	g_return_val_if_fail(count > 0, NULL);
	/* Make sure strings is not NULL. */
	g_return_val_if_fail(strings != NULL, NULL);

	cl_int ocl_status;
	
	CL4Program* prg = NULL;
		
	prg = g_slice_new(CL4Program);
	cl4_wrapper_init(&prg->base);
	
	prg->base.cl_object = (cl_program) clCreateProgramWithSource(
		cl4_context_unwrap(ctx), count, strings, NULL, &ocl_status);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"Function '%s': unable to create cl_program from source (OpenCL error %d: %s).", 
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

	/* Destroy what was built for the context wrapper. */
	cl4_program_destroy(prg);
	prg = NULL;
	
finish:

	/* Return prg. */
	return prg;	
}

/** 
 * @brief Decrements the reference count of the program wrapper 
 * object. If it reaches 0, the program wrapper object is 
 * destroyed.
 *
 * @param prg The program wrapper object.
 * */
void cl4_program_destroy(CL4Program* prg) {
	
	/* Make sure program wrapper object is not NULL. */
	g_return_if_fail(prg != NULL);
	
	/* Wrapped OpenCL object (a program in this case), returned by
	 * the parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_program program;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	program = (cl_program) cl4_wrapper_unref((CL4Wrapper*) prg);
	
	/* If an OpenCL program was returned, the reference count of 
	 * the wrapper object reached 0, so we must destroy remaining 
	 * program wrapper properties and the OpenCL program
	 * itself. */
	if (program != NULL) {

		/* Release prg. */
		g_slice_free(CL4Program, prg);
		
		/* Release OpenCL program, ignore possible errors. */
		cl4_wrapper_release_cl_object(program, 
			(cl4_wrapper_release_function) clReleaseProgram);		
		
	}

}


