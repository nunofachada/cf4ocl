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
 * @brief Utility to query OpenCL platforms and devices header file.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */

#ifndef CL4_DEVICE_QUERY_H
#define CL4_DEVICE_QUERY_H

#include "platforms.h"
#include "platform.h"
#include "device.h"
#include "devquery.h"
#include "common.h"

#ifndef CL4_DEVICE_QUERY_OUT
	#define CL4_DEVICE_QUERY_OUT stdout
#endif

#define CL4_DEVICE_QUERY_MAXINFOLEN 500

/** @brief Parse and verify command line arguments. */
void cl4_device_query_args_parse(int argc, char* argv[], GError** err);

void cl4_device_query_show_platform_info(CL4Platform* p);
void cl4_device_query_show_device_info_all(CL4Device* d);
void cl4_device_query_show_device_info_custom(CL4Device* d);
void cl4_device_query_show_device_info_basic(CL4Device* d);


#endif
