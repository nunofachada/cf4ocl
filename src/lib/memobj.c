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
 
 #include "memobj.h"
 
 /** 
 * @file
 * @brief OpenCL cl_mem wrapper object.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

/** 
 * @brief Create a ::CL4MemObj wrapper object by wrapping a given
 * OpenCL cl_mem object. 
 * 
 * @param mem_object OpenCL cl_mem object to wrap.
 * @return ::CL4MemObj wrapper object.
 * */
CL4MemObj* cl4_memobj_new(cl_mem mem_object) {

	/* The cl_mem wrapper object. */
	CL4MemObj* mo;
		
	/* Allocate memory for the kernel wrapper object. */
	mo = g_slice_new(CL4MemObj);
	
	/* Initialize parent object. */
	cl4_wrapper_init(mo);
	
	/* Set the OpenCL cl_mem object. */
	mo->cl_object = mem_object;
	
	/* Return the new cl_mem wrapper object. */
	return mo;

}

/** 
 * @brief Decrements the reference count of the cl_mem wrapper 
 * object. If it reaches 0, the cl_mem wrapper object is destroyed.
 *
 * @param mo The cl_mem wrapper object.
 * */
void cl4_memobj_destroy(CL4MemObj* mo) {
	
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

		/* Release mo. */
		g_slice_free(CL4MemObj, mo);
		
		/* Release OpenCL memory object, ignore possible errors. */
		cl4_wrapper_release_cl_object(mem_object, 
			(cl4_wrapper_release_function) clReleaseMemObject);
		
	}

}

CL4Event* cl4_memobj_unmap(CL4MemObj* mo, CL4CQueue* cq, 
	void* mapped_ptr, CL4EventWaitList evt_wait_lst, GError** err) {

	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure mo is not NULL. */
	g_return_val_if_fail(mo != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;

	/* OpenCL event. */
	cl_event event;
	/* Event wrapper. */
	CL4Event* evt;
	
	ocl_status = clEnqueueUnmapMemObject (cl4_cqueue_unwrap(cq),
		cl4_memobj_unwrap(mo), mapped_ptr, 
		cl4_event_wait_list_get_num_events(evt_wait_lst),
		cl4_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"Function '%s': unable to unmap memory object (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = cl4_cqueue_produce_event(cq, event);
	
	/* Clear event wait list. */
	cl4_event_wait_list_clear(evt_wait_lst);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:

	/* Return evt. */
	return evt;


}
	
#ifdef CL_VERSION_1_2

CL4Event* cl4_memobj_migrate(CL4MemObj** mos, cl_uint num_mos,
 	CL4CQueue* cq, cl_mem_migration_flags flags, 
 	CL4EventWaitList evt_wait_lst, GError** err) {
		
	/* Make sure cq is not NULL. */
	g_return_val_if_fail(cq != NULL, NULL);
	/* Make sure mos is not NULL. */
	g_return_val_if_fail(mos != NULL, NULL);
	/* Make sure num_mos > 0 is not NULL. */
	g_return_val_if_fail(num_mos > 0, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);
	
	cl_int ocl_status;

	/* OpenCL event. */
	cl_event event;
	/* Event wrapper. */
	CL4Event* evt;
	
	cl_mem mem_objects[num_mos];
	for (cl_uint i = 0; i < num_mos; ++i) {
		mem_objects[i] = cl4_memobj_unwrap(mos[i]);
	}
	
	ocl_status = clEnqueueMigrateMemObjects(cl4_cqueue_unwrap(cq),
		num_mos, (const cl_mem*) mem_objects, flags,
		cl4_event_wait_list_get_num_events(evt_wait_lst),
		cl4_event_wait_list_get_clevents(evt_wait_lst), &event);		
	gef_if_error_create_goto(*err, CL4_ERROR, CL_SUCCESS != ocl_status, 
		CL4_ERROR_OCL, error_handler, 
		"Function '%s': unable to migrate memory objects (OpenCL error %d: %s).",
		__func__, ocl_status, cl4_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = cl4_cqueue_produce_event(cq, event);
	
	/* Clear event wait list. */
	cl4_event_wait_list_clear(evt_wait_lst);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	
	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:

	/* Return evt. */
	return evt;		

}

#endif

