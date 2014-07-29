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

#include "event_wrapper.h"

/**
 * @brief Event wrapper object.
 */
struct ccl_event {

	/** Parent wrapper object. */
	CCLWrapper base;
	
	/** Event name, for profiling purposes only. */
	const char* name;
	
	/** Final event name, for profiling purposes only. It is 
	 * automatically determined based on event type when event name is 
	 * not set. */
	const char* final_name;
	
};

/** 
 * @addtogroup EVENT_WRAPPER
 * @{
 */

/** 
 * @brief Decrements the reference count of the event wrapper object. 
 * If it reaches 0, the event wrapper object is destroyed.
 *
 * @param evt The event wrapper object.
 * */
void ccl_event_destroy(CCLEvent* evt) {
	
	ccl_wrapper_unref((CCLWrapper*) evt, sizeof(CCLEvent),
		NULL, (ccl_wrapper_release_cl_object) clReleaseEvent, NULL); 

}

void ccl_event_set_name(CCLEvent* evt, const char* name) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Set event name. */
	evt->name = name;

}

const char* ccl_event_get_name(CCLEvent* evt) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Return event name. */
	return evt->name;
	
}

const char* ccl_event_get_final_name(CCLEvent* evt) {

	/* Make sure evt wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Check if final name is NULL... */
	if (evt->final_name == NULL) {
		/* ...if so, check if name is also NULL. */
		if (evt->name == NULL) {
			/* Name is NULL, determine a final name based on type of
			 * command which produced the event. */
			
			GError* err_internal = NULL;
			
			cl_command_type ct = 
				ccl_event_get_command_type(evt, &err_internal);
			
			if (err_internal != NULL) {
				g_warning("Unable to determine final event name due to" \
					"the following error: %s", err_internal->message);
				g_error_free(err_internal);
				return NULL;
			}
			
			switch (ct) {
				case CL_COMMAND_NDRANGE_KERNEL:
					evt->final_name = "NDRANGE_KERNEL"; 
					break;
				case CL_COMMAND_NATIVE_KERNEL:
					evt->final_name = "NATIVE_KERNEL"; 
					break;
				case CL_COMMAND_READ_BUFFER:
					evt->final_name = "READ_BUFFER"; 
					break;
				case CL_COMMAND_WRITE_BUFFER:
					evt->final_name = "WRITE_BUFFER"; 
					break;
				case CL_COMMAND_COPY_BUFFER:
					evt->final_name = "COPY_BUFFER";
					break;
				case CL_COMMAND_READ_IMAGE:
					evt->final_name = "READ_IMAGE";
					break;
				case CL_COMMAND_WRITE_IMAGE:
					evt->final_name = "WRITE_IMAGE";
					break;
				case CL_COMMAND_COPY_IMAGE:
					evt->final_name = "COPY_IMAGE";
					break;
				case CL_COMMAND_COPY_BUFFER_TO_IMAGE:
					evt->final_name = "COPY_BUFFER_TO_IMAGE"; 
					break;
				case CL_COMMAND_COPY_IMAGE_TO_BUFFER:
					evt->final_name = "COPY_IMAGE_TO_BUFFER"; 
					break;
				case CL_COMMAND_MAP_BUFFER:
					evt->final_name = "MAP_BUFFER"; 
					break;
				case CL_COMMAND_MAP_IMAGE:
					evt->final_name = "MAP_IMAGE"; 
					break;
				case CL_COMMAND_UNMAP_MEM_OBJECT:
					evt->final_name = "UNMAP_MEM_OBJECT"; 
					break;
				case CL_COMMAND_MARKER:
					evt->final_name = "MARKER"; 
					break;
				case CL_COMMAND_ACQUIRE_GL_OBJECTS:
					evt->final_name = "ACQUIRE_GL_OBJECTS"; 
					break;
				case CL_COMMAND_RELEASE_GL_OBJECTS:
					evt->final_name = "RELEASE_GL_OBJECTS"; 
					break;
				case CL_COMMAND_READ_BUFFER_RECT:
					evt->final_name = "READ_BUFFER_RECT"; 
					break;
				case CL_COMMAND_WRITE_BUFFER_RECT:
					evt->final_name = "WRITE_BUFFER_RECT"; 
					break;
				case CL_COMMAND_COPY_BUFFER_RECT:
					evt->final_name = "COPY_BUFFER_RECT"; 
					break;
				case CL_COMMAND_USER:
					/* This is here just for completeness, as a user
					 * event can't be profiled. */
					evt->final_name = "USER"; 
					break;
				case CL_COMMAND_BARRIER:
					evt->final_name = "BARRIER"; 
					break;
				case CL_COMMAND_MIGRATE_MEM_OBJECTS:
					evt->final_name = "MIGRATE_MEM_OBJECTS"; 
					break;
				case CL_COMMAND_FILL_BUFFER:
					evt->final_name = "FILL_BUFFER"; 
					break;
				case CL_COMMAND_FILL_IMAGE:
					evt->final_name = "FILL_IMAGE"; 
					break;
				case CL_COMMAND_SVM_FREE:
					evt->final_name = "SVM_FREE"; 
					break;
				case CL_COMMAND_SVM_MEMCPY:
					evt->final_name = "SVM_MEMCPY"; 
					break;
				case CL_COMMAND_SVM_MEMFILL:
					evt->final_name = "SVM_MEMFILL"; 
					break;
				case CL_COMMAND_SVM_MAP:
					evt->final_name = "SVM_MAP"; 
					break;
				case CL_COMMAND_SVM_UNMAP:
					evt->final_name = "SVM_UNMAP"; 
					break;
				case CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR:
					evt->final_name = "GL_FENCE_SYNC_OBJECT_KHR"; 
					break;
				case CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR:
					evt->final_name = "ACQUIRE_D3D10_OBJECTS_KHR"; 
					break;
				case CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR:
					evt->final_name = "RELEASE_D3D10_OBJECTS_KHR"; 
					break;
				case CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR:
					evt->final_name = "ACQUIRE_DX9_MEDIA_SURFACES_KHR"; 
					break;
				case CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR:
					evt->final_name = "RELEASE_DX9_MEDIA_SURFACES_KHR"; 
					break;
				case CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR:
					evt->final_name = "ACQUIRE_D3D11_OBJECTS_KHR"; 
					break;
				case CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR:
					evt->final_name = "RELEASE_D3D11_OBJECTS_KHR"; 
					break;
				case CL_COMMAND_EGL_FENCE_SYNC_OBJECT_KHR:
					evt->final_name = "EGL_FENCE_SYNC_OBJECT_KHR"; 
					break;
				default:
					evt->final_name = NULL;
					g_warning("Unknown event command type"); 
					break;
			}
			 
		} else {
			/* Name is not NULL, use it as final name. */
			evt->final_name = evt->name;
		}
	}
	
	/* Return final name. */
	return evt->final_name;
}

cl_command_type ccl_event_get_command_type(
	CCLEvent* evt, GError** err) {
	
	/* The command type to return. */
	cl_command_type ct;
	
	/* Make sure evt wrapper object is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Determine the command type. */
	CCLWrapperInfo* info = 
		ccl_event_get_info(evt, CL_EVENT_COMMAND_TYPE, err);
	
	if (info == NULL) {
		/* Some error ocurred, return 0. */
		ct = 0;
	} else {
		/* Get the command type. */
		ct = *((cl_command_type*) info->value);
	} 
	
	/* Return the command type. */
	return ct;
}

/** 
 * @addtogroup EVENT_WAIT_LIST
 * @{
 */
 
cl_int ccl_event_wait(CCLEventWaitList evt_wait_lst, GError** err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	cl_int ocl_status;
	
	ocl_status = clWaitForEvents(
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst));
	gef_if_error_create_goto(*err, CCL_ERROR, 
		CL_SUCCESS != ocl_status, CCL_ERROR_OCL, error_handler, 
		"%s: error while waiting for events (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
		
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:
	
	/* Return event. */
	return ocl_status;

}

/** @} */

/** @} */

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
CCLEvent* ccl_event_new_wrap(cl_event event) {
	
	CCLEvent* evt = (CCLEvent*) ccl_wrapper_new(
		(void*) event, sizeof(CCLEvent));
	
	evt->name = NULL;
	evt->final_name = NULL;
	
	return evt;
		
}
