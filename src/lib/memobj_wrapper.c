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
 
 #include "memobj_wrapper.h"
 
 /** 
 * @file
 * @brief OpenCL cl_mem wrapper object.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

/**
 * @brief Implementation of ccl_wrapper_release_fields() function for
 * ::CCLMemObj wrapper objects.
 * 
 * @protected @memberof ccl_memobj
 * 
 * @param[in] mo A ::CCLMemObj wrapper object.
 * */
void ccl_memobj_release_fields(CCLMemObj* mo) {

	/* Make sure mo wrapper object is not NULL. */
	g_return_if_fail(mo != NULL);

	/* Reduce reference count of memory object context wrapper. */
	if (mo->ctx != NULL)
		ccl_context_unref(mo->ctx);

}

/** 
 * @addtogroup MEMOBJ_WRAPPER
 * @{
 */

CCLEvent* ccl_memobj_enqueue_unmap(CCLMemObj* mo, CCLQueue* cq, 
	void* mapped_ptr, CCLEventWaitList evt_wait_lst, GError** err) {

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
	CCLEvent* evt;
	
	ocl_status = clEnqueueUnmapMemObject (ccl_queue_unwrap(cq),
		ccl_memobj_unwrap(mo), mapped_ptr, 
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);
	gef_if_err_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to unmap memory object (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:

	/* Return evt. */
	return evt;


}
	
#ifdef CL_VERSION_1_2

CCLEvent* ccl_memobj_enqueue_migrate(CCLMemObj** mos, cl_uint num_mos,
 	CCLQueue* cq, cl_mem_migration_flags flags, 
 	CCLEventWaitList evt_wait_lst, GError** err) {
		
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
	CCLEvent* evt;
	
	cl_mem mem_objects[num_mos];
	for (cl_uint i = 0; i < num_mos; ++i) {
		mem_objects[i] = ccl_memobj_unwrap(mos[i]);
	}
	
	ocl_status = clEnqueueMigrateMemObjects(ccl_queue_unwrap(cq),
		num_mos, (const cl_mem*) mem_objects, flags,
		ccl_event_wait_list_get_num_events(evt_wait_lst),
		ccl_event_wait_list_get_clevents(evt_wait_lst), &event);		
	gef_if_err_create_goto(*err, CCL_ERROR, CL_SUCCESS != ocl_status, 
		CCL_ERROR_OCL, error_handler, 
		"%s: unable to migrate memory objects (OpenCL error %d: %s).",
		G_STRLOC, ocl_status, ccl_err(ocl_status));
	
	/* Wrap event and associate it with the respective command queue. 
	 * The event object will be released automatically when the command
	 * queue is released. */
	evt = ccl_queue_produce_event(cq, event);
	
	/* Clear event wait list. */
	ccl_event_wait_list_clear(evt_wait_lst);
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	
	/* An error occurred, return NULL to signal it. */
	evt = NULL;
	
finish:

	/* Return evt. */
	return evt;		

}

#endif


/** @} */
