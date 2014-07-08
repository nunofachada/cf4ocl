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
 * @brief OpenCL cl_mem wrapper object.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

/** 
 * @brief Decrements the reference count of the cl_mem wrapper 
 * object. If it reaches 0, the cl_mem wrapper object is destroyed.
 *
 * @param mo The cl_mem wrapper object.
 * */
void cl4_event_destroy(CL4MemObject* mo) {
	
	/* Make sure cl_mem wrapper object is not NULL. */
	g_return_if_fail(mo != NULL);
	
	/* Wrapped OpenCL memory object, returned by the parent wrapper 
	 * unref function in case its reference count reaches 0. */
	cl_mem mem_object;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	mem_object = (cl_mem) cl4_wrapper_unref((CL4Wrapper*) mo);
	
	/* If an OpenCL mem. object was returned, the reference count of 
	 * the wrapper object reached 0, so we must destroy remaining 
	 * cl_mem wrapper properties and the OpenCL mem. object itself. */
	if (mem_object != NULL) {

		/* Release evt. */
		g_slice_free(CL4Event, evt);
		
		/* Release OpenCL event, ignore possible errors. */
		cl4_wrapper_release_cl_object(event, 
			(cl4_wrapper_release_function) clReleaseEvent);
		
	}

}

