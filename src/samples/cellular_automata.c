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
 * Sample code which runs a cellular automata simulation (Conway's
 * Game of Life) in OpenCL using _cf4ocl_. This code demonstrates the
 * use of images, several command queues and profiling.
 *
 * A series of images will be saved in the folder where this program
 * runs.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cf4ocl2.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(-1); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }


#define IMAGE_FILE_PREFIX "out"
#define IMAGE_FILE_NUM_DIGITS 5

#define CA_WIDTH 128
#define CA_HEIGHT 128
#define CA_ITERS 12

#define CA_KERNEL "\
\n\
__constant uint2 neighbors[] = { \n\
	(uint2) (-1,-1), (uint2) (0,-1), (uint2) (1,-1), (uint2) (1,0), \n\
	(uint2) (1,1), (uint2) (0,1), (uint2) (-1,1), (uint2) (-1,0)};\n\
\n\
__constant uint2 live_rule = (uint2) (2, 3);\n\
__constant uint2 dead_rule = (uint2) (3, 3);\n\
\n\
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST; \n\
\n\
__kernel void ca(__read_only image2d_t in_img, __write_only image2d_t out_img) {\n\
\n\
	uint2 imdim = convert_uint2(get_image_dim(in_img));\n\
	size_t x = get_global_id(0);\n\
	size_t y = get_global_id(1);\n\
	if ((x < imdim.x) && (y < imdim.y)) {\n\
		uint4 neighs_state = { 0, 0, 0, 0 };\n\
		uint neighs_alive;\n\
		uint4 state;\n\
		uint alive;\n\
		uint4 new_state = {0, 0, 0, 1};\n\
		for(int i = 0; i < 8; ++i) {\n\
			uint2 n = ((uint2) (x, y)) + neighbors[i]; \n\
			n = select(n, n - imdim, n >= imdim); \n\
			neighs_state += read_imageui(in_img, sampler, convert_int2(n));\n\
		}\n\
		neighs_alive = neighs_state.x / 0xFF;\n\
		state = read_imageui(in_img, sampler, (int2) (x, y));\n\
		alive = state.x / 0xFF;\n\
		if ((alive & ((neighs_alive < live_rule.s0) || (neighs_alive > live_rule.s1))) \n\
			|| (!alive & (neighs_alive >= dead_rule.s0) && (neighs_alive <= dead_rule.s1))) {\n\
			new_state.x = 0xFF;\n\
		}\n\
		write_imageui(out_img, (int2) (x, y), new_state);\n\
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
	CCLImage* img1;
	CCLImage* img2;
	CCLImage* img_aux;
	CCLQueue* queue_exec;
	CCLQueue* queue_comm;
	CCLProgram* prg;
	CCLKernel* krnl;
	CCLEvent* evt_iter;

	CCLEventWaitList ewl = NULL;

	char* filename;

	/* Device selected specified in the command line. */
	int dev_idx = -1;

	/* Error handling object (must be initialized to NULL). */
	GError* err = NULL;

	/* Does selected device support images? */
	cl_bool image_ok;

	/* Image data in host. */
	unsigned char* input_image;
	unsigned char** output_images;

	/* RNG seed. */
	unsigned int seed;

	/* Image file write status. */
	int file_write_status;

	/* Image parameters. */
	cl_image_format image_format = { CL_R, CL_UNSIGNED_INT8 };

	/* Origin and region of complete image. */
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { CA_WIDTH, CA_HEIGHT, 1 };

	/* Global and local worksizes. */
	size_t gws[2];
	size_t lws[2];

	/* Check arguments. */
	if (argc >= 2) {
		/* Check if a device was specified in the command line. */
		dev_idx = atoi(argv[1]);
	}
	if (argc >= 3) {
		/* Check if a RNG seed was specified. */
		seed = atoi(argv[2]);
	} else {
		seed = (unsigned int) time(NULL);
	}

	/* Initialize RNG. */
	srand(seed);

	/* Create random initial state. */
	input_image = (unsigned char*)
		malloc(CA_WIDTH * CA_HEIGHT * sizeof(unsigned char));
	for (cl_uint i = 0; i < CA_WIDTH * CA_HEIGHT; ++i)
		input_image[i] = rand() & 0x1 ? 0xFF : 0x0;

	/* Allocate space for simulation results. */
	output_images = (unsigned char**)
		malloc((CA_ITERS + 1) * sizeof(unsigned char*));
	for (cl_uint i = 0; i < CA_WIDTH * CA_HEIGHT; ++i)
		output_images[i] = (unsigned char*)
			malloc(CA_WIDTH * CA_HEIGHT * sizeof(unsigned char));

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

	/* Create command queues. */
	queue_exec = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	HANDLE_ERROR(err);
	queue_comm = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	HANDLE_ERROR(err);

	/* Create 2D image using random CA data. */
	img1 = ccl_image_new(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		&image_format, input_image, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CA_WIDTH,
		"image_height", (size_t) CA_HEIGHT,
		NULL);
	HANDLE_ERROR(err);

	/* Create another 2D image for double buffering. */
	img2 = ccl_image_new(ctx, CL_MEM_READ_WRITE,
		&image_format, NULL, &err,
		"image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
		"image_width", (size_t) CA_WIDTH,
		"image_height", (size_t) CA_HEIGHT,
		NULL);
	HANDLE_ERROR(err);

	/* Create program from kernel source and compile it. */
	prg = ccl_program_new_from_source(ctx, CA_KERNEL, &err);
	HANDLE_ERROR(err);

	ccl_program_build(prg, NULL, &err);

	if (err != NULL) {
		char* build_log = ccl_program_get_build_info_array(prg, dev, CL_PROGRAM_BUILD_LOG, char*, NULL);
		fprintf(stderr, "\n***** BUILD LOG *****\n\n%s\n\n*********************", build_log);
	}
	HANDLE_ERROR(err);

	/* Get kernel wrapper. */
	krnl = ccl_program_get_kernel(prg, "ca", &err);
	HANDLE_ERROR(err);

	/* Determine nice local and global worksizes. */
	get_global_and_local_worksizes(krnl, dev, CA_WIDTH, CA_HEIGHT, gws, lws);
	printf(" * Global work-size: (%d, %d)\n", (int) gws[0], (int) gws[1]);
	printf(" * Local work-size: (%d, %d)\n", (int) lws[0], (int) lws[1]);

	//~ /* Run CA_ITERS iterations of the CA. */
	//~ for (cl_uint i = 0; i < CA_ITERS; ++i) {
//~
		//~ /* Read result of last iteration. On first run it the initial
		 //~ * state. */
		//~ ccl_image_enqueue_read(queue_comm, img1, CL_FALSE,
			//~ origin, region, 0, 0, output_images[i], &ewl, &err);
		//~ HANDLE_ERROR(err);
//~
		//~ /* Process iteration. */
		//~ evt_iter = ccl_kernel_set_args_and_enqueue_ndrange(
			//~ krnl, queue_exec, 2, NULL, gws, lws, NULL, &err,
			//~ img1, img2, NULL);
		//~ HANDLE_ERROR(err);
//~
		//~ /* Can't start new read until this iteration is over. */
		//~ ccl_event_wait_list_add(&ewl, evt_iter);
//~
		//~ /* Swap buffers. */
		//~ img_aux = img1;
		//~ img1 = img2;
		//~ img2 = img_aux;
//~
	//~ }

	/* Read result of last iteration. */
	ccl_image_enqueue_read(queue_comm, img1, CL_TRUE,
		origin, region, 0, 0, output_images[CA_ITERS], &ewl, &err);
	HANDLE_ERROR(err);

	//~ filename = (char*) malloc(
		//~ (strlen(IMAGE_FILE_PREFIX ".png") + IMAGE_FILE_NUM_DIGITS + 1) * sizeof(char));
	//~ /* Write results to image files. */
	//~ for (cl_uint i = 0; i < CA_ITERS; ++i) {
//~
		//~ sprintf(filename, "%s%0" G_STRINGIFY(IMAGE_FILE_NUM_DIGITS) "d.png", IMAGE_FILE_PREFIX, i);
//~
		//~ file_write_status = stbi_write_png(filename, CA_WIDTH, CA_HEIGHT, 1,
			//~ output_images[i], CA_WIDTH);
//~
		//~ /* Give feedback. */
		//~ if (!file_write_status) {
			//~ ERROR_MSG_AND_EXIT("Unable to save image in file.");
		//~ }
	//~ }
//~
//~
	//~ /* Release host buffers. */
	//~ free(filename);
	free(input_image);
	//~ for (cl_uint i = 0; i < CA_WIDTH * CA_HEIGHT; ++i)
		//~ free(output_images[i]);
	//~ free(output_images);

	/* Release wrappers. */
	ccl_image_destroy(img1);
	ccl_image_destroy(img2);
	ccl_program_destroy(prg);
	ccl_queue_destroy(queue_comm);
	ccl_queue_destroy(queue_exec);
	ccl_device_destroy(dev);
	ccl_context_destroy(ctx);

	/* Terminate. */
	return 0;

}


