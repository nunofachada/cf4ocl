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
 * Convert OpenCL error codes to readable strings (function header).
 *
 * @author Nuno Fachada
 * @date 2016
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_ERROR_H_
#define _CCL_ERROR_H_

#include "ccl_common.h"

/**
 * @defgroup CCL_ERRORS Errors
 *
 * This module offers a function to convert OpenCL error codes into
 * human-readable strings. It is widely used by other _cf4ocl_
 * modules, but may also be useful to client code which directly uses
 * OpenCL functions.
 *
 * _Example:_
 *
 * @code{.c}
 * cl_int status;
 * cl_event event;
 * @endcode
 * @code{.c}
 * status = clWaitForEvents(1, &event);
 * if (status != CL_SUCCESS) {
 *     fprintf(stderr, "OpenCL error %d: %s", status, ccl_err(status));
 * }
 * @endcode
 *
 * @{
 */

/* Convert OpenCL error code to a readable string. */
CCL_EXPORT
const char * ccl_err(int code);

/** @} */

#endif
