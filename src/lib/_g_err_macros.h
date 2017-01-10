/*
 * The MIT License (MIT)
 * Copyright (c) 2016 Nuno Fachada
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * */

/**
 * @file
 * Error-handling macros for applications using Glib.
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [The MIT License (MIT)](https://opensource.org/licenses/MIT)
*/

#ifndef __G_ERR_MACROS_H
#define __G_ERR_MACROS_H

#include <glib.h>

/* Macro which determines part of the information which appears in debug log
 * messages. */
#ifndef G_ERR_DEBUG_STR
	#ifdef NDEBUG
		#define G_ERR_DEBUG_STR G_STRFUNC
	#else
		#define G_ERR_DEBUG_STR G_STRLOC
	#endif
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
#define g_if_err_create_goto( \
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
 * @param[in] err GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define g_if_err_goto(err, label) \
	if ((err) != NULL) { \
		g_debug(G_ERR_DEBUG_STR); \
		goto label; \
	}

/**
 * Same as g_if_err_goto(), but rethrows error in a source GError object to
 * a new destination GError object.
 *
 * @param[out] err_dest Destination GError** object.
 * @param[in] err_src Source GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define g_if_err_propagate_goto(err_dest, err_src, label) \
	if ((err_src) != NULL) { \
		g_debug(G_ERR_DEBUG_STR); \
		g_propagate_error((err_dest), (err_src)); \
		err_src = NULL; \
		goto label; \
	}

#endif
