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
 * Program which determines where the system shared data directory for
 * placing the OpenCL kernels.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 */
 
#include <glib.h>
#include <glib/gstdio.h>

/**
 * Test if a given path is a valid location for the kernel files.
 * 
 * @param[in] path Path to test.
 * @return `TRUE` if `path` is a valid location for the kernel files,
 * `FALSE` otherwise.
 * */
gboolean ccl_config_test_path(const gchar* path) {
	
	gchar* filename;
	gboolean status;

	filename = g_build_filename(path, "remove.me", NULL);
	if (g_file_set_contents(filename, "\n", -1, NULL)) {
		g_unlink(filename);
		status = TRUE;
	} else {
		status = FALSE;
	}
	g_free(filename);
	return status;
}

/**
 * Determines where to place the _cf4ocl_ kernel examples in a 
 * OS-independent fashion, and prints the full path to `stdout`.
 * 
 * @return 0 if program successfully determines where to place the
 * kernel examples, or -1 otherwise.
 * */
int main() {

	/* Get system-wide data paths. */
	const gchar* const* sdd = g_get_system_data_dirs();
	/* Valid location flag. */
	gboolean valid;
	
	/* Try system-wide data paths. */
	for (int i = 0; sdd[i] != NULL; ++i) {
		valid = ccl_config_test_path(sdd[i]);
		if (valid) {
			g_fprintf(stdout, "%s", sdd[i]);
			break;
		}
	}

	/* If a valid system-wide location wasn't obtained, try a local
	 * user path. */
	if (!valid) {
		valid = ccl_config_test_path(g_get_user_data_dir());
		if (valid) {
			g_fprintf(stdout, "%s", g_get_user_data_dir());
		}
	}
	
	/* Return 0 if valid path was found or -1 if not. */
	return valid ? 0 : -1;
}
