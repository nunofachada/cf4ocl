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
 * Example which demonstrates applying a filter to an image using a
 * convolution matrix.
 *
 * @note Requires OpenCL >= 1.1.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/*
 * Description
 * -----------
 *
 * Example which demonstrates applying a filter to an image using a
 * convolution matrix.
 *
 * The first argument should be the image file to filter, and the second
 * (optional) argument can be the index of the device to use.
 *
 * The program will save the filtered image to file IMAGE_FILE in PNG
 * format.
 *
 * This example requires OpenCL >= 1.1.
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <cf4ocl2.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Kernel source string, will be hardwired in this location during the build
 * process, before compilation. The kernel source is available in
 * image_filter.cl. */
#define FILTER_KERNEL \
@image_filter_KERNEL_SRC@

/* Output image name. */
#define IMAGE_FILE "out.png"

/* Error handling macros. */
#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(EXIT_FAILURE); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }

/**
 * Image filter main function.
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

	/* Real worksize. */
	size_t real_ws[2];

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

	printf("\n * Image size: %d x %d, %d channels\n",
		width, height, n_channels);
	real_ws[0] = width; real_ws[1] = height;

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
	ccl_kernel_suggest_worksizes(krnl, dev, 2, real_ws, gws, lws, &err);
	HANDLE_ERROR(err);

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
	ccl_image_enqueue_read(img_out, queue, CL_TRUE, origin, region,
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
	assert(ccl_wrapper_memcheck());

	/* Terminate. */
	return EXIT_SUCCESS;

}


