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
 * @internal
 *
 * @file
 * Useful definitions used internally by _cf4ocl_. This header is not part of
 * the public API.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_DEFS_H_
#define _CCL_DEFS_H_

/* GLib must be included before this file. */
#ifndef __G_LIB_H__
    #error "GLib must be included before this file"
#endif

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

/**
 * If error is detected (`error_code != no_error_code`),
 * create an error object (CCLErr) and go to the specified label.
 *
 * @param[out] err CCLErr* object.
 * @param[in] quark Quark indicating the error domain.
 * @param[in] error_condition Must result to true in order to create
 * error.
 * @param[in] error_code Error code to set.
 * @param[in] label Label to goto if error is detected.
 * @param[in] msg Error message in case of error.
 * @param[in] ... Extra parameters for error message.
 * */
#define ccl_if_err_create_goto( \
    err, quark, error_condition, error_code, label, msg, ...) \
    if (error_condition) { \
        g_set_error(&(err), (quark), (error_code), (msg), ##__VA_ARGS__); \
        g_debug(G_ERR_DEBUG_STR); \
        goto label; \
    }

/**
 * If error is detected in `err` object (`err != NULL`), go to the specified
 * label.
 *
 * @param[in] err CCLErr* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_goto(err, label) \
    if ((err) != NULL) { \
        g_debug(G_ERR_DEBUG_STR); \
        goto label; \
    }

/**
 * Same as ccl_if_err_goto(), but rethrows error in a source CCLErr object to
 * a new destination CCLErr object.
 *
 * @param[out] err_dest Destination CCLErr** object.
 * @param[in] err_src Source CCLErr* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define ccl_if_err_propagate_goto(err_dest, err_src, label) \
    if ((err_src) != NULL) { \
        g_debug(G_ERR_DEBUG_STR); \
        g_propagate_error((err_dest), (err_src)); \
        err_src = NULL; \
        goto label; \
    }

#endif /* _CCL_DEFS_H_ */
