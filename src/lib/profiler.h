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

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/opencl.h>
#else
	#include <CL/opencl.h>
#endif

#include <glib.h>
#include <string.h>
#include "gerrorf.h"
#include "errors.h"
#include "common.h"


/** @brief Profile class, contains profiling information of OpenCL 
 * queues and events. */
typedef struct cl4_prof CL4Prof;

/** @brief Event instant. */
typedef struct cl4_prof_evinst CL4ProfEvInst;

/** @brief Aggregate event info. */
typedef struct cl4_prof_ev_aggregate CL4ProfEvAggregate;

/** @brief Export options. */
typedef struct cl4_prof_export_options CL4ProfExportOptions;

/** @brief Associates an OpenCL event with a name.  */
typedef struct cl4_prof_evname CL4ProfEvName;

/**
 * @brief Type of event instant (CL4ProfEvInst).
 */
typedef enum {
	/** CL4ProfEvInst is a start event instant. */
	CL4_PROF_EV_START,
	/** CL4ProfEvInst is an end event instant. */
	CL4_PROF_EV_END
} CL4ProfEvInstType;

/**
 * @brief Sorting strategy for event instants (CL4ProfEvInst).
 */
typedef enum {
	/** Sort event instants by instant. */
	CL4_PROF_EV_SORT_INSTANT,
	/** Sort event instants by event id. */
	CL4_PROF_EV_SORT_ID
} CL4ProfEvSort;

/**
 * @brief Sorting strategy for aggregate event data instances.
 */
typedef enum {
	 /** Sort aggregate event data instances by name. */
	CL4_PROF_AGGEVDATA_SORT_NAME,
	/** Sort aggregate event data instances by time. */
	CL4_PROF_AGGEVDATA_SORT_TIME
}  CL4ProfEvAggDataSort;

/** @brief Create a new OpenCL events profile. */
CL4Prof* cl4_prof_new();

/** @brief Free an OpenCL events profile. */
void cl4_prof_free(CL4Prof* profile);

/** @brief Indication that profiling sessions has started. */
void cl4_prof_start(CL4Prof* profile);

/** @brief Indication that profiling sessions has ended. */
void cl4_prof_stop(CL4Prof* profile);

/** @brief If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
gdouble cl4_prof_time_elapsed(CL4Prof* profile);

/** @brief Add OpenCL event to events profile, more specifically adds
 * the start and end instants of the given event to the profile. */
int cl4_prof_add(CL4Prof* profile, const char* event_name, cl_event ev, GError** err);

/** @brief Add OpenCL event to events profile. Receives a CL4ProfEvName
 * instead of a separate event and name like cl4_prof_add(). */
int cl4_prof_add_evname(CL4Prof* profile, CL4ProfEvName event_with_name, GError** err);

/** @brief Add OpenCL events to events profile, more specifically adds
 * the start of ev1 and end of ev2 to the profile. */
int cl4_prof_add_composite(CL4Prof* profile, const char* event_name, cl_event ev1, cl_event ev2, GError** err);

/** @brief Create new event instant. */
CL4ProfEvInst* cl4_prof_evinst_new(const char* eventName, guint id, cl_ulong instant, CL4ProfEvInstType type, cl_command_queue queue);

/** @brief Free an event instant. */
void cl4_prof_evinst_free(gpointer event_instant);

/** @brief Compares two event instants for sorting purposes. */
gint cl4_prof_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata);

/** @brief Determine overlap matrix for the given OpenCL events profile. */
int cl4_prof_overmat(CL4Prof* profile, GError** err);

/** @brief Determine aggregate statistics for the given OpenCL events profile. */
int cl4_prof_aggregate(CL4Prof* profile, GError** err);

/** @brief Create a new aggregate statistic for events of a given type. */
CL4ProfEvAggregate* cl4_prof_aggregate_new(const char* eventName);

/** @brief Free an aggregate statistic. */
void cl4_prof_aggregate_free(gpointer agg);

/** @brief Print profiling info. */
int cl4_prof_print_info(CL4Prof* profile, CL4ProfEvAggDataSort evAggSortType, GError** err);

/** @brief Export profiling info to a given stream. */
int cl4_prof_export_info(CL4Prof* profile, FILE* stream, GError** err);

/** @brief Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * cl4_prof_export_info() for more information. */
int cl4_prof_export_info_file(CL4Prof* profile, const char* filename, GError** err);

/** @brief Set export options using a ::CL4ProfExportOptions struct. */
void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts);

/** @brief Get current export options. */
 CL4ProfExportOptions cl4_prof_export_opts_get();

#endif
