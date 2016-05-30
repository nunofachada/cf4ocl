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
 * Utility to query OpenCL platforms and devices header file.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#ifndef _CCL_DEVINFO_H_
#define _CCL_DEVINFO_H_

#include "ccl_utils.h"

#ifndef CCL_DEVINFO_OUT
	/** Default device information output stream. */
	#define CCL_DEVINFO_OUT stdout
#endif

#ifndef CCL_DEVINFO_NA
	#define CCL_DEVINFO_NA "N/A"
#endif

/** Program description. */
#define CCL_DEVINFO_DESCRIPTION "Utility for querying OpenCL \
platforms and devices"

/** Maximum length of device information output, per parameter. */
#define CCL_DEVINFO_MAXINFOLEN 500

/** Parse and verify command line arguments. */
void ccl_devinfo_args_parse(int argc, char* argv[], GError** err);

/** Show platform information. */
void ccl_devinfo_show_platform_info(CCLPlatform* p, guint idx);

/** Show all available device information. */
void ccl_devinfo_show_device_info_all(CCLDevice* d);

/** Show user specified device information. */
void ccl_devinfo_show_device_info_custom(CCLDevice* d);

/** Show basic device information. */
void ccl_devinfo_show_device_info_basic(CCLDevice* d);

#endif
