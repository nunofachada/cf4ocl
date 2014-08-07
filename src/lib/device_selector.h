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
 * Functions for filtering and selecting OpenCL devices
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_DEVICE_SELECTOR_H_
#define _CCL_DEVICE_SELECTOR_H_

#include <glib.h>
#include "oclversions.h"
#include "device_wrapper.h"
#include "platforms.h"

/**
 * @defgroup DEVICE_SELECTOR Device selector
 *
 * This module provides objects and functions for selecting
 * OpenCL devices for context creation.
 * 
 * Todo: detailed description of module with code examples.
 * 
 * @{
 */

/** A set of device wrappers, used between filtering steps. */
typedef GPtrArray* CCLDevSelDevices;

/**
 * Independent filter function: Abstract function for filtering 
 * one OpenCL device at a time.
 *
 * @param[in] device OpenCL device to filter.
 * @param[in] data Filter data, implementation dependent.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return CL_TRUE if filter accepts device, CL_FALSE otherwise.
 */
typedef cl_bool (*ccl_devsel_indep)(
	CCLDevice* device, void *data, GError **err);
	
/**
 * Dependent filter function: Abstract function for filtering 
 * several OpenCL devices depending on the available device choices.
 *
 * @param[in] devices OpenCL devices to filter.
 * @param[in] data Filter data, implementation dependent.
 * @param[out] err Return location for a GError, or NULL if error
 * reporting is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 */
typedef CCLDevSelDevices (*ccl_devsel_dep)(
	CCLDevSelDevices devices, void *data, GError **err);

/**
 * A set of independent and dependent device filters.
 * 
 * Use the ccl_devsel_add_indep_filter() function to add independent
 * filters and the ccl_devsel_add_dep_filter() function to add dependent
 * device filters.
 * 
 * This object should be initialized to NULL:
 * 
 *     CCLDevSelFilters filters = NULL;
 * 
 * And its location should be passed to the ccl_devsel_add_*_filter() 
 * functions:
 * 
 *     ccl_devsel_add_indep_filter(&filters, ccl_devsel_indep_type_cpu, NULL);
 * 
 * Filters are processed in the order they are added to the set.
 * */
typedef GPtrArray* CCLDevSelFilters;

/** Returns a NULL-terminated array of strings, each one
 * containing the name and vendor of each device in the system. */
gchar** ccl_devsel_get_device_strings(GError** err);

/** Print to stdout a device description string for each device 
 * in the system. */
void ccl_devsel_print_device_strings(GError** err);

/** Add a independent device filter to the filter set. */
void ccl_devsel_add_indep_filter(
	CCLDevSelFilters* filters, ccl_devsel_indep filter, void* data);

/** Add a dependent device filter to the filter set. */
void ccl_devsel_add_dep_filter(
	CCLDevSelFilters* filters, ccl_devsel_dep filter, void* data);

/** Select one or more OpenCL devices based on the provided
 * filters.  */
CCLDevSelDevices ccl_devsel_select(
	CCLDevSelFilters* filters, GError **err);

/**
 * @defgroup DEVICE_SELECTOR_INDEP_FILTERS Independent filters
 *
 * @{
 */
 
/** Independent filter function which accepts devices of the type
 * given in the data parameter. */
cl_bool ccl_devsel_indep_type(
	CCLDevice* device, void* data, GError **err);

/** Independent filter function which only accepts GPU devices. */
cl_bool ccl_devsel_indep_type_gpu(
	CCLDevice* device, void *data, GError **err);

/** Independent filter function which only accepts CPU devices. */
cl_bool ccl_devsel_indep_type_cpu(
	CCLDevice* device, void *data, GError **err);

/** Independent filter function which only accepts accelerator 
 * devices. */
cl_bool ccl_devsel_indep_type_accel(
	CCLDevice* device, void *data, GError **err);
	
/** Independent filter which selects devices based on device 
 * name, device vendor and/or platform name. */
cl_bool ccl_devsel_indep_string(
	CCLDevice* dev, void *data, GError **err);

/** Independent filter function which only accepts devices of a
 * specified platform. */
cl_bool ccl_devsel_indep_platform(
	CCLDevice* device, void *data, GError **err);

/** @} */

/**
 * @defgroup DEVICE_SELECTOR_DEP_FILTERS Dependent filters
 *
 * @{
 */

/** Dependent filter function which only accepts devices of the
 * same platform (the platform to which the first device belong to). */
CCLDevSelDevices ccl_devsel_dep_platform(
	CCLDevSelDevices devices, void *data, GError **err);
	
/** Dependent filter function which presents a menu to the user
 * allowing him to select the desired device. */
CCLDevSelDevices ccl_devsel_dep_menu(
	CCLDevSelDevices devices, void *data, GError **err);

/** @} */

/** @} */

#endif
