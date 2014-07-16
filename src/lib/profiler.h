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
 * @brief Function headers of a profiling tool for OpenCL.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef CL4_PROF_H
#define CL4_PROF_H

#include <glib.h>
#include <string.h>
#include "oclversions.h"
#include "gerrorf.h"
#include "errors.h"
#include "common.h"
#include "cqueue.h"

/** @brief Profile class, contains profiling information of OpenCL 
 * queues and events. */
typedef struct cl4_prof CL4Prof;

/**
 * @brief Export options.
 * */
typedef struct cl4_prof_export_options {

	/** Field separator, defaults to tab (\\t). */
	const char* separator;

	/** Newline character, Defaults to Unix newline (\\n). */
	const char* newline;

	/** Queue name delimiter, defaults to empty string. */
	const char* queue_delim;

	/** Event name delimiter, defaults to empty string. */
	const char* evname_delim;

	/** Use simple queue IDs (0,1...n) instead of using the queue memory 
	 * location as ID (defaults to TRUE). */
	gboolean simple_queue_id;

	/** Start at instant 0 (TRUE, default), or start at oldest instant 
	 * returned by OpenCL (FALSE). */
	gboolean zero_start;
	
}  CL4ProfExportOptions;

/** @brief Create a new profile object. */
CL4Prof* cl4_prof_new();

/** @brief Destroy a profile object. */
void cl4_prof_destroy(CL4Prof* profile);

/** @brief Starts the global profiler timer. Only required if client
* wishes to compare the effectively ellapsed time with the OpenCL
* kernels time. */
void cl4_prof_start(CL4Prof* profile);

/** @brief Stops the global profiler timer. Only required if 
 * cl4_prof_start() was called. */
void cl4_prof_stop(CL4Prof* profile);

/** @brief If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
double cl4_prof_time_elapsed(CL4Prof* profile);

/** @brief Add a command queue wrapper for profiling. */
void cl4_prof_add_queue(
	CL4Prof* prof, const char* cq_name, CL4CQueue* cq);

/** @brief Determine aggregate statistics for the given profile object. */
cl_bool cl4_prof_calc(CL4Prof* profile, GError** err);

//~ /** @brief Add OpenCL event to events profile, more specifically adds
 //~ * the start and end instants of the given event to the profile. */
//~ int cl4_prof_add(CL4Prof* profile, const char* event_name, cl_event ev, GError** err);
//~ 
//~ /** @brief Add OpenCL event to events profile. Receives a CL4ProfEvName
 //~ * instead of a separate event and name like cl4_prof_add(). */
//~ int cl4_prof_add_evname(CL4Prof* profile, CL4ProfEvName event_with_name, GError** err);
//~ 
//~ /** @brief Add OpenCL events to events profile, more specifically adds
 //~ * the start of ev1 and end of ev2 to the profile. */
//~ int cl4_prof_add_composite(CL4Prof* profile, const char* event_name, cl_event ev1, cl_event ev2, GError** err);
//~ 
//~ /** @brief Create new event instant. */
//~ CL4ProfEvInst* cl4_prof_evinst_new(const char* eventName, guint id, cl_ulong instant, CL4ProfEvInstType type, cl_command_queue queue);
//~ 
//~ /** @brief Free an event instant. */
//~ void cl4_prof_evinst_destroy(gpointer event_instant);
//~ 
//~ /** @brief Compares two event instants for sorting purposes. */
//~ gint cl4_prof_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata);
//~ 
//~ /** @brief Determine overlap matrix for the given OpenCL events profile. */
//~ int cl4_prof_overmat(CL4Prof* profile, GError** err);
//~ 
//~ /** @brief Create a new aggregate statistic for events of a given type. */
//~ CL4ProfEvAgg* cl4_prof_agg_new(const char* eventName);
//~ 
//~ /** @brief Free an aggregate statistic. */
//~ void cl4_prof_agg_destroy(gpointer agg);
//~ 
//~ /** @brief Print profiling info. */
//~ int cl4_prof_print_info(CL4Prof* profile, CL4ProfEvAggDataSort evAggSortType, GError** err);
//~ 
//~ /** @brief Export profiling info to a given stream. */
//~ int cl4_prof_export_info(CL4Prof* profile, FILE* stream, GError** err);
//~ 
//~ /** @brief Helper function which exports profiling info to a given file,
 //~ * automatically opening and closing the file. Check the
 //~ * cl4_prof_export_info() for more information. */
//~ int cl4_prof_export_info_file(CL4Prof* profile, const char* filename, GError** err);
//~ 
//~ /** @brief Set export options using a ::CL4ProfExportOptions struct. */
//~ void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts);
//~ 
//~ /** @brief Get current export options. */
//~ CL4ProfExportOptions cl4_prof_export_opts_get();

#endif
