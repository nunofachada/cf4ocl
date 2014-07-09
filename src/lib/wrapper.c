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
 * @brief Functions for obtaining information about OpenCL entities
 * such as platforms, devices, contexts, queues, kernels, etc.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "wrapper.h"

#define CL4_WRAPPER_UNINIT() GINT_TO_POINTER(-1)

static GHashTable* wrappers = NULL;
static GMutex wrappers_mutex;

CL4Wrapper* cl4_wrapper_get(void* cl_object, cl4_wrapper_wrap wrap_fun) {

	CL4Wrapper* w;
	g_mutex_lock(&wrappers_mutex);
	if (wrappers == NULL) {
		wrappers = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, NULL);
	}
	if (!(w = g_hash_table_lookup(wrappers, cl_object))) {
		w = wrap_fun(cl_object);
		g_hash_table_insert(wrappers, cl_object, w);
	}
	g_mutex_unlock(&wrappers_mutex);
	return w;
}

/** 
 * @brief Initialize wrapper fields. 
 * 
 * @param wrapper Wrapper object.
 * */
void cl4_wrapper_init(CL4Wrapper* wrapper) {
	
	wrapper->cl_object = CL4_WRAPPER_UNINIT();
	wrapper->info = NULL;
	
	wrapper->ref_count = 1;
	
}

/** 
 * @brief Increase the reference count of the wrapper object.
 * 
 * @param wrapper The wrapper object. 
 * */
void cl4_wrapper_ref(CL4Wrapper* wrapper) {
	
	/* Make sure wrapper object is not NULL. */
	g_return_if_fail(wrapper != NULL);
	
	/* Increment wrapper reference count. */
	g_atomic_int_inc(&wrapper->ref_count);
	
}

/** 
 * @brief Decrements the reference count of the wrapper object.
 * If it reaches 0, the wrapper object is destroyed.
 *
 * @param wrapper The wrapper object. 
 * @return The OpenCL wrapped object if the wrapper object was 
 * effectively destroyed due to its reference count becoming zero, or 
 * NULL otherwie.
 * */
gpointer cl4_wrapper_unref(CL4Wrapper* wrapper) {
	
	/* Make sure wrapper object is not NULL. */
	g_return_val_if_fail(wrapper != NULL, FALSE);
	
	/* OpenCL wrapped object, NULL by default. */
	gpointer cl_object = NULL;

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&wrapper->ref_count)) {

		/* Keep reference to the OpenCL wrapped object, so we can
		 * return it to caller. */
		cl_object = wrapper->cl_object;
		
		/* Destroy hash table containing information. */
		if (wrapper->info) {
			g_hash_table_destroy(wrapper->info);
		}
		
		/* Remove wrapper from static table. */
		g_mutex_lock(&wrappers_mutex);
		g_hash_table_remove(wrappers, cl_object);
		g_mutex_unlock(&wrappers_mutex);
		
	}
	
	/* Return OpenCL wrapped object if wrapper was destroyed. */
	return cl_object;

}

/**
 * @brief Returns the wrapper object reference count. For debugging and 
 * testing purposes only.
 * 
 * @param wrapper The wrapper object.
 * @return The wrapper object reference count or -1 if device is NULL.
 * */
gint cl4_wrapper_ref_count(CL4Wrapper* wrapper) {
	
	/* Make sure wrapper is not NULL. */
	g_return_val_if_fail(wrapper != NULL, -1);
	
	/* Return reference count. */
	return wrapper->ref_count;

}

/**
 * @brief Get the wrapped OpenCL object.
 * 
 * @param wrapper The wrapper object.
 * @return The wrapped OpenCL object.
 * */
void* cl4_wrapper_unwrap(CL4Wrapper* wrapper) {

	/* Make sure wrapper is not NULL. */
	g_return_val_if_fail(wrapper != NULL, NULL);
	
	/* Return the OpenCL wrapped object. */
	return wrapper->cl_object;
}

/** 
 * @brief Release an OpenCL object using the provided function if it's
 * safe to do so (i.e. if the object has been initialized). 
 * 
 * @param cl_object OpenCL object to be released.
 * @param cl_release_function Function to release OpenCL object.
 * @return CL_SUCCESS if (i) object was successfully released, or 
 * (ii) if the object wasn't initialized; or an OpenCL error code 
 * provided by the release function if there was an error releasing the 
 * object.
 * */
cl_int cl4_wrapper_release_cl_object(gpointer cl_object, 
	cl4_wrapper_release_function cl_release_function) {

	if ((cl_object != CL4_WRAPPER_UNINIT()) && (cl_object != NULL))
		return cl_release_function(cl_object);
	else
		return CL_SUCCESS;

}

/**
 * @brief Create a new CL4WrapperInfo* object with a given value size.
 * 
 * @param size Parameter size in bytes.
 * @return A new CL4WrapperInfo* object.
 * */
CL4WrapperInfo* cl4_wrapper_info_new(gsize size) {
	
	CL4WrapperInfo* info = g_slice_new(CL4WrapperInfo);
	
	if (size > 0) 
		info->value = g_slice_alloc0(size);
	else
		info->value = NULL;
	info->size = size;
	
	return info;
	
}

/**
 * @brief Destroy a ::CL4WrapperInfo object.
 * 
 * @param info Object to destroy.
 * */
void cl4_wrapper_info_destroy(CL4WrapperInfo* info) {
		
	/* Make sure info is not NULL. */
	g_return_if_fail(info != NULL);

	if (info->size > 0)
		g_slice_free1(info->size, info->value);
	g_slice_free(CL4WrapperInfo, info);
	
}

/**
 * @brief Get information about any wrapped OpenCL object.
 * 
 * This function should not be called directly, but using the
 * cl4_*_info() macros instead.
 * 
 * @param wrapper1 The wrapper object to query.
 * @param wrapper2 A second wrapper object, required in some queries.
 * @param param_name Name of information/parameter to get.
 * @param info_fun OpenCL clGet*Info function.
 * @param use_cache TRUE if cached information is to be used, FALSE to 
 * force a new query even if information is in cache.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested information object. This object will
 * be automatically freed when the respective wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
CL4WrapperInfo* cl4_wrapper_get_info(CL4Wrapper* wrapper1, 
	CL4Wrapper* wrapper2, cl_uint param_name, 
	cl4_wrapper_info_fp info_fun, cl_bool use_cache, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);
	
	/* Make sure wrapper1 is not NULL. */
	g_return_val_if_fail(wrapper1 != NULL, NULL);
	
	/* Information object. */
	CL4WrapperInfo* info = NULL;
	
	/* If information table is not yet initialized, then
	 * initialize it. */
	if (!wrapper1->info) {
		wrapper1->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal,
			NULL, (GDestroyNotify) cl4_wrapper_info_destroy);
	}
	
	/* If cache is not to be used... */
	if (!use_cache) {
		/* ...force removal. */
		g_hash_table_remove(
			wrapper1->info, GUINT_TO_POINTER(param_name));
	}
	
	/* Check if requested information is already present in the
	 * information table. */
	if (g_hash_table_contains(
		wrapper1->info, GUINT_TO_POINTER(param_name))) {
		
		/* If so, retrieve it from there. */
		info = g_hash_table_lookup(
			wrapper1->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		/* Otherwise, get it from OpenCL device.*/
		cl_int ocl_status;
		/* Size of device information in bytes. */
		gsize size_ret = 0;
		
		/* Get size of information. */
		ocl_status = (wrapper2 == NULL)
			? ((cl4_wrapper_info_fp1) info_fun)(wrapper1->cl_object, 
				param_name, 0, NULL, &size_ret)
			: ((cl4_wrapper_info_fp2) info_fun)(wrapper1->cl_object, 
				wrapper2->cl_object, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR,
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler,
			"Function '%s': get info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		gef_if_error_create_goto(*err, CL4_ERROR,
			size_ret == 0, CL4_ERROR_OCL, error_handler,
			"Function '%s': get info [size] (size is 0).",
			__func__);
		
		/* Allocate memory for information. */
		info = cl4_wrapper_info_new(size_ret);
		
		/* Get information. */
		ocl_status = (wrapper2 == NULL)
			? ((cl4_wrapper_info_fp1) info_fun)(wrapper1->cl_object, 
				param_name, size_ret, info->value, NULL)
			: ((cl4_wrapper_info_fp2) info_fun)(wrapper1->cl_object, 
				wrapper2->cl_object, param_name, size_ret, info->value, 
				NULL);
		gef_if_error_create_goto(*err, CL4_ERROR,
			CL_SUCCESS != ocl_status, CL4_ERROR_OCL, error_handler,
			"Function '%s': get context info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Keep information in information table. */
		g_hash_table_insert(wrapper1->info,
			GUINT_TO_POINTER(param_name),
			info);
		
	}
	
	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	info = NULL;
	
finish:
	
	/* Return the requested information. */
	return info;
}

/** 
 * @brief Get pointer to information value.
 * 
 * @param wrapper1 The wrapper object to query.
 * @param wrapper2 A second wrapper object, required in some queries.
 * @param param_name Name of information/parameter to get value of.
 * @param info_fun OpenCL clGet*Info function.
 * @param use_cache TRUE if cached information is to be used, FALSE to 
 * force a new query even if information is in cache.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return A pointer to the requested information value. This 
 * value will be automatically freed when the wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
gpointer cl4_wrapper_get_info_value(CL4Wrapper* wrapper1, 
	CL4Wrapper* wrapper2, cl_uint param_name, 
	cl4_wrapper_info_fp info_fun, cl_bool use_cache, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure wrapper1 is not NULL. */
	g_return_val_if_fail(wrapper1 != NULL, NULL);
	
	/* Get information object. */
	CL4WrapperInfo* diw = cl4_wrapper_get_info(wrapper1, wrapper2, 
		param_name, info_fun, use_cache, err);
	
	/* Return value if information object is not NULL. */	
	return diw != NULL ? diw->value : NULL;
}

/** 
 * @brief Get information size.
 * 
 * @param wrapper1 The wrapper object to query.
 * @param wrapper2 A second wrapper object, required in some queries.
 * @param param_name Name of information/parameter to get value of.
 * @param info_fun OpenCL clGet*Info function.
 * @param use_cache TRUE if cached information is to be used, FALSE to 
 * force a new query even if information is in cache.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested information size. If an error occurs, 
 * a size of 0 is returned.
 * */
gsize cl4_wrapper_get_info_size(CL4Wrapper* wrapper1, 
	CL4Wrapper* wrapper2, cl_uint param_name, 
	cl4_wrapper_info_fp info_fun, cl_bool use_cache, GError** err) {
	
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure wrapper1 is not NULL. */
	g_return_val_if_fail(wrapper1 != NULL, NULL);
	
	/* Get information object. */
	CL4WrapperInfo* diw = cl4_wrapper_get_info(wrapper1, wrapper2, 
		param_name, info_fun, use_cache, err);
	
	/* Return value if information object is not NULL. */	
	return diw != NULL ? diw->size : 0;
}
