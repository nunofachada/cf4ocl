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
 * Example which demonstrates image fills.
 *
 * @note Requires OpenCL >= 1.2.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

/*
 * Description
 * -----------
 *
 * This example demonstrates image fills. The program accepts the index of the
 * device to use as the first command-line argument.
 *
 * This example requires OpenCL >= 1.2.
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <cf4ocl2.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* Image properties. */
#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128
#define IMAGE_FILE "out.png"

/* Error handling macros. */
#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(EXIT_FAILURE); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }

#ifdef CL_VERSION_1_2

/**
 * Image fill main function.
 * */
int main(int argc, char* argv[]) {

	/* Wrappers for OpenCL objects. */
	CCLContext* ctx;
	CCLDevice* dev;
	CCLImage* img;
	CCLQueue* queue;

	/* Image file write status. */
	int file_write_status;

	/* Device selected specified in the command line. */
	int dev_idx = -1;

	/* Error handling object (must be initialized to NULL). */
	GError* err = NULL;

	/* Does selected device support images? */
	cl_bool image_ok;

	/* Image data in host. */
	cl_uchar4 base_color = {{255, 255, 255, 255}}; /* White. */
	cl_uchar4 img_host[IMAGE_WIDTH * IMAGE_HEIGHT];
	for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; ++i)
		img_host[i] = base_color;

	/* Image parameters. */
	cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };

	/* Origin and region of complete image. */
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { IMAGE_WIDTH, IMAGE_HEIGHT, 1};

	/* Origin and region for color 1. */
	size_t c1_origin[3] = { 0, 0, 0 };
	size_t c1_region[3] = {  IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 1 };
	cl_uint4 c1_color = {{ 255, 0, 0, 255 }}; /* This should be red. */

	/* Origin and region for color 2. */
	size_t c2_origin[3] = { IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2, 0 };
	size_t c2_region[3] = { IMAGE_WIDTH / 2,  IMAGE_HEIGHT / 2, 1};
	cl_uint4 c2_color = {{ 0, 255, 0, 255 }}; /* This should be green. */

	/* Check if a device was specified in the command line. */
	if (argc >= 2) {
		dev_idx = atoi(argv[1]);
	}

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

	/* Create 2D image. */
	img = ccl_image_new(ctx, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, img_host, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) IMAGE_WIDTH,
		"image_height", (size_t) IMAGE_HEIGHT,
		NULL);
	HANDLE_ERROR(err);

	/* Fill image with color 1. */
	ccl_image_enqueue_fill(img, queue, &c1_color, c1_origin, c1_region,
		NULL, &err);
	HANDLE_ERROR(err);

	/* Fill image with color 2. */
	ccl_image_enqueue_fill(img, queue, &c2_color, c2_origin, c2_region,
		NULL, &err);
	HANDLE_ERROR(err);

	/* Read image data back to host. */
	ccl_image_enqueue_read(img, queue, CL_TRUE, origin, region,
		0, 0, img_host, NULL, &err);
	HANDLE_ERROR(err);

	/* Write image to file. */
	file_write_status = stbi_write_png(IMAGE_FILE, IMAGE_WIDTH,
		IMAGE_HEIGHT, 4, img_host, IMAGE_WIDTH * 4);

	/* Give feedback. */
	if (file_write_status) {
		fprintf(stdout, "\nImage saved in file '" IMAGE_FILE "'\n");
	} else {
		ERROR_MSG_AND_EXIT("Unable to save image in file.");
	}

	/* Release wrappers. */
	ccl_image_destroy(img);
	ccl_queue_destroy(queue);
	ccl_context_destroy(ctx);

	/* Check all wrappers have been destroyed. */
	g_assert(ccl_wrapper_memcheck());

	/* Terminate. */
	return EXIT_SUCCESS;

}

#else
int main() {
	fprintf(stderr, "This sample requires OpenCL 1.2\n");
	return EXIT_FAILURE;
}
#endif

