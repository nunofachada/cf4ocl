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
 * Common _cf4ocl_ definitions.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_COMMON_H_
#define _CCL_COMMON_H_

/* GLib must be not included before this file. */
#ifdef __G_LIB_H__
    #error "GLib must not be included before this file"
#endif

#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_40

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

/** _cf4ocl_ version string in vx.x.x format. */
#define CCL_VERSION_STRING "@cf4ocl2_VERSION_STRING@"

/** Full _cf4ocl_ version string, includes commit SHA, etc. */
#define CCL_VERSION_STRING_FULL "@cf4ocl2_VERSION_STRING_FULL@"

/** Final _cf4ocl_ version string, either CCL_VERSION_STRING for stable
 * versions or cf4ocl2_VERSION_STRING_FULL for development versions. */
#define CCL_VERSION_STRING_FINAL "@cf4ocl2_VERSION_STRING_FINAL@"

/**
 * Version of OpenCL headers used to compile _cf4ocl_.
 *
 * @def CCL_OPENCL_VERSION
 * */
#if defined CL_VERSION_2_2
    #define CCL_OPENCL_VERSION "2.2"
#elif defined CL_VERSION_2_1
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
    #error "OpenCL version is not defined"
#endif

/** Compiler used to compile _cf4ocl_. */
#define CCL_COMPILER "@CMAKE_C_COMPILER_ID@"

/** Macro to avoid warning in unused variables. */
#define CCL_UNUSED(x) (void)(x)

/**
 * Class or type of wrapped OpenCL object.
 * */
typedef enum ccl_class {

    /** Buffer object. */
    CCL_BUFFER    = 0,
    /** Context object. */
    CCL_CONTEXT   = 1,
    /** Device object. */
    CCL_DEVICE    = 2,
    /** Event object. */
    CCL_EVENT     = 3,
    /** Image object. */
    CCL_IMAGE     = 4,
    /** Kernel object. */
    CCL_KERNEL    = 5,
    /** Platform object. */
    CCL_PLATFORM  = 6,
    /** Program object. */
    CCL_PROGRAM   = 7,
    /** Sampler object. */
    CCL_SAMPLER   = 8,
    /** Queue object. */
    CCL_QUEUE     = 9,
    /** No object, enumeration termination marker. */
    CCL_NONE      = 10

} CCLClass;

/**
 * Type of information to obtain using ccl_wrapper_get_info(),
 * ccl_wrapper_get_info_value() and ccl_wrapper_get_info_size(). This
 * enumeration is used by the `ccl_*_get_info()` macros and should rarely be
 * used in client code.
 * */
typedef enum ccl_info {

    /** Request information about context objects. */
    CCL_INFO_CONTEXT          = 0,
    /** Request information about device objects. */
    CCL_INFO_DEVICE           = 1,
    /** Request information about event objects. */
    CCL_INFO_EVENT            = 2,
    /** Request information about event profiling. */
    CCL_INFO_EVENT_PROFILING  = 3,
    /** Request information about image objects. */
    CCL_INFO_IMAGE            = 4,
    /** Request information about kernel objects. */
    CCL_INFO_KERNEL           = 5,
    /** Request information about kernel arguments. */
    CCL_INFO_KERNEL_ARG       = 6,
    /** Request information about kernel work-groups. */
    CCL_INFO_KERNEL_WORKGROUP = 7,
    /** Request information about kernel sub-groups. */
    CCL_INFO_KERNEL_SUBGROUP  = 8,
    /** Request information about memory objects. */
    CCL_INFO_MEMOBJ           = 9,
    /** Request information about platform objects. */
    CCL_INFO_PLATFORM         = 10,
    /** Request information about program objects. */
    CCL_INFO_PROGRAM          = 11,
    /** Request information about program builds. */
    CCL_INFO_PROGRAM_BUILD    = 12,
    /** Request information about sampler objects. */
    CCL_INFO_SAMPLER          = 13,
    /** Request information about queue objects. */
    CCL_INFO_QUEUE            = 14,
    /** Request information about pipe objects. */
    CCL_INFO_PIPE             = 15,
    /** Enumeration termination marker. */
    CCL_INFO_END              = 16

} CCLInfo;

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
 * Error handling class.
 *
 * This class is a wrapper for the
 * [GError](https://developer.gnome.org/glib/stable/glib-Error-Reporting.html#GError)
 * structure from [GLib](https://developer.gnome.org/glib/stable/), and has the
 * following fields:
 *
 * ```c
 * GQuark domain;
 * int    code;
 * char *  message;
 * ```
 *
 * @see @ref ug_errorhandle "Error handling" in _cf4ocl_.
 * @see @ref ug_deps "The GLib and OpenCL dependencies".
 */
typedef GError CCLErr;

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

/** Resolves to error category identifying string, in this case an error in
 * _cf4ocl_. */
#define CCL_ERROR ccl_error_quark()

/** Resolves to error category identifying string, in this case an error in the
 * OpenCL library. */
#define CCL_OCL_ERROR ccl_ocl_error_quark()

/* Print executable version. */
CCL_EXPORT
void ccl_common_version_print(const char * exec_name);

/* Frees a `NULL`-terminated array of strings, as well as each string it
 * contains. */
CCL_EXPORT
void ccl_strv_clear(char ** str_array);

/* If `err` or `*err` is `NULL`, does nothing. Otherwise, releases memory
 * occupied by `*err` and sets `*err` to `NULL`. */
CCL_EXPORT
void ccl_err_clear(CCLErr ** err);

/* Resolves to error category identifying string, in this case an error in
 * _cf4ocl_. */
CCL_EXPORT
GQuark ccl_error_quark(void);

/* Resolves to error category identifying string, in this case an error in the
 * OpenCL library. */
CCL_EXPORT
GQuark ccl_ocl_error_quark(void);

#endif
