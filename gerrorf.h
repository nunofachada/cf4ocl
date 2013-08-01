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

#ifndef GERRORF_H
#define GERRORF_H

#include <glib.h>

/**
 * @brief Program error codes.
 * */ 
enum gerrorf_flags {
	GEF_USE_STATUS = -6000,				/**< Don't change status variable. */
	GEF_USE_GERROR = -7000				/**< Use error code in GError object. */
};


/** 
 * @brief If error is detected (`error_code != no_error_code`), 
 * create an error object (GError) and go to the specified label. 
 * 
 * @param err GError* object.
 * @param quark Quark indicating the error domain.
 * @param error_condition Must result to true in order to create error.
 * @param error_code Error code to set.
 * @param label Label to goto if error is detected.
 * @param msg Error message in case of error.
 * @param ... Extra parameters for error message. 
 * */
#define gef_if_error_create_goto(err, quark, error_condition, error_code, label, msg, ...) \
	if (error_condition) { \
		g_set_error(&(err), (quark), (error_code), (msg), ##__VA_ARGS__); \
		goto label; \
	}
	
	
/** 
 * @brief If error is detected in `err` object (`err != NULL`),
 * set `status` to specified `error_code`
 * OR to error code set in the GError object if 
 * `error_code = ` @link gerrorf_flags::GEF_USE_GERROR @endlink) 
 * OR leave status untouched if
 * `error_code = ` @link gerrorf_flags::GEF_USE_STATUS @endlink) 
 * and go to the specified label.
 * 
 * @param err GError* object.
 * @param error_code Error code.
 * @param status Error status variable.
 * @param label Label to goto if error is detected.
 * */
#define gef_if_error_goto(err, error_code, status, label)	\
	if ((err) != NULL) { \
		if ((error_code) != (int) GEF_USE_STATUS) { \
			status = ((int) (error_code) == (int) GEF_USE_GERROR) ? (err)->code : (error_code); \
		} \
		goto label; \
	}
#endif
