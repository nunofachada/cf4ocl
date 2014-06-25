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

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include <glib.h>
#include "device.h"
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
 *     cl4_devsel_add_indep_filter(&filters, cl4_devsel_indep_cpu, NULL);
 * 
 * Filters are processed in the order they are added to the set.
 * */
typedef GPtrArray* CL4DevSelFilters;

/** @brief Select one or more OpenCL devices based on the provided
 * filters.  */
CL4DevSelDevices cl4_devsel_select(CL4DevSelFilters* filters, GError **err);

/** @brief Add a independent device filter to the filter set. */
void cl4_devsel_add_indep_filter(
	CL4DevSelFilters* filters, cl4_devsel_indep filter, gpointer data);

/** @brief Add a multi-device filter to the filter set. */
void cl4_devsel_add_dep_filter(
	CL4DevSelFilters* filters, cl4_devsel_dep filter, gpointer data);

/**
 * @defgroup CL4_DEVSEL_INDEP_FILTERS Single-device filters.
 *
 * @{
 */

gboolean cl4_devsel_indep_gpu(
	CL4Device* device, void *data, GError **err);

gboolean cl4_devsel_indep_cpu(
	CL4Device* device, void *data, GError **err);

gboolean cl4_devsel_indep_accel(
	CL4Device* device, void *data, GError **err);

/** @} */

/**
 * @defgroup CL4_DEVSEL_DEP_FILTERS Multi-device filters.
 *
 * @{
 */

CL4DevSelDevices cl4_devsel_dep_platform(
	CL4DevSelDevices devices, void *data, GError **err);


/** @} */

#endif
