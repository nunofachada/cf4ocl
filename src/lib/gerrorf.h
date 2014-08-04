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
 * @brief Error handling framework based on GLib's GError object.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _GERRORF_H_
#define _GERRORF_H_

#include <glib.h>

/** 
 * @brief If error is detected (`error_code != no_error_code`), 
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
#define gef_if_err_create_goto(err, quark, error_condition, error_code, label, msg, ...) \
	if (error_condition) { \
		g_set_error(&(err), (quark), (error_code), (msg), ##__VA_ARGS__); \
		goto label; \
	}

/** 
 * @brief If error is detected in `err` object (`err != NULL`),
 * go to the specified label.
 * 
 * @param[in] err GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define gef_if_err_goto(err, label)	\
	if ((err) != NULL) { \
		goto label; \
	}

/** 
 * @brief Same as gef_if_err_goto(), but rethrows error in a source
 * GError to a new destination GError object.
 * 
 * @param[out] err_dest Destination GError** object.
 * @param[in] err_src Source GError* object.
 * @param[in] label Label to goto if error is detected.
 * */
#define gef_if_err_propagate_goto(err_dest, err_src, label) \
	if ((err_src) != NULL) { \
		g_propagate_error(err_dest, err_src); \
		goto label; \
	}

#endif
