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
 * @brief Common implementations for examples.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "exp_common.h"

/**
 * @brief Print device requirements for program.
 * 
 * @param gws Global work size.
 * @param lws Local work size.
 * @param gmem Global memory required.
 * @param lmem Local memory required.
 * */
void clexp_reqs_print(size_t* gws, size_t* lws, size_t gmem, size_t lmem) {
	
	printf("\n   ========================= Execution requirements ========================\n\n");
	printf("     Global work size       : (%lu, %lu)\n", (unsigned long) gws[0], (unsigned long) gws[1]);
	printf("     Local work size        : (%lu, %lu)\n", (unsigned long) lws[0], (unsigned long) lws[1]);
	printf("     Global memory required : %lu bytes (%lu Kb = %lu Mb)\n", (unsigned long) gmem, (unsigned long) (gmem / 1024), (unsigned long) (gmem / 1024 / 1024));
	printf("     Local memory required  : %lu bytes (%lu Kb)\n", (unsigned long) lmem, (unsigned long) (lmem / 1024));
}


/** 
 * @brief Resolves to error category identifying string, in this case an error related to the OpenCL examples.
 * 
 * @return A GQuark structure defined by category identifying string, which identifies the error as OpenCL examples generated error.
 */
GQuark clexp_error_quark() {
	return g_quark_from_static_string("clexp-error-quark");
}
