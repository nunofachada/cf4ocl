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
#include <glib/gprintf.h>

/**
 * @brief Error codes.
 * */
enum cl4_error_codes {
	CL4_SUCCESS = 0,        /**< Successful operation. */
	CL4_ERROR_NOALLOC = 1,  /**< Error code thrown when no memory allocation is possible. */
	CL4_ERROR_OPENFILE = 2, /**< Error code thrown when it's not possible to open file. */
	CL4_ERROR_ARGS = 3,     /**< Error code thrown when passed arguments are invalid. */
	CL4_ERROR_INVALID_DATA = 4,     /**< Error code thrown when invalid data is passed to a function. */
	CL4_ERROR_STREAM_WRITE = 5,     /**< Error code thrown when an error occurs while writing to a stream. */
	CL4_ERROR_DEVICE_NOT_FOUND = 6, /**< The requested OpenCL device was not found. */
	CL4_ERROR_OCL = 10      /**< An OpenCL error ocurred. */
};

/** Resolves to error category identifying string, in this case an error in the OpenCL utilities library. */
#define CL4_ERROR cl4_error_quark()

/**
 * @brief Information about a wrapped OpenCL entity.
 * */
typedef struct cl4_info {
	/** Device information. */
	gpointer value;
	/** Size in bytes of device information. */
	gsize size;
} CL4Info;

/** @brief Create a new CL4Info* object. */
CL4Info* cl4_info_new(gpointer value, gsize size);

/** @brief Destroy a CL4Info* object. */
void cl4_info_destroy(void* info_value);

/** @brief Resolves to error category identifying string, in this case
 * an error in the OpenCL utilities library. */
GQuark cl4_error_quark(void);

#endif
