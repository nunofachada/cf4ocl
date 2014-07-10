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
 * @brief Get the event wrapper for the given OpenCL event.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to wrap the OpenCL event directly.
 * 
 * @param event The OpenCL event to be wrapped.
 * @return The event wrapper for the given OpenCL event.
 * */
CL4Event* cl4_event_new_wrap(cl_event event) {
	
	return (CL4Event*) cl4_wrapper_new(
		(void*) event, sizeof(CL4Event));
		
}

/** 
 * @brief Decrements the reference count of the event wrapper object. 
 * If it reaches 0, the event wrapper object is destroyed.
 *
 * @param evt The event wrapper object.
 * */
void cl4_event_destroy(CL4Event* evt) {
	
	cl4_wrapper_unref((CL4Wrapper*) evt, sizeof(CL4Event),
		NULL, (cl4_wrapper_release_cl_object) clReleaseEvent, NULL); 

}
