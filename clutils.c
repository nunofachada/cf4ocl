/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with cf4ocl. If not, see 
 * <http://www.gnu.org/licenses/>.
 * */
 
/** 
 * @file
 * @brief OpenCL utilities function implementations.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "clutils.h"

/** 
 * @brief Private helper function, prints a list of available devices. 
 * 
 * @param devInfos List of device information.
 * @param numDevices Number of devices on list.
 * */
static void clu_menu_device_selector_list(CLUDeviceInfo* devInfos, cl_uint numDevices, cl_uint selected) {
	char* selectedStr;
	printf("\n   =========================== Device Selection ============================\n\n");
	for (cl_uint i = 0; i < numDevices; i++) {
		selectedStr = "            ";
		if (i == selected) {
			selectedStr = "  [SELECTED]";
		}
		printf(" %s %d. %s\n                 %s\n", selectedStr, i, devInfos[i].device_name, devInfos[i].platform_name);
	}
}

/** 
 * @brief Private helper function, asks the user to select a device 
 * from a list. 
 * 
 * @param devInfos List of device information.
 * @param numDevices Number of devices on list.
 * @return The list index of the selected device.
 * */
static cl_uint clu_menu_device_selector_query(CLUDeviceInfo* devInfos, cl_uint numDevices) {
	
	/* Index of selected device. */
	cl_int index = -1;
	/* Number of results read from user input. */
	int result;
	
	/* Print available devices */
	clu_menu_device_selector_list(devInfos, numDevices, -1);
	
	/* If only one device exists, return that one. */
	if (numDevices == 1) {
		index = 0;
	} else {
		/* Otherwise, query the user. */
		do {
			printf("   (?) Select device (0-%d) > ", numDevices - 1);
			result = scanf("%u", &index);
			/* Clean keyboard buffer */
			int c;
			do { c = getchar(); } while (c != '\n' && c != EOF);
			/* Check if result is Ok and break the loop if so */
			if (1 == result) {
				if ((index >= 0) && (index < (cl_int) numDevices))
					break;
			}
			/* Result not Ok, print error message */
			printf("   (!) Invalid choice, please insert a value between 0 and %u.\n", numDevices - 1);
		} while (1);
	}
	/* Return device index. */
	return index;

}

/** 
 * @brief Get kernel workgroup info. 
 * 
 * @param kernel OpenCL kernel to obtain info of.
 * @param device OpenCL device where kernel will run.
 * @param kwgi Kernel workgroup information structure, which will be populated by this function.
 * @param err Error structure, to be populated if an error occurs.
 * @return CL_SUCCESS if no error ocurred, or any other value otherwise.
 */
cl_uint clu_workgroup_info_get(cl_kernel kernel, cl_device_id device, CLUKernelWorkgroupInfo* kwgi, GError **err) {

	/* Aux. var. */
	cl_uint status;

	status = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &(kwgi->preferred_work_group_size_multiple), NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_workgroup_info_get: Unable to get CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE")
	
	status = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, 3 * sizeof(size_t), kwgi->compile_work_group_size, NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_workgroup_info_get: Unable to get CL_KERNEL_COMPILE_WORK_GROUP_SIZE")

	status = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &(kwgi->max_work_group_size), NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_workgroup_info_get: Unable to get CL_KERNEL_WORK_GROUP_SIZE")

	status = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &(kwgi->local_mem_size), NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_workgroup_info_get: Unable to get CL_KERNEL_LOCAL_MEM_SIZE")

	status = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_PRIVATE_MEM_SIZE, sizeof(cl_ulong), &(kwgi->private_mem_size), NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_workgroup_info_get: Unable to get CL_KERNEL_PRIVATE_MEM_SIZE")

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	/* Return. */
	return status;
}

/** 
 * @brief Print kernel workgroup info. 
 * 
 * @param kwgi Kernel workgroup information to print.
 */
void clu_workgroup_info_print(CLUKernelWorkgroupInfo* kwgi) {
	
	printf("\n   =========================== Kernel Information ==========================\n\n");
	printf("     Maximum workgroup size                  : %zu\n", kwgi->max_work_group_size);
	printf("     Preferred multiple of workgroup size    : %zu\n", kwgi->preferred_work_group_size_multiple);
	printf("     WG size in __attribute__ qualifier      : (%zu, %zu, %zu)\n", kwgi->compile_work_group_size[0], kwgi->compile_work_group_size[1], kwgi->compile_work_group_size[2]);
	printf("     Local memory used by kernel             : %ld bytes\n", (long) kwgi->local_mem_size);
	printf("     Min. private mem. used by each workitem : %ld bytes\n\n", (long) kwgi->private_mem_size);

}

/** 
 * @brief Get a string identifying the type of device. 
 * 
 * @param cldt Device type (OpenCL bitfield)
 * @param full Boolean true if full string is required, boolean false (0) if short string suffices.
 * @param str Memory space where to put string identifying device type.
 * @param strSize Size of memory space where to put string.
 * @return String identifying device type (i.e. the str parameter).
 */
char* clu_device_type_str_get(cl_device_type cldt, int full, char* str, int strSize) {

	int occuSpace = 0;
	char temp[30];
	*str = 0;

	if (cldt & CL_DEVICE_TYPE_DEFAULT) {
		strcpy(temp, full ? CLU_DEVICE_TYPE_DEFAULT_STR_FULL : CLU_DEVICE_TYPE_DEFAULT_STR);
		int availSpace = strSize - occuSpace - 2; /* 1 for space + 1 for \0 */
		if ((int) strlen(temp) <= availSpace) {
			strcat(str, " ");
			strcat(str, temp);
			availSpace -= strlen(temp);
		}
	}
	if (cldt & CL_DEVICE_TYPE_CPU) {
		strcpy(temp, full ? CLU_DEVICE_TYPE_CPU_STR_FULL : CLU_DEVICE_TYPE_CPU_STR);
		int availSpace = strSize - occuSpace - 2; /* 1 for space + 1 for \0 */
		if ((int) strlen(temp) <= availSpace) {
			strcat(str, " ");
			strcat(str, temp);
			availSpace -= strlen(temp);
		}
	}
	if (cldt & CL_DEVICE_TYPE_GPU) {
		strcpy(temp, full ? CLU_DEVICE_TYPE_GPU_STR_FULL : CLU_DEVICE_TYPE_GPU_STR);
		int availSpace = strSize - occuSpace - 2; /* 1 for space + 1 for \0 */
		if ((int) strlen(temp) <= availSpace) {
			strcat(str, " ");
			strcat(str, temp);
			availSpace -= strlen(temp);
		}
	}
	if (cldt & CL_DEVICE_TYPE_ACCELERATOR) {
		strcpy(temp, full ? CLU_DEVICE_TYPE_ACCELERATOR_STR_FULL : CLU_DEVICE_TYPE_ACCELERATOR_STR);
		int availSpace = strSize - occuSpace - 2; /* 1 for space + 1 for \0 */
		if ((int) strlen(temp) <= availSpace) {
			strcat(str, " ");
			strcat(str, temp);
			availSpace -= strlen(temp);
		}
	}
	if (cldt == CL_DEVICE_TYPE_ALL) {
		strcpy(temp, full ? CLU_DEVICE_TYPE_ALL_STR_FULL : CLU_DEVICE_TYPE_ALL_STR);
		int availSpace = strSize - occuSpace - 2; /* 1 for space + 1 for \0 */
		if ((int) strlen(temp) <= availSpace) {
			strcat(str, " ");
			strcat(str, temp);
			availSpace -= strlen(temp);
		}
	}
	return str;
}

/** 
 * @brief Create a new OpenCL zone, which will contain complete information for an OpenCL execution session on a specific device. 
 * 
 * @param deviceType Device type (OpenCL bitfield).
 * @param numQueues Number of command queues.
 * @param queueProperties Properties for the command queues.
 * @param devSel Pointer to function which will select device, if more than one is available.
 * @param dsExtraArg Extra argument for (*deviceSelector) function.
 * @param err Error structure, to be populated if an error occurs.
 * @return OpenCL zone or NULL if device wasn't properly initialized.
 */
CLUZone* clu_zone_new(cl_uint deviceType, cl_uint numQueues, cl_int queueProperties, clu_device_selector devSel, void* dsExtraArg, GError **err) {
	
	/* Helper variables */
	cl_int status;
	
	/* OpenCL zone to initialize and return */
	CLUZone* zone;
	
	/* Information about devices */
	CLUDeviceInfo devInfos[CLU_MAX_DEVICES_TOTAL];

	/* Number of devices. */
	cl_uint numDevices;

	/* Index of device information */
	cl_int deviceInfoIndex;

	/* Context properties, */
	cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, 0, 0};

	/* List of platform Ids. */
	cl_platform_id platfIds[CLU_MAX_PLATFORMS];

	/* Number of platforms. */
	cl_uint numPlatforms;

	/* Total number of devices. */
	unsigned int totalNumDevices;

	/* Device IDs for a given platform. */
	cl_device_id devIds[CLU_MAX_DEVICES_PER_PLATFORM];
	
	/* Initialize zone */
	zone = (CLUZone*) malloc(sizeof(CLUZone));
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, NULL == zone, CLU_ERROR_NOALLOC, error_handler, "Unable to allocate memory OpenCL zone. ");
	zone->context = NULL;
	zone->queues = NULL;
	zone->program = NULL;
	zone->device_info.device_id = NULL;
	zone->device_info.platform_id = NULL;
	zone->device_info.device_name[0] = '\0';
	zone->device_info.device_vendor[0] = '\0';
	zone->device_info.platform_name[0] = '\0';
		
	/* Get number of platforms */
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get number of platforms");

	/* Get existing platforms */
	status = clGetPlatformIDs(numPlatforms, platfIds, NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get platform Ids");

	/* Cycle through platforms, get specified devices in existing platforms */
	totalNumDevices = 0;
	for(unsigned int i = 0; i < numPlatforms; i++) 	{
		/* Get specified devices for current platform */
		status = clGetDeviceIDs( platfIds[i], deviceType, CLU_MAX_DEVICES_PER_PLATFORM, devIds, &numDevices );
		if (status != CL_DEVICE_NOT_FOUND) {
			/* At least one device found, lets take note */
			gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get device Ids");
			for (unsigned int j = 0; j < numDevices; j++) {
				/* Keep device and platform IDs. */
				devInfos[totalNumDevices].device_id = devIds[j];
				devInfos[totalNumDevices].platform_id = platfIds[i];
				/* Get device name. */
				status = clGetDeviceInfo(devIds[j], CL_DEVICE_NAME, sizeof(devInfos[totalNumDevices].device_name), devInfos[totalNumDevices].device_name, NULL);
				gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get device name info");
				/* Get device vendor. */
				status = clGetDeviceInfo(devIds[j], CL_DEVICE_VENDOR, sizeof(devInfos[totalNumDevices].device_vendor), devInfos[totalNumDevices].device_vendor, NULL);
				gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get device vendor info");
				/* Get platform name. */
				status = clGetPlatformInfo( platfIds[i], CL_PLATFORM_VENDOR, sizeof(devInfos[totalNumDevices].platform_name), devInfos[totalNumDevices].platform_name, NULL);
				gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "clu_zone_new: get platform info");
				/* Increment total number of found devices. */
				totalNumDevices++;
			}
		}
	}
	
	/* Check whether any devices of the specified type were found */
	if (totalNumDevices == 0) {
		/* No devices of the specified type where found, return with error. */
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, 1, CL_DEVICE_NOT_FOUND, error_handler, "clu_zone_new: device not found");
	} else {
		/* Several compatible devices found, choose one with given selector function. */
		deviceInfoIndex = devSel(devInfos, totalNumDevices, dsExtraArg);
		/* Test return value of selector function (if it is out of range, 
		 * there is a programming error). */
		g_assert_cmpint(deviceInfoIndex, >=, -1);
		g_assert_cmpint(deviceInfoIndex, <, totalNumDevices);
		/* If selector function returned -1, then no device is selectable. */
		if (deviceInfoIndex == -1) {
			gef_if_error_create_goto(*err, CLU_UTILS_ERROR, 1, CL_DEVICE_NOT_FOUND, error_handler, "clu_zone_new: specified device not found");
		}
	}

	/* Store info about the selected device and platform. */
	zone->device_type = deviceType;
	zone->device_info = devInfos[deviceInfoIndex];

	/* Determine number of compute units for that device */
	status = clGetDeviceInfo(zone->device_info.device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &zone->cu, NULL);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "cl_int clu_zone_new: get target device info");
	
	/* Create a context on that device. */
	cps[1] = (cl_context_properties) devInfos[deviceInfoIndex].platform_id;
	zone->context = clCreateContext(cps, 1, &zone->device_info.device_id, NULL, NULL, &status);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "cl_int clu_zone_new: creating context");
	
	/* Create the specified command queues on that device */
	zone->numQueues = numQueues;
	zone->queues = (cl_command_queue*) malloc(numQueues * sizeof(cl_command_queue));
	for (unsigned int i = 0; i < numQueues; i++) {
		zone->queues[i] = clCreateCommandQueue(zone->context, zone->device_info.device_id, queueProperties, &status);
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "cl_int clu_zone_new: creating command queue");
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	/* Free OpenCL zone. */
	if (zone != NULL) {
		clu_zone_free(zone);
		zone = NULL;
	}

finish:	

	/* Return. */
	return zone;

}

/** 
 * @brief Create an OpenCL program given a set of source kernel files. 
 * 
 * @param zone OpenCL zone where to create program.
 * @param kernelFiles Array of strings identifying filenames containing kernels.
 * @param numKernelFiles Number of strings identifying filenames containing kernels.
 * @param compilerOpts OpenCL compiler options.
 * @param err Error structure, to be populated if an error occurs.
 * @return CL_SUCCESS if no error ocurred, or any other value otherwise.
 */
cl_int clu_program_create(CLUZone* zone, const char** kernelFiles, cl_uint numKernelFiles, const char* compilerOpts, GError **err) {

	/* Helper variables */
	cl_int status, bpStatus;
	char * buildLog = NULL;
	size_t logsize;
	char** source = NULL;
	
	/* Import kernels */
	source = (char**) malloc(numKernelFiles * sizeof(char*));
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, NULL == source, CLU_ERROR_NOALLOC, error_handler, "Unable to allocate memory for kernels source file. ");
	for (unsigned int i = 0; i < numKernelFiles; i++) { source[i] = NULL; }
	for (unsigned int i = 0; i < numKernelFiles; i++) {
		source[i] = clu_source_load(kernelFiles[i], err);
		gef_if_error_goto(*err, GEF_USE_GERROR, status, error_handler);
	}
	
	/* Load kernels sources and create program */
	cl_program program = clCreateProgramWithSource(zone->context, numKernelFiles, (const char**) source, NULL, &status);
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "Create program with source");
	
	/* Perform runtime source compilation of program */
	bpStatus = clBuildProgram( program, 1, &zone->device_info.device_id, compilerOpts, NULL, NULL );
	/* Check for errors. */
	if (bpStatus != CL_SUCCESS) {
		/* If where here it's because program failed to build. However, error will only be thrown after getting build information. */
		/* Get build log size. */
		status = clGetProgramBuildInfo(program, zone->device_info.device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "Error getting program build info (log size) after program failed to build with error %d", bpStatus);
		/* Alocate memory for build log. */
		buildLog = (char*) malloc(logsize);
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, NULL == buildLog, CLU_ERROR_NOALLOC, error_handler, "Unable to allocate memory for build log after program failed to build with error %d", bpStatus);
		/* Get build log. */
		status = clGetProgramBuildInfo(program, zone->device_info.device_id, CL_PROGRAM_BUILD_LOG, logsize, buildLog, NULL);
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, CL_SUCCESS != status, status, error_handler, "Error getting program build info (build log) after program failed to build with error %d", bpStatus);
		/* Throw error. */
		gef_if_error_create_goto(*err, CLU_UTILS_ERROR, 1, bpStatus, error_handler, "Failed to build program. \n\n **** Start of build log **** \n\n%s\n **** End of build log **** \n", buildLog);
	}
	zone->program = program;

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	

	/* Free stuff. */
	if (source != NULL) {
		for (unsigned int i = 0; i < numKernelFiles; i++) {
			if (source[i] != NULL) {
				clu_source_free(source[i]);
			}
		}
		free(source);
	}
	if (buildLog != NULL) {
		free(buildLog);
	}
	
	/* Return. */
	return status;
}

/** 
 * @brief Free a previously created OpenCL zone. 
 * @todo clReleaseCommandQueue segfault's with AMDAPPSDK 2.8.
 * 
 * @param zone OpenCL zone to free.
 */
void clu_zone_free(CLUZone* zone) {
	g_assert(zone != NULL);
	if (zone->queues) {
		for (unsigned int i = 0; i < zone->numQueues; i++)
			if (zone->queues[i]) clReleaseCommandQueue(zone->queues[i]);
		free(zone->queues);
	}
	if (zone->program) clReleaseProgram(zone->program);
	if (zone->context) clReleaseContext(zone->context);
	free(zone);

}

/** 
 * @brief Load kernel source from given file. 
 * 
 * @param filename Filename from where to load kernel source.
 * @param err GLib error object for error reporting.
 * @return Kernel source code.
 */
char* clu_source_load(const char * filename, GError** err) {
	
	/* Function vars. */
	FILE * fp = NULL;
	char * sourcetmp = NULL;

	/* Open file. */
	fp = fopen(filename, "r");
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, fp == NULL, CLU_ERROR_OPENFILE, error_handler, "Unable to open kernels file '%s'.", filename);
	
	/* Get file size. */
	fseek(fp, 0L, SEEK_END);
	int prog_size = (int) ftell(fp);
	
	/* Allocate memory for file contents. */
	sourcetmp = (char*) malloc((prog_size + 1)*sizeof(char));
	gef_if_error_create_goto(*err, CLU_UTILS_ERROR, sourcetmp == NULL, CLU_ERROR_NOALLOC, error_handler, "Unable to allocate memory to place contents of file '%s'.", filename);

	/* Read file contents in string buffer. */
	rewind(fp);
	fread(sourcetmp, sizeof(char), prog_size, fp);
	sourcetmp[prog_size] = '\0';
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	

	/* Close file. */
	if (fp) fclose(fp);

	/* Return file contents in string form. */
	return sourcetmp;
}

/** 
 * @brief Free kernel source.
 * 
 * @param source Kernel source code to free
 */
void clu_source_free(char* source) {
	g_assert(source != NULL);
	free(source);
}

/** 
 * @brief Implementation of ::clu_device_selector function which 
 * queries the user in order to select a device.
 * 
 * If `extraArg` points to an integer within the index 
 * interval of available devices, then the respective device is selected.
 * Otherwise (e.g. if `extraArg` is NULL), the user 
 * is queried to select a device from a list of available devices.
 * 
 * @param devInfos List of device information.
 * @param numDevices Number of devices on list.
 * @param extraArg Pointer to device index or NULL.
 * @return The index of the selected device.
 */
cl_uint clu_menu_device_selector(CLUDeviceInfo* devInfos, cl_uint numDevices, void* extraArg) {
	
	/* numDevices must be greater than 0. */
	g_assert_cmpuint(numDevices, >, 0);
	
	/* Index of selected device. */
	cl_int index = -1;
	
	/* If extra argument is given, perform auto-selection. */
	if (extraArg != NULL) {
		/* If extraArg contains a valid device index, set return value
		 * to that index. */
		index = *((cl_uint*) extraArg);
		/* Check if index is within bounds. */
		if ((index >= 0) && (index < (cl_int) numDevices)) {
			/* Device is within bounds, print list with selection. */
			clu_menu_device_selector_list(devInfos, numDevices, index);
		} else {
			/* If we get here, an invalid device index was given. */
			printf("\n   (!) No device at index %d!\n", index);
			index = -1;
		}
	}
	
	/* If no proper index was given ask the user for the correct index. */
	if (index == -1) {
		index = clu_menu_device_selector_query(devInfos, numDevices);
	}
	
	/* Return device index. */
	return index;
}

/** 
 * @brief Implementation of ::clu_device_selector function which selects a 
 * device based on device information such as device name, device vendor
 * and platform name.
 * 
 * `extraArg` must point to a ::CLUDeviceInfo structure, which should
 * have at least one of the `char`-type fields containing partial 
 * information about the respective device information. The non-used
 * `char'-type fields must be set to NULL.
 * 
 * @param devInfos List of device information.
 * @param numDevices Number of devices on list.
 * @param extraArg Pointer to ::CLUDeviceInfo structure.
 * @return The index of the selected device of -1 if no device is
 * selectable.
 */
cl_uint clu_info_device_selector(CLUDeviceInfo* devInfos, cl_uint numDevices, void* extraArg) {
	
	/* numDevices must be greater than 0. */
	g_assert_cmpuint(numDevices, >, 0);
	
	/* Index of selected device. */
	cl_int index = -1;
	/* Pointer to CLUDeviceInfo structure. */
	CLUDeviceInfo* info;
	/* Number of devices found which conform to the information. */
	int numValidDevs = 0;
	/* If more than 1 device is found this aux. struct. will be passed 
	 * to a user query function. */
    CLUDeviceInfo validDevInfos[CLU_MAX_DEVICES_TOTAL];
    /* Maps the aux. struct. dev. index to the main struct. dev. index. */
	int map[CLU_MAX_DEVICES_TOTAL];
	/* Flag to check if a device is conformant with the information. */
	gboolean validDev;
	/* Partial name must be a substring of complete name. */
	gchar *partialName, *completeName;

	/* Check if extraArg contains a valid NULL-terminated array of strings. */
	if (extraArg != NULL) {
		info = (CLUDeviceInfo*) extraArg;
		g_assert(info != NULL);
		/* Cycle through available devices. */
		for (unsigned int i = 0; i < numDevices; i++) {
			/* Check for the three types of information: device name.
			 * device vendor and platform name. */
			for (unsigned int j = 0; j < 3; j++) {
				/* Not known if device will pass this filter. */
				validDev = FALSE;
				/* Obtain complete and partial information for comparison. */
				switch (j) {
					case 0:
					if (info->device_name != NULL) {
						completeName = g_ascii_strdown(devInfos[i].device_name, -1);
						partialName = g_ascii_strdown(info->device_name, -1);
					}
					break;
					case 1:
					if (info->device_vendor != NULL) {
						completeName = g_ascii_strdown(devInfos[i].device_vendor, -1);
						partialName = g_ascii_strdown(info->device_vendor, -1);
					}
					break;
					case 2:
					if (info->platform_name != NULL) {
						completeName = g_ascii_strdown(devInfos[i].platform_name, -1);
						partialName = g_ascii_strdown(info->platform_name, -1);
					}
					break;
					default:
					g_assert_not_reached();
				}
				/* Check if partial name is within in the complete name. */
				if (g_strrstr(completeName, partialName)) {
					/* Valid device so far. */
					validDev = TRUE;
				}
				/* Free temporary strings. */
				g_free(partialName);
				g_free(completeName);
				/* If device didn't conform to info, go to next device. */
				if (!validDev) {
					break;
				}
			}
			/* If device passed filters, add it to valid devices. */
			if (validDev) {
				validDevInfos[numValidDevs] = devInfos[i];
				map[numValidDevs] = i;
				numValidDevs++;
			}
		}
	}
	
	/* Check result. */
	if (numValidDevs == 1) {
		/* Only one valid device found, return that one. */
		index = map[0];
	} else if (numValidDevs > 1) {
		/* Several valid devices found, ask the user to chose one. */
		index = map[clu_menu_device_selector_query(validDevInfos, numValidDevs)];
	}
	
	/* Return device index. */
	return index;
}


/** 
 * @brief Resolves to error category identifying string, in this case an error in the OpenCL utilities library.
 * 
 * @return A GQuark structure defined by category identifying string, which identifies the error as a CL utilities generated error.
 */
GQuark clu_utils_error_quark() {
	return g_quark_from_static_string("clu-utils-error-quark");
}

