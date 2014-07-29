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
 * @brief OpenCL event stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name,
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
		
	cl_int status = CL_SUCCESS;

	if (event == NULL) {
		status = CL_INVALID_EVENT;
	} else {
		switch (param_name) {
			case CL_PROFILING_COMMAND_QUEUED:
				ccl_test_basic_info(cl_ulong, event, t_queued);
			case CL_PROFILING_COMMAND_SUBMIT:
				ccl_test_basic_info(cl_ulong, event, t_submit);
			case CL_PROFILING_COMMAND_START:
				ccl_test_basic_info(cl_ulong, event, t_start);
			case CL_PROFILING_COMMAND_END:
				ccl_test_basic_info(cl_ulong, event, t_end);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;

}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event event, cl_event_info param_name, 
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {

	cl_int status = CL_SUCCESS;

	if (event == NULL) {
		status = CL_INVALID_EVENT;
	} else {
		switch (param_name) {
			case CL_EVENT_COMMAND_QUEUE:
				ccl_test_basic_info(cl_command_queue, event, command_queue);
			case CL_EVENT_CONTEXT:
				ccl_test_basic_info(cl_context, event, context);
			case CL_EVENT_COMMAND_TYPE:
				ccl_test_basic_info(cl_command_type, event, command_type);
			case CL_EVENT_COMMAND_EXECUTION_STATUS:
				ccl_test_basic_info(cl_int, event, exec_status);
			case CL_EVENT_REFERENCE_COUNT:
				ccl_test_basic_info(cl_uint, event, ref_count);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;

}


CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0 {
	
	g_atomic_int_inc(&event->ref_count);
	return CL_SUCCESS;
	
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0 {
	
	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&event->ref_count)) {

		g_slice_free(struct _cl_event, event);
		
	}
	
	return CL_SUCCESS;
	
}

CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint num_events, const cl_event* event_list) 
	CL_API_SUFFIX__VERSION_1_0 {
	
	num_events = num_events;
	event_list = event_list;
	
	return CL_SUCCESS;
}


