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
 * OpenCL context stub functions.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"

CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties * properties,
    cl_uint num_devices, const cl_device_id * devices,
    void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
    void * user_data, cl_int * errcode_ret) {

    /* Allocate memory for context. */
    cl_context ctx = g_slice_new(struct _cl_context);

    /* Copy properties to local context object. */
    if (properties != NULL) {
        for (ctx->prop_len = 0;
            properties[ctx->prop_len] != 0; ctx->prop_len++);
        ctx->prop_len++; /* Space for the last element: 0 */
        ctx->properties = g_slice_copy(
            ctx->prop_len * sizeof(cl_context_properties), properties);
    } else {
        ctx->properties = NULL;
    }
    ctx->devices =
        g_slice_copy(num_devices * sizeof(cl_device_id), devices);
    ctx->num_devices = num_devices;
    ctx->d3d = FALSE;
    ctx->ref_count = 1;
    (void)(pfn_notify);
    (void)(user_data);
    seterrcode(errcode_ret, CL_SUCCESS);

    return ctx;

}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) {

    /* Decrement reference count and check if it reaches 0. */
    if (g_atomic_int_dec_and_test(&context->ref_count)) {

        if (context->properties != NULL)
            g_slice_free1(context->prop_len * sizeof(cl_context_properties),
                context->properties);
        g_slice_free1(
            context->num_devices * sizeof(cl_device_id),
            context->devices);
        g_slice_free(struct _cl_context, context);

    }

    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(cl_context context) {

    g_atomic_int_inc(&context->ref_count);
    return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context context, cl_context_info param_name,
    size_t param_value_size, void * param_value,
    size_t * param_value_size_ret) {

    cl_int status = CL_SUCCESS;

    if (context == NULL) {
        status = CL_INVALID_CONTEXT;
    } else {
        switch (param_name) {

            case CL_CONTEXT_REFERENCE_COUNT:
                ccl_test_basic_info(cl_uint, context, ref_count);
#ifdef CL_VERSION_1_1
            case CL_CONTEXT_NUM_DEVICES:
                ccl_test_basic_info(cl_uint, context, num_devices);
#endif
            case CL_CONTEXT_DEVICES:
                ccl_test_predefvector_info(
                    cl_device_id, context->num_devices, context, devices);
            case CL_CONTEXT_PROPERTIES:
                ccl_test_vector_info(cl_context_properties, context, properties);
            //~ case CL_CONTEXT_D3D10_PREFER_SHARED_RESOURCES_KHR:
                //~ ccl_test_basic_info(cl_bool, context, d3d);
            //~ case CL_CONTEXT_D3D11_PREFER_SHARED_RESOURCES_KHR:
                //~ ccl_test_basic_info(cl_bool, context, d3d);
            default:
                status = CL_INVALID_VALUE;
        }
    }

    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context context, cl_mem_flags flags,
    cl_mem_object_type image_type, cl_uint num_entries,
    cl_image_format * image_formats, cl_uint * num_image_formats) {

    cl_int status = CL_SUCCESS;
    (void)(flags);
    (void)(image_type);

    if (context == NULL) {
        status = CL_INVALID_CONTEXT;
    } else if ((num_entries == 0) && (image_formats != NULL)) {
        status = CL_INVALID_VALUE;
    } else {
        if (image_formats != NULL) {
            g_memmove(image_formats,
                context->devices[0]->platform_id->image_formats,
                num_entries * sizeof(cl_image_format));
        }
        if (num_image_formats != NULL) {
            *num_image_formats =
                context->devices[0]->platform_id->num_image_formats;
        }
    }
    return status;
}
