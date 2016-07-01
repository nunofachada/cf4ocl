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
 *
 * Classes and functions for filtering and selecting OpenCL contexts
 * and associated devices.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_device_selector.h"
#include "_ccl_defs.h"

/**
 * Generic filter function pointer. Used to keep either a
 * dependent or independent filter function in a ::CCLDevSelFilter
 * object.
 * */
typedef void (*ccl_devsel_fp)(void);

/**
 * Device filter class, includes a filter function (independent
 * or dependent) and the respective filter data.
 * */
typedef struct ccl_devsel_filter {

	/**
	 * Filter function.
	 * @private
	 * */
	ccl_devsel_fp function;

	/**
	 * Filter data.
	 * @private
	 * */
	void* data;

	/**
	 * Filter type.
	 * @private
	 * */
	CCLDevSelFilterType type;

} CCLDevSelFilter;

/**
 * @internal
 * Add any filter to the filter set.
 *
 * This private function is used by the public ccl_devsel_add_*_filter()
 * functions.
 *
 * @param[in] filters The filter set.
 * @param[in] function Filter function (independent or dependent).
 * @param[in] data Filter data.
 * @param[in] type Type of filter: independent or dependent.
 * */
static void ccl_devsel_add_filter(CCLDevSelFilters* filters,
	ccl_devsel_fp function, void* data, CCLDevSelFilterType type) {

	/* Initialize filters if required. */
	if (*filters == NULL)
		*filters = g_ptr_array_new();

	/* Allocate space for new filter. */
	CCLDevSelFilter* filter = g_slice_new0(CCLDevSelFilter);

	/* Set filter function, filter data and type. */
	filter->function = function;
	filter->data = data;
	filter->type = type;

	/* Add filter to filters array. */
	g_ptr_array_add(*filters, filter);

}

/**
 * @internal
 * Returns a NULL-terminated array of strings, each one
 * containing the name and vendor of each device in the given device
 * array.
 *
 * @param[in] devices Array of devices.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A NULL-terminated array of strings, each one containing the
 * name and vendor of each device in the device array. The array of
 * strings should be freed with the g_strfreev() function from GLib. If
 * an error occurs, NULL is returned.
 *  */
static gchar** ccl_get_device_strings_from_array(
	CCLDevSelDevices devices, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Make sure devices array is not null. */
	g_return_val_if_fail(devices != NULL, NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Array of strings. */
	gchar** dev_strings;

	/* Initialize array of strings. */
	dev_strings = g_new0(gchar*, devices->len + 1);

	/* Create array of strings describing devices. */
	for (guint i = 0; i < devices->len; i++) {

		/* Get device name. */
		gchar* name = ccl_device_get_info_array(
				devices->pdata[i], CL_DEVICE_NAME, char*, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Get device platform wrapper. */
		CCLPlatform* platf_wrap = ccl_platform_new_from_device(
			devices->pdata[i], &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Get platform name. */
		gchar* platf_name = ccl_platform_get_info_string(
			platf_wrap, CL_PLATFORM_NAME, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Put info in string. */
		dev_strings[i] = g_strdup_printf(
			"%d. %s [%s]", i, name, platf_name);

		/* Release platform wrapper. */
		ccl_platform_destroy(platf_wrap);

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Free what was built of the array of strings. */
	g_strfreev(dev_strings);
	dev_strings = NULL;

finish:

	/* Return array of strings describing devices. */
	return dev_strings;
}

/**
 * @internal
 * Private helper function, prints a list of the devices
 * specified in the given list.
 *
 * @param[in] devices List of devices.
 * @param[in] selected Index of selected device (a message will appear
 * near the device name indicating the device is selected). Pass -1 to
 * ignore it.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * */
static void ccl_devsel_dep_menu_list(CCLDevSelDevices devices,
	cl_int selected, GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail(err == NULL || *err == NULL);

	/* Make sure devices array is not null. */
	g_return_if_fail(devices != NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Array of device description strings. */
	gchar** dev_strings;

	/* Get device description strings. */
	dev_strings = ccl_get_device_strings_from_array(
		devices, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	g_printf("\n");

	/* Print each device description string. */
	for (cl_int i = 0; i < (cl_int) devices->len; i++) {

		/* Current device string. */
		gchar* sel_str;

		/* If a device is pre-selected, show it. */
		sel_str = "   ";
		if (i == selected) {
			sel_str = "(*)";
		}

		/* Print string. */
		g_printf(" %s %s\n", sel_str, dev_strings[i]);

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free what the array of device description strings. */
	g_strfreev(dev_strings);

}

/**
 * @internal
 * Private helper function, asks the user to select a device
 * from a list.
 *
 * @param[in] devices Array of devices.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The list index of the selected device or -1 if an error
 * ocurrs.
 * */
static cl_int ccl_devsel_dep_menu_query(CCLDevSelDevices devices,
	GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, -1);
	/* Make sure devices array is not null. */
	g_return_val_if_fail(devices != NULL, -1);

	/* Index of selected device. */
	cl_int index = -1;
	/* Number of results read from user input. */
	int result;
	/* Internal error handler. */
	GError* err_internal = NULL;

	/* Print available devices */
	g_printf("\nList of available OpenCL devices:\n");
	ccl_devsel_dep_menu_list(devices, -1, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If only one device exists, return that one. */
	if (devices->len == 1) {
		index = 0;
	} else {
		/* Otherwise, query the user. */
		do {
			g_printf("\n (?) Select device (0-%d) > ", devices->len - 1);
			fflush(stdout);
			result = scanf("%u", &index);
			/* Clean keyboard buffer */
			int c;
			do { c = getchar(); } while (c != '\n' && c != EOF);
			/* Check if result is Ok and break the loop if so */
			if (1 == result) {
				if ((index >= 0) && (index < (cl_int) devices->len))
					break;
			}
			/* Result not Ok, print error message */
			g_printf(" (!) Invalid choice, please insert a value " \
				"between 0 and %u.\n", devices->len - 1);
		} while (1);
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	index = -1;

finish:

	/* Return device index. */
	return index;

}

/**
 * @addtogroup CCL_DEVICE_SELECTOR
 * @{
 */

/**
 * Create and return an object with device wrappers for all OpenCL devices
 * present in the system.
 *
 * See ::CCLDevSelDevices for information on how to access individual device
 * wrappers within the object.
 *
 * @param[out] err Return location for a GError, or `NULL` if error reporting is
 * to be ignored.
 * @return An object containing device wrappers for all OpenCL devices present
 * in the system, or `NULL` if an error occurs. The object should be freed with
 * ccl_devsel_devices_destroy().
 * @sa ::CCLDevSelDevices
 * */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_devices_new(GError **err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Platforms wrapper object. */
	CCLPlatforms* platforms = NULL;

	/* Platform wrapper object. */
	CCLPlatform* platform = NULL;

	/* Device wrapper object. */
	CCLDevice* device = NULL;

	/* Array of device wrapper objects. Devices will be selected from
	 * this array.  */
	GPtrArray* devices = NULL;

	/* Number of platforms. */
	guint num_platfs;

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Get all OpenCL platforms in system wrapped in a CCLPlatforms
	 * object. */
	platforms = ccl_platforms_new(&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Determine number of platforms. */
	num_platfs = ccl_platforms_count(platforms);

	/* Create array of device wrapper objects. */
	devices = g_ptr_array_new_with_free_func(
		(GDestroyNotify) ccl_device_destroy);

	/* Cycle through OpenCL platforms. */
	for (guint i = 0; i < num_platfs; i++) {

		/* Get next platform wrapper. */
		platform = ccl_platforms_get(platforms, i);

		/* Get number of devices in current platform.*/
		guint num_devices = ccl_platform_get_num_devices(
			platform, &err_internal);

		/* Is this a platform without devices? */
		if ((err_internal) && (err_internal->domain == CCL_OCL_ERROR) &&
				(err_internal->code == CL_DEVICE_NOT_FOUND)) {

			/* Clear "device not found" error. */
			g_clear_error(&err_internal);

			/* Skip this platform. */
			continue;
		}
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* Cycle through devices in current platform. */
		for (guint j = 0; j < num_devices; j++) {

			/* Get current device wrapper. */
			device = ccl_platform_get_device(platform, j, &err_internal);
			ccl_if_err_propagate_goto(err, err_internal, error_handler);

			/* Add device wrapper to array of device wrapper objects. */
			g_ptr_array_add(devices, (gpointer) device);

			/* Update device reference (because it is kept also in
			 * the array of device wrapper objects). */
			ccl_device_ref(device);

		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free platforms wrapper object. */
	if (platforms != NULL) ccl_platforms_destroy(platforms);

	/* Return the selected devices. */
	return devices;

}

/**
 * Destroy an object containing device wrappers.
 *
 * This function will rarely be used in client code, unless in cases where
 * low-level management of device selection is required.
 *
 * @param[in] devices Object containing device wrappers.
 * @return Always returns `NULL`.
 * */
CCL_EXPORT
void ccl_devsel_devices_destroy(CCLDevSelDevices devices) {
	g_ptr_array_free(devices, TRUE);
}

/**
 * Returns a NULL-terminated array of strings, each one
 * containing the name and vendor of each device in the system.
 *
 * The array of strings should be freed with the g_strfreev() function
 * from GLib.
 *
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return A NULL-terminated array of strings, each one containing the
 * name and vendor of each device in the system. The array of strings
 * should be freed with the g_strfreev() function from GLib. If an error
 * occurs, NULL is returned.
 * */
CCL_EXPORT
gchar** ccl_devsel_get_device_strings(GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Array of devices. */
	CCLDevSelDevices devices = NULL;

	/* Array of strings. */
	gchar** dev_strings = NULL;

	/* Get all devices present in the system. */
	devices = ccl_devsel_devices_new(&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get the array of strings describing the devices. */
	dev_strings = ccl_get_device_strings_from_array(
		devices, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Free what was built of the array of strings. */
	g_strfreev(dev_strings);
	dev_strings = NULL;

finish:

	/* Destroy array of devices. */
	g_ptr_array_free(devices, CL_TRUE);

	/* Return array of strings describing devices. */
	return dev_strings;

}

/**
 * Print to stdout a device description string for each device in
 * the system.
 *
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * */
CCL_EXPORT
void ccl_devsel_print_device_strings(GError** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail(err == NULL || *err == NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Array of strings. */
	gchar** dev_strings = NULL;

	/* Get the array of strings describing the devices. */
	dev_strings = ccl_devsel_get_device_strings(&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Print device list to stdout. */
	for (guint i = 0; dev_strings[i] != NULL; ++i) {
		g_printf("%s\n", dev_strings[i]);
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:

	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free array of strings. */
	g_strfreev(dev_strings);

}

/**
 * Add an independent filter to the filter set.
 *
 * @param[in] filters The filter set.
 * @param[in] filter Indendent filter function.
 * @param[in] data Filter data.
 * */
CCL_EXPORT
void ccl_devsel_add_indep_filter(
	CCLDevSelFilters* filters, ccl_devsel_indep filter, void* data) {

	ccl_devsel_add_filter(
		filters, (ccl_devsel_fp) filter, data, CCL_DEVSEL_INDEP);
}

/**
 * Add a dependent filter to the filter set.
 *
 * @param[in] filters The filter set.
 * @param[in] filter Indendent filter function.
 * @param[in] data Filter data.
 * */
CCL_EXPORT
void ccl_devsel_add_dep_filter(
	CCLDevSelFilters* filters, ccl_devsel_dep filter, void* data) {

	ccl_devsel_add_filter(
		filters, (ccl_devsel_fp) filter, data, CCL_DEVSEL_DEP);
}

/**
 * Select one or more OpenCL devices based on the provided filters.
 *
 * This function is internally used by the ccl_context_new_from_filters_full()
 * function for selecting context devices. Clients should not need to use it
 * frequently.
 *
 * @param[in] filters Filters used to select device(s).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return One or more OpenCL devices selected based on the provided
 * filters.
 *  */
CCLDevSelDevices ccl_devsel_select(
	CCLDevSelFilters* filters, GError **err) {

	/* Make sure filters is not NULL. */
	g_return_val_if_fail(filters != NULL, NULL);

	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Internal error handling object. */
	GError* err_internal = NULL;

	/* Array of devices. */
	CCLDevSelDevices devices;

	/* Get all devices present in the system. */
	devices = ccl_devsel_devices_new(&err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* *** Filter devices. *** */

	/* Cycle through all filters. */
	for (guint i = 0; i < (*filters)->len; i++) {

		/* If there are no more devices left, get out... */
		if (devices->len == 0) break;

		/* Get current filter. */
		CCLDevSelFilter* curr_filter = g_ptr_array_index(*filters, i);

		/* Check type of filter, proceed accordingly. */
		if (curr_filter->type == CCL_DEVSEL_DEP) {

			/* It's a dependent filter.*/
			devices = ((ccl_devsel_dep) (curr_filter->function))(
				devices, curr_filter->data, &err_internal);
			ccl_if_err_propagate_goto(err, err_internal, error_handler);

		} else {
			/* It's an independent filter. */

			/* Cycle through all devices. */
			for (guint j = 0; j < devices->len; j++) {

				/* Get current device wrapper. */
				CCLDevice* curr_device =
					(CCLDevice*) g_ptr_array_index(devices, j);

				/* Check if current device is accepted by current
				 * filter. */
				cl_bool pass =
					((ccl_devsel_indep) curr_filter->function)(
						curr_device, curr_filter->data, &err_internal);
				ccl_if_err_propagate_goto(
					err, err_internal, error_handler);

				/* If current device didn't pass current filter... */
				if (!pass) {

					/* Remove device wrapper from device wrapper array. */
					g_ptr_array_remove_index(devices, j);

					/* Force device index decrement, because next device to
					 * be checked will be in this index. */
					j--;

				}
			}
		}
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Free individual filters. */
	for (guint i = 0; i < (*filters)->len; i++)
		g_slice_free(CCLDevSelFilter, g_ptr_array_index(*filters, i));

	/* Free filter array. */
	g_ptr_array_free(*filters, CL_TRUE);

	/* Set filters to NULL, so variable can be reused by client. */
	*filters = NULL;

	/* Return the selected devices. */
	return devices;

}

/**
 * @addtogroup CCL_DEVICE_SELECTOR_INDEP_FILTERS
 * @{
 */

/**
 * Independent filter function which accepts devices of the type given in the
 * data parameter.
 *
 * This function is used by the `ccl_devsel_indep_type_*()` group of functions.
 * Using these is simpler than using this function directly.
 *
 * @param[in] dev OpenCL device to filter depending on type.
 * @param[in] data Filter data, must point to a cl_device_type value.
 * @param[out] err Return location for a GError, or `NULL` if error reporting is
 * to be ignored.
 * @return `CL_TRUE` if device is of the given type, `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_type(
	CCLDevice* dev, void* data, GError **err) {

	/* Make sure dev is not NULL. */
	g_return_val_if_fail(dev != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Internal error object. */
	GError* err_internal = NULL;

	/* Device type. */
	cl_device_type type = CL_DEVICE_TYPE_DEFAULT;
	/* Device type to check for. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_DEFAULT;

	/* Make sure data is not NULL. */
	ccl_if_err_create_goto(*err, CCL_ERROR, data == NULL,
		CCL_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", CCL_STRD);

	/* Get type to check for. */
	type_to_check = *((cl_device_type*) data);

	/* Get device type. */
	type = ccl_device_get_info_scalar(
		dev, CL_DEVICE_TYPE, cl_device_type, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return the selected devices. */
	return (cl_bool) (type & type_to_check);
}

/**
 * Independent filter function which only accepts GPU devices.
 *
 * @param[in] dev OpenCL device to check for GPU type.
 * @param[in] data Filter data, ignored.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device is a GPU, `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_gpu(
	CCLDevice* dev, void *data, GError **err) {

	/* Set device type to GPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_GPU;

	/* Data not used, ignore compiler warnings. */
	CCL_UNUSED(data);

	/* Return result. */
	return ccl_devsel_indep_type(dev, (void*) &type_to_check, err);

}

/**
 * Independent filter function which only accepts CPU devices.
 *
 * @param[in] dev OpenCL device to check for CPU type.
 * @param[in] data Filter data, ignored.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device is a CPU, `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_cpu(
	CCLDevice* dev, void *data, GError **err) {

	/* Set device type to CPU. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_CPU;

	/* Data not used, ignore compiler warnings. */
	CCL_UNUSED(data);

	/* Return result. */
	return ccl_devsel_indep_type(dev, (void*) &type_to_check, err);

}

/**
 * Independent filter function which only accepts accelerator
 * devices.
 *
 * @param[in] dev OpenCL device to check for accelerator type.
 * @param[in] data Filter data, ignored.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device is a accelerator, `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_accel(
	CCLDevice* dev, void *data, GError **err) {

	/* Set device type to Accelerator. */
	cl_device_type type_to_check = CL_DEVICE_TYPE_ACCELERATOR;

	/* Data not used, ignore compiler warnings. */
	CCL_UNUSED(data);

	/* Return result. */
	return ccl_devsel_indep_type(dev, (void*) &type_to_check, err);

}

/**
 * Independent filter which selects devices based on device name,
 * device vendor and/or platform name.
 *
 * @param[in] dev OpenCL device to filter by platform.
 * @param[in] data Filter data, must be a string.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device is accepted by filter, `CL_FALSE` otherwise.
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_string(
	CCLDevice* dev, void *data, GError **err) {

	/* Make sure device is not NULL. */
	g_return_val_if_fail(dev != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Internal error object. */
	GError* err_internal = NULL;

	/* Return value, i.e., flag indicating if device belongs to the
	 * specified platform. */
	cl_bool pass = CL_FALSE;

	/* Partial name must be a substring of complete name. */
	gchar *complt_info, *complt_info_lowr, *part_info;

	/* Make sure data is not NULL. */
	ccl_if_err_create_goto(*err, CCL_ERROR, data == NULL,
		CCL_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", CCL_STRD);

	/* Lower-case partial name for comparison. */
	part_info = g_ascii_strdown((gchar*) data, -1);

	/* Compare with device name. */
	complt_info = ccl_device_get_info_array(
		dev, CL_DEVICE_NAME, char*, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	complt_info_lowr = g_ascii_strdown(complt_info, -1);

	if (!g_strrstr(complt_info_lowr, part_info)) {
		/* Device name does not match, check device vendor. */

		/* Free string allocated for previous comparison. */
		g_free(complt_info_lowr);

		/* Compare with device vendor. */
		complt_info = ccl_device_get_info_array(
			dev, CL_DEVICE_VENDOR, char*, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		complt_info_lowr = g_ascii_strdown(complt_info, -1);

		if (!g_strrstr(complt_info_lowr, part_info)) {
			/* Device vendor does not match, check platform name. */

			/* Free string allocated for previous comparison. */
			g_free(complt_info_lowr);

			/* Compare with platform name. */
			CCLPlatform* platf;

			/* Get device platform. */
			platf = ccl_platform_new_from_device(dev, &err_internal);
			ccl_if_err_propagate_goto(err, err_internal, error_handler);

			/* Get platform name. */
			complt_info = ccl_platform_get_info_string(
				platf, CL_PLATFORM_NAME, &err_internal);
			ccl_if_err_propagate_goto(err, err_internal, error_handler);

			complt_info_lowr = g_ascii_strdown(complt_info, -1);

			/* Compare. */
			if (g_strrstr(complt_info_lowr, part_info)) {
				pass = CL_TRUE;
			}

			/* Destroy device platform. */
			ccl_platform_destroy(platf);

		} else {
			pass = CL_TRUE;
		}

	} else {
		pass = CL_TRUE;
	}

	g_free(complt_info_lowr);
	g_free(part_info);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Filter will not accept device in case an error occurs. */
	pass = CL_FALSE;

finish:

	/* Return filtering result. */
	return pass;

}

/**
 * Independent filter function which only accepts devices of a
 * specified platform.
 *
 * @param[in] device OpenCL device to filter by platform.
 * @param[in] data Filter data, must be a cl_platform_id.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return `CL_TRUE` if device belongs to the specified platform, `CL_FALSE`
 * otherwise (or if an error occurs).
 * */
CCL_EXPORT
cl_bool ccl_devsel_indep_platform(
	CCLDevice* device, void *data, GError **err) {

	/* Make sure device is not NULL. */
	g_return_val_if_fail(device != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

	/* Device platform. */
	cl_platform_id platf;

	/* Internal error object. */
	GError* err_internal = NULL;

	/* Return value, i.e., flag indicating if device belongs to the
	 * specified platform. */
	cl_bool pass;

	/* Check if data is NULL, throw error if so. */
	ccl_if_err_create_goto(*err, CCL_ERROR, data == NULL,
		CCL_ERROR_INVALID_DATA, error_handler,
		"%s: invalid filter data", CCL_STRD);

	/* Get device platform. */
	platf = ccl_device_get_info_scalar(device, CL_DEVICE_PLATFORM,
		cl_platform_id, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Determine filtering result, i.e. if device platform is the same
	 * as the specified platform. */
	pass = (platf == (cl_platform_id) data) ? CL_TRUE : CL_FALSE;

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Filter will not accept device in case an error occurs. */
	pass = CL_FALSE;

finish:

	/* Return filtering result. */
	return pass;
}

/** @} */

/**
 * @addtogroup CCL_DEVICE_SELECTOR_DEP_FILTERS
 * @{
 */

/**
 * Dependent filter function which only accepts devices of the
 * same platform (the platform to which the first device belong to).
 *
 * @param[in] devices Currently available OpenCL devices.
 * @param[in] data Filter data, ignored.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 * */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_dep_platform(
	CCLDevSelDevices devices, void *data, GError **err) {

	/* Make sure devices is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Current device. */
	CCLDevice* dev;

	/* Reference platform and current device platform.*/
	cl_platform_id platf_ref, platf_curr;

	/* Internal error object. */
	GError *err_internal = NULL;

	/* Filter data is ignored by this filter. */
	CCL_UNUSED(data);

	/* Get first device, which will determine the reference platform. */
	dev = (CCLDevice*) g_ptr_array_index(devices, 0);

	/* Determine reference platform (i.e. platform of first device). */
	platf_ref = ccl_device_get_info_scalar(dev, CL_DEVICE_PLATFORM,
		cl_platform_id, &err_internal);
	ccl_if_err_propagate_goto(err, err_internal, error_handler);

	/* Check if devices belong to the reference platform, remove them if
	 * they don't. */
	for (guint i = 1; i < devices->len; i++) {

		/* Get current device. */
		dev = (CCLDevice*) g_ptr_array_index(devices, i);

		/* Get current device platform. */
		platf_curr = ccl_device_get_info_scalar(
			dev, CL_DEVICE_PLATFORM, cl_platform_id, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);

		/* If current device doesn't belong to the reference
		 * platform... */
		if (platf_ref != platf_curr) {

			/* Remove device wrapper from device wrapper array. */
			g_ptr_array_remove_index(devices, i);

			/* Force device index decrement, because next device to
			 * be checked will be in this index. */
			i--;

		}

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Set return value to NULL to conform to specification. */
	devices = NULL;

finish:

	/* Return filtered devices. */
	return devices;
}

/**
 * Dependent filter function which presents a menu to the user
 * allowing him to select the desired device.
 *
 * @param[in] devices List of devices.
 * @param[in] data If not NULL, can contain a device index, such that
 * the device is automatically selected by this filter.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 * */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_dep_menu(
	CCLDevSelDevices devices, void *data, GError **err) {

	/* Make sure devices is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Internal error object. */
	GError *err_internal = NULL;

	/* Index of selected device. */
	cl_int index = -1;

	/* Device to be selected. */
	gpointer sel_dev;

	/* If data argument is given, perform auto-selection. */
	if (data != NULL) {
		/* Check if data contains a valid device index. */
		index = *((cl_uint*) data);
		/* Check if index is within bounds. */
		if ((index >= 0) && (index < (cl_int) devices->len)) {
			/* Device index is within bounds, print list with
			 * selection. */
			ccl_devsel_dep_menu_list(devices, index, &err_internal);
			ccl_if_err_propagate_goto(err, err_internal, error_handler);
		} else if (index >= 0) {
			/* If we get here, an invalid device index was given. */
			g_printf("\n   (!) No device at index %d!\n", index);
			index = -1;
		}
		/* Don't show warning if a negative index is given, just
		 * show the menu. */
	}

	/* If no proper index was given ask the user for the correct
	 * index. */
	if (index < 0) {
		index = ccl_devsel_dep_menu_query(devices, &err_internal);
		ccl_if_err_propagate_goto(err, err_internal, error_handler);
	}

	/* Remove all devices except the selected device. */
	sel_dev = g_ptr_array_index(devices, index);
	ccl_device_ref((CCLDevice*) sel_dev);
	g_ptr_array_remove_range(devices, 0, devices->len);
	g_ptr_array_add(devices, sel_dev);
	g_assert_cmpuint(1, ==, devices->len);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Free array object containing device wrappers and set it to NULL. */
	if (devices != NULL) {
		g_ptr_array_free(devices, TRUE);
		devices = NULL;
	}

finish:

	/* Return filtered devices. */
	return devices;
}

/**
 * Dependent filter function which selects the device at the specified
 * index, failing if no device is found at that index.
 *
 * Device indexes depend on the order in which devices appear in the
 * `devices` parameter.
 *
 * @param[in] devices List of devices.
 * @param[in] data Must point to a valid device index of type `cl_uint`.
 * @param[out] err Return location for a GError, or `NULL` if error reporting is
 * to be ignored.
 * @return The OpenCL device which was selected by the filter.
 * */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_dep_index(
	CCLDevSelDevices devices, void *data, GError **err) {

	/* Make sure devices is not NULL. */
	g_return_val_if_fail(devices != NULL, NULL);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, NULL);

	/* Index of selected device. */
	cl_uint index;

	/* Device to be selected. */
	gpointer sel_dev;

	/* If data is NULL, throw error, because we expect a valid device
	 * index. */
	ccl_if_err_create_goto(*err, CCL_ERROR, data == NULL,
		CCL_ERROR_INVALID_DATA, error_handler,
		"%s: The 'data' parameter must not be NULL.", CCL_STRD);

	/* Check if data contains a valid device index. */
	index = *((cl_uint*) data);

	/* Check if index is within bounds. */
	ccl_if_err_create_goto(*err, CCL_ERROR, index >= devices->len,
		CCL_ERROR_DEVICE_NOT_FOUND, error_handler,
		"%s: No device found at index %d.", CCL_STRD, index);

	/* Select device: remove all devices from list except the selected
	 * device. */
	sel_dev = g_ptr_array_index(devices, index);
	ccl_device_ref((CCLDevice*) sel_dev);
	g_ptr_array_remove_range(devices, 0, devices->len);
	g_ptr_array_add(devices, sel_dev);
	g_assert_cmpuint(1, ==, devices->len);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

	/* Free array object containing device wrappers and set it to NULL. */
	if (devices != NULL) {
		g_ptr_array_free(devices, TRUE);
		devices = NULL;
	}

finish:

	/* Return filtered devices. */
	return devices;
}

/** @} */

/** @} */

