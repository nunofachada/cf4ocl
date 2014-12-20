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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 * Utilities used by the OpenCL testing stub.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "utils.h"

guint veclen(void* vector, size_t elem_size) {
	cl_ulong value;
	guint len = 0;
	do {
		value = 0;
		g_memmove(&value, ((char*) vector) + elem_size * len, elem_size);
		len++;
	} while (value != 0);
	return len;
}


void ocl_stub_create_event(
	cl_event* event, cl_command_queue queue, cl_command_type ctype) {

	if (event != NULL) { \
		*event = g_slice_new0(struct _cl_event);
		(*event)->t_queued = g_get_real_time();
		(*event)->t_submit = g_get_real_time();
		(*event)->t_start = g_get_real_time();
		(*event)->t_end = g_get_real_time();
		(*event)->command_queue = queue;
		if (queue != NULL) (*event)->context = queue->context;
		(*event)->command_type = ctype;
		(*event)->exec_status = CL_COMPLETE;
		(*event)->ref_count = 1;
	}
}
