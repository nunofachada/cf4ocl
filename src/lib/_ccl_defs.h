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
 * Useful definitions used internally by _cf4ocl_. This header is not part of
 * the public API.
 *
 * @author Nuno Fachada
 * @date 2017
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_DEFS_H_
#define _CCL_DEFS_H_

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
 * @internal
 * A `g_info()` macro for older GLib versions which do not provide it.
 *
 * @def g_info()
 * */
#ifndef g_info
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

/* The CCL_STRD macro determines which debug information appear in error
 * messages. */
#ifndef CCL_STRD
    #ifdef NDEBUG
        /* If the NDEBUG macro is set, CCL_STRD will expand to a string
         * identifying the current code position. */
        #define CCL_STRD G_STRFUNC
    #else
        /* Otherwise it will expand to a string identifying the current
         * function. */
        #define CCL_STRD G_STRLOC
    #endif
#endif

/* The error-handling macros output to the debug log the stack trace when an
 * error occurs. This will be either the code position or function, as defined
 * by CCL_STRD. */
#define G_ERR_DEBUG_STR CCL_STRD

/* Include error handling macros. */
#include "_g_err_macros.h"

#endif /* _CCL_DEFS_H_ */
