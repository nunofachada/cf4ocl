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
 * Example program which demonstrates device selection using filters.
 *
 * The program accepts one command-line argument which specifies if
 * (1) device filtering is to be performed for context creation, or (2)
 * just to list the filtered devices.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#include <stdlib.h>
#include <stdio.h>
#include <cf4ocl2.h>

#define ERROR_MSG_AND_EXIT(msg) \
	do { fprintf(stderr, "\n%s\n", msg); exit(-1); } while(0)

#define HANDLE_ERROR(err) \
	if (err != NULL) { ERROR_MSG_AND_EXIT(err->message); }

#define USAGE "Usage: device_filter 1|2\n\n1 - "\
			"Create context with filtered devices\n"\
			"2 - List filtered devices\n"

/**
 * Device filter example main function.
 * */
int main(int argc, char* argv[]) {

	/* cf4ocl objects. */
	CCLContext* ctx;
	CCLDevSelFilters filters = NULL;
	CCLDevSelDevices devices;

	/* Other variables. */
	cl_uint num_devs;
	char* dev_name;
	int op;

	/* Error handling object. */
	GError* err = NULL;

	/* Check command-line arguments. */
	if (argc < 2)
		ERROR_MSG_AND_EXIT(USAGE);
	else
		op = atoi(argv[1]);

	/* Add independent filter which accepts CPU devices. */
	ccl_devsel_add_indep_filter(
		&filters, ccl_devsel_indep_type_cpu, NULL);

	/* Add same platform dependent filter. This filter should always
	 * be added (usually in last position) for context creation, because
	 * all devices in a context must belong to the same platform. */
	ccl_devsel_add_dep_filter(&filters, ccl_devsel_dep_platform, NULL);

	/* Proceed depending on user option. */
	if (op == 1) {
		/* Option 1: use set of filters for context creation. */

		/* Create context wrapper, which must have at least one device. */
		ctx = ccl_context_new_from_filters(&filters, &err);
		HANDLE_ERROR(err);

		/* Get number of devices used in context. */
		num_devs = ccl_context_get_num_devices(ctx, &err);
		HANDLE_ERROR(err);

		/* Give some feedback. */
		printf("Context was successfully created with %d device(s)!\n",
			num_devs);

		/* Free context. */
		ccl_context_destroy(ctx);

	} else if (op == 2) {
		/* Option 2: use set of filters for explicit device selection
		 * and listing. Here we manipulate a CCLDevSelDevices object
		 * directly, which will be uncommon in client code, except in
		 * cases were the the final purpose of device selection is not
		 * context creation. */

		/* Perform device selection. */
		devices = ccl_devsel_select(&filters, &err);
		HANDLE_ERROR(err);

		/* List selected devices. */
		if (devices->len > 0) {
			printf("%d devices were accepted by the filters:\n",
				devices->len);
			for (guint i = 0; i < devices->len; ++i) {
				dev_name = ccl_device_get_info_array(
					devices->pdata[i], CL_DEVICE_NAME, char*, &err);
				HANDLE_ERROR(err);
				printf("\t%d - %s\n", i + 1, dev_name);
			} /* For */
		} else {
			ERROR_MSG_AND_EXIT("No devices were accepted by the filters.");
		} /* If */

		/* Free array object containing device wrappers. The
		 * CCLDevSelDevices class is just another name for GPtrArray*,
		 * which is a GLib pointer array. As such, we use the GLib
		 * pointer array destructor to release this object. */
		g_ptr_array_free(devices, TRUE);

	} else {

		/* Unknown option. */
		ERROR_MSG_AND_EXIT(USAGE);

	}

	/* Check all wrappers have been destroyed. */
	g_assert(ccl_wrapper_memcheck());

	/* Bye. */
	return 0;

}


