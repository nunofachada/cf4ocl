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
 * File containing an image filtering kernel.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/*
 * This is the OpenCL kernel for the image filtering example image_filter.c.
 */

/* Filter to apply (a convolution matrix). */
__constant float filter[9] = {
    1.0f/9, 1.0f/9, 1.0f/9,
    1.0f/9, 1.0f/9, 1.0f/9,
    1.0f/9, 1.0f/9, 1.0f/9 };

/* Filter size (one dimension). */
__constant int filter_size = 3;

/**
 * Filter kernel.
 *
 * @param[in] input_img Input image.
 * @param[out] output_img Output image.
 * @param[in] sampler Sampler for reading image values.
 * */
__kernel void do_filter(__read_only image2d_t input_img,
    __write_only image2d_t output_img, sampler_t sampler) {

    int2 imdim = get_image_dim(input_img);
    int x = get_global_id(0);
    int y = get_global_id(1);

    if ((x < imdim.x) && (y < imdim.y)) {

        int half_filter = filter_size / 2;
        uint4 px_val;
        float4 px_filt = { 0.0f, 0.0f, 0.0f, 0.0f };
        uint4 px_filt_int;
        int i, j, filter_i, filter_j;

        for(i = -half_filter, filter_i = 0; i <= half_filter; i++, filter_i++) {
            for(j = -half_filter, filter_j = 0;
                j <= half_filter;
                j++, filter_j++) {

                px_val =
                    read_imageui(input_img, sampler, (int2) (x + i, y + j));
                px_filt +=
                    filter[filter_i * filter_size + filter_j]
                    * convert_float4(px_val);

            }
        }

        px_filt_int = convert_uint4(px_filt);
        write_imageui(output_img, (int2)(x, y), px_filt_int);

    }
}
