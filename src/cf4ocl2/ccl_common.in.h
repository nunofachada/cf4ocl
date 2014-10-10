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
 *
 * Common _cf4ocl_ definitions.
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
#include "ccl_oclversions.h"
#include "ccl_export.h"

#define CCL_VERSION_MAJOR @cf4ocl2_VERSION_MAJOR@
#define CCL_VERSION_MINOR @cf4ocl2_VERSION_MINOR@
#define CCL_VERSION_PATCH @cf4ocl2_VERSION_PATCH@
#define CCL_VERSION_TWEAK "@cf4ocl2_VERSION_TWEAK@"
#define CCL_VERSION_STRING "@cf4ocl2_VERSION_STRING@"
#define CCL_VERSION_STRING_FULL "@cf4ocl2_VERSION_STRING_FULL@"

#define CCL_KERNELS_PATH "@KERNELS_PATH@"

#define CCL_VALIDFILECHARS "abcdefghijklmnopqrstuvwxyzABCDEFGH" \
	"IJKLMNOPQRSTUVWXYZ0123456789_."

#define CCL_UNUSED(x) (void)(x)

/* These deprecation macros are copied from GLib latest version in
 * order to support Clang. */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#define CCL_BEGIN_IGNORE_DEPRECATIONS \
	_Pragma ("GCC diagnostic push") \
	_Pragma ("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define CCL_END_IGNORE_DEPRECATIONS \
	_Pragma ("GCC diagnostic pop")
#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#define CCL_BEGIN_IGNORE_DEPRECATIONS \
	__pragma (warning (push)) \
	__pragma (warning (disable : 4996))
#define CCL_END_IGNORE_DEPRECATIONS \
	__pragma (warning (pop))
#elif defined (__clang__)
#define CCL_BEGIN_IGNORE_DEPRECATIONS \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define CCL_END_IGNORE_DEPRECATIONS \
	_Pragma("clang diagnostic pop")
#else
#define CCL_BEGIN_IGNORE_DEPRECATIONS
#define CCL_END_IGNORE_DEPRECATIONS
#endif

/**
 * Buffer wrapper class
 *
 * @extends ccl_memobj
 * */
typedef struct ccl_buffer CCLBuffer;

/**
 * Context wrapper class.
 *
 * @extends ccl_dev_container
 * */
typedef struct ccl_context CCLContext;

/**
 * Device wrapper class.
 *
 * @extends ccl_wrapper
 * */
typedef struct ccl_device CCLDevice;

/**
 * Event wrapper class.
 *
 * @extends ccl_wrapper
 * */
typedef struct ccl_event CCLEvent;

/**
 * Image wrapper class
 *
 * @extends ccl_memobj
 * */
typedef struct ccl_image CCLImage;

/**
 * Kernel wrapper class.
 *
 * @extends ccl_wrapper
 */
typedef struct ccl_kernel CCLKernel;

/**
 * Platform wrapper class.
 *
 * @extends ccl_dev_container
 * */
typedef struct ccl_platform CCLPlatform;

/**
 * Program wrapper class.
 *
 * @extends ccl_dev_container
 */
typedef struct ccl_program CCLProgram;

/**
 * Sampler wrapper class
 *
 * @extends ccl_wrapper
 * */
typedef struct ccl_sampler CCLSampler;

/**
 * Command queue wrapper class.
 *
 * @extends ccl_wrapper
 */
typedef struct ccl_queue CCLQueue;

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
	/** Invalid data passed to a function or returned from function. */
	CCL_ERROR_INVALID_DATA     = 3,
	/** Error writing to a stream. */
	CCL_ERROR_STREAM_WRITE     = 4,
	/** The requested OpenCL device was not found. */
	CCL_ERROR_DEVICE_NOT_FOUND = 5,
	/** The operation is not supported by the version of the
	 * selected OpenCL platform. */
	CCL_ERROR_UNSUPPORTED_OCL  = 6,
	/** Object information is unavailable. */
	CCL_ERROR_INFO_UNAVAILABLE_OCL = 7,
	/** Any other errors. */
	CCL_ERROR_OTHER            = 15
} CCLErrorCode;


/**
 * If error is detected (`error_code != no_error_code`),
 * create an error object (GError) and go to the specified label.
 *
 * @param[out] err GError* object.
 * @param[in] quark Quark indicating the error domain.
 * @param[in] error_condition Must result to true in order to create
 * error.
 * @param[in] error_code Error code to set.
 * @param[in] label Label to goto if error is detected.
 * @param[in] msg Error message in case of error.
 * @param[in] ... Extra parameters for error message.
 * */
#define ccl_if_err_create_goto(err, quark, error_condition, error_code, label, msg, ...) \
	if (error_condition) { \
		g_debug(G_STRLOC); \
		g_set_error(&(err), (quark), (error_code), (msg), ##__VA_ARGS__); \
		goto label; \
	}

/**
 * If error is detected in `err` object (`err != NULL`),
 * go to the specified label.
 *
 * @param[in] err GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_goto(err, label)	\
	if ((err) != NULL) { \
		g_debug(G_STRLOC); \
		goto label; \
	}

/**
 * Same as ccl_if_err_goto(), but rethrows error in a source
 * GError to a new destination GError object.
 *
 * @param[out] err_dest Destination GError** object.
 * @param[in] err_src Source GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_propagate_goto(err_dest, err_src, label) \
	if ((err_src) != NULL) { \
		g_debug(G_STRLOC); \
		g_propagate_error(err_dest, err_src); \
		goto label; \
	}

/** Resolves to error category identifying string, in this case an error
 * in _cf4ocl_. */
#define CCL_ERROR ccl_error_quark()

/** Resolves to error category identifying string, in this case
 * an error in the OpenCL library. */
#define CCL_OCL_ERROR ccl_ocl_error_quark()

/* Print executable version. */
CCL_EXPORT
void ccl_common_version_print(const char* exec_name);

/* Resolves to error category identifying string, in this case
 * an error in _cf4ocl_. */
CCL_EXPORT
GQuark ccl_error_quark(void);

/* Resolves to error category identifying string, in this case
 * an error in the OpenCL library. */
CCL_EXPORT
GQuark ccl_ocl_error_quark(void);

#endif
