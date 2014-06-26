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
		
		/* Release OpenCL program. */
		clReleaseProgram(program);
		
	}

}


