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
 * use of double-buffering with images, multiple command queues and
 * profiling.
 *
 * A series of images will be saved in the folder where this program
 * runs.
 *
 * The program accepts two command-line arguments:
 *
 * 1. Device index
 * 2. RNG seed
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
#define CA_ITERS 64

#define CA_KERNEL "\
\n\
__constant int2 neighbors[] = { \n\
	(int2) (-1,-1), (int2) (0,-1), (int2) (1,-1), (int2) (1,0), \n\
	(int2) (1,1), (int2) (0,1), (int2) (-1,1), (int2) (-1,0)};\n\
\n\
__constant uint2 live_rule = (uint2) (2, 3);\n\
__constant uint2 dead_rule = (uint2) (3, 3);\n\
\n\
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST; \n\
\n\
__kernel void ca(__read_only image2d_t in_img, __write_only image2d_t out_img) {\n\
\n\
	int2 imdim = get_image_dim(in_img);\n\
	int2 coord = (int2) (get_global_id(0), get_global_id(1));\n\
	if (all(coord < imdim)) {\n\
		uint4 neighs_state;\n\
		uint neighs_alive = 0;\n\
		uint4 state;\n\
		uint alive;\n\
		uint4 new_state = { 0xFF, 0, 0, 1};\n\
		for(int i = 0; i < 8; ++i) {\n\
			int2 n = coord + neighbors[i]; \n\
			n = select(n, n - imdim, n >= imdim); \n\
			n = select(n, imdim - 1, n < 0); \n\
			neighs_state = read_imageui(in_img, sampler, n);\n\
			if (neighs_state.x == 0x0) neighs_alive++; \n\
		}\n\
		state = read_imageui(in_img, sampler, coord);\n\
		alive = (state.x == 0x0);\n\
		if ((alive && (neighs_alive >= live_rule.s0) && (neighs_alive <= live_rule.s1)) \n\
			|| (!alive && (neighs_alive >= dead_rule.s0) && (neighs_alive <= dead_rule.s1))) {\n\
			new_state.x = 0x00;\n\
		}\n\
		write_imageui(out_img, coord, new_state);\n\
	}\n\
}"

/**
 * Cellular automata sample main function.
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
	CCLEvent* evt_comm;
	CCLEvent* evt_exec;
	/* Other variables. */
	CCLEventWaitList ewl = NULL;
	/* Profiler object. */
	CCLProf* prof;
	/* Output images filename. */
	char* filename;
	/* Selected device, may be given in command line. */
	int dev_idx = -1;
	/* Error handling object (must be NULL). */
	GError* err = NULL;
	/* Does selected device support images? */
	cl_bool image_ok;
	/* Initial sim state. */
	unsigned char* input_image;
	/* Simulation states. */
	unsigned char** output_images;
	/* RNG seed, may be given in command line. */
	unsigned int seed;
	/* Image file write status. */
	int file_write_status;
	/* Image format. */
	cl_image_format image_format = { CL_R, CL_UNSIGNED_INT8 };
	/* Origin of sim space. */
	size_t origin[3] = { 0, 0, 0 };
	/* Region of sim space. */
	size_t region[3] = { CA_WIDTH, CA_HEIGHT, 1 };
	/* Real worksize. */
	size_t real_ws[] = { CA_WIDTH, CA_HEIGHT };
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
		input_image[i] = (rand() & 0x3) ? 0xFF : 0x00;

	/* Allocate space for simulation results. */
	output_images = (unsigned char**)
		malloc((CA_ITERS + 1) * sizeof(unsigned char*));
	for (cl_uint i = 0; i < CA_ITERS + 1; ++i)
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
	img1 = ccl_image_new(ctx, CL_MEM_READ_WRITE,
		&image_format, NULL, &err,
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
	HANDLE_ERROR(err);

	/* Get kernel wrapper. */
	krnl = ccl_program_get_kernel(prg, "ca", &err);
	HANDLE_ERROR(err);

	/* Determine nice local and global worksizes. */
	ccl_kernel_suggest_worksizes(krnl, dev, 2, real_ws, gws, lws, &err);
	HANDLE_ERROR(err);

	printf("\n * Global work-size: (%d, %d)\n", (int) gws[0], (int) gws[1]);
	printf(" * Local work-size: (%d, %d)\n", (int) lws[0], (int) lws[1]);

	/* Start profiling. */
	prof = ccl_prof_new();
	ccl_prof_start(prof);

	/* Write initial state. */
	ccl_image_enqueue_write(queue_comm, img1, CL_TRUE,
		origin, region, 0, 0, input_image, NULL, &err);
	HANDLE_ERROR(err);

	/* Run CA_ITERS iterations of the CA. */
	for (cl_uint i = 0; i < CA_ITERS; ++i) {

		/* Read result of last iteration. On first run it the initial
		 * state. */
		evt_comm = ccl_image_enqueue_read(queue_comm, img1, CL_FALSE,
			origin, region, 0, 0, output_images[i], NULL, &err);
		HANDLE_ERROR(err);

		/* Process iteration. */
		evt_exec = ccl_kernel_set_args_and_enqueue_ndrange(
			krnl, queue_exec, 2, NULL, gws, lws, NULL, &err,
			img1, img2, NULL);
		HANDLE_ERROR(err);

		/* Can't start new read until this iteration is over. */
		ccl_event_wait_list_add(&ewl, evt_comm);
		ccl_event_wait_list_add(&ewl, evt_exec);

		/* Wait for*/
		ccl_event_wait(&ewl, &err);
		HANDLE_ERROR(err);

		/* Swap buffers. */
		img_aux = img1;
		img1 = img2;
		img2 = img_aux;

	}

	/* Read result of last iteration. */
	ccl_image_enqueue_read(queue_comm, img1, CL_TRUE,
		origin, region, 0, 0, output_images[CA_ITERS], &ewl, &err);
	HANDLE_ERROR(err);

	/* Stop profiling timer and add queues for analysis. */
	ccl_prof_stop(prof);
	ccl_prof_add_queue(prof, "Comms", queue_comm);
	ccl_prof_add_queue(prof, "Exec", queue_exec);

	/* Allocate space for base filename. */
	filename = (char*) malloc(
		(strlen(IMAGE_FILE_PREFIX ".png") + IMAGE_FILE_NUM_DIGITS + 1) * sizeof(char));

	/* Write results to image files. */
	for (cl_uint i = 0; i < CA_ITERS; ++i) {

		/* Determine next filename. */
		sprintf(filename, "%s%0" G_STRINGIFY(IMAGE_FILE_NUM_DIGITS) "d.png", IMAGE_FILE_PREFIX, i);

		/* Save next image. */
		file_write_status = stbi_write_png(filename, CA_WIDTH, CA_HEIGHT, 1,
			output_images[i], CA_WIDTH);

		/* Give feedback if unable to save image. */
		if (!file_write_status) {
			ERROR_MSG_AND_EXIT("Unable to save image in file.");
		}
	}

	/* Process profiling info. */
	ccl_prof_calc(prof, &err);
	HANDLE_ERROR(err);

	/* Print profiling info. */
	ccl_prof_print_summary(prof);

	/* Save profiling info. */
	ccl_prof_export_info_file(prof, IMAGE_FILE_PREFIX ".tsv", &err);
	HANDLE_ERROR(err);

	/* Release host buffers. */
	free(filename);
	free(input_image);
	for (cl_uint i = 0; i < CA_ITERS + 1; ++i)
		free(output_images[i]);
	free(output_images);

	/* Release wrappers. */
	ccl_image_destroy(img1);
	ccl_image_destroy(img2);
	ccl_program_destroy(prg);
	ccl_queue_destroy(queue_comm);
	ccl_queue_destroy(queue_exec);
	ccl_context_destroy(ctx);

	/* Destroy profiler. */
	ccl_prof_destroy(prof);

	/* Check all wrappers have been destroyed. */
	g_assert(ccl_wrapper_memcheck());

	/* Terminate. */
	return 0;

}


