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
 * @brief Common cf4ocl definitions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_COMMON_H
#define CL4_COMMON_H 

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

/**
 * @brief Error codes.
 * */
enum cl4_error_codes {
	CL4_SUCCESS = 0,        /**< Successful operation. */
	CL4_ERROR_NOALLOC = 1,  /**< Error code thrown when no memory allocation is possible. */
	CL4_ERROR_OPENFILE = 2, /**< Error code thrown when it's not possible to open file. */
	CL4_ERROR_ARGS = 3,     /**< Error code thrown when passed arguments are invalid. */
	CL4_ERROR_STREAM_WRITE = 5,     /**< Error code thrown when an error occurs while writing to a stream. */
	CL4_OCL_ERROR = 10      /**< An OpenCL error ocurred. */
};

/** Resolves to error category identifying string, in this case an error in the OpenCL utilities library. */
#define CL4_ERROR cl4_error_quark()

/** @brief Resolves to error category identifying string, in this case
 * an error in the OpenCL utilities library. */
GQuark cl4_error_quark(void);

#endif
