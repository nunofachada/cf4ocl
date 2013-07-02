/*   
 * This file is part of CFrameworkOCL (C Framework for OpenCL).
 * 
 * CFrameworkOCL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CFrameworkOCL is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CFrameworkOCL.  If not, see <http://www.gnu.org/licenses/>.
 * */

/** 
 * @file
 * @brief Stubs for testing profiling framework.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#ifndef TEST_PROFILER_H
#define TEST_PROFILER_H

#include <stdlib.h>

/** @brief Mimicks OpenCL success code. */
#define CL_SUCCESS 0

/** @brief Replaces OpenCL cl_int type. */ 
typedef int cl_int;

/** @brief Replaces OpenCL cl_ulong type. */ 
typedef unsigned long cl_ulong;

/** @brief Stub for cl_event objects. */ 
typedef struct clEvent {
	cl_ulong start;
	cl_ulong end;
} cl_event;

/** @brief Stub for cl_profiling_info enum. */ 
typedef enum clProfilingInfo {
	CL_PROFILING_COMMAND_START,
	CL_PROFILING_COMMAND_END	
} cl_profiling_info;

/** @brief Stub for clGetEventProfilingInfo function. */ 
cl_int clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);

#endif
