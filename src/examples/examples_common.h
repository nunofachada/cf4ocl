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
 * Common includes and definitions for examples.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#ifndef _CCL_EXAMPLES_COMMON_H_
#define _CCL_EXAMPLES_COMMON_H_

#include <math.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <cf4ocl2.h>

/**
 * Parse a pair of positive integers from a string separated by a comma.
 * 
 * @param[in] in Input string from where to extract pair of integers.
 * @param[in] out Array where to put pair of integers.
 * @param[in] option_name Not used.
 * @param[in] data Not used.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return TRUE if pair of numbers exists, FALSE otherwise.
 * */
#define ccl_ex_parse_pairs(in, out, option_name, data, err) \
	/* Avoid compiler warnings. */ \
	option_name = option_name; data = data; \
	/* Two numbers must be read... */ \
	if (sscanf(in, "%6d,%6d", (int*) &out[0], (int*) &out[1]) == 2) { \
		/* Ok! */ \
		return TRUE; \
	} else { \
		/* Bad argument. */ \
		g_set_error(err, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE, \
			"The option '%s' does not accept the argument '%s'", \
			option_name, value); \
		return FALSE; \
	}


/* Print device requirements for program. */
void ccl_ex_reqs_print(size_t* gws, size_t* lws, size_t gmem, size_t lmem);

/* Get full kernel path name. */
gchar* ccl_ex_kernelpath_get(gchar* kernel_filename);

/**
 * Error codes.
 * */
enum ccl_ex_error_codes {
	/** Code for operation successful. */
	CCL_EX_SUCCESS = 0,
	/** Code for operation failed. */
	CCL_EX_FAIL = -1,
};

/** Resolves to error category identifying string, in this case an error 
 * in the cf4ocl examples. */
#define CCL_EX_ERROR ccl_ex_error_quark()

/* Resolves to error category identifying string, in this case
 * an error in the cf4ocl examples. */
GQuark ccl_ex_error_quark(void);

#endif
