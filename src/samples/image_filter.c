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
 * Sample code which demonstrates applying a filter to an image using
 * a convolution matrix.
 *
 * The first argument should be the image file to filter, and the second
 * (optional) argument can be the index of the device to use.
 *
 * The program will save the filtered image to file IMAGE_FILE in PNG
 * format.
 *
 * @note Requires OpenCL >= 1.1.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include <stdlib.h>
#include <stdio.h>
#include <cf4ocl2.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define IMAGE_FILE "out.png"

#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(-1); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }

#define FILTER_KERNEL "\
__constant float filter[9] = { \n\
	1.0f/9, 1.0f/9, 1.0f/9,\n\
	1.0f/9, 1.0f/9, 1.0f/9,\n\
	1.0f/9, 1.0f/9, 1.0f/9 };\n\
\n\
__constant int filter_size = 3;\n\
\n\
__kernel void do_filter(__read_only image2d_t input_img,\n\
	__write_only image2d_t output_img, sampler_t sampler) {\n\
\n\
	int2 imdim = get_image_dim(input_img);\n\
	size_t x = get_global_id(0);\n\
	size_t y = get_global_id(1);\n\
	if ((x < imdim.x) && (y < imdim.y)) {\n\
		int half_filter = filter_size / 2;\n\
		uint4 px_val;\n\
		float4 px_filt = { 0.0f, 0.0f, 0.0f, 0.0f };\n\
		uint4 px_filt_int;\n\
		int i, j, filter_i, filter_j;\n\
	\n\
		for(i = -half_filter, filter_i = 0; i <= half_filter; i++, filter_i++) {\n\
			for(j = -half_filter, filter_j = 0; j <= half_filter; j++, filter_j++) {\n\
				px_val = read_imageui(input_img, sampler, (int2) (x + i, y + j));\n\
				px_filt += filter[filter_i * filter_size + filter_j] * convert_float4(px_val);\n\
			}\n\
		}\n\
		px_filt_int = convert_uint4(px_filt);\n\
		write_imageui(output_img, (int2)(x, y), px_filt_int);\n\
	}\n\
}"

/**
 * Helper function which determines good global and local worksize
 * values for the given image dimensions and the given kernel and
 * device.
 * */
static void get_global_and_local_worksizes(CCLKernel* krnl,
	CCLDevice* dev, int width, int height, size_t* gws, size_t* lws) {

	/* The preferred workgroup size. */
	size_t wg_size_mult = 0;
	size_t wg_size_max;

	/* Error handling object. */
	GError* err = NULL;

	/* Determine maximum workgroup size. */
	wg_size_max = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_WORK_GROUP_SIZE, size_t, &err);
	HANDLE_ERROR(err);

#ifdef CL_VERSION_1_1
	/* Determine preferred workgroup size multiple (OpenCL >= 1.1). */
	wg_size_mult = ccl_kernel_get_workgroup_info_scalar(krnl, dev,
		CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, size_t, &err);
	HANDLE_ERROR(err);
#else
	wg_size_mult = wg_size_max;
#endif

	/* Try to find a mostly square local worksize. */
	lws[0] = wg_size_mult;
	lws[1] = wg_size_mult;
	while (lws[0] * lws[1] > wg_size_max) {
		lws[1] /= 2;
		if (lws[0] * lws[1] < wg_size_max) break;
		lws[0] /= 2;
	}

	/* Now get a global worksize which is a multiple of the local
	 * worksize and is big enough to handle the image dimensions. */
	gws[0] = ((width / lws[0]) + (width % lws[0])) * lws[0];
	gws[1] = ((height / lws[1]) + (height % lws[1])) * lws[1];

}

/**
 * Image fill main function.
 * */
int main(int argc, char* argv[]) {

	/* Wrappers for OpenCL objects. */
	CCLContext* ctx;
	CCLDevice* dev;
	CCLImage* img_in;
	CCLImage* img_out;
	CCLQueue* queue;
	CCLProgram* prg;
	CCLKernel* krnl;
	CCLSampler* smplr;

	/* Device selected specified in the command line. */
	int dev_idx = -1;

	/* Error handling object (must be initialized to NULL). */
	GError* err = NULL;

	/* Does selected device support images? */
	cl_bool image_ok;

	/* Image data in host. */
	unsigned char* input_image;
	unsigned char* output_image;

	/* Image properties. */
	int width, height, n_channels;

	/* Image file write status. */
	int file_write_status;

	/* Image parameters. */
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };

	/* Origin and region of complete image. */
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3];

	/* Global and local worksizes. */
	size_t gws[2];
	size_t lws[2];

	/* Check arguments. */
	if (argc < 2) {
		ERROR_MSG_AND_EXIT("Usage: image_filter <image_file> [device_index]");
	} else if (argc >= 3) {
		/* Check if a device was specified in the command line. */
		dev_idx = atoi(argv[2]);
	}

	/* Load image. */
	input_image = stbi_load(argv[1], &width, &height, &n_channels, 4);
	if (!input_image) ERROR_MSG_AND_EXIT(stbi_failure_reason());

	printf("\n * Image size: %d x %d, %d channels\n", width, height, n_channels);

	/* Set image region. */
	region[0] = width; region[1] = height; region[2] = 1;

	/* Create context using device selected from menu. */
	ctx = ccl_context_new_from_menu_full(&dev_idx, &err);
	HANDLE_ERROR(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	HANDLE_ERROR(err);

	/* Ask device if it supports images. */
	image_ok = ccl_device_get_info_scalar(
		dev, CL_DEVICE_IMAGE_SUPPORT, cl_bool, &err);
	HANDLE_ERROR(err);
	if (!image_ok)
		ERROR_MSG_AND_EXIT("Selected device doesn't support images.");

	/* Create a command queue. */
	queue = ccl_queue_new(ctx, dev, 0, &err);
	HANDLE_ERROR(err);

	/* Create 2D input image using loaded image data. */
	img_in = ccl_image_new(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, input_image, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) width,
		"image_height", (size_t) height,
		NULL);
	HANDLE_ERROR(err);

	/* Create 2D output image. */
	img_out = ccl_image_new(ctx, CL_MEM_WRITE_ONLY,
		&image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) width,
		"image_height", (size_t) height,
		NULL);
	HANDLE_ERROR(err);

	/* Create program from kernel source and compile it. */
	prg = ccl_program_new_from_source(ctx, FILTER_KERNEL, &err);
	HANDLE_ERROR(err);

	ccl_program_build(prg, NULL, &err);
	HANDLE_ERROR(err);

	/* Get kernel wrapper. */
	krnl = ccl_program_get_kernel(prg, "do_filter", &err);
	HANDLE_ERROR(err);

	/* Determine nice local and global worksizes. */
	get_global_and_local_worksizes(krnl, dev, width, height, gws, lws);
	printf(" * Global work-size: (%d, %d)\n", (int) gws[0], (int) gws[1]);
	printf(" * Local work-size: (%d, %d)\n", (int) lws[0], (int) lws[1]);

	/* Create sampler (this could also be created in-kernel). */
	smplr = ccl_sampler_new(ctx, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE,
		CL_FILTER_NEAREST, &err);
	HANDLE_ERROR(err);

	/* Apply filter. */
	ccl_kernel_set_args_and_enqueue_ndrange(
		krnl, queue, 2, NULL, gws, lws, NULL, &err,
		img_in, img_out, smplr, NULL);
	HANDLE_ERROR(err);

	/* Allocate space for output image. */
	output_image = (unsigned char*)
		malloc(width * height * 4 * sizeof(unsigned char));

	/* Read image data back to host. */
	ccl_image_enqueue_read(queue, img_out, CL_TRUE, origin, region,
		0, 0, output_image, NULL, &err);
	HANDLE_ERROR(err);

	/* Write image to file. */
	file_write_status = stbi_write_png(IMAGE_FILE, width, height, 4,
		output_image, width * 4);

	/* Give feedback. */
	if (file_write_status) {
		fprintf(stdout, "\nImage saved in file '" IMAGE_FILE "'\n");
	} else {
		ERROR_MSG_AND_EXIT("Unable to save image in file.");
	}

	/* Release host images. */
	free(output_image);
	stbi_image_free(input_image);

	/* Release wrappers. */
	ccl_image_destroy(img_in);
	ccl_image_destroy(img_out);
	ccl_sampler_destroy(smplr);
	ccl_program_destroy(prg);
	ccl_queue_destroy(queue);
	ccl_context_destroy(ctx);

	/* Check all wrappers have been destroyed. */
	g_assert(ccl_wrapper_memcheck());

	/* Terminate. */
	return 0;

}


