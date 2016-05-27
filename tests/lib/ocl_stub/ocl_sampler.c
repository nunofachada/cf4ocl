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
 * OpenCL sampler stub functions.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSamplerWithProperties(cl_context context,
	const cl_sampler_properties* sampler_properties,
	cl_int* errcode_ret) {

	/* Allocate sampler. */
	cl_sampler sampler = g_slice_new(struct _cl_sampler);
	sampler->ref_count = 1;

	/* Set context. */
	sampler->context = context;
	/* Set defaults (only in OpenCL 2.0). */
	sampler->normalized_coords = CL_TRUE;
	sampler->addressing_mode = CL_ADDRESS_CLAMP;
	sampler->filter_mode = CL_FILTER_NEAREST;
	/* Set defined properties: */
	if (sampler_properties != NULL) {
		cl_sampler_properties sp_key;
		cl_sampler_properties sp_value;
		for (guint i = 0; (sp_key = sampler_properties[i]) != 0; ++i) {
			i++;
			sp_value = sampler_properties[i];
			switch (sp_key) {
				case CL_SAMPLER_NORMALIZED_COORDS:
					sampler->normalized_coords = (cl_bool) sp_value; break;
				case CL_SAMPLER_ADDRESSING_MODE:
					sampler->addressing_mode = (cl_addressing_mode) sp_value; break;
				case CL_SAMPLER_FILTER_MODE:
					sampler->filter_mode = (cl_filter_mode) sp_value; break;
				default:
					seterrcode(errcode_ret, CL_INVALID_VALUE);
					clReleaseSampler(sampler);
					return NULL;
				/* Not checking if sp_value is valid property and not
				 * checking if same sp_key is given more than once. */
			}
		}
	}

	/* All good. */
	seterrcode(errcode_ret, CL_SUCCESS);
	return sampler;
}

CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context context, cl_bool normalized_coords,
	cl_addressing_mode addressing_mode, cl_filter_mode filter_mode,
	cl_int* errcode_ret) {

	const cl_sampler_properties sp[] = {
		CL_SAMPLER_NORMALIZED_COORDS, normalized_coords,
		CL_SAMPLER_ADDRESSING_MODE, addressing_mode,
		CL_SAMPLER_FILTER_MODE, filter_mode,
		0};

	return clCreateSamplerWithProperties(context, sp, errcode_ret);

}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler sampler) {

	g_atomic_int_inc(&sampler->ref_count);
	return CL_SUCCESS;

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler sampler) {

	/* Decrement reference count and check if it reaches 0. */
	if (g_atomic_int_dec_and_test(&sampler->ref_count)) {

		g_slice_free(struct _cl_sampler, sampler);

	}

	return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler sampler, cl_sampler_info param_name,
	size_t param_value_size, void* param_value,
	size_t* param_value_size_ret) {

	cl_int status = CL_SUCCESS;

	if (sampler == NULL) {
		status = CL_INVALID_SAMPLER;
	} else {
		switch (param_name) {

			case CL_SAMPLER_CONTEXT:
				ccl_test_basic_info(cl_context, sampler, context);
			case CL_SAMPLER_REFERENCE_COUNT:
				ccl_test_basic_info(cl_uint, sampler, ref_count);
			case CL_SAMPLER_NORMALIZED_COORDS:
				ccl_test_basic_info(cl_bool, sampler, normalized_coords);
			case CL_SAMPLER_ADDRESSING_MODE:
				ccl_test_basic_info(cl_addressing_mode, sampler, addressing_mode);
			case CL_SAMPLER_FILTER_MODE:
				ccl_test_basic_info(cl_filter_mode, sampler, filter_mode);
			default:
				status = CL_INVALID_VALUE;
		}
	}

	return status;

}

