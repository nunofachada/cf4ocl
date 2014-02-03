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
 * @brief Basic program to query OpenCL devices about their features.
 * 
 * If any argument is passed, more detailed info is given.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include "clutils.h"

/** Maximum number of devices to query. */
#define MAX_DEVICES_QUERY 10

/** Maximum size of information string to get with each query. */
#define MAX_INFO_STRING 250

/**
 * @brief Main program function.
 * 
 * @param argc Number of cli parameters. If argc > 1, more detailed information will be shown.
 * @param argv Not relevant.
 * @return 
 */
int main(int argc, char ** argv) {
	
	/* Program variables. */
	GError* err = NULL;                      /* Error reporting object. */
	cl_int status;                           /* Program/function return status variable. */
	cl_uint numPlatforms;                    /* Number of platforms. */
	cl_platform_id* platforms = NULL;        /* Array of platform IDs. */
	cl_uint numDevices;                      /* Number of devices. */
	cl_device_id devices[MAX_DEVICES_QUERY]; /* Array of devices for a given platform. */

	/* Auxiliary variables for getting information about platforms and devices. */
	char pbuff[MAX_INFO_STRING];
	size_t sizetaux;
	cl_uint uintaux;
	cl_ulong ulongaux, ulongaux2;
	cl_device_type dtypeaux;
	cl_device_local_mem_type dlmt;
	cl_bool boolaux;
	cl_command_queue_properties cqpaux;
	
	/* Avoid compiler warning. */
	argv = argv;

	/* Get number of platforms. */
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get number of platforms.", status);
	
	/* Get platform IDs. */
	platforms = (cl_platform_id*) malloc(numPlatforms * sizeof(cl_platform_id));
	gef_if_error_create_goto(err, CLU_UTILS_ERROR, platforms == NULL, CLU_ERROR_NOALLOC, error_handler, "Unable to allocate memory for list of platform IDs.");

	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get list of platform IDs.", status);
	
	/* Print number of platforms. */
	printf("Number of platforms: %d\n", numPlatforms);

	/* Cycle through platforms */
	for(unsigned int i = 0; i < numPlatforms; i++) {
	
		/* Get platform vendor. */
		status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(pbuff), pbuff, NULL);
		gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get platform vendor.", status);
		
		/* Print plaform vendor. */
		printf("Platform #%d: %s\n", i, pbuff);
		
		/* Get devices in platform */
		status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, MAX_DEVICES_QUERY, devices, &numDevices);
		gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to platform devices.", status);

		/* Cycle through devices in current platform. */
		for (unsigned int j = 0; j < numDevices; j++) {
			
			/* Device name. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(pbuff), pbuff, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device name.", status);

			printf("\tDevice #%d: %s\n", j, pbuff);
			
			/* Device vendor. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(pbuff), pbuff, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device vendor.", status);

			printf("\t           Vendor: %s\n", pbuff);

			/* Device type. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &dtypeaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device type.", status);

			printf("\t           Type: %s\n", clu_device_type_str_get(dtypeaux, 0, pbuff, MAX_INFO_STRING));

			/* OpenCL C version. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, sizeof(pbuff), pbuff, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device OpenCL C version.", status);
			
			printf("\t           %s\n", pbuff);
			
			/* Max. compute units. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uintaux), &uintaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device max. compute units.", status);

			printf("\t           Max. Compute units: %d\n", uintaux);

			/* Global memory info. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulongaux), &ulongaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device global memory size.", status);

			status = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(ulongaux2), &ulongaux2, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device maximum allocable memory.", status);

			status = clGetDeviceInfo(devices[j], CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(boolaux), &boolaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get determine if device memory is unified with host.", status);

			printf("\t           Global mem. size: %ld Mb %s (max. alloc. %ld Mb)\n", (unsigned long int) ulongaux / 1024l / 1024l, boolaux ? "shared with host" : "dedicated", (unsigned long int) ulongaux2 / 1024l / 1024l);

			/* Local memory info. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(dlmt), &dlmt, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get type of local memory in device.", status);

			status = clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ulongaux), &ulongaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get size of local memory in device.", status);

			printf("\t           Local mem. size (type): %ld Kb (%s)\n", (unsigned long int) ulongaux / 1024l, (dlmt == CL_LOCAL ? "local" : "global"));

			/* Maximum work group size. */
			status = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(sizetaux), &sizetaux, NULL);
			gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get size of local memory in device.", status);

			printf("\t           Max. work-group size: %d\n", (int) sizetaux);

			/* Print extra info if any arg is given */
			if (argc > 1) {

				/* Maximum constant buffer size. */
				status = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(ulongaux), &ulongaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get maximum constant buffer size.", status);
				
				printf("\t           Max. constant buffer size: %lu Kb\n", (unsigned long) (ulongaux / 1024));

				/* Device endianess.*/
				status = clGetDeviceInfo(devices[j], CL_DEVICE_ENDIAN_LITTLE, sizeof(boolaux), &boolaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get device endianess.", status);

				printf("\t           Endianness: %s\n", boolaux ? "Little" : "Big");
				
				/* Preferred vector width.  */
				printf("\t           Pref. vec. width:");

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for char.", status);

				printf(" Char=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for short.", status);

				printf(" Short=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for int.", status);

				printf(" Int=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for long.", status);

				printf(" Long=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for float.", status);

				printf(" Float=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for double.", status);

				printf(" Double=%d,", uintaux);

				status = clGetDeviceInfo(devices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(uintaux), &uintaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get preferred vector width for half.", status);

				printf(" Half=%d.\n", uintaux);
				
				/* Acceptable command queue properties. */
				status = clGetDeviceInfo(devices[j], CL_DEVICE_QUEUE_PROPERTIES, sizeof(cqpaux), &cqpaux, NULL);
				gef_if_error_create_goto(err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "OpenCL error %d: unable to get acceptable command queue properties.", status);
				
				printf("\t           Command queue properties:");
				if (cqpaux & CL_QUEUE_PROFILING_ENABLE) printf(" Prof. OK,"); else printf("Prof. KO,");
				if (cqpaux & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) printf(" Out-of-order OK\n"); else printf(" Out-of-order KO\n");

			}
		}
	}
	
	/* If we get here, no need for error checking, jump to cleanup. */
	g_assert (err == NULL);
	status = CL_SUCCESS;
	goto cleanup;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err != NULL);
	fprintf(stderr, "%s", err->message);
	status = err->code;
	g_error_free(err);

cleanup:
		
	/* Free stuff! */
	if (platforms) free(platforms);

	/* Return status. */
	return status;

}

