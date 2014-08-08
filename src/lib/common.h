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
 * Common cf4ocl definitions.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_COMMON_H_
#define _CCL_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "oclversions.h"

#define CCL_VALIDFILECHARS "abcdefghijklmnopqrstuvwxyzABCDEFGH" \
	"IJKLMNOPQRSTUVWXYZ0123456789_."

/**
 * Error codes.
 * */
typedef enum ccl_error_code {
	/** Successful operation. */
	CCL_SUCCESS                = 0,
	/** Unable to open file. */
	CCL_ERROR_OPENFILE         = 1,
	/** Invalid program arguments. */
	CCL_ERROR_ARGS             = 2,
	/** Invalid data passed to a function. */
	CCL_ERROR_INVALID_DATA     = 3,
	/** Error writing to a stream. */
	CCL_ERROR_STREAM_WRITE     = 4,
	/** The requested OpenCL device was not found. */
	CCL_ERROR_DEVICE_NOT_FOUND = 5,
	/** The operation is not supported by the version of the
	 * selected OpenCL platform. */
	CCL_ERROR_UNSUPPORTED_OCL  = 6,
	/** Any other errors. */
	CCL_ERROR_OTHER            = 15
} CCLErrorCode;

/** Resolves to error category identifying string, in this case an error 
 * in the cf4ocl. */
#define CCL_ERROR ccl_error_quark()

/** Resolves to error category identifying string, in this case an error 
 * in the cf4ocl. */
#define CCL_OCL_ERROR ccl_ocl_error_quark()

/** Resolves to error category identifying string, in this case
 * an error in cf4ocl. */
GQuark ccl_error_quark(void);

/** Resolves to error category identifying string, in this case
 * an error in the OpenCL library. */
GQuark ccl_ocl_error_quark(void);

#endif
