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
 * @brief Functions for obtaining information about OpenCL entities
 * such as platforms, devices, contexts, queues, kernels, etc.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#include "info.h"

/**
 * @brief Create a new CL4Info* object.
 * 
 * @param value Parameter value.
 * @param size Parameter size in bytes.
 * @return A new CL4Info* object.
 * */
CL4Info* cl4_info_new(gpointer value, gsize size) {
	
	CL4Info* info_value = g_slice_new(CL4Info);
	
	info_value->value = value;
	info_value->size = size;
	
	return info_value;
	
}

/**
 * @brief Destroy a CL4Info* object.
 * 
 * @param info_value Object to destroy.
 * */
void cl4_info_destroy(void* info_value) {
		
	g_free(((CL4Info*) info_value)->value);
	g_slice_free(CL4Info, info_value);
	
}


