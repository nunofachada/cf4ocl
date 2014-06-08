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
 * @brief OpenCL stub for testing the profile module.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.h>
#else
	#include <CL/cl.h>
#endif

/** @brief Stub for cl_event objects. */ 
struct _cl_event {
	cl_ulong start;
	cl_ulong end;
	cl_command_queue queue;
};

/** @brief Stub for cl_command_queue objects. */ 
struct _cl_command_queue {
	int filler;
};

/** 
 * @brief Stub for clGetEventProfilingInfo function. 
 * 
 * @param event Stub event.
 * @param param_name Specifies the profiling data to query.
 * @param param_value_size Ignored.
 * @param param_value A pointer to memory where the appropriate result being queried is returned.
 * @param param_value_size_ret Ignored.
 * @return Always returns CL_SUCCESS.
 * */ 
cl_int clGetEventProfilingInfo(cl_event event, 
	cl_profiling_info param_name, size_t param_value_size, 
	void* param_value, size_t* param_value_size_ret) {
		
	/* Ignore compiler warnings. */
	param_value_size = param_value_size; param_value_size_ret = param_value_size_ret;
	
	/* Return start or end instants in given memory location. */
	if (param_name == CL_PROFILING_COMMAND_START)
		*((cl_ulong*) param_value) = event->start;
	else
		*((cl_ulong*) param_value) = event->end;
	
	/* Always return success. */
	return CL_SUCCESS;
}

/** 
 * @brief Stub for clGetEventInfo function. 
 * 
 * @param event Stub event.
 * @param param_name Ignored (assumes CL_EVENT_COMMAND_QUEUE).
 * @param param_value_size Ignored.
 * @param param_value Memory location where to place fake queue.
 * @param param_value_size_ret Ignored.
 * @return Always returns CL_SUCCESS.
 * */ 
cl_int clGetEventInfo(cl_event event, cl_event_info param_name, 
	size_t param_value_size, void *param_value, 
	size_t *param_value_size_ret) {
		
	/* Ignore compiler warnings. */
	param_name = param_name; param_value_size = param_value_size; param_value_size_ret = param_value_size_ret;
	/* Return the event command queue in given memor location. */
	*((cl_command_queue*) param_value) = event->queue;
	/* Always return success. */
	return CL_SUCCESS;

}

cl_command_queue clCreateCommandQueue(cl_context context,
	cl_device_id device, cl_command_queue_properties properties,
	cl_int* errcode_ret) {
		
	context = context; device = device; properties = properties;
	
	if (errcode_ret != NULL)
		*errcode_ret = CL_SUCCESS;
		
	cl_command_queue queue = 
		(cl_command_queue) malloc(sizeof(struct _cl_command_queue));
		
	queue->filler = 0;
	
	return queue;
		
}

cl_int clReleaseCommandQueue(cl_command_queue command_queue) {
	free(command_queue);
	return CL_SUCCESS;
}
