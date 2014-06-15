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
 * @brief Functions for querying OpenCL devices.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "devquery.h"

static gchar* cl4_devquery_format_uint(gpointer info, gchar* out, guint size) {
	g_snprintf(out, size, "%d", *((cl_uint*) info));
	return out;
}

static gchar* cl4_devquery_format_yesno(gpointer info, gchar* out, guint size) {
	g_snprintf(out, size, "%s", *((cl_bool*) info) ? "Yes" : "No");
	return out;
}

/* Size of information map. */
static const gint info_map_size = 74;

/* Map of strings to respective cl_device_info bitfields. */
static const CL4DevQueryMap info_map[] = {

	{"address_bits", CL_DEVICE_ADDRESS_BITS, "Address space size (bits)", cl4_devquery_format_uint},
	{"available", CL_DEVICE_AVAILABLE, "Device available", cl4_devquery_format_yesno},
	{"built_in_kernels", CL_DEVICE_BUILT_IN_KERNELS, "Built-in kernels", cl4_devquery_format_uint},
	{"compiler_available", CL_DEVICE_COMPILER_AVAILABLE, "Compiler available", cl4_devquery_format_yesno},
	{"double_fp_config", CL_DEVICE_DOUBLE_FP_CONFIG, "Floating-point config. (double)", cl4_devquery_format_uint},
	{"driver_version", CL_DRIVER_VERSION, "Driver version", cl4_devquery_format_uint},
	{"endian_little", CL_DEVICE_ENDIAN_LITTLE, "Little endian", cl4_devquery_format_yesno},
	{"error_correction_support", CL_DEVICE_ERROR_CORRECTION_SUPPORT, "Error correction support", cl4_devquery_format_yesno},
	{"execution_capabilities", CL_DEVICE_EXECUTION_CAPABILITIES, "Execution capabilities", cl4_devquery_format_uint},
	{"extensions", CL_DEVICE_EXTENSIONS, "Extensions", cl4_devquery_format_uint},
	{"global_mem_cache_size", CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, "Global mem. cache size", cl4_devquery_format_uint},
	{"global_mem_cache_type", CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, "Global mem. cache type", cl4_devquery_format_uint},
	{"global_mem_cacheline_size", CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "Global mem. cache line size", cl4_devquery_format_uint},
	{"global_mem_size", CL_DEVICE_GLOBAL_MEM_SIZE, "Global mem. size", cl4_devquery_format_uint},
	{"half_fp_config", CL_DEVICE_HALF_FP_CONFIG, "Floating-point config (half)", cl4_devquery_format_uint},
	{"host_unified_memory", CL_DEVICE_HOST_UNIFIED_MEMORY, "Host unified memory subsystem", cl4_devquery_format_yesno},
	{"image_support", CL_DEVICE_IMAGE_SUPPORT, "Image support", cl4_devquery_format_yesno},
	{"image2d_max_height", CL_DEVICE_IMAGE2D_MAX_HEIGHT, "Max. height of 2D image (pixels)", cl4_devquery_format_uint},
	{"image2d_max_width", CL_DEVICE_IMAGE2D_MAX_WIDTH, "Max. width of 1D/2D image (pixels)", cl4_devquery_format_uint},
	{"image3d_max_depth", CL_DEVICE_IMAGE3D_MAX_DEPTH, "Max. depth of 3D image (pixels)", cl4_devquery_format_uint},
	{"image3d_max_height", CL_DEVICE_IMAGE3D_MAX_HEIGHT, "Max. height of 3D image (pixels)", cl4_devquery_format_uint},
	{"image3d_max_width", CL_DEVICE_IMAGE3D_MAX_WIDTH, "Max. width of 3D image (pixels)", cl4_devquery_format_uint},
	{"image_max_buffer_size", CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, "Max. pixels for 1D image from buffer object", cl4_devquery_format_uint},
	{"image_max_array_size", CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, "Max. images in a 1D or 2D image array", cl4_devquery_format_uint},
	{"linker_available", CL_DEVICE_LINKER_AVAILABLE, "Linker available", cl4_devquery_format_yesno},
	{"local_mem_size", CL_DEVICE_LOCAL_MEM_SIZE, "Local mem. size", cl4_devquery_format_uint},
	{"local_mem_type", CL_DEVICE_LOCAL_MEM_TYPE, "Local mem. type", cl4_devquery_format_uint},
	{"max_clock_frequency", CL_DEVICE_MAX_CLOCK_FREQUENCY, "Max. clock frequency", cl4_devquery_format_uint},
	{"max_compute_units", CL_DEVICE_MAX_COMPUTE_UNITS, "Compute units", cl4_devquery_format_uint},
	{"max_constant_args", CL_DEVICE_MAX_CONSTANT_ARGS, "", cl4_devquery_format_uint},
	{"max_constant_buffer_size", CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, "", cl4_devquery_format_uint},
	{"max_mem_alloc_size", CL_DEVICE_MAX_MEM_ALLOC_SIZE, "", cl4_devquery_format_uint},
	{"max_parameter_size", CL_DEVICE_MAX_PARAMETER_SIZE, "", cl4_devquery_format_uint},
	{"max_read_image_args", CL_DEVICE_MAX_READ_IMAGE_ARGS, "", cl4_devquery_format_uint},
	{"max_samplers", CL_DEVICE_MAX_SAMPLERS, "", cl4_devquery_format_uint},
	{"max_work_group_size", CL_DEVICE_MAX_WORK_GROUP_SIZE, "", cl4_devquery_format_uint},
	{"max_work_item_dimensions", CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "", cl4_devquery_format_uint},
	{"max_work_item_sizes", CL_DEVICE_MAX_WORK_ITEM_SIZES, "", cl4_devquery_format_uint},
	{"max_write_image_args", CL_DEVICE_MAX_WRITE_IMAGE_ARGS, "", cl4_devquery_format_uint},
	{"mem_base_addr_align", CL_DEVICE_MEM_BASE_ADDR_ALIGN, "", cl4_devquery_format_uint},
	{"min_data_type_align_size", CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "", cl4_devquery_format_uint},
	{"name", CL_DEVICE_NAME, "", cl4_devquery_format_uint},
	{"native_vector_width_char", CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, "", cl4_devquery_format_uint},
	{"native_vector_width_short", CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, "", cl4_devquery_format_uint},
	{"native_vector_width_int", CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, "", cl4_devquery_format_uint},
	{"native_vector_width_long", CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, "", cl4_devquery_format_uint},
	{"native_vector_width_float", CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, "", cl4_devquery_format_uint},
	{"native_vector_width_double", CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, "", cl4_devquery_format_uint},
	{"native_vector_width_half", CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, "", cl4_devquery_format_uint},
	{"opencl_c_version", CL_DEVICE_OPENCL_C_VERSION, "", cl4_devquery_format_uint},
	{"parent_device", CL_DEVICE_PARENT_DEVICE, "", cl4_devquery_format_uint},
	{"partition_max_sub_devices", CL_DEVICE_PARTITION_MAX_SUB_DEVICES, "", cl4_devquery_format_uint},
	{"partition_properties", CL_DEVICE_PARTITION_PROPERTIES, "", cl4_devquery_format_uint},
	{"partition_affinity_domain", CL_DEVICE_PARTITION_AFFINITY_DOMAIN, "", cl4_devquery_format_uint},
	{"partition_type", CL_DEVICE_PARTITION_TYPE, "", cl4_devquery_format_uint},
	{"platform", CL_DEVICE_PLATFORM, "", cl4_devquery_format_uint},
	{"preferred_interop_user_sync", CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, "Preferrable for user to sync. mem. w/ APIs", cl4_devquery_format_yesno},
	{"preferred_vector_width_char", CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, "", cl4_devquery_format_uint},
	{"preferred_vector_width_short", CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, "", cl4_devquery_format_uint},
	{"preferred_vector_width_int", CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, "", cl4_devquery_format_uint},
	{"preferred_vector_width_long", CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, "", cl4_devquery_format_uint},
	{"preferred_vector_width_float", CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, "", cl4_devquery_format_uint},
	{"preferred_vector_width_double", CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, "", cl4_devquery_format_uint},
	{"preferred_vector_width_half", CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, "", cl4_devquery_format_uint},
	{"printf_buffer_size", CL_DEVICE_PRINTF_BUFFER_SIZE, "", cl4_devquery_format_uint},
	{"profile", CL_DEVICE_PROFILE, "", cl4_devquery_format_uint},
	{"profiling_timer_resolution", CL_DEVICE_PROFILING_TIMER_RESOLUTION, "", cl4_devquery_format_uint},
	{"queue_properties", CL_DEVICE_QUEUE_PROPERTIES, "", cl4_devquery_format_uint},
	{"reference_count", CL_DEVICE_REFERENCE_COUNT, "", cl4_devquery_format_uint},
	{"single_fp_config", CL_DEVICE_SINGLE_FP_CONFIG, "", cl4_devquery_format_uint},
	{"type", CL_DEVICE_TYPE, "", cl4_devquery_format_uint},
	{"vendor", CL_DEVICE_VENDOR, "", cl4_devquery_format_uint},
	{"vendor_id", CL_DEVICE_VENDOR_ID, "", cl4_devquery_format_uint},
	{"version", CL_DEVICE_VERSION, "", cl4_devquery_format_uint}
};

/**
 * @brief Get a final device info prefix in the same format as 
 * kept in the info_map.
 * 
 * @param prefix Raw device information prefix. Several forms are 
 * accepted. For example, for CL_DEVICE_ENDIAN_LITTLE, strings such as
 * "CL_DEVICE_ENDIAN_LITTLE", "ENDIAN_LITTLE" or "endian_little" are
 * accepted.
 * @return A final device info prefix in the same format as 
 * kept in the info_map.
 * */
static gchar* cl4_devquery_get_prefix_final(gchar* prefix) {
	
	/* Make sure prefix is not NULL. */
	g_return_val_if_fail(prefix != NULL, 0);
	
	/* Auxiliary string variables. */
	gchar* str_lower;
	gchar* str_aux;
	gchar* str_final;
	
	/* Make string lower-case. */
	str_lower = g_ascii_strdown(prefix, -1);
	
	/* Remove possible cl_device or cl_ prefix */
	if (g_str_has_prefix(str_lower, "cl_device_")) {
		str_aux = str_lower + strlen("cl_device_");
	} else if (g_str_has_prefix(str_lower, "cl_")) {
		str_aux = str_lower + strlen("cl_");
	} else {
		str_aux = str_lower;
	}
	
	/* Create a new string for returning. */
	str_final = g_strdup(str_aux);

	/* Free lowercase string. */
	g_free(str_lower);
	
	/* Return result */
	return str_final;
	
}

/**
 * @brief Return the index of the device information map object of the
 * given parameter name.
 * 
 * @param A parameter name, in the format stored in the info_map static
 * array.
 * @return Index of the device information map object of the given 
 * parameter name, or -1 if device information map is not found.
 * */
static gint cl4_devquery_get_index(gchar* name) {

	/* Make sure name is not NULL. */
	g_return_val_if_fail(name != NULL, 0);
	
	/* Search indexes. */
	gint idx_middle;
	guint idx_start, idx_end;
	
	/* String comparison result. */
	gint cmp_res;
	
	/* Found info flag. */
	gboolean found = FALSE;
	
	/* Size of name. */
	gint len_name = strlen(name);

	/* Binary search. */
	idx_start = 0;
	idx_end = info_map_size - 1;
	while (idx_end - idx_start > 1) { /// @todo This is a bug, we can't see address_bits like this
		idx_middle = (idx_start + idx_end) / 2;
		cmp_res = g_ascii_strncasecmp(
			name, info_map[idx_middle].param_name, len_name);
		if (cmp_res == 0) {
			found = TRUE;
			break;
		}
		if (cmp_res > 0)
			idx_start = idx_middle;
		else 
			idx_end = idx_middle;
	}
	
	/* Return result */
	return found ? idx_middle : -1;
}

/**
 * @brief Return a cl_device_info object given its name.
 * 
 * @param name Name of cl_device_info object. Several forms are 
 * accepted. For example, for CL_DEVICE_ENDIAN_LITTLE, strings such as
 * "CL_DEVICE_ENDIAN_LITTLE", "ENDIAN_LITTLE" or "endian_little" are
 * accepted.
 * @return A cl_device_info object given its name or 0 if no suitable
 * cl_device_info is found for the given name.
 * */
cl_device_info cl4_devquery_name(gchar* name) {

	/* Make sure name is not NULL. */
	g_return_val_if_fail(name != NULL, 0);

	/* Final name */
	gchar* name_final;
	
	/* Index of device info. */
	gint idx;
	
	/* Get final name. */
	name_final = cl4_devquery_get_prefix_final(name);
	
	/* Get index of cl_device_info given its final name. */
	idx = cl4_devquery_get_index(name_final);
	
	/* Release memory holding the final name. */
	g_free(name_final);
	
	/* Return the cl_device_info object if found, or 0 otherwise. */
	if (idx >= 0)
		return info_map[idx].device_info;
	else
		return 0;
	
}

/**
 * @brief Get a list of device information parameters which have the 
 * given prefix.
 * 
 * @param prefix Device information parameter prefix. Can be in lower
 * or uppercase, and start with "cl_device_" or not.
 * @param size Size of returned list.
 * @return List of device information parameters which have the given
 * prefix.
 * */
const CL4DevQueryMap* cl4_devquery_list_prefix(
	gchar* prefix, gint* size) {
	
	/* Make sure prefix is not NULL. */
	g_return_val_if_fail(prefix != NULL, 0);
	
	/* Final prefix to search for. */
	gchar* prefix_final;
	
	/* Size of final prefix. */
	gint len_prefix_final;
	
	/* Search index. */
	gint idx_middle, idx_start, idx_end;
	
	/* Found info. */
	const CL4DevQueryMap* found_info_map = NULL;
	
	/* Determine final prefix according to how parameter names are
	 * stored in info_map. */
	prefix_final = cl4_devquery_get_prefix_final(prefix);
	
	/* Determine prefix size. */
	len_prefix_final = strlen(prefix_final);
	
	/* Binary search. */
	idx_middle = cl4_devquery_get_index(prefix_final);
	
	if (idx_middle >= 0) {
		/* Search for beginning. */
		idx_start = idx_middle;
		while (TRUE) {
			if (idx_start == 0)
				break;
			if (g_ascii_strncasecmp(prefix_final, 
				info_map[idx_start - 1].param_name, len_prefix_final) == 0)
				idx_start--;
			else
				break;
		}
		/* Search for ending. */
		idx_end = idx_middle;
		while (TRUE) {
			if (idx_end == info_map_size - 1)
				break;
			if (g_ascii_strncasecmp(prefix_final, 
				info_map[idx_end + 1].param_name, len_prefix_final) == 0)
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

	/* Free final prefix. */
	g_free(prefix_final);
	
	/* Return result */
	return found_info_map;
}
