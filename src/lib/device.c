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

	/** OpenCL device ID. */
	cl_device_id id;
	/** Device information. */
	GHashTable* info;
	/** Reference count. */
	gint ref_count;    
	
};

/* Size of information map. */
static const gint info_map_size = 74;

/* Map of strings to respective cl_device_info bitfields. */
static const CL4DeviceInfoMap info_map[] = {

	{"address_bits", CL_DEVICE_ADDRESS_BITS},
	{"available", CL_DEVICE_AVAILABLE},
	{"built_in_kernels", CL_DEVICE_BUILT_IN_KERNELS},
	{"compiler_available", CL_DEVICE_COMPILER_AVAILABLE},
	{"double_fp_config", CL_DEVICE_DOUBLE_FP_CONFIG},
	{"endian_little", CL_DEVICE_ENDIAN_LITTLE},
	{"error_correction_support", CL_DEVICE_ERROR_CORRECTION_SUPPORT},
	{"execution_capabilities", CL_DEVICE_EXECUTION_CAPABILITIES},
	{"extensions", CL_DEVICE_EXTENSIONS},
	{"global_mem_cache_size", CL_DEVICE_GLOBAL_MEM_CACHE_SIZE},
	{"global_mem_cache_type", CL_DEVICE_GLOBAL_MEM_CACHE_TYPE},
	{"global_mem_cacheline_size", CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE},
	{"global_mem_size", CL_DEVICE_GLOBAL_MEM_SIZE},
	{"half_fp_config", CL_DEVICE_HALF_FP_CONFIG},
	{"host_unified_memory", CL_DEVICE_HOST_UNIFIED_MEMORY},
	{"image_support", CL_DEVICE_IMAGE_SUPPORT},
	{"image2d_max_height", CL_DEVICE_IMAGE2D_MAX_HEIGHT},
	{"image2d_max_width", CL_DEVICE_IMAGE2D_MAX_WIDTH},
	{"image3d_max_depth", CL_DEVICE_IMAGE3D_MAX_DEPTH},
	{"image3d_max_height", CL_DEVICE_IMAGE3D_MAX_HEIGHT},
	{"image3d_max_width", CL_DEVICE_IMAGE3D_MAX_WIDTH},
	{"image_max_buffer_size", CL_DEVICE_IMAGE_MAX_BUFFER_SIZE},
	{"image_max_array_size", CL_DEVICE_IMAGE_MAX_ARRAY_SIZE},
	{"linker_available", CL_DEVICE_LINKER_AVAILABLE},
	{"local_mem_size", CL_DEVICE_LOCAL_MEM_SIZE},
	{"local_mem_type", CL_DEVICE_LOCAL_MEM_TYPE},
	{"max_clock_frequency", CL_DEVICE_MAX_CLOCK_FREQUENCY},
	{"max_compute_units", CL_DEVICE_MAX_COMPUTE_UNITS},
	{"max_constant_args", CL_DEVICE_MAX_CONSTANT_ARGS},
	{"max_constant_buffer_size", CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE},
	{"max_mem_alloc_size", CL_DEVICE_MAX_MEM_ALLOC_SIZE},
	{"max_parameter_size", CL_DEVICE_MAX_PARAMETER_SIZE},
	{"max_read_image_args", CL_DEVICE_MAX_READ_IMAGE_ARGS},
	{"max_samplers", CL_DEVICE_MAX_SAMPLERS},
	{"max_work_group_size", CL_DEVICE_MAX_WORK_GROUP_SIZE},
	{"max_work_item_dimensions", CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS},
	{"max_work_item_sizes", CL_DEVICE_MAX_WORK_ITEM_SIZES},
	{"max_write_image_args", CL_DEVICE_MAX_WRITE_IMAGE_ARGS},
	{"mem_base_addr_align", CL_DEVICE_MEM_BASE_ADDR_ALIGN},
	{"min_data_type_align_size", CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE},
	{"name", CL_DEVICE_NAME},
	{"native_vector_width_char", CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR},
	{"native_vector_width_short", CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT},
	{"native_vector_width_int", CL_DEVICE_NATIVE_VECTOR_WIDTH_INT},
	{"native_vector_width_long", CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG},
	{"native_vector_width_float", CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT},
	{"native_vector_width_double", CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE},
	{"native_vector_width_half", CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF},
	{"opencl_c_version", CL_DEVICE_OPENCL_C_VERSION},
	{"parent_device", CL_DEVICE_PARENT_DEVICE},
	{"partition_max_sub_devices", CL_DEVICE_PARTITION_MAX_SUB_DEVICES},
	{"partition_properties", CL_DEVICE_PARTITION_PROPERTIES},
	{"partition_affinity_domain", CL_DEVICE_PARTITION_AFFINITY_DOMAIN},
	{"partition_type", CL_DEVICE_PARTITION_TYPE},
	{"platform", CL_DEVICE_PLATFORM},
	{"preferred_vector_width_char", CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR},
	{"preferred_vector_width_short", CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT},
	{"preferred_vector_width_int", CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT},
	{"preferred_vector_width_long", CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG},
	{"preferred_vector_width_float", CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT},
	{"preferred_vector_width_double", CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE},
	{"preferred_vector_width_half", CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF},
	{"printf_buffer_size", CL_DEVICE_PRINTF_BUFFER_SIZE},
	{"preferred_interop_user_sync", CL_DEVICE_PREFERRED_INTEROP_USER_SYNC},
	{"profile", CL_DEVICE_PROFILE},
	{"profiling_timer_resolution", CL_DEVICE_PROFILING_TIMER_RESOLUTION},
	{"queue_properties", CL_DEVICE_QUEUE_PROPERTIES},
	{"reference_count", CL_DEVICE_REFERENCE_COUNT},
	{"single_fp_config", CL_DEVICE_SINGLE_FP_CONFIG},
	{"type", CL_DEVICE_TYPE},
	{"vendor", CL_DEVICE_VENDOR},
	{"vendor_id", CL_DEVICE_VENDOR_ID},
	{"version", CL_DEVICE_VERSION},
	{"driver_version", CL_DRIVER_VERSION}
};

const CL4DeviceInfoMap* cl4_device_str2infolist(gchar* str, gint* size) {
	
	/* Make sure str is not NULL. */
	g_return_val_if_fail(str != NULL, 0);
	
	/* Auxiliary string variables. */
	gchar* str_lower;
	gchar* str_final;
	gsize len_str_final;
	
	/* Binary search indexes. */
	guint idx_middle, idx_start, idx_end;
	
	/* String comparison result. */
	gint cmp_res;
	
	/* Found info flag. */
	gboolean found = FALSE;
	
	/* Found info. */
	const CL4DeviceInfoMap* found_info_map = NULL;
	
	/* Make string lower-case. */
	str_lower = g_ascii_strdown(str, -1);
	
	/* Remove possible cl_device or cl_ prefix */
	if (g_str_has_prefix(str_lower, "cl_device_")) {
		str_final = str_lower + strlen("cl_device_");
	} else if (g_str_has_prefix(str_lower, "cl_")) {
		str_final = str_lower + strlen("cl_");
	} else {
		str_final = str_lower;
	}
	len_str_final = strlen(str_final);
	
	/* Binary search. */
	idx_start = 0;
	idx_end = info_map_size - 1;
	while (idx_end - idx_start > 1) {
		idx_middle = (idx_start + idx_end) / 2;
		cmp_res = g_ascii_strncasecmp(
			str_final, info_map[idx_middle].param_name, len_str_final);
		if (cmp_res == 0) {
			found = TRUE;
			break;
		}
		if (cmp_res > 0)
			idx_start = idx_middle;
		else 
			idx_end = idx_middle;
	}
	if (found) {
		/* Search for beginning. */
		idx_start = idx_middle;
		while (TRUE) {
			if (idx_start == 0)
				break;
			if (g_ascii_strncasecmp(str_final, 
				info_map[idx_start - 1].param_name, len_str_final) == 0)
				idx_start--;
			else
				break;
		}
		/* Search for ending. */
		idx_end = idx_middle;
		while (TRUE) {
			if (idx_end == (guint) (info_map_size - 1))
				break;
			if (g_ascii_strncasecmp(str_final, 
				info_map[idx_end + 1].param_name, len_str_final) == 0)
				idx_end++;
			else
				break;
		}
		
		/* Set return values. */
		*size = idx_end - idx_start + 1;
		found_info_map = &info_map[idx_start];
		
	} else {
		
		/* Nothing found. */
		*size = -1;
		
	}

	/* Free string. */
	g_free(str_lower);
	
	/* Return result */
	return found_info_map;
}


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
	
	/* Set the device ID. */
	device->id = id;
		
	/* Device information will be lazy initialized when required. */
	device->info = NULL;

	/* Reference count is one initially. */
	device->ref_count = 1;

	/* Return the new device wrapper object. */
	return device;
	
}

/** 
 * @brief Increase the reference count of the device wrapper object.
 * 
 * @param device The device wrapper object. 
 * */
void cl4_device_ref(CL4Device* device) {
	
	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(device != NULL);
	
	/* Increase reference count. */
	g_atomic_int_inc(&device->ref_count);
	
}

/** 
 * @brief Alias for cl4_device_unref().
 *
 * @param device The device wrapper object. 
 * */
void cl4_device_destroy(CL4Device* device) {
	
	cl4_device_unref(device);

}

/** 
 * @brief Decrements the reference count of the device wrapper object.
 * If it reaches 0, the device wrapper object is destroyed.
 *
 * @param device The device wrapper object. 
 * */
void cl4_device_unref(CL4Device* device) {
	
	/* Make sure device wrapper object is not NULL. */
	g_return_if_fail(device != NULL);

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&device->ref_count)) {

		/* Destroy hash table containing device information. */
		if (device->info) {
			g_hash_table_destroy(device->info);
		}
		
		/* Free the device wrapper object. */
		g_slice_free(CL4Device, device);
	}	

}

/**
 * @brief Returns the device wrapper object reference count. For
 * debugging and testing purposes only.
 * 
 * @param device The device wrapper object.
 * @return The device wrapper object reference count or -1 if device
 * is NULL.
 * */
gint cl4_device_ref_count(CL4Device* device) {
	
	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, -1);
	
	/* Return reference count. */
	return device->ref_count;

}

/**
 * @brief Get device information.
 * 
 * @param device The device wrapper object.
 * @param param_name Name of information/parameter to get.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The requested device information. This information will
 * be automatically freed when the device wrapper object is 
 * destroyed. If an error occurs, NULL is returned.
 * */
gpointer cl4_device_info(CL4Device* device, 
	cl_device_info param_name, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Device information placeholder. */
	gpointer param_value;
	
	/* If device information table is not yet initialized, then 
	 * initialize it. */
	if (!device->info) {
		device->info = g_hash_table_new_full(
			g_direct_hash, g_direct_equal, NULL, g_free);
	}

	/* Check if requested information is already present in the 
	 * device information table. */
	if (g_hash_table_contains(
		device->info, GUINT_TO_POINTER(param_name))) {
		
		/* If so, retrieve it from there. */
		param_value = g_hash_table_lookup(
			device->info, GUINT_TO_POINTER(param_name));
		
	} else {
		
		/* Otherwise, get it from OpenCL device.*/
		cl_int ocl_status;
		size_t size_ret;
		
		/* Get size of information. */
		ocl_status = clGetDeviceInfo(
			device->id, param_name, 0, NULL, &size_ret);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device info [size] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
		
		/* Allocate memory for information. */
		param_value = g_malloc(size_ret);
		
		/* Get information. */
		ocl_status = clGetDeviceInfo(
			device->id, param_name, size_ret, param_value, NULL);
		gef_if_error_create_goto(*err, CL4_ERROR, 
			CL_SUCCESS != ocl_status, CL4_OCL_ERROR, error_handler, 
			"Function '%s': get device info [info] (OpenCL error %d: %s).",
			__func__, ocl_status, cl4_err(ocl_status));
			
		/* Keep information in device information table. */
		g_hash_table_insert(
			device->info, GUINT_TO_POINTER(param_name), param_value);
		
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	param_value = NULL;

finish:
	
	/* Return the requested device information. */
	return param_value;

}

/**
 * @brief Get the OpenCL device ID object.
 * 
 * @param device The device wrapper object.
 * @return The OpenCL device ID object.
 * */
cl_device_id cl4_device_id(CL4Device* device) {

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, NULL);
	
	/* Return the OpenCL device ID. */
	return device->id;
}
