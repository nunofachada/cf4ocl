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
 * Common implementations for examples.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "examples_common.h"

/**
 * Print device requirements for program.
 * 
 * @param[in] gws Global work size.
 * @param[in] lws Local work size.
 * @param[in] gmem Global memory required.
 * @param[in] lmem Local memory required.
 * */
void ccl_ex_reqs_print(size_t* gws, size_t* lws, size_t gmem, size_t lmem) {
	
	g_printf("\n   ========================= Execution requirements ========================\n\n");
	g_printf("     Global work size       : (%lu, %lu)\n", 
		(unsigned long) gws[0], (unsigned long) gws[1]);
	g_printf("     Local work size        : (%lu, %lu)\n", 
		(unsigned long) lws[0], (unsigned long) lws[1]);
	g_printf("     Global memory required : %lu bytes (%lu Kb = %lu Mb)\n", 
		(unsigned long) gmem, (unsigned long) (gmem / 1024), (unsigned long) (gmem / 1024 / 1024));
	g_printf("     Local memory required  : %lu bytes (%lu Kb)\n", 
		(unsigned long) lmem, (unsigned long) (lmem / 1024));
}

/** 
 * Get full kernel path name. 
 * 
 * @param[in] kernel_filename Name of file containing kernels.
 * @return The full path of the kernel file (should be freed with 
 * g_free()), or `NULL` if file doesn't exist.
 * */
gchar* ccl_ex_kernelpath_get(gchar* kernel_filename) {
	
	
	/* System-wide data paths. */
	const gchar* const* sdd;
	/* Full kernel path name. */
	gchar* fullpath;
	
	/* Give priority to local user path. */
	fullpath = g_build_filename(g_get_user_data_dir(), "cf4ocl2", "cl", 
		kernel_filename, NULL);
	
	/* If file doesn't exist, reset variable and try system-wide data
	 * paths. */
	if (!g_file_test(fullpath, G_FILE_TEST_EXISTS)) {
		
		g_free(fullpath);
		sdd = g_get_system_data_dirs();
		
		for (int i = 0; sdd[i] != NULL; ++i) {
			fullpath = g_build_filename(sdd[i], "cf4ocl2", "cl", 
				kernel_filename, NULL);
				
			if (g_file_test(fullpath, G_FILE_TEST_EXISTS)) {
				break;
			}
			
			g_free(fullpath);
			fullpath = NULL;
		}

	}
	
	/* Return full kernel file path. */
	return fullpath;

}

/** 
 * Resolves to error category identifying string, in this case
 * an error in the cf4ocl examples.
 * 
 * @return A GQuark structure defined by category identifying string,
 * which identifies the error as a cf4ocl examples generated error.
 */
GQuark ccl_ex_error_quark() {
	return g_quark_from_static_string("cclexp-error-quark");
}
