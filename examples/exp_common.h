/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */
 
/** 
 * @file
 * @brief Common includes and definitions for examples.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#ifndef CLEXP_COMMON_H
#define CLEXP_COMMON_H

#include <math.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include "clprofiler.h"
#include "clutils.h"
#include "gerrorf.h"

/** Helper macro to convert int to string at compile time. */
#define STR_HELPER(x) #x
/** Macro to convert int to string at compile time. */
#define STR(x) STR_HELPER(x)

/**
 * @brief Parse a pair of positive integers from a string separated by a comma.
 * 
 * @param in Input string from where to extract pair of integers.
 * @param out Array where to put pair of integers.
 * @param option_name Not used.
 * @param data Not used.
 * @param err GLib error object for error reporting.
 * @return TRUE if pair of numbers exists, FALSE otherwise.
 * */
#define clexp_parse_pairs(in, out, option_name, data, err) \
	/* Avoid compiler warnings. */ \
	option_name = option_name; data = data; \
	/* Two numbers must be read... */ \
	if (sscanf(in, "%6d,%6d", (int*) &out[0], (int*) &out[1]) == 2) { \
		/* Ok! */ \
		return TRUE; \
	} else { \
		/* Bad argument. */ \
		g_set_error(err, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, "The option '%s' does not accept the argument '%s'", option_name, value); \
		return FALSE; \
	} \

/** @brief Code for operation successful. */
#define CLEXP_SUCCESS 0
/** @brief Code for operation failed. */
#define CLEXP_FAIL -1

/** @brief Print device requirements for program. */
void clexp_reqs_print(size_t* gws, size_t* lws, size_t gmem, size_t lmem);

/** @brief Resolves to error category identifying string. Required by glib error reporting system. */
#define CLEXP_ERROR clexp_error_quark()

/** @brief Resolves to error category identifying string, in this case an error related to the OpenCL examples. */
GQuark clexp_error_quark();

#endif
