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
 * @brief Functions for filtering and selecting OpenCL devices.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef CL4_DEVSEL_H
#define CL4_DEVSEL_H

#include <glib.h>
#include "oclversions.h"
#include "device_wrapper.h"
#include "platforms.h"

/** @brief A set of device wrappers, used between filtering steps. */
typedef GPtrArray* CL4DevSelDevices;

/**
 * @brief Independent filter function: Abstract function for filtering 
 * one OpenCL device at a time.
 *
 * @param device OpenCL device to filter.
 * @param data Filter data, implementation dependent.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return TRUE if filter accepts device, FALSE otherwise.
 */
typedef gboolean (*cl4_devsel_indep)(
	CL4Device* device, void *data, GError **err);
	
/**
 * @brief Dependent filter function: Abstract function for filtering 
 * several OpenCL devices depending on the available device choices.
 *
 * @param devices OpenCL devices to filter.
 * @param data Filter data, implementation dependent.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 */
typedef CL4DevSelDevices (*cl4_devsel_dep)(
	CL4DevSelDevices devices, void *data, GError **err);

/**
 * @brief A set of independent and dependent device filters.
 * 
 * Use the cl4_devsel_add_indep_filter() function to add independent
 * filters and the cl4_devsel_add_dep_filter() function to add dependent
 * device filters.
 * 
 * This object should be initialized to NULL:
 * 
 *     CL4DevSelFilters filters = NULL;
 * 
 * And its location should be passed to the cl4_devsel_add_*_filter() 
 * functions:
 * 
 *     cl4_devsel_add_indep_filter(&filters, cl4_devsel_indep_type_cpu, NULL);
 * 
 * Filters are processed in the order they are added to the set.
 * */
typedef GPtrArray* CL4DevSelFilters;

/** @brief Add a independent device filter to the filter set. */
void cl4_devsel_add_indep_filter(
	CL4DevSelFilters* filters, cl4_devsel_indep filter, gpointer data);

/** @brief Add a dependent device filter to the filter set. */
void cl4_devsel_add_dep_filter(
	CL4DevSelFilters* filters, cl4_devsel_dep filter, gpointer data);

/** @brief Select one or more OpenCL devices based on the provided
 * filters.  */
CL4DevSelDevices cl4_devsel_select(
	CL4DevSelFilters* filters, GError **err);

/**
 * @defgroup CL4_DEVSEL_INDEP_FILTERS Independent filters.
 *
 * @{
 */
 
/** @brief Independent filter function which accepts devices of the type
 * given in the data parameter. */
gboolean cl4_devsel_indep_type(
	CL4Device* device, void* data, GError **err);

/** @brief Independent filter function which only accepts GPU devices. */
gboolean cl4_devsel_indep_type_gpu(
	CL4Device* device, void *data, GError **err);

/** @brief Independent filter function which only accepts CPU devices. */
gboolean cl4_devsel_indep_type_cpu(
	CL4Device* device, void *data, GError **err);

/** @brief Independent filter function which only accepts accelerator 
 * devices. */
gboolean cl4_devsel_indep_type_accel(
	CL4Device* device, void *data, GError **err);
	
/** @brief Independent filter which selects devices based on device 
 * name, device vendor and/or platform name. */
gboolean cl4_devsel_indep_string(
	CL4Device* dev, void *data, GError **err);

/** @brief Independent filter function which only accepts devices of a
 * specified platform. */
gboolean cl4_devsel_indep_platform(
	CL4Device* device, void *data, GError **err);

/** @} */

/**
 * @defgroup CL4_DEVSEL_DEP_FILTERS Dependent filters.
 *
 * @{
 */

/** @brief Dependent filter function which only accepts devices of the
 * same platform (the platform to which the first device belong to). */
CL4DevSelDevices cl4_devsel_dep_platform(
	CL4DevSelDevices devices, void *data, GError **err);
	
/** @brief Dependent filter function which presents a menu to the user
 * allowing him to select the desired device. */
CL4DevSelDevices cl4_devsel_dep_menu(
	CL4DevSelDevices devices, void *data, GError **err);

/** @} */

#endif
