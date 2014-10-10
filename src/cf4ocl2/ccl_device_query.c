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
 *
 * Classes and functions to query OpenCL devices.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_device_query.h"

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting unsigned integers.
 * */
static char* ccl_devquery_format_uint(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	g_snprintf(out, (gulong) size, "%u %s", *((cl_uint*) info->value), units);
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting device information as a hexadecimal number.
 * */
static char* ccl_devquery_format_hex(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	GString* str = g_string_new("0x");
	gboolean start = FALSE;
	gchar val;

	for (gint i = (gint) info->size - 1; i >= 0 ; i--) {
		val = ((cl_char*) info->value)[i];
		if (val) start = TRUE;
		if (start)
			g_string_append_printf(str, "%.2x", val);
	}
	if (units && units[0])
		g_string_append_printf(str, " %s", units);

	g_snprintf(out, (gulong) size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting size_t unsigned integers.
 * */
static char* ccl_devquery_format_sizet(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	g_snprintf(out, (gulong) size, "%lu %s", (gulong) *((size_t*) info->value), units);
	return out;

}

/**
 * @internal
 * Helper macro for format functions outputting values in bytes.
 *
 * @param[in] spec A positive integer value representing bytes.
 * */
#define ccl_devquery_format_bytes(spec) \
	if (bytes < 1024) \
		g_snprintf(out, (gulong) size, "%" spec " bytes", bytes); \
	else if (bytes < 1048576) \
		g_snprintf(out, (gulong) size, "%.1lf KiB (%" spec " bytes)", \
			bytes / 1024.0, bytes); \
	else if (bytes < 1073741824) \
		g_snprintf(out, (gulong) size, "%.1lf MiB (%" spec " bytes)", \
			bytes / (1024.0 * 1024), bytes); \
	else \
		g_snprintf(out, (gulong) size, "%.1lf GiB (%" spec " bytes)", \
			bytes / (1024.0 * 1024 * 1024), bytes);

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting unsigned long integers which represent bytes.
 * */
static char* ccl_devquery_format_ulongbytes(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	unsigned long bytes = (unsigned long) *((cl_ulong*) info->value);
	ccl_devquery_format_bytes("lu");
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting unsigned integers which represent bytes.
 * */
static char* ccl_devquery_format_uintbytes(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	unsigned int bytes = (unsigned int) *((cl_uint*) info->value);
	ccl_devquery_format_bytes("u");
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting unsigned size_t integers which represent bytes.
 * */
static char* ccl_devquery_format_sizetbytes(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	unsigned long bytes = (unsigned long) *((size_t*) info->value);
	ccl_devquery_format_bytes("lu");
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting a vector of size_t integers.
 * */
static char* ccl_devquery_format_sizetvec(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	GString* str = g_string_new("(");
	size_t* vec = (size_t*) info->value;
	guint count = (guint) (info->size / sizeof(gsize));

	for (guint i = 0; i < count; i++) {
		if (i > 0) g_string_append(str, ", ");
		g_string_append_printf(str, "%lu", (gulong) vec[i]);
	}

	g_string_append(str, ")");

	g_snprintf(out, (gulong) size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;
}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting boolean values as a "Yes" or "No" string.
 * */
static char* ccl_devquery_format_yesno(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	g_snprintf(out, (gulong) size, "%s", *((cl_bool*)info->value) ? "Yes" : "No");
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting strings.
 * */
static char* ccl_devquery_format_char(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	g_snprintf(out, (gulong) size, "%s %s", (gchar*)info->value, units);
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting memory addresses.
 * */
static char* ccl_devquery_format_ptr(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	g_snprintf(out, (gulong) size, "%p", *((void**)info->value));
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting a string representing a device type.
 * */
static char* ccl_devquery_format_type(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	g_snprintf(out, (gulong) size, "%s",
		ccl_devquery_type2str(*((cl_device_type*) info->value)));
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the device floating-point (FP) configuration for a FP
 * type. */
static char* ccl_devquery_format_fpconfig(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_fp_config fpc = *((cl_device_fp_config*) info->value);
	g_snprintf(out, (gulong) size, "%s%s%s%s%s%s%s",
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
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the device execution capabilities.
 * */
static char* ccl_devquery_format_execcap(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_exec_capabilities exc =
		*((cl_device_exec_capabilities*) info->value);
	g_snprintf(out, (gulong) size, "%s%s",
		exc & CL_EXEC_KERNEL ? "KERNEL " : "",
		exc & CL_EXEC_NATIVE_KERNEL ? "NATIVE_KERNEL " : "");
	return out;
}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting a local memory type. */
static char* ccl_devquery_format_locmemtype(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_local_mem_type lmt =
		*((cl_device_local_mem_type*) info->value);
	g_snprintf(out, (gulong) size, "%s%s%s",
		lmt & CL_LOCAL ? "LOCAL" : "",
		lmt & CL_GLOBAL ? "GLOBAL" : "",
		lmt & CL_NONE ? "NONE" : "");
	return out;
}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the partition properties of a device.
 * */
static char* ccl_devquery_format_partprop(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_partition_property* pp =
		(cl_device_partition_property*) info->value;
	GString* str = g_string_new("");
	guint count = (guint) (info->size / sizeof(cl_device_partition_property));
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
				g_string_append_printf(str, "UNKNOWN(0x%lx) ",
					(unsigned long) pp[i]);
		}
	}
	g_snprintf(out, (gulong) size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the supported affinity domains for partitioning a device
 * using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN.
 * */
static char* ccl_devquery_format_affdom(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_affinity_domain ad =
		*((cl_device_affinity_domain*) info->value);
	g_snprintf(out, (gulong) size, "%s%s%s%s%s%s",
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
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the supported affinity domains for partitioning a device
 * using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT.
 * */
static char* ccl_devquery_format_affdom_ext(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_partition_property_ext* ade =
		(cl_device_partition_property_ext*) info->value;
	GString* str = g_string_new("");
	guint count = (guint) (info->size / sizeof(cl_device_partition_property_ext));
	for (guint i = 0; i < count; i++) {
		switch (ade[i]) {
			case CL_AFFINITY_DOMAIN_L1_CACHE_EXT:
				g_string_append_printf(str, "L1_CACHE_EXT ");
				break;
			case CL_AFFINITY_DOMAIN_L2_CACHE_EXT:
				g_string_append_printf(str, "L2_CACHE_EXT ");
				break;
			case CL_AFFINITY_DOMAIN_L3_CACHE_EXT:
				g_string_append_printf(str, "L3_CACHE_EXT ");
				break;
			case CL_AFFINITY_DOMAIN_L4_CACHE_EXT:
				g_string_append_printf(str, "L4_CACHE_EXT ");
				break;
			case CL_AFFINITY_DOMAIN_NUMA_EXT:
				g_string_append_printf(str, "NUMA_EXT ");
				break;
			case CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT:
				g_string_append_printf(str, "NEXT_FISSIONABLE_EXT ");
				break;
			case CL_PROPERTIES_LIST_END_EXT:
				break;
			default:
				g_string_append_printf(str, "UNKNOWN(0x%lx) ",
					(unsigned long) ade[i]);
		}
	}
	g_snprintf(out, (gulong) size, "%s", str->str);
	g_string_free(str, TRUE);
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the cache type of a device.
 * */
static char* ccl_devquery_format_cachetype(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_mem_cache_type mct =
		*((cl_device_mem_cache_type*) info->value);
	g_snprintf(out, (gulong) size, "%s%s%s",
		mct & CL_READ_ONLY_CACHE ? "READ_ONLY" : "",
		mct & CL_READ_WRITE_CACHE ? "READ_WRITE" : "",
		mct & CL_NONE ? "NONE" : "");
	return out;
}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the queue properties of a device.
 * */
static char* ccl_devquery_format_queueprop(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_command_queue_properties qp =
		*((cl_command_queue_properties*) info->value);
	g_snprintf(out, (gulong) size, "%s%s",
		qp & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ? "OUT_OF_ORDER_EXEC_MODE_ENABLE " : "",
		qp & CL_QUEUE_PROFILING_ENABLE ? "PROFILING_ENABLE " : "");
	return out;

}

/**
 * @internal
 * Implementation of ccl_devquery_format() function for
 * outputting the shared virtual memory (SVM) memory allocation
 * capabilities of the device.
 * */
static char* ccl_devquery_format_svmc(CCLWrapperInfo* info,
	char* out, size_t size, const char* units) {

	CCL_UNUSED(units);
	cl_device_svm_capabilities svmc =
		*((cl_device_svm_capabilities*) info->value);
	g_snprintf(out, (gulong) size, "%s%s%s%s",
		svmc & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER ? "COARSE_GRAIN_BUFFER " : "",
		svmc & CL_DEVICE_SVM_FINE_GRAIN_BUFFER ? "FINE_GRAIN_BUFFER " : "",
		svmc & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM ? "FINE_GRAIN_SYSTEM " : "",
		svmc & CL_DEVICE_SVM_ATOMICS ? "ATOMICS" : "");
	return out;
}

//~ /**
 //~ * @internal
 //~ * Implementation of ccl_devquery_format() function for debugging
 //~ * purposes.
 //~ * */
//~ static char* ccl_devquery_format_testsize(CCLWrapperInfo* info,
	//~ gchar* out, guint size, const gchar const* units) {
	//~
	//~ g_snprintf(out, size, "%ld %s", (gulong) info->size, units);
	//~ return out;
//~
//~ }

/* Size of parameter information map. */
CCL_EXPORT
const int ccl_devquery_info_map_size = 123;

/* Map of parameter name strings to respective cl_device_info
 * bitfields, long description string, format output function and a
 * units suffix. */
CCL_EXPORT
const CCLDevQueryMap ccl_devquery_info_map[] = {

	{"ADDRESS_BITS", CL_DEVICE_ADDRESS_BITS,
		"Address space size in bits",
		ccl_devquery_format_uint, "bits"},
	{"AFFINITY_DOMAINS_EXT", CL_DEVICE_AFFINITY_DOMAINS_EXT,
		"Ext.: List of supported affinity domains for partitioning the device",
		ccl_devquery_format_affdom_ext, ""},
	{"AVAILABLE", CL_DEVICE_AVAILABLE,
		"Is device available",
		ccl_devquery_format_yesno, ""},
	{"BOARD_NAME_AMD", CL_DEVICE_BOARD_NAME_AMD,
		"AMD ext.: Name of the GPU board and model of the specific device",
		ccl_devquery_format_char, ""},
	{"BUILT_IN_KERNELS", CL_DEVICE_BUILT_IN_KERNELS,
		"Device built-in kernels",
		ccl_devquery_format_char, ""},
	{"COMPILER_AVAILABLE", CL_DEVICE_COMPILER_AVAILABLE,
		"Is a compiler available for device",
		ccl_devquery_format_yesno, ""},
	{"COMPUTE_CAPABILITY_MAJOR_NV", CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV,
		"NVidia ext.: Major revision number that defines the CUDA compute capability of the device",
		ccl_devquery_format_uint, ""},
	{"COMPUTE_CAPABILITY_MINOR_NV", CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV,
		"NVidia ext.: Minor revision number that defines the CUDA compute capability of the device",
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
	{"EXT_MEM_PADDING_IN_BYTES_QCOM", CL_DEVICE_EXT_MEM_PADDING_IN_BYTES_QCOM,
		"Qualcomm ext.: Amount of padding necessary at the end of the buffer",
		ccl_devquery_format_sizetbytes, ""},
	/* The parameter bellow has in fact a length of 16 bytes,
	 * but due to the lack of documentation, only the first half
	 * of it is shown for now. */
	{"GLOBAL_FREE_MEMORY_AMD", CL_DEVICE_GLOBAL_FREE_MEMORY_AMD,
		"AMD ext.: Free device memory",
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
		"AMD ext.: Global mem. channels",
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_CHANNEL_BANKS_AMD", CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD,
		"AMD ext.: Global mem. channel banks",
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD", CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD,
		"AMD ext.: Global mem. channel bank width",
		ccl_devquery_format_uint, ""},
	{"GLOBAL_MEM_SIZE", CL_DEVICE_GLOBAL_MEM_SIZE,
		"Global mem. size",
		ccl_devquery_format_ulongbytes, ""},
	{"GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE", CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE,
		"Max. pref. total size of all program variables in the global address space",
		ccl_devquery_format_sizetbytes, ""},
	{"GPU_OVERLAP_NV", CL_DEVICE_GPU_OVERLAP_NV,
		"NVidia ext.: Can device concurrently copy memory between host and device while executing a kernel",
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
	{"IMAGE_BASE_ADDRESS_ALIGNMENT", CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT,
		"Min. alignment of the host_ptr specified to clCreateBuffer (for 2D images created from a buffer in turn created using CL_MEM_USE_HOST_PTR)",
		ccl_devquery_format_uint, "px"},
	{"IMAGE_MAX_ARRAY_SIZE", CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,
		"Max. images in a 1D or 2D image array",
		ccl_devquery_format_sizet, "images"},
	{"IMAGE_MAX_BUFFER_SIZE", CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,
		"Max. pixels for 1D image from buffer object",
		ccl_devquery_format_sizet, "px"},
	{"IMAGE_PITCH_ALIGNMENT", CL_DEVICE_IMAGE_PITCH_ALIGNMENT,
		"Row pitch alignment size in pixels for 2D images created from a buffer",
		ccl_devquery_format_uint, "px"},
	{"IMAGE_SUPPORT", CL_DEVICE_IMAGE_SUPPORT,
		"Image support",
		ccl_devquery_format_yesno, ""},
	{"INTEGRATED_MEMORY_NV", CL_DEVICE_INTEGRATED_MEMORY_NV,
		"NVidia ext.: Is device integrated with the memory subsystem?",
		ccl_devquery_format_yesno, ""},
	{"KERNEL_EXEC_TIMEOUT_NV", CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV,
		"Nvidia ext.: Is there a limit for kernels executed on device?",
		ccl_devquery_format_yesno, ""},
	{"LINKER_AVAILABLE", CL_DEVICE_LINKER_AVAILABLE,
		"Linker available",
		ccl_devquery_format_yesno, ""},
	{"LOCAL_MEM_BANKS_AMD", CL_DEVICE_LOCAL_MEM_BANKS_AMD,
		"AMD ext.: Local mem. banks",
		ccl_devquery_format_uint, ""},
	{"LOCAL_MEM_SIZE", CL_DEVICE_LOCAL_MEM_SIZE,
		"Local mem. size",
		ccl_devquery_format_ulongbytes, ""},
	{"LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD", CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD,
		"AMD ext.: Local mem. size per compute unit",
		ccl_devquery_format_uintbytes, ""},
	{"LOCAL_MEM_TYPE", CL_DEVICE_LOCAL_MEM_TYPE,
		"Local mem. type",
		ccl_devquery_format_locmemtype, ""},
	{"MAX_ATOMIC_COUNTERS_EXT", CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT,
		"Ext.: Max. atomic counters",
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
	{"MAX_GLOBAL_VARIABLE_SIZE", CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE,
		"Max. storage that may be allocated for any single variable in program scope or inside a function in OpenCL C declared in the global address space",
		ccl_devquery_format_sizetbytes, ""},
	{"MAX_MEM_ALLOC_SIZE", CL_DEVICE_MAX_MEM_ALLOC_SIZE,
		"Max. size of memory object allocation in bytes",
		ccl_devquery_format_ulongbytes, ""},
	{"MAX_ON_DEVICE_EVENTS", CL_DEVICE_MAX_ON_DEVICE_EVENTS,
		"The maximum number of events in use by a device queue",
		ccl_devquery_format_uint, ""},
	{"MAX_ON_DEVICE_QUEUES", CL_DEVICE_MAX_ON_DEVICE_QUEUES,
		"Max. number of device queues that can be created per context",
		ccl_devquery_format_uint, ""},
	{"MAX_PARAMETER_SIZE", CL_DEVICE_MAX_PARAMETER_SIZE,
		"Max. size in bytes of the arguments that can be passed to a kernel",
		ccl_devquery_format_sizetbytes, ""},
	{"MAX_PIPE_ARGS", CL_DEVICE_MAX_PIPE_ARGS,
		"Max. pipe objects that can be passed as arguments to a kernel",
		ccl_devquery_format_uint, ""},
	{"MAX_READ_IMAGE_ARGS", CL_DEVICE_MAX_READ_IMAGE_ARGS,
		"Max. number of simultaneous image objects that can be read by a kernel",
		ccl_devquery_format_uint, "images"},
	{"MAX_READ_WRITE_IMAGE_ARGS", CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS,
		"Max. image objects arguments of a kernel declared with the read_only qualifier",
		ccl_devquery_format_uint, ""},
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
	{"PAGE_SIZE_QCOM", CL_DEVICE_PAGE_SIZE_QCOM,
		"Qualcomm ext.: Page size of device",
		ccl_devquery_format_sizetbytes, ""},
	{"PARENT_DEVICE", CL_DEVICE_PARENT_DEVICE,
		"The cl_device_id of the parent device to which the sub-device belongs",
		ccl_devquery_format_ptr, ""},
	{"PARENT_DEVICE_EXT", CL_DEVICE_PARENT_DEVICE_EXT,
		"Ext.: Get the parent device to which the sub-device belongs",
		ccl_devquery_format_hex, ""},
	{"PARTITION_AFFINITY_DOMAIN", CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
		"Supported affinity domains for partitioning the device using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN",
		ccl_devquery_format_affdom, ""},
	{"PARTITION_MAX_SUB_DEVICES", CL_DEVICE_PARTITION_MAX_SUB_DEVICES,
		"Max. sub-devices that can be created when device is partitioned",
		ccl_devquery_format_uint, "devices"},
	{"PARTITION_PROPERTIES", CL_DEVICE_PARTITION_PROPERTIES,
		"Partition types supported by device",
		ccl_devquery_format_partprop, ""},
	{"PARTITION_STYLE_EXT", CL_DEVICE_PARTITION_STYLE_EXT,
		"Ext.: Partition properties used to create the sub-device",
		ccl_devquery_format_affdom_ext, ""},
	{"PARTITION_TYPE", CL_DEVICE_PARTITION_TYPE,
		"Properties specified in clCreateSubDevices if device is a subdevice",
		ccl_devquery_format_uint, ""},
	{"PARTITION_TYPES_EXT", CL_DEVICE_PARTITION_TYPES_EXT,
		"Ext.: List of supported partition types for partitioning a device",
		ccl_devquery_format_partprop, ""},
	{"PIPE_MAX_ACTIVE_RESERVATIONS", CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS,
		"Max. reservations that can be active for a pipe per work-item in a kernel",
		ccl_devquery_format_uint, ""},
	{"PIPE_MAX_PACKET_SIZE", CL_DEVICE_PIPE_MAX_PACKET_SIZE,
		"Max. size of pipe packet",
		ccl_devquery_format_uintbytes, ""},
	{"PLATFORM", CL_DEVICE_PLATFORM,
		"The platform associated with device",
		ccl_devquery_format_ptr, ""},
	{"PREFERRED_GLOBAL_ATOMIC_ALIGNMENT", CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT,
		"Preferred alignment for OpenCL 2.0 atomic types to global memory",
		ccl_devquery_format_uintbytes, ""},
	{"PREFERRED_INTEROP_USER_SYNC", CL_DEVICE_PREFERRED_INTEROP_USER_SYNC,
		"'Yes' if device prefers user to be responsible for sync. when sharing memory objects between OpenCL and other APIs, 'No' if device has a performant path for performing such sync.",
		ccl_devquery_format_yesno, ""},
	{"PREFERRED_LOCAL_ATOMIC_ALIGNMENT", CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT,
		"Preferred alignment for OpenCL 2.0 atomic types to local memory",
		ccl_devquery_format_uintbytes, ""},
	{"PREFERRED_PLATFORM_ATOMIC_ALIGNMENT", CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT,
		"Preferred alignment for OpenCL 2.0 fine-grained SVM atomic types",
		ccl_devquery_format_uintbytes, ""},
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
		"AMD ext.: Offset between event timestamps in nanoseconds",
		ccl_devquery_format_sizet, "ns"},
	{"PROFILING_TIMER_RESOLUTION", CL_DEVICE_PROFILING_TIMER_RESOLUTION,
		"Resolution of device timer in nanoseconds",
		ccl_devquery_format_sizet, "ns"},
	{"QUEUE_ON_DEVICE_MAX_SIZE", CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE,
		"Max. size of the device queue",
		ccl_devquery_format_uintbytes, ""},
	{"QUEUE_ON_DEVICE_PREFERRED_SIZE", CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE,
		"Size of the device queue preferred by the implementation",
		ccl_devquery_format_uintbytes, ""},
	{"QUEUE_ON_DEVICE_PROPERTIES", CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES,
		"On-device command-queue properties supported by the device",
		ccl_devquery_format_queueprop , ""},
	{"QUEUE_ON_HOST_PROPERTIES", CL_DEVICE_QUEUE_ON_HOST_PROPERTIES,
		"On-host command-queue properties supported by the device",
		ccl_devquery_format_queueprop, ""},
	{"QUEUE_PROPERTIES", CL_DEVICE_QUEUE_PROPERTIES,
		"Command-queue properties supported by device",
		ccl_devquery_format_queueprop, ""},
	{"REFERENCE_COUNT", CL_DEVICE_REFERENCE_COUNT,
		"Device reference count",
		ccl_devquery_format_uint, ""},
	{"REFERENCE_COUNT_EXT", CL_DEVICE_REFERENCE_COUNT_EXT,
		"Ext.: Device reference count",
		ccl_devquery_format_uint, ""},
	{"REGISTERS_PER_BLOCK_NV", CL_DEVICE_REGISTERS_PER_BLOCK_NV,
		"NVidia ext.: Maximum number of 32-bit registers available to a work-group",
		ccl_devquery_format_uint, ""},
	{"SIMD_INSTRUCTION_WIDTH_AMD", CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD,
		"AMD ext.: SIMD instruction width",
		ccl_devquery_format_uint, ""},
	{"SIMD_PER_COMPUTE_UNIT_AMD", CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD,
		"AMD ext.: SIMD per compute unit",
		ccl_devquery_format_uint, ""},
	{"SIMD_WIDTH_AMD", CL_DEVICE_SIMD_WIDTH_AMD,
		"AMD ext.: SIMD width",
		ccl_devquery_format_uint, ""},
	{"SINGLE_FP_CONFIG", CL_DEVICE_SINGLE_FP_CONFIG,
		"Floating-point device configuration (single)",
		ccl_devquery_format_fpconfig, ""},
	{"SPIR_VERSIONS", CL_DEVICE_SPIR_VERSIONS,
		"Space separated list of SPIR versions supported by the device",
		ccl_devquery_format_char, ""},
	{"SVM_CAPABILITIES", CL_DEVICE_SVM_CAPABILITIES,
		"Shared virtual memory (SVM) memory allocation types the device supports",
		ccl_devquery_format_svmc, ""},
	{"TERMINATE_CAPABILITY_KHR", CL_DEVICE_TERMINATE_CAPABILITY_KHR,
		"Termination capability of the OpenCL device",
		ccl_devquery_format_hex, ""},
	{"THREAD_TRACE_SUPPORTED_AMD", CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD,
		"AMD ext.: Is thread trace supported",
		ccl_devquery_format_yesno, ""},
	{"TOPOLOGY_AMD", CL_DEVICE_TOPOLOGY_AMD,
		"AMD ext.: Description of the topology used to connect the device to the host",
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
		"NVidia ext.: Warp size in work-items",
		ccl_devquery_format_uint, ""},
	{"WAVEFRONT_WIDTH_AMD", CL_DEVICE_WAVEFRONT_WIDTH_AMD,
		"AMD ext.: Wavefront width",
		ccl_devquery_format_uint, ""},
	{NULL, 0, NULL, NULL, NULL}

};

/**
 * @addtogroup CCL_DEVICE_QUERY
 * @{
 */

/**
 * @internal
 * Return the index of the device information map object of the
 * given parameter name.
 *
 * @private @memberof ccl_devquery_map
 *
 * @param[in] name A parameter name, in the format stored in the
 * ::ccl_devquery_info_map array.
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

	/* Binary search. */
	idx_start = 0;
	idx_end = ccl_devquery_info_map_size - 1;
	while (idx_end >= idx_start) {
		idx_middle = (idx_start + idx_end) / 2;
		cmp_res = g_strcmp0(
			name, ccl_devquery_info_map[idx_middle].param_name);
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
 * Get a final device info prefix in the same format as
 * kept in the ::ccl_devquery_info_map.
 *
 * @public @memberof ccl_devquery_map
 *
 * @param[in] prefix Raw device information prefix. Several forms are
 * accepted. For example, for CL_DEVICE_ENDIAN_LITTLE, strings such as
 * "CL_DEVICE_ENDIAN_LITTLE", "ENDIAN_LITTLE" or "endian_little" are
 * accepted.
 * @return A final device info prefix in the same format as
 * kept in the ::ccl_devquery_info_map, or `NULL` if given prefix is not
 * valid. Should be freed with g_free() function from GLib.
 * */
CCL_EXPORT
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
 * Return a `cl_device_info` object given its name.
 *
 * @public @memberof ccl_devquery_map
 *
 * @param[in] name Name of `cl_device_info` object. Several forms are
 * accepted. For example, for CL_DEVICE_ENDIAN_LITTLE, strings such as
 * "CL_DEVICE_ENDIAN_LITTLE", "ENDIAN_LITTLE" or "endian_little" are
 * accepted.
 * @return A `cl_device_info` object given its name or 0 if no suitable
 * `cl_device_info` is found for the given name.
 * */
CCL_EXPORT
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
 * Get a pointer to the first device information parameter which
 * has the given prefix.
 *
 * @public @memberof ccl_devquery_map
 *
 * @param[in] prefix Device information parameter prefix. Can be in
 * lower or uppercase, and start with "cl_device_" or not.
 * @param[out] size Location where to put number of matching parameters,
 * or `NULL` if it is to be ignored.
 * @return pointer to the first device information parameter which
 * has the given prefix or `NULL` if nothing found.
 * */
CCL_EXPORT
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
	len_prefix_final = (gint) strlen(prefix_final);

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
 * Search for a device information parameter by matching part
 * of its name. This function is supposed to be used in a loop.
 *
 * @public @memberof ccl_devquery_map
 *
 * @param[in] substr String to match with parameter name.
 * @param[in,out] idx Next index, should be zero in the first call, and
 * the function updates within calls.
 * @return A matching ::CCLDevQueryMap*, or `NULL` if search is over.
 * */
CCL_EXPORT
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
