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
 
#include "device_query.h"

/** 
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting unsigned integers. 
 * */
static char* ccl_devquery_format_uint(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
		
	g_snprintf(out, size, "%d %s", *((cl_uint*) info->value), units);
	return out;
	
}

/** 
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting device information as a hexadecimal number. 
 * */
static char* ccl_devquery_format_hex(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
		
	GString* str = g_string_new("0x");
	gboolean start = FALSE;
	gchar val;
	
	for (gint i = info->size - 1; i >= 0 ; i--) {
		val = ((cl_char*) info->value)[i];
		if (val) start = TRUE;
		if (start)
			g_string_append_printf(str, "%.2x", val);
	}
	if (units && units[0])
		g_string_append_printf(str, " %s", units);
		
	g_snprintf(out, size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;
	
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting size_t unsigned integers. 
 * */
static char* ccl_devquery_format_sizet(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
		
	g_snprintf(out, size, "%ld %s", (gulong) *((size_t*) info->value), units);
	return out;
	
}

/**
 * @brief Helper macro for format functions outputting values in bytes.
 * 
 * @param spec A positive integer value representing bytes.
 * */
#define ccl_devquery_format_bytes(spec) \
	if (bytes < 1024) \
		g_snprintf(out, size, "%" spec " bytes", bytes); \
	else if (bytes < 1048576) \
		g_snprintf(out, size, "%.1lf KiB (%" spec " bytes)", \
			bytes / 1024.0, bytes); \
	else if (bytes < 1073741824) \
		g_snprintf(out, size, "%.1lf MiB (%" spec " bytes)", \
			bytes / (1024.0 * 1024), bytes); \
	else \
		g_snprintf(out, size, "%.1lf GiB (%" spec " bytes)", \
			bytes / (1024.0 * 1024 * 1024), bytes);

/**
 * @brief Implementation of ccl_devquery_format() function for
 * outputting unsigned long integers which represent bytes. 
 * */
static char* ccl_devquery_format_ulongbytes(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_ulong bytes = *((cl_ulong*) info->value);
	ccl_devquery_format_bytes("ld");
	return out;
	
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting unsigned integers which represent bytes. 
 * */
static char* ccl_devquery_format_uintbytes(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_uint bytes = *((cl_uint*) info->value);
	ccl_devquery_format_bytes("d");
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting unsigned size_t integers which represent bytes. 
 * */
static char* ccl_devquery_format_sizetbytes(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_ulong bytes = *((size_t*) info->value);
	ccl_devquery_format_bytes("ld");
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting a vector of size_t integers. 
 * */
static char* ccl_devquery_format_sizetvec(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	GString* str = g_string_new("(");
	size_t* vec = (size_t*) info->value;
	guint count = info->size / sizeof(gsize);

	for (guint i = 0; i < count; i++) {
		if (i > 0) g_string_append(str, ", ");
		g_string_append_printf(str, "%ld", (gulong) vec[i]);
	}
	
	g_string_append(str, ")");

	g_snprintf(out, size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting boolean values as a "Yes" or "No" string. 
 * */
static char* ccl_devquery_format_yesno(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	g_snprintf(out, size, "%s", *((cl_bool*) info->value) ? "Yes" : "No");
	return out;
	
}

/**
 * @brief Implementation of ccl_devquery_format() function for
 * outputting strings. 
 * */
static char* ccl_devquery_format_char(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	g_snprintf(out, size, "%s %s", (gchar*) info->value, units);
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting memory addresses. 
 * */
static char* ccl_devquery_format_ptr(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	g_snprintf(out, size, "%p", *((void**) info->value));
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for
 * outputting a string representing a device type. 
 * */
static char* ccl_devquery_format_type(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	g_snprintf(out, size, "%s", 
		ccl_devquery_type2str(*((cl_device_type*) info->value)));
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the device floating-point (FP) configuration for a FP 
 * type. */
static char* ccl_devquery_format_fpconfig(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_device_fp_config fpc = *((cl_device_fp_config*) info->value);
	g_snprintf(out, size, "%s%s%s%s%s%s%s", 
		fpc & CL_FP_DENORM ? "DENORM " : "",
		fpc & CL_FP_INF_NAN  ? "INF_NAN " : "",
		fpc & CL_FP_ROUND_TO_NEAREST ? "ROUND_TO_NEAREST " : "",
		fpc & CL_FP_ROUND_TO_ZERO ? "ROUND_TO_ZERO " : "",
		fpc & CL_FP_ROUND_TO_INF ? "ROUND_TO_INF " : "",
		fpc & CL_FP_FMA ? "FMA " : "",
		fpc & CL_FP_SOFT_FLOAT ? "SOFT_FLOAT" : "");
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the device execution capabilities. 
 * */
static char* ccl_devquery_format_execcap(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {

	units = units;
	cl_device_exec_capabilities exc = 
		*((cl_device_exec_capabilities*) info->value);
	g_snprintf(out, size, "%s%s", 
		exc & CL_EXEC_KERNEL ? "KERNEL " : "",
		exc & CL_EXEC_NATIVE_KERNEL ? "NATIVE_KERNEL " : "");
	return out;
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting a local memory type. */
static char* ccl_devquery_format_locmemtype(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_device_local_mem_type lmt = 
		*((cl_device_local_mem_type*) info->value);
	g_snprintf(out, size, "%s%s%s", 
		lmt & CL_LOCAL ? "LOCAL" : "",
		lmt & CL_GLOBAL ? "GLOBAL" : "",
		lmt & CL_NONE ? "NONE" : "");
	return out;
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the partition properties of a device. 
 * */
static char* ccl_devquery_format_partprop(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_device_partition_property* pp = 
		(cl_device_partition_property*) info->value;
	GString* str = g_string_new("");
	guint count = info->size / sizeof(cl_device_partition_property);
	for (guint i = 0; i < count; i++) {
		switch (pp[i]) {
			case CL_DEVICE_PARTITION_EQUALLY:
				g_string_append_printf(str, "EQUALLY ");
				break;
			case CL_DEVICE_PARTITION_BY_COUNTS:
				g_string_append_printf(str, "BY_COUNTS ");
				break;
			case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
				g_string_append_printf(str, "BY_AFFINITY_DOMAIN ");
				break;
			case CL_DEVICE_PARTITION_EQUALLY_EXT:
				g_string_append_printf(str, "EQUALLY_EXT ");
				break;
			case CL_DEVICE_PARTITION_BY_COUNTS_EXT:
				g_string_append_printf(str, "BY_COUNTS_EXT ");
				break;
			case CL_DEVICE_PARTITION_BY_NAMES_EXT:
				g_string_append_printf(str, "BY_NAMES_EXT ");
				break;
			case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT:
				g_string_append_printf(str, "BY_AFFINITY_DOMAIN_EXT ");
				break;
			default:
				g_string_append_printf(str, "UNKNOWN(0x%lx) ", pp[i]);
		}
	}
	g_snprintf(out, size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;
	
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the supported affinity domains for partitioning a device 
 * using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN. 
 * */
static char* ccl_devquery_format_affdom(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_device_affinity_domain ad = 
		*((cl_device_affinity_domain*) info->value);
	g_snprintf(out, size, "%s%s%s%s%s%s", 
		ad & CL_DEVICE_AFFINITY_DOMAIN_NUMA ? "NUMA " : "",
		ad & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE ? "L4_CACHE " : "",
		ad & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE ? "L3_CACHE " : "",
		ad & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE ? "L2_CACHE " : "",
		ad & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE ? "L1_CACHE " : "",
		ad & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE ? 
			"NEXT_PARTITIONABLE " : "");
	return out;

}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the cache type of a device. 
 * */
static char* ccl_devquery_format_cachetype(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_device_mem_cache_type mct = 
		*((cl_device_mem_cache_type*) info->value);
	g_snprintf(out, size, "%s%s%s", 
		mct & CL_READ_ONLY_CACHE ? "READ_ONLY" : "",
		mct & CL_READ_WRITE_CACHE ? "READ_WRITE" : "",
		mct & CL_NONE ? "NONE" : "");
	return out;
}

/**
 * @brief Implementation of ccl_devquery_format() function for 
 * outputting the queue properties of a device. 
 * */
static char* ccl_devquery_format_queueprop(CCLWrapperInfo* info, 
	char* out, size_t size, const char* units) {
	
	units = units;
	cl_command_queue_properties qp = 
		*((cl_command_queue_properties*) info->value);
	g_snprintf(out, size, "%s%s", 
		qp & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ? "OUT_OF_ORDER_EXEC_MODE_ENABLE " : "",
		qp & CL_QUEUE_PROFILING_ENABLE ? "PROFILING_ENABLE " : "");
	return out;

}

//~ /**
 //~ * @brief Implementation of ccl_devquery_format() function for debugging
 //~ * purposes. 
 //~ * */
//~ static char* ccl_devquery_format_testsize(CCLWrapperInfo* info, 
	//~ gchar* out, guint size, const gchar const* units) {
	//~ 
	//~ g_snprintf(out, size, "%ld %s", (gulong) info->size, units);
	//~ return out;
//~ 
//~ }

/** 
 * @addtogroup DEVICE_QUERY
 * @{
 */

/** 
 * @brief Size of parameter information map. 
 * */
const int ccl_devquery_info_map_size = 96;

/** 
 * @brief Map of parameter name strings to respective cl_device_info 
 * bitfields, long description string, format output function and a
 * units suffix. 
 * */
const CCLDevQueryMap ccl_devquery_info_map[] = {

	{"ADDRESS_BITS", CL_DEVICE_ADDRESS_BITS, 
		"Address space size in bits", 
		ccl_devquery_format_uint, "bits"},
	{"AVAILABLE", CL_DEVICE_AVAILABLE, 
		"Is device available", 
		ccl_devquery_format_yesno, ""},
	{"BOARD_NAME_AMD", CL_DEVICE_BOARD_NAME_AMD, 
		"Name of the GPU board and model of the specific device", 
		ccl_devquery_format_char, ""},
	{"BUILT_IN_KERNELS", CL_DEVICE_BUILT_IN_KERNELS, 
		"Device built-in kernels", 
		ccl_devquery_format_char, ""},
	{"COMPILER_AVAILABLE", CL_DEVICE_COMPILER_AVAILABLE, 
		"Is a compiler available for device", 
		ccl_devquery_format_yesno, ""},
	{"COMPUTE_CAPABILITY_MAJOR_NV", CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, 
		"Major revision number that defines the CUDA compute capability of the device", 
		ccl_devquery_format_uint, ""},
	{"COMPUTE_CAPABILITY_MINOR_NV", CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, 
		"Minor revision number that defines the CUDA compute capability of the device", 
		ccl_devquery_format_uint, ""},
	{"DOUBLE_FP_CONFIG", CL_DEVICE_DOUBLE_FP_CONFIG, 
		"Floating-point device configuration (double)", 
		ccl_devquery_format_fpconfig, ""},
	{"DRIVER_VERSION", CL_DRIVER_VERSION, 
		"Driver version", 
		ccl_devquery_format_char, ""},
	{"ENDIAN_LITTLE", CL_DEVICE_ENDIAN_LITTLE, 
		"Is device little endian", 
		ccl_devquery_format_yesno, ""},
	{"ERROR_CORRECTION_SUPPORT", CL_DEVICE_ERROR_CORRECTION_SUPPORT, 
		"Error correction support", 
		ccl_devquery_format_yesno, ""},
	{"EXECUTION_CAPABILITIES", CL_DEVICE_EXECUTION_CAPABILITIES, 
		"Execution capabilities", 
		ccl_devquery_format_execcap, ""},
	{"EXTENSIONS", CL_DEVICE_EXTENSIONS, 
		"Extensions", 
		ccl_devquery_format_char, ""},
	/* The parameter bellow has in fact a length of 16 bytes,
	 * but due to the lack of documentation, only the first half 
	 * of it is shown for now. */
	{"GLOBAL_FREE_MEMORY_AMD", CL_DEVICE_GLOBAL_FREE_MEMORY_AMD, 
		"Free device memory", 
		ccl_devquery_format_ulongbytes, ""},
	{"GLOBAL_MEM_CACHELINE_SIZE", CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, 
		"Global mem. cache line size", 
		ccl_devquery_format_uintbytes, ""},
	{"GLOBAL_MEM_CACHE_SIZE", CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, 
		"Global mem. cache size", 
		ccl_devquery_format_ulongbytes, ""},
	{"GLOBAL_MEM_CACHE_TYPE", CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, 
		"Global mem. cache type", 
		ccl_devquery_format_cachetype, ""},
	{"GLOBAL_MEM_CHANNELS_AMD", CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD, 
		"Global mem. channels", 
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_CHANNEL_BANKS_AMD", CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD, 
		"Global mem. channel banks", 
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD", CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD, 
		"Global mem. channel bank width", 
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_SIZE", CL_DEVICE_GLOBAL_MEM_SIZE, 
		"Global mem. size", 
		ccl_devquery_format_ulongbytes, ""},
	{"GPU_OVERLAP_NV", CL_DEVICE_GPU_OVERLAP_NV, 
		"Can device concurrently copy memory between host and device while executing a kernel", 
		ccl_devquery_format_yesno, ""},
	{"HALF_FP_CONFIG", CL_DEVICE_HALF_FP_CONFIG, 
		"Floating-point device configuration (half)", 
		ccl_devquery_format_fpconfig, ""},
	{"HOST_UNIFIED_MEMORY", CL_DEVICE_HOST_UNIFIED_MEMORY, 
		"Host unified memory subsystem", 
		ccl_devquery_format_yesno, ""},
	{"IMAGE2D_MAX_HEIGHT", CL_DEVICE_IMAGE2D_MAX_HEIGHT, 
		"Max. height of 2D image (pixels)", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE2D_MAX_WIDTH", CL_DEVICE_IMAGE2D_MAX_WIDTH, 
		"Max. width of 1D/2D image (pixels)", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE3D_MAX_DEPTH", CL_DEVICE_IMAGE3D_MAX_DEPTH, 
		"Max. depth of 3D image (pixels)", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE3D_MAX_HEIGHT", CL_DEVICE_IMAGE3D_MAX_HEIGHT, 
		"Max. height of 3D image (pixels)", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE3D_MAX_WIDTH", CL_DEVICE_IMAGE3D_MAX_WIDTH, 
		"Max. width of 3D image (pixels)", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE_MAX_ARRAY_SIZE", CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, 
		"Max. images in a 1D or 2D image array", 
		ccl_devquery_format_sizet, "images"},
	{"IMAGE_MAX_BUFFER_SIZE", CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, 
		"Max. pixels for 1D image from buffer object", 
		ccl_devquery_format_sizet, "px"},
	{"IMAGE_SUPPORT", CL_DEVICE_IMAGE_SUPPORT, 
		"Image support", 
		ccl_devquery_format_yesno, ""},
	{"INTEGRATED_MEMORY_NV", CL_DEVICE_INTEGRATED_MEMORY_NV, 
		"Is device integrated with the memory subsystem?", 
		ccl_devquery_format_yesno, ""},
	{"KERNEL_EXEC_TIMEOUT_NV", CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV, 
		"Is there a limit for kernels executed on device?", 
		ccl_devquery_format_yesno, ""},
	{"LINKER_AVAILABLE", CL_DEVICE_LINKER_AVAILABLE, 
		"Linker available", 
		ccl_devquery_format_yesno, ""},
	{"LOCAL_MEM_BANKS_AMD", CL_DEVICE_LOCAL_MEM_BANKS_AMD, 
		"Local mem. banks", 
		ccl_devquery_format_uint, ""},
	{"LOCAL_MEM_SIZE", CL_DEVICE_LOCAL_MEM_SIZE, 
		"Local mem. size", 
		ccl_devquery_format_ulongbytes, ""},
	{"LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD", CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD, 
		"Local mem. size per compute unit", 
		ccl_devquery_format_uintbytes, ""},
	{"LOCAL_MEM_TYPE", CL_DEVICE_LOCAL_MEM_TYPE, 
		"Local mem. type", 
		ccl_devquery_format_locmemtype, ""},
	{"MAX_ATOMIC_COUNTERS_EXT", CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT, 
		"Max. atomic counters", 
		ccl_devquery_format_uint, ""},
	{"MAX_CLOCK_FREQUENCY", CL_DEVICE_MAX_CLOCK_FREQUENCY, 
		"Max. clock frequency (MHz)", 
		ccl_devquery_format_uint, "MHz"},
	{"MAX_COMPUTE_UNITS", CL_DEVICE_MAX_COMPUTE_UNITS, 
		"Number of compute units in device", 
		ccl_devquery_format_uint, ""},
	{"MAX_CONSTANT_ARGS", CL_DEVICE_MAX_CONSTANT_ARGS, 
		"Max. number of __constant args in kernel", 
		ccl_devquery_format_uint, ""},
	{"MAX_CONSTANT_BUFFER_SIZE", CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, 
		"Max. size in bytes of a constant buffer allocation", 
		ccl_devquery_format_ulongbytes, ""},
	{"MAX_MEM_ALLOC_SIZE", CL_DEVICE_MAX_MEM_ALLOC_SIZE, 
		"Max. size of memory object allocation in bytes", 
		ccl_devquery_format_ulongbytes, ""},
	{"MAX_PARAMETER_SIZE", CL_DEVICE_MAX_PARAMETER_SIZE, 
		"Max. size in bytes of the arguments that can be passed to a kernel", 
		ccl_devquery_format_sizetbytes, ""},
	{"MAX_READ_IMAGE_ARGS", CL_DEVICE_MAX_READ_IMAGE_ARGS, 
		"Max. number of simultaneous image objects that can be read by a kernel", 
		ccl_devquery_format_uint, "images"},
	{"MAX_SAMPLERS", CL_DEVICE_MAX_SAMPLERS, 
		"Max. samplers that can be used in kernel", 
		ccl_devquery_format_uint, "samplers"},
	{"MAX_WORK_GROUP_SIZE", CL_DEVICE_MAX_WORK_GROUP_SIZE, 
		"Max. work-items in work-group executing a kernel on a single compute unit, using the data parallel execution model", 
		ccl_devquery_format_sizet, "work-items"},
	{"MAX_WORK_ITEM_DIMENSIONS", CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 
		"Max. dims that specify the global and local work-item IDs used by the data parallel execution model", 
		ccl_devquery_format_uint, ""},
	{"MAX_WORK_ITEM_SIZES", CL_DEVICE_MAX_WORK_ITEM_SIZES, 
		"Max. work-items in each dimension of work-group", 
		ccl_devquery_format_sizetvec, ""},
	{"MAX_WRITE_IMAGE_ARGS", CL_DEVICE_MAX_WRITE_IMAGE_ARGS, 
		"Max. simultaneous image objects that can be written to by a kernel", 
		ccl_devquery_format_uint, "images"},
	{"MEM_BASE_ADDR_ALIGN", CL_DEVICE_MEM_BASE_ADDR_ALIGN, 
		"Size in bits of the largest OpenCL built-in data type supported by the device", 
		ccl_devquery_format_uint, "bits"},
	{"MIN_DATA_TYPE_ALIGN_SIZE", CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, 
		"Smallest alignment which can be used for any data type (deprecated in OpenCL 1.2)", 
		ccl_devquery_format_uintbytes, ""},
	{"NAME", CL_DEVICE_NAME, 
		"Name of device", 
		ccl_devquery_format_char, ""},
	{"NATIVE_VECTOR_WIDTH_CHAR", CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, 
		"Native ISA char vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_DOUBLE", CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, 
		"Native ISA double vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_FLOAT", CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, 
		"Native ISA float vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_HALF", CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, 
		"Native ISA half vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_INT", CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, 
		"Native ISA int vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_LONG", CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, 
		"Native ISA long vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"NATIVE_VECTOR_WIDTH_SHORT", CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, 
		"Native ISA short vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"OPENCL_C_VERSION", CL_DEVICE_OPENCL_C_VERSION, 
		"Highest OpenCL C version supported by the device compiler", 
		ccl_devquery_format_char, ""},
	{"PARENT_DEVICE", CL_DEVICE_PARENT_DEVICE, 
		"The cl_device_id of the parent device to which the sub-device belongs", 
		ccl_devquery_format_ptr, ""},
	{"PARTITION_AFFINITY_DOMAIN", CL_DEVICE_PARTITION_AFFINITY_DOMAIN, 
		"Supported affinity domains for partitioning the device using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN", 
		ccl_devquery_format_affdom, ""},
	{"PARTITION_MAX_SUB_DEVICES", CL_DEVICE_PARTITION_MAX_SUB_DEVICES, 
		"Max. sub-devices that can be created when device is partitioned", 
		ccl_devquery_format_uint, "devices"},
	{"PARTITION_PROPERTIES", CL_DEVICE_PARTITION_PROPERTIES, 
		"Partition types supported by device", 
		ccl_devquery_format_partprop, ""},
	{"PARTITION_TYPE", CL_DEVICE_PARTITION_TYPE, 
		"Properties specified in clCreateSubDevices if device is a subdevice", 
		ccl_devquery_format_uint, ""},
	{"PLATFORM", CL_DEVICE_PLATFORM, 
		"The platform associated with device", 
		ccl_devquery_format_ptr, ""},
	{"PREFERRED_INTEROP_USER_SYNC", CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, 
		"'Yes' if device prefers user to be responsible for sync. when sharing memory objects between OpenCL and other APIs, 'No' if device has a performant path for performing such sync.", 
		ccl_devquery_format_yesno, ""},
	{"PREFERRED_VECTOR_WIDTH_CHAR", CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, 
		"Preferred ISA char vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_DOUBLE", CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, 
		"Preferred ISA double vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_FLOAT", CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, 
		"Preferred ISA float vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_HALF", CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, 
		"Preferred ISA half vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_INT", CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, 
		"Preferred ISA int vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_LONG", CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, 
		"Preferred ISA long vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PREFERRED_VECTOR_WIDTH_SHORT", CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, 
		"Preferred ISA short vector width (number of scalar elements that can be stored in the vector)", 
		ccl_devquery_format_uint, ""},
	{"PRINTF_BUFFER_SIZE", CL_DEVICE_PRINTF_BUFFER_SIZE, 
		"Max. size of internal buffer that holds the output of printf calls from kernel", 
		ccl_devquery_format_sizetbytes, ""},
	{"PROFILE", CL_DEVICE_PROFILE, 
		"Profile name supported by the device (FULL or EMBEDDED)", 
		ccl_devquery_format_char, ""},
	{"PROFILING_TIMER_OFFSET_AMD", CL_DEVICE_PROFILING_TIMER_OFFSET_AMD, 
		"Offset between event timestamps in nanoseconds", 
		ccl_devquery_format_sizet, "ns"},
	{"PROFILING_TIMER_RESOLUTION", CL_DEVICE_PROFILING_TIMER_RESOLUTION, 
		"Resolution of device timer in nanoseconds", 
		ccl_devquery_format_sizet, "ns"},
	{"QUEUE_PROPERTIES", CL_DEVICE_QUEUE_PROPERTIES, 
		"Command-queue properties supported by device", 
		ccl_devquery_format_queueprop, ""},
	{"REFERENCE_COUNT", CL_DEVICE_REFERENCE_COUNT, 
		"Device reference count", 
		ccl_devquery_format_uint, ""},
	{"REGISTERS_PER_BLOCK_NV", CL_DEVICE_REGISTERS_PER_BLOCK_NV, 
		"Maximum number of 32-bit registers available to a work-group", 
		ccl_devquery_format_uint, ""},
	{"SIMD_INSTRUCTION_WIDTH_AMD", CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD, 
		"SIMD instruction width", 
		ccl_devquery_format_uint, ""},
	{"SIMD_PER_COMPUTE_UNIT_AMD", CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD, 
		"SIMD per compute unit", 
		ccl_devquery_format_uint, ""},
	{"SIMD_WIDTH_AMD", CL_DEVICE_SIMD_WIDTH_AMD, 
		"SIMD width", 
		ccl_devquery_format_uint, ""},
	{"SINGLE_FP_CONFIG", CL_DEVICE_SINGLE_FP_CONFIG, 
		"Floating-point device configuration (single)", 
		ccl_devquery_format_fpconfig, ""},
	{"THREAD_TRACE_SUPPORTED_AMD", CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD, 
		"Is thread trace supported", 
		ccl_devquery_format_yesno, ""},
	{"TOPOLOGY_AMD", CL_DEVICE_TOPOLOGY_AMD, 
		"Description of the topology used to connect the device to the host", 
		ccl_devquery_format_hex, ""},
	{"TYPE", CL_DEVICE_TYPE, 
		"Type of OpenCL device", 
		ccl_devquery_format_type, ""},
	{"VENDOR", CL_DEVICE_VENDOR, 
		"Vendor of OpenCL device", 
		ccl_devquery_format_char, ""},
	{"VENDOR_ID", CL_DEVICE_VENDOR_ID, 
		"Unique device vendor identifier", 
		ccl_devquery_format_hex, ""},
	{"VERSION", CL_DEVICE_VERSION, 
		"OpenCL software driver version", 
		ccl_devquery_format_char, ""},
	{"WARP_SIZE_NV", CL_DEVICE_WARP_SIZE_NV, 
		"Warp size in work-items", 
		ccl_devquery_format_uint, ""},
	{"WAVEFRONT_WIDTH_AMD", CL_DEVICE_WAVEFRONT_WIDTH_AMD, 
		"Wavefront width", 
		ccl_devquery_format_uint, ""},
	{NULL, 0, NULL, NULL, NULL}	
		
};

/**
 * @brief Return the index of the device information map object of the
 * given parameter name.
 * 
 * @param A parameter name, in the format stored in the 
 * ccl_devquery_info_map array.
 * @return Index of the device information map object of the given 
 * parameter name, or -1 if device information map is not found.
 * */
static int ccl_devquery_get_index(const char* name) {

	/* Make sure name is not NULL. */
	g_return_val_if_fail(name != NULL, -1);
	
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
	idx_end = ccl_devquery_info_map_size - 1;
	while (idx_end >= idx_start) {
		idx_middle = (idx_start + idx_end) / 2;
		cmp_res = g_ascii_strncasecmp(
			name, ccl_devquery_info_map[idx_middle].param_name, len_name);
		if (cmp_res == 0) {
			found = TRUE;
			break;
		}
		if (cmp_res > 0)
			idx_start = idx_middle + 1;
		else 
			idx_end = idx_middle - 1;
	}
	
	/* Return result */
	return found ? idx_middle : -1;
}

/**
 * @brief Get a final device info prefix in the same format as 
 * kept in the ccl_devquery_info_map.
 * 
 * @param prefix Raw device information prefix. Several forms are 
 * accepted. For example, for CL_DEVICE_ENDIAN_LITTLE, strings such as
 * "CL_DEVICE_ENDIAN_LITTLE", "ENDIAN_LITTLE" or "endian_little" are
 * accepted.
 * @return A final device info prefix in the same format as 
 * kept in the ccl_devquery_info_map, or NULL if given prefix is not
 * valid. Should be freed with g_free() function from GLib.
 * */
gchar* ccl_devquery_get_prefix_final(const char* prefix) {
	
	/* Make sure prefix is not NULL. */
	g_return_val_if_fail(prefix != NULL, NULL);
	
	/* Auxiliary string variables. */
	gchar* str_upper;
	gchar* str_aux;
	gchar* str_final;
	
	/* Make string uppercase. */
	str_upper = g_ascii_strup(prefix, -1);
	
	/* Remove possible cl_device or cl_ prefix */
	if (g_str_has_prefix(str_upper, "CL_DEVICE_")) {
		str_aux = str_upper + strlen("CL_DEVICE_");
	} else if (g_str_has_prefix(str_upper, "CL_")) {
		str_aux = str_upper + strlen("CL_");
	} else {
		str_aux = str_upper;
	}
	
	/* Create a new string for returning. */
	str_final = g_strdup(str_aux);

	/* Free lowercase string. */
	g_free(str_upper);
	
	/* Return result */
	return str_final;
	
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
cl_device_info ccl_devquery_name(const char* name) {

	/* Make sure name is not NULL. */
	g_return_val_if_fail(name != NULL, 0);

	/* Final name */
	gchar* name_final;
	
	/* Index of device info. */
	gint idx;
	
	/* Get final name. */
	name_final = ccl_devquery_get_prefix_final(name);
	
	/* Get index of cl_device_info given its final name. */
	idx = ccl_devquery_get_index(name_final);
	
	/* Release memory holding the final name. */
	g_free(name_final);
	
	/* Return the cl_device_info object if found, or 0 otherwise. */
	if (idx >= 0)
		return ccl_devquery_info_map[idx].device_info;
	else
		return 0;
	
}

/**
 * @brief Get a pointer to the first device information parameter which 
 * has the given prefix.
 * 
 * @param prefix Device information parameter prefix. Can be in lower
 * or uppercase, and start with "cl_device_" or not.
 * @param size Location where to put number of matching parameters, or
 * NULL if it is to be ignored.
 * @return pointer to the first device information parameter which 
 * has the given prefix or NULL if nothing found.
 * */
const CCLDevQueryMap* ccl_devquery_prefix(
	const char* prefix, int* size) {
	
	/* Make sure prefix is not NULL. */
	g_return_val_if_fail(prefix != NULL, NULL);
	
	/* Final prefix to search for. */
	gchar* prefix_final;
	
	/* Size of final prefix. */
	gint len_prefix_final;
	
	/* Search index. */
	gint idx_middle, idx_start, idx_end;
	
	/* Found info. */
	const CCLDevQueryMap* found_ccl_devquery_info_map = NULL;
	
	/* Determine final prefix according to how parameter names are
	 * stored in ccl_devquery_info_map. */
	prefix_final = ccl_devquery_get_prefix_final(prefix);
	
	/* Determine prefix size. */
	len_prefix_final = strlen(prefix_final);
	
	/* Binary search. */
	idx_middle = ccl_devquery_get_index(prefix_final);
	
	if (idx_middle >= 0) {
		/* Search for beginning. */
		idx_start = idx_middle;
		while (TRUE) {
			if (idx_start == 0)
				break;
			if (g_ascii_strncasecmp(prefix_final, 
					ccl_devquery_info_map[idx_start - 1].param_name, 
					len_prefix_final)
				 == 0)
				idx_start--;
			else
				break;
		}
		/* Search for ending. */
		idx_end = idx_middle;
		while (TRUE) {
			if (idx_end == ccl_devquery_info_map_size - 1)
				break;
			if (g_ascii_strncasecmp(prefix_final, 
					ccl_devquery_info_map[idx_end + 1].param_name, 
					len_prefix_final) 
				== 0)
				idx_end++;
			else
				break;
		}
		
		/* Set return values. */
		if (size != NULL)
			*size = idx_end - idx_start + 1;
		found_ccl_devquery_info_map = &ccl_devquery_info_map[idx_start];
		
	} else {
		
		/* Nothing found. */
		if (size != NULL)
			*size = -1;

	}

	/* Free final prefix. */
	g_free(prefix_final);
	
	/* Return result */
	return found_ccl_devquery_info_map;
}

/**
 * @brief Search for a device information parameter by matching part
 * of its name. This function is supposed to be used in a loop. 
 * 
 * @param substr String to match with parameter name.
 * @param idx Next index, should be zero in the first call, and the 
 * function updates within calls.
 * @return A matching CCLDevQueryMap, or NULL if search is over.
 * */
const CCLDevQueryMap* ccl_devquery_match(const char* substr, int* idx) {
	
	/* Make sure substr is not NULL. */
	g_return_val_if_fail(substr != NULL, NULL);
	
	/* Make sure idx is not NULL. */
	g_return_val_if_fail(idx != NULL, NULL);
	
	/* Found flag. */
	gboolean found = FALSE;

	/* Found result. */
	const CCLDevQueryMap* info_row = NULL;
	
	/* Linear search. */
	for ( ; *idx < ccl_devquery_info_map_size; (*idx)++) {
		if (g_strstr_len(
			ccl_devquery_info_map[*idx].param_name, -1, substr)) {
				
			found = TRUE;
			break;
		}
	}
	
	/* Set found result. */
	if (found)
		info_row = &ccl_devquery_info_map[*idx];

	/* Increment index (for next iteration). */
	(*idx)++;

	/* Return result. */
	return info_row;
	
}

/** @} */
