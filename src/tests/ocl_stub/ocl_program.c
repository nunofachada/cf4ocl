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
 * @brief OpenCL program stub functions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "ocl_env.h"
#include "utils.h"


static cl_program clCreateProgram(cl_context context, 
	cl_uint num_devices, const cl_device_id* device_list, char* source, 
	const size_t* lengths, const unsigned char** binaries) {

	/* Allocate memory for program. */
	cl_program program = g_slice_new(struct _cl_program);
	
	program->ref_count = 1;
	program->context = context;
	program->num_devices = (num_devices > 0)
		? num_devices
		: context->num_devices;
	program->devices = g_slice_copy(
		program->num_devices * sizeof(cl_device_id), 
		(device_list != NULL) ? device_list : context->devices);
	program->source = g_strdup(source);
	program->binary_sizes = (lengths != NULL)
		? g_slice_copy(program->num_devices * sizeof(size_t), lengths)
		: g_slice_alloc0(program->num_devices * sizeof(size_t));
	program->binaries = g_slice_alloc0(
		program->num_devices * sizeof(unsigned char*));
	program->binary_type = g_slice_alloc0(
		program->num_devices * sizeof(cl_program_binary_type));
	if (binaries != NULL) {
		for (cl_uint i = 0; i < program->num_devices; ++i) {
			if ((binaries[i] != NULL) && (lengths[i] > 0)) {
				program->binaries[i] = (unsigned char*)
					g_strndup((const char*) binaries[i], lengths[i]);
				program->binary_type[i] = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
			} else {
				program->binary_type[i] = CL_PROGRAM_BINARY_TYPE_NONE;
			}
		}
	}
	program->build_status =
		g_slice_alloc(program->num_devices * sizeof(cl_build_status));
	program->build_log = 
		g_slice_alloc(program->num_devices * sizeof(char**));
	program->build_options =
		g_slice_alloc(program->num_devices * sizeof(char**));
	for (cl_uint i = 0; i < program->num_devices; ++i) {
		program->build_status[i] = CL_BUILD_NONE;
		program->build_options[i] = NULL;
		program->build_log[i] = NULL;
		if (binaries == NULL)
			program->binary_type[i] = CL_PROGRAM_BINARY_TYPE_NONE;
	}
	
	program->num_kernels = 0;
	program->kernel_names = NULL;
	
	return program;

}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context context, cl_uint count, 
	const char** strings, const size_t* lengths, 
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 {

	seterrcode(errcode_ret, CL_SUCCESS);
	
	/* New program. */
	cl_program program = NULL;
	
	/* Complete source code string. */
	GString* src = NULL;

	/* Parameter check. */
	if (context == NULL) {
		seterrcode(errcode_ret, CL_INVALID_CONTEXT);
		goto ERROR;
	}
	if ((count == 0) || (strings == NULL)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
		goto ERROR;
	}

	/* Prepare complete source code string. */
	src = g_string_new("");
	for (cl_uint i = 0; i < count; ++i) {
		if (strings[i] == NULL) {
			seterrcode(errcode_ret, CL_INVALID_VALUE);
			goto ERROR_FREE_SRC;
		}
		size_t len = (lengths != NULL) ? lengths[i] : strlen(strings[i]);
		g_string_append_len(src, strings[i], len);
	}
	g_string_append(src, "\0");
	
	/* Create program. */
	program = clCreateProgram(context, 0, NULL, src->str, NULL, NULL);

ERROR_FREE_SRC:
	g_string_free(src, TRUE);
	
ERROR:
	
	return program;
		
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context context, cl_uint num_devices,
	const cl_device_id* device_list, const size_t* lengths,
	const unsigned char** binaries, cl_int* binary_status,
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 {

	seterrcode(errcode_ret, CL_SUCCESS);

	/* New program. */
	cl_program program = NULL;
	
	/* Parameter check. */
	if (context == NULL) {
		seterrcode(errcode_ret, CL_INVALID_CONTEXT);
		goto ERROR;
	}
	if ((num_devices == 0) || (device_list == NULL) || (lengths == NULL) 
		|| (binaries == NULL)) {
		seterrcode(errcode_ret, CL_INVALID_VALUE);
		goto ERROR;
	}
	for (cl_uint i = 0; i < num_devices; ++i) {
		cl_bool found = FALSE;
		for (cl_uint j = 0; j < context->num_devices; j++) {
			if (device_list[i] == context->devices[j])
				found = TRUE;
		}
		if (!found) {
			seterrcode(errcode_ret, CL_INVALID_DEVICE);
			goto ERROR;
		}
		if ((lengths[i] == 0) || (binaries[i] == NULL)) {
			seterrcode(errcode_ret, CL_INVALID_VALUE);
			goto ERROR;
		}
	}
	cl_bool ok = TRUE;
	for (cl_uint i = 0; i < num_devices; ++i) {
		if ((lengths[i] == 0) || (binaries[i] == NULL)) {
			seterrcode(errcode_ret, CL_INVALID_VALUE);
			ok = FALSE;
			if (binary_status != NULL) {
				binary_status[i] = CL_INVALID_VALUE;
			}
		} else {
			if (binary_status != NULL) {
				binary_status[i] = CL_SUCCESS;
			}
		}
	}
	if (!ok) goto ERROR;
	
	/* Create program. */
	program = clCreateProgram(context, num_devices, device_list, NULL, 
		lengths, binaries);

ERROR:
	
	return program;
}


CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {

	g_atomic_int_inc(&program->ref_count);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {
	
	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&program->ref_count)) {
		
		/* Free kernel names string. */
		if (program->kernel_names != NULL)
			g_free(program->kernel_names);
		
		/* Free build information. */
		if (program->build_status != NULL) {
			g_slice_free1(
				program->num_devices * sizeof(cl_build_status),
				program->build_status);
		}
		if (program->build_options != NULL) {
			for (cl_uint i = 0; i < program->num_devices; ++i) {
				if (program->build_options[i] != NULL)
					g_free(program->build_options[i]);
			}
			g_slice_free1(
				program->num_devices * sizeof(char**),
				program->build_options);
		}
		if (program->build_log != NULL) {
			for (cl_uint i = 0; i < program->num_devices; ++i) {
				if (program->build_log[i] != NULL)
					g_free(program->build_log[i]);
			}
			g_slice_free1(
				program->num_devices * sizeof(char**),
				program->build_log);
		}
		/* Free binaries. */
		if (program->binaries != NULL) {
			for (cl_uint i = 0; i < program->num_devices; ++i) {
				if (program->binaries[i] != NULL)
					g_free(program->binaries[i]);
			}
			g_slice_free1(
				program->num_devices * sizeof(unsigned char*), 
				program->binaries);
		}
		
		/* Free binary lengths. */
		if (program->binary_sizes != NULL) {
			g_slice_free1(program->num_devices * sizeof(size_t), 
				program->binary_sizes);
		}
		
		/* Free binary types. */
		if (program->binary_type != NULL)
			g_slice_free1(program->num_devices * sizeof(cl_program_binary_type), 
				program->binary_type);
		
		/* Free program source. */
		if (program->source != NULL) {
			g_free(program->source);
		}
		
		/* Free device list. */
		if (program->devices != NULL) {
			g_slice_free1(program->num_devices * sizeof(cl_device_id), 
				program->devices);
		}

		/* Free program object. */
		g_slice_free(struct _cl_program, program);
		
	}
	
	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program program, cl_uint num_devices,
	const cl_device_id* device_list, const char* options,
	void (CL_CALLBACK* pfn_notify)(cl_program, void*),
    void* user_data) CL_API_SUFFIX__VERSION_1_0 {

	cl_int status = CL_SUCCESS;
	
	/* Parameter check. */
	if (program == NULL) {
		status = CL_INVALID_PROGRAM;
		goto ERROR;
	}
	if (((num_devices == 0) && (device_list != NULL)) 
		|| ((num_devices > 0) && (device_list == NULL)) 
		|| ((pfn_notify == NULL) && (user_data != NULL))) {
		status = CL_INVALID_VALUE;
		goto ERROR;
	}
	for (cl_uint i = 0; i < num_devices; ++i) {
		cl_bool found = FALSE;
		for (cl_uint j = 0; j < program->num_devices; j++) {
			if (device_list[i] == program->devices[j])
				found = TRUE;
		}
		if (!found) {
			status = CL_INVALID_DEVICE;
			goto ERROR;
		}
	}
	if ((program->kernel_names != NULL) || (program->num_kernels > 0)) {
		status = CL_INVALID_OPERATION;
		goto ERROR;
	}
	
	/* "Compile" source for given devices. If binary already exists for
	 * device, simply mark build status as CL_BUILD_SUCCESS. */
	for (cl_uint i = 0; i < num_devices; ++i) {
		/* Find index of device in program. */
		cl_uint j;
		cl_bool found = CL_FALSE;
		for (j = 0; j < program->num_devices; ++j) {
			if (device_list[i] == program->devices[j]) {
				found = CL_TRUE;
				break;
			}
		}
		g_assert_cmpint(found, ==, TRUE);
		/* Compile if build status is NONE. */
		if (program->build_status[j] == CL_BUILD_NONE) {
			program->build_status[j] = CL_BUILD_SUCCESS;
			program->build_options[j] = g_strdup(options);
			program->build_log[j] = g_strdup_printf(
				"Compilation successful for device '%s'", 
				program->devices[j]->name);
			/* Do some bogus compilation of source code. */
			if (program->binaries[j] == NULL) {
				program->binaries[j] = (unsigned char*)
					g_compute_checksum_for_string(
						G_CHECKSUM_SHA256, program->source, -1);
				program->binary_sizes[j] = strlen((const char*) program->binaries[j]);
			}
		}
		
	}
	
	ERROR:
	
	return status;

}

//~ CL_API_ENTRY cl_int CL_API_CALL
//~ clCompileProgram(cl_program program, cl_uint num_devices, 
	//~ const cl_device_id* device_list, const char* options,
	//~ cl_uint num_input_headers, const cl_program* input_headers,
	//~ const char** header_include_names,
	//~ void (CL_CALLBACK*)(cl_program, void*),
	//~ void* user_data) CL_API_SUFFIX__VERSION_1_2 {
//~ 
//~ }
//~ 
//~ CL_API_ENTRY cl_program CL_API_CALL
//~ clLinkProgram(cl_context context, cl_uint num_devices,
	//~ const cl_device_id* device_list, const char* options, 
	//~ cl_uint num_input_programs, const cl_program* input_programs,
	//~ void (CL_CALLBACK*)(cl_program program, void*),
	//~ void* user_data, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
//~ 
//~ }
//~ 
//~ CL_API_ENTRY cl_int CL_API_CALL
//~ clUnloadPlatformCompiler(cl_platform_id platform) CL_API_SUFFIX__VERSION_1_2 {
//~ 
//~ }

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program program, cl_program_info param_name,
	size_t param_value_size, void* param_value, 
	size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
		
	cl_int status = CL_SUCCESS;

	if (program == NULL) {
		status = CL_INVALID_PROGRAM;
	} else {
		switch (param_name) {
			case CL_PROGRAM_REFERENCE_COUNT:
				cl4_test_basic_info(cl_uint, program, ref_count);
			case CL_PROGRAM_CONTEXT:
				cl4_test_basic_info(cl_context, program, context);
			case CL_PROGRAM_NUM_DEVICES:
				cl4_test_basic_info(cl_uint, program, num_devices);
			case CL_PROGRAM_DEVICES:
				cl4_test_vector_info(cl_device_id, program, devices);
			case CL_PROGRAM_SOURCE:
				cl4_test_char_info(program, source);
			case CL_PROGRAM_BINARY_SIZES:
				cl4_test_vector_info(size_t, program, binary_sizes);
			case CL_PROGRAM_BINARIES:
				cl4_test_predefvector2d_info(unsigned char, program->num_devices, 
					program, binary_sizes, binaries);
			case CL_PROGRAM_NUM_KERNELS:
				cl4_test_basic_info(size_t, program, num_kernels);
			case CL_PROGRAM_KERNEL_NAMES:
				cl4_test_char_info(program, kernel_names);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;

		
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program program, cl_device_id device,
	cl_program_build_info param_name, size_t param_value_size,
	void* param_value, size_t* param_value_size_ret) 
	CL_API_SUFFIX__VERSION_1_0 {
		
	cl_int status = CL_SUCCESS;
	cl_bool found = CL_FALSE;
	cl_uint i;
	
	if (program == NULL) {
		status = CL_INVALID_PROGRAM;
	} else if (device == NULL) {
		status = CL_INVALID_DEVICE;
	} else {
		/* Find index of device in program. */
		for (i = 0; i < program->num_devices; ++i) {
			if (device == program->devices[i]) {
				found = CL_TRUE;
				break;
			}
		}
	}
		
	if (!found) {
		status = CL_INVALID_DEVICE;
	} else {
		
		/* Get info. */
		switch (param_name) {
			case CL_PROGRAM_BUILD_STATUS:
				cl4_test_basic_info(cl_build_status, program, build_status[i]);
			case CL_PROGRAM_BUILD_OPTIONS:
				cl4_test_char_info(program, build_options[i]);
			case CL_PROGRAM_BUILD_LOG:
				cl4_test_char_info(program, build_log[i]);
			case CL_PROGRAM_BINARY_TYPE:
				cl4_test_basic_info(cl_program_binary_type, program, binary_type[i]);
			default:
				status = CL_INVALID_VALUE;
		}
	}
		
	return status;

	

}
