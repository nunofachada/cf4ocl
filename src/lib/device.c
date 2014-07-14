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
	
	/** Device platform. */
	//~ CL4Platform* platf;
	
};

/**
 * @brief Get the device wrapper for the given OpenCL device.
 * 
 * If the wrapper doesn't exist, its created with a reference count of 
 * 1. Otherwise, the existing wrapper is returned and its reference 
 * count is incremented by 1.
 * 
 * This function will rarely be called from client code, except when
 * clients wish to get the OpenCL device directly (using the
 * clGetDeviceIDs() function) and then wrap the OpenCL device in a
 * ::CL4Device wrapper object.
 * 
 * @param device_id The OpenCL device to be wrapped.
 * @return The device wrapper for the given OpenCL device.
 * */
CL4Device* cl4_device_new_wrap(cl_device_id device) {
	
	return (CL4Device*) cl4_wrapper_new(
		(void*) device, sizeof(CL4Device));
		
}

/** 
 * @brief Decrements the reference count of the device wrapper object. 
 * If it reaches 0, the device wrapper object is destroyed.
 *
 * @param dev The device wrapper object.
 * */
void cl4_device_destroy(CL4Device* dev) {
	
	cl4_wrapper_unref((CL4Wrapper*) dev, sizeof(CL4Device),
		NULL, NULL, NULL); 

}

//~ CL4Platform* cl4_device_get_platform(CL4Device* dev, GError** err) {
	//~ 
	//~ /* Make sure dev is not NULL. */
	//~ g_return_val_if_fail(cq != NULL, NULL);
	//~ /* Make sure err is NULL or it is not set. */
	//~ g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	//~ 
	//~ CL4Platform* platf = NULL;
	//~ 
	//~ /* Internal error object. */
	//~ GError* err_internal = NULL;
	//~ 
	//~ if (dev->platf != NULL) {
		//~ platf = dev->platf;
	//~ } else {
		//~ CL4WrapperInfo* info = NULL;
		//~ info = cl4_device_info(
			//~ dev, CL_DEVICE_PLATFORM, &err_internal);
		//~ gef_if_err_propagate_goto(err, err_internal, error_handler);
		//~ platf = cl4_platform_new_wrap(*((cl_platform_id*) info->value));
		//~ gef_if_err_propagate_goto(err, err_internal, error_handler);
		//~ dev->platf = platf;
	//~ }
//~ 
	//~ /* If we got here, everything is OK. */
	//~ g_assert (err == NULL || *err == NULL);
	//~ goto finish;
//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert(err == NULL || *err != NULL);
	//~ 
//~ finish:		
//~ 
	//~ /* Return the device platform wrapper. */
	//~ return platf;	
	//~ 
//~ }
