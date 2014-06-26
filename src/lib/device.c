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
 * @brief Wrapper object for OpenCL devices. Contains device and device
 * information.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "device.h"

/**
 * @brief Device wrapper object.
 */
struct cl4_device {

	/** Parent wrapper object. */
	CL4Wrapper base;
	
};

/**
 * @brief Creates a new device wrapper object.
 * 
 * @param id The OpenCL device ID object.
 * @return A new device wrapper object.
 * */
CL4Device* cl4_device_new(cl_device_id id) {
	
	/* The device wrapper object. */
	CL4Device* device;
		
	/* Allocate memory for the device wrapper object. */
	device = g_slice_new(CL4Device);
	
	/* Initialize parent object. */
	cl4_wrapper_init(&device->base);
	
	/* Set the device ID. */
	device->base.cl_object = id;

	/* Return the new device wrapper object. */
	return device;
	
}

/** 
 * @brief Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed.
 *
 * @param device The device wrapper object. 
 * */
void cl4_device_destroy(CL4Device* device) {
	
	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(device != NULL);

	/* Wrapped OpenCL object (a device_id in this case), returned by the
	 * parent wrapper unref function in case its reference count 
	 * reaches 0. */
	cl_device_id device_id;
	
	/* Decrease reference count using the parent wrapper object unref 
	 * function. */
	device_id = (cl_device_id) cl4_wrapper_unref((CL4Wrapper*) device);
	
	/* If an OpenCL device_id was returned, the reference count of the
	 * wrapper object reached 0, so we must destroy remaining device
	 * wrapper properties. */
	if (device_id != NULL) {

		/* Free the device wrapper object. */
		g_slice_free(CL4Device, device);
		
	}

}
