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
 * @date 2016
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

/** Major _cf4ocl_ version digit. */
#define CCL_VERSION_MAJOR @cf4ocl2_VERSION_MAJOR@

/** Minor _cf4ocl_ version digit. */
#define CCL_VERSION_MINOR @cf4ocl2_VERSION_MINOR@

/** Patch/micro _cf4ocl_ version digit. */
#define CCL_VERSION_PATCH @cf4ocl2_VERSION_PATCH@

/** Tweak _cf4ocl_ version string. */
#define CCL_VERSION_TWEAK "@cf4ocl2_VERSION_TWEAK@"

/** _cf4ocl_ version string. */
#define CCL_VERSION_STRING "@cf4ocl2_VERSION_STRING@"

/** Full _cf4ocl_ version string. */
#define CCL_VERSION_STRING_FULL "@cf4ocl2_VERSION_STRING_FULL@"

/**
 * Version of OpenCL headers used to compile _cf4ocl_.
 *
 * @def CCL_OPENCL_VERSION
 * */
#if defined CL_VERSION_2_1
	#define CCL_OPENCL_VERSION "2.1"
#elif defined CL_VERSION_2_0
	#define CCL_OPENCL_VERSION "2.0"
#elif defined CL_VERSION_1_2
	#define CCL_OPENCL_VERSION "1.2"
#elif defined CL_VERSION_1_1
	#define CCL_OPENCL_VERSION "1.1"
#elif defined CL_VERSION_1_0
	#define CCL_OPENCL_VERSION "1.0"
#else
	#define CCL_OPENCL_VERSION "0.0"
#endif

/** Version of GLib headers used to compile _cf4ocl_. */
#define CCL_GLIB_VERSION G_STRINGIFY(GLIB_MAJOR_VERSION) "." \
	G_STRINGIFY(GLIB_MINOR_VERSION) "." G_STRINGIFY(GLIB_MICRO_VERSION)

/** Compiler used to compile _cf4ocl_. */
#define CCL_COMPILER "@CMAKE_C_COMPILER_ID@"

/** Macro to avoid warning in unused variables. */
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
 * A `g_info()` macro for older GLib versions which do not provide it.
 *
 * @def g_info()
 * */
#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

/**
 * Class or type of wrapped OpenCL object.
 * */
typedef enum ccl_class {

	CCL_BUFFER    = 0,
	CCL_CONTEXT   = 1,
	CCL_DEVICE    = 2,
	CCL_EVENT     = 3,
	CCL_IMAGE     = 4,
	CCL_KERNEL    = 5,
	CCL_PLATFORM  = 6,
	CCL_PROGRAM   = 7,
	CCL_SAMPLER   = 8,
	CCL_QUEUE     = 9,
	CCL_NONE      = 10

} CCLClass;

/**
 * Base class for all OpenCL wrappers.
 * */
typedef struct ccl_wrapper CCLWrapper;

/**
 * Base class for wrappers which contain devices, i.e.,
 * ::CCLPlatform, ::CCLProgram and ::CCLContext. This class extends
 * ::CCLWrapper.
 *
 * @extends ccl_wrapper
 * */
typedef struct ccl_dev_container CCLDevContainer;

/**
 * Base class for memory object wrappers, i.e., ::CCLBuffer and
 * ::CCLImage.
 *
 * @ingroup CCL_MEMOBJ_WRAPPER
 * @extends ccl_wrapper
 * */
typedef struct ccl_memobj CCLMemObj;

/**
 * Buffer wrapper class
 *
 * @ingroup CCL_BUFFER_WRAPPER
 * @extends ccl_memobj
 * */
typedef struct ccl_buffer CCLBuffer;

/**
 * Context wrapper class.
 *
 * @ingroup CCL_CONTEXT_WRAPPER
 * @extends ccl_dev_container
 * */
typedef struct ccl_context CCLContext;

/**
 * Device wrapper class.
 *
 * @ingroup CCL_DEVICE_WRAPPER
 * @extends ccl_wrapper
 * */
typedef struct ccl_device CCLDevice;

/**
 * Event wrapper class.
 *
 * @ingroup CCL_EVENT_WRAPPER
 * @extends ccl_wrapper
 * */
typedef struct ccl_event CCLEvent;

/**
 * Image wrapper class
 *
 * @ingroup CCL_IMAGE_WRAPPER
 * @extends ccl_memobj
 * */
typedef struct ccl_image CCLImage;

/**
 * Kernel wrapper class.
 *
 * @ingroup CCL_KERNEL_WRAPPER
 * @extends ccl_wrapper
 */
typedef struct ccl_kernel CCLKernel;

/**
 * Platform wrapper class.
 *
 * @ingroup CCL_PLATFORM_WRAPPER
 * @extends ccl_dev_container
 * */
typedef struct ccl_platform CCLPlatform;

/**
 * Program wrapper class.
 *
 * @ingroup CCL_PROGRAM_WRAPPER
 * @extends ccl_dev_container
 */
typedef struct ccl_program CCLProgram;

/**
 * Sampler wrapper class
 *
 * @ingroup CCL_SAMPLER_WRAPPER
 * @extends ccl_wrapper
 * */
typedef struct ccl_sampler CCLSampler;

/**
 * Command queue wrapper class.
 *
 * @ingroup CCL_QUEUE_WRAPPER
 * @extends ccl_wrapper
 */
typedef struct ccl_queue CCLQueue;

/**
 * Class which represents the OpenCL platforms available
 * in the system.
 *
 * @ingroup CCL_PLATFORMS
 */
typedef struct ccl_platforms CCLPlatforms;

/**
 * Error codes.
 * */
typedef enum ccl_error_code {
	/** Successful operation. */
	CCL_SUCCESS                    = 0,
	/** Unable to open file. */
	CCL_ERROR_OPENFILE             = 1,
	/** Invalid function arguments. */
	CCL_ERROR_ARGS                 = 2,
	/** Invalid data passed to a function or returned from function. */
	CCL_ERROR_INVALID_DATA         = 3,
	/** Error writing to a stream. */
	CCL_ERROR_STREAM_WRITE         = 4,
	/** The requested OpenCL device was not found. */
	CCL_ERROR_DEVICE_NOT_FOUND     = 5,
	/** The operation is not supported by the version of the selected
	 * OpenCL platform. */
	CCL_ERROR_UNSUPPORTED_OCL      = 6,
	/** Object information is unavailable. */
	CCL_ERROR_INFO_UNAVAILABLE_OCL = 7,
	/** Any other errors. */
	CCL_ERROR_OTHER                = 15
} CCLErrorCode;

/* Macro which determines part of the information which appears in debug log
 * messages. */
#ifndef CCL_STRD
	#ifdef NDEBUG
		#define CCL_STRD G_STRFUNC
	#else
		#define CCL_STRD G_STRLOC
	#endif
#endif

/* Macro which determines how error messages are logged in the debug stream. */
#ifdef NDEBUG
	#define CCL_DEBUG_ERR(err) g_debug(CCL_STRD)
#else
	#define CCL_DEBUG_ERR(err) \
		g_debug("%s: %s [%s, error %d]", \
			CCL_STRD, (err)->message, \
			g_quark_to_string((err)->domain), \
			(err)->code)
#endif

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
		g_set_error(&(err), (quark), (error_code), (msg), ##__VA_ARGS__); \
		CCL_DEBUG_ERR(err); \
		goto label; \
	}

/**
 * If error is detected in `err` object (`err != NULL`), go to the specified
 * label.
 *
 * @param[in] err GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_goto(err, label)	\
	if ((err) != NULL) { \
		CCL_DEBUG_ERR(err); \
		goto label; \
	}

/**
 * Same as ccl_if_err_goto(), but rethrows error in a source GError to a new
 * destination GError object.
 *
 * @param[out] err_dest Destination GError** object.
 * @param[in] err_src Source GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_propagate_goto(err_dest, err_src, label) \
	if ((err_src) != NULL) { \
		CCL_DEBUG_ERR(err_src); \
		g_propagate_error(err_dest, err_src); \
		goto label; \
	}

/** Resolves to error category identifying string, in this case an error in
 * _cf4ocl_. */
#define CCL_ERROR ccl_error_quark()

/** Resolves to error category identifying string, in this case an error in the
 * OpenCL library. */
#define CCL_OCL_ERROR ccl_ocl_error_quark()

/* Print executable version. */
CCL_EXPORT
void ccl_common_version_print(const char* exec_name);

/* Resolves to error category identifying string, in this case an error in
 * _cf4ocl_. */
CCL_EXPORT
GQuark ccl_error_quark(void);

/* Resolves to error category identifying string, in this case an error in the
 * OpenCL library. */
CCL_EXPORT
GQuark ccl_ocl_error_quark(void);

#endif
