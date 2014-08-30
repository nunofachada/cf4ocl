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
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_ERRORS_H_
#define _CCL_ERRORS_H_

/**
 * @defgroup ERRORS Errors
 *
 * This module offers a function to convert OpenCL error codes into
 * human-readable strings. It is widely used by other _cf4ocl_
 * modules, but may also be useful to client code.
 *
 * _Example:_
 *
 * @code{.c}
 *
 * buf = ccl_buffer_new(ctx, flags, size, host_ptr, &err);
 * if ((err) && (err->domain == CCL_OCL_ERROR)) {
 *     fprintf(
 *         stderr, "OpenCL error %d: %s", err->code, cl_err(err->code));
 * }
 *
 * @endcode
 *
 * @{
 */

/* Convert OpenCL error code to a readable string. */
const char* ccl_err(int code);

/** @} */

#endif
