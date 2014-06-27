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
 * @brief OpenCL event wrapper.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "event.h"

/**
 * @brief Event wrapper object.
 */
struct cl4_event {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
};

/** 
 * @brief Decrements the reference count of the event wrapper 
 * object. If it reaches 0, the event wrapper object is 
 * destroyed.
 *
 * @param evt The event wrapper object.
 * */
void cl4_event_destroy(CL4Event* evt) {
	
	/* Make sure event wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);
	
	/* Wrapped OpenCL object (a event in this case), returned by
	 * the parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_event event;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	event = (cl_event) cl4_wrapper_unref((CL4Wrapper*) evt);
	
	/* If an OpenCL event was returned, the reference count of 
	 * the wrapper object reached 0, so we must destroy remaining 
	 * event wrapper properties and the OpenCL event
	 * itself. */
	if (event != NULL) {

		/* Release evt. */
		g_slice_free(CL4Event, evt);
		
		/* Release OpenCL event, ignore possible errors. */
		cl4_wrapper_release_cl_object(event, 
			(cl4_wrapper_release_function) clReleaseEvent);
		
	}

}
