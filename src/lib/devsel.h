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

/**
 * @brief Abstract function for selecting OpenCL devices.
 *
 * @param device
 * @param select_info 
 * @param err 
 * @return 
 */
typedef gboolean (*cl4_devsel)(
	CL4Device* device, void *select_info, GError **err);
	
typedef struct cl4_devsel_filter CL4DevSelFilter;
	
#define CL4DevSelFilters GPtrArray

#define cl4_devsel_filters_init() g_ptr_array_new()

GPtrArray* cl4_devsel_select(CL4DevSelFilters* filters, GError **err);

void cl4_devsel_add_filter(
	CL4DevSelFilters* filters, cl4_devsel filter, gpointer data);
	

#endif
