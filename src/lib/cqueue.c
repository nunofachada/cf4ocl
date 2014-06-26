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
 * @brief OpenCL Command Queue wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "cqueue.h"

/**
 * @brief Command queue wrapper object.
 */
struct cl4_cqueue {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
};

/** 
 * @brief Decrements the reference count of the command queue wrapper 
 * object. If it reaches 0, the context wrapper object is destroyed.
 *
 * @param cq The context wrapper object.
 * */
void cl4_cqueue_destroy(CL4CQueue* cq) {
	
	/* Make sure command queue wrapper object is not NULL. */
	g_return_if_fail(cq != NULL);
	
	/* Wrapped OpenCL object (a command queue in this case), returned by
	 * the parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_command_queue command_queue;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	command_queue = 
		(cl_command_queue) cl4_wrapper_unref((CL4Wrapper*) cq);
	
	/* If an OpenCL command queue was returned, the reference count of 
	 * the wrapper object reached 0, so we must destroy remaining 
	 * command queue wrapper properties and the OpenCL command queue
	 * itself. */
	if (command_queue != NULL) {

		/* Release cq. */
		g_slice_free(CL4CQueue, cq);
		
		/* Release OpenCL context. */
		clReleaseCommandQueue(command_queue);
		
	}

}
