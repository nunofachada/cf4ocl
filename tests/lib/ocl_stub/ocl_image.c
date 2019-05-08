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
 * OpenCL image stub functions.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "ocl_env.h"
#include "utils.h"

/* This function only works with the main image formats. */
static size_t image_elem_size(cl_image_format image_format) {

    size_t num_channels;
    size_t channel_size;

    switch (image_format.image_channel_data_type) {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            channel_size = 1; break;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
        case CL_HALF_FLOAT:
            channel_size = 2; break;
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            channel_size = 4; break;
        case CL_UNORM_SHORT_565:
            return 2;
        case CL_UNORM_SHORT_555:
            return 2;
        case CL_UNORM_INT_101010:
            return 4;
        default:
            return 0;
    }

    switch (image_format.image_channel_order) {
        case CL_R:
#ifdef CL_VERSION_1_1
        case CL_Rx:
#endif
        case CL_A:
        case CL_INTENSITY:
        case CL_LUMINANCE:
            num_channels = 1; break;
        case CL_RG:
#ifdef CL_VERSION_1_1
        case CL_RGx:
#endif
        case CL_RA:
            num_channels = 2; break;
        case CL_RGBA:
         case CL_ARGB:
        case CL_BGRA:
            num_channels = 4; break;
        default:
            return 0;
    }

    return channel_size * num_channels;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage(cl_context context, cl_mem_flags flags,
    const cl_image_format* image_format,
    const cl_image_desc* image_desc, void* host_ptr,
    cl_int* errcode_ret) {

    /* Very basic, only support 2D and 3D images and no image-buffer
     * auto mapping.*/

    cl_mem image = NULL;

    /* Lots of errors are not checked! */
    if (image_format == NULL) {
        seterrcode(errcode_ret, CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
#ifdef CL_VERSION_1_2
    } else if (image_desc == NULL) {
        seterrcode(errcode_ret, CL_INVALID_IMAGE_DESCRIPTOR);
#endif
    } else { /* No error, create image. */

        /* Determine image size. */
        size_t size;
        switch (image_desc->image_type) {
            //~ case CL_MEM_OBJECT_IMAGE1D:
                //~ size = image_desc->image_row_pitch;
                //~ break;
            //~ case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            //~ case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            case CL_MEM_OBJECT_IMAGE2D:
                size = image_desc->image_width *
                    image_desc->image_height *
                    image_elem_size(*image_format);
                ((cl_image_desc*) image_desc)->image_depth = 1;
                break;
            //~ case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            case CL_MEM_OBJECT_IMAGE3D:
                size = image_desc->image_width *
                    image_desc->image_height * image_desc->image_depth *
                    image_elem_size(*image_format);
                break;
            default:
                seterrcode(errcode_ret, CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
                return NULL;
        }
        image = clCreateBuffer(
            context, flags, size, host_ptr, errcode_ret);
        image->image_elem_size = image_elem_size(*image_format);
        image->image_format = *image_format;
        image->image_desc = *image_desc;
        image->type = image_desc->image_type;
    }
    return image;
}

CL_API_ENTRY cl_mem CL_API_CALL clCreateImage2D(cl_context context,
    cl_mem_flags flags, const cl_image_format* image_format,
    size_t image_width, size_t image_height, size_t image_row_pitch,
    void* host_ptr, cl_int* errcode_ret) {

#ifdef CL_VERSION_2_0
    cl_image_desc img_dsc = {0, 0, 0, 0, 0, 0, 0, 0, 0, {.buffer = NULL}};
#else
    cl_image_desc img_dsc = {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
#endif
    img_dsc.image_type = CL_MEM_OBJECT_IMAGE2D;
    img_dsc.image_width = image_width;
    img_dsc.image_height = image_height;
    img_dsc.image_row_pitch = image_row_pitch;
    return clCreateImage(context, flags, image_format, &img_dsc,
        host_ptr, errcode_ret);

}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage3D(cl_context context, cl_mem_flags flags,
    const cl_image_format* image_format, size_t image_width,
    size_t image_height, size_t image_depth, size_t image_row_pitch,
    size_t image_slice_pitch, void* host_ptr, cl_int* errcode_ret) {

#ifdef CL_VERSION_2_0
    cl_image_desc img_dsc = {0, 0, 0, 0, 0, 0, 0, 0, 0, {.buffer = NULL}};
#else
    cl_image_desc img_dsc = {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
#endif
    img_dsc.image_type = CL_MEM_OBJECT_IMAGE3D;
    img_dsc.image_width = image_width;
    img_dsc.image_height = image_height;
    img_dsc.image_depth = image_depth;
    img_dsc.image_row_pitch = image_row_pitch;
    img_dsc.image_slice_pitch = image_slice_pitch;
    return clCreateImage(context, flags, image_format, &img_dsc,
        host_ptr, errcode_ret);

}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem image, cl_image_info param_name,
    size_t param_value_size, void* param_value,
    size_t* param_value_size_ret) {

    cl_int status = CL_SUCCESS;

    if (image == NULL) {
        status = CL_INVALID_MEM_OBJECT;
    } else {
        switch (param_name) {
            case CL_IMAGE_FORMAT:
                ccl_test_basic_info(cl_image_format, image, image_format);
            case CL_IMAGE_ELEMENT_SIZE:
                ccl_test_basic_info(size_t, image, image_elem_size);
            case CL_IMAGE_ROW_PITCH:
                ccl_test_basic_info(size_t, image, image_desc.image_row_pitch);
            case CL_IMAGE_SLICE_PITCH:
                ccl_test_basic_info(size_t, image, image_desc.image_slice_pitch);
            case CL_IMAGE_WIDTH:
                ccl_test_basic_info(size_t, image, image_desc.image_width);
            case CL_IMAGE_HEIGHT:
                ccl_test_basic_info(size_t, image, image_desc.image_height);
            case CL_IMAGE_DEPTH:
                ccl_test_basic_info(size_t, image, image_desc.image_array_size);
#ifdef CL_VERSION_1_2
            case CL_IMAGE_ARRAY_SIZE:
                ccl_test_basic_info(size_t, image, image_desc.image_depth);
            case CL_IMAGE_NUM_MIP_LEVELS:
                ccl_test_basic_info(cl_uint, image, image_desc.num_mip_levels);
            case CL_IMAGE_NUM_SAMPLES:
                ccl_test_basic_info(cl_uint, image, image_desc.num_samples);
#endif
            default:
                status = CL_INVALID_VALUE;
        }
    }

    return status;
}
