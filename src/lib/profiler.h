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
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
#ifndef CL4_PROFILER_H
#define CL4_PROFILER_H

#ifdef TESTING
	#include "mocks_stubs/profiler/cl_stub.h"
#else
    #if defined(__APPLE__) || defined(__MACOSX)
        #include <OpenCL/cl.h>
    #else
        #include <CL/cl.h>
    #endif
#endif

#include <glib.h>
#include <string.h>
#include "gerrorf.h"
#include "errors.h"
#include "common.h"


/**
 * @brief Contains the profiling info of an OpenCL application.
 */
typedef struct cl4_prof_profile {
	GHashTable* unique_events;  /**< Hash table with keys equal to the unique events name, and values equal to a unique event id. */
	GHashTable* command_queues; /**< Table of existing OpenCL command queues. */
	GList* event_instants;      /**< Instants (start and end) of all events occuring in an OpenCL application. */
	guint num_event_instants;   /**< Total number of event instants in CL4ProfProfile#event_instants. */
	GHashTable* aggregate;      /**< Aggregate statistics for all events in CL4ProfProfile#event_instants. */
	cl_ulong* overmat;          /**< Overlap matrix for all events in event_instants. */
	cl_ulong totalEventsTime;   /**< Total time taken by all events. */
	cl_ulong totalEventsEffTime;/**< Total time taken by all events except intervals where events overlaped. */
	cl_ulong startTime;         /**< Time at which the first (oldest) event started. */
	GTimer* timer;              /**< Keeps track of time during the complete profiling session. */
} CL4ProfProfile;

/**
 * @brief Type of event instant (CL4ProfEvInst).
 */
typedef enum {
	PROFCL_EV_START, /**< CL4ProfEvInst is a start event instant. */
	PROFCL_EV_END    /**< CL4ProfEvInst is a end event instant. */
} CL4ProfEvInstType;

/**
 * @brief Sorting strategy for event instants (CL4ProfEvInst).
 */
typedef enum {
	PROFCL_EV_SORT_INSTANT, /**< Sort event instants by instant. */
	PROFCL_EV_SORT_ID       /**< Sort event instants by event id. */
} CL4ProfEvSort;

/**
 * @brief Sorting strategy for aggregate event data instances.
 */
typedef enum {
	PROFCL_AGGEVDATA_SORT_NAME, /**< Sort aggregate event data instances by name. */
	PROFCL_AGGEVDATA_SORT_TIME  /**< Sort aggregate event data instances by time. */
}  CL4ProfEvAggDataSort;

/**
 * @brief Event instant.
 */
typedef struct cl4_prof_evinst {
	const char* eventName; /**< Name of event which the instant refers to (CL4ProfEvInfo#eventName). */
	guint id;              /**< Event instant ID. */
	cl_ulong instant;      /**< Event instant in nanoseconds from current device time counter. */
	CL4ProfEvInstType type; /**< Type of event instant (CL4ProfEvInstType#PROFCL_EV_START or CL4ProfEvInstType#PROFCL_EV_END). */
	cl_command_queue queue;/**< The command queue where the event took place. */
} CL4ProfEvInst;

/**
 * @brief Aggregate event info.
 */
typedef struct cl4_prof_ev_aggregate {
	const char* eventName; /**< Name of event which the instant refers to (CL4ProfEvInfo#eventName). */
	cl_ulong totalTime;    /**< Total time of events with name equal to CL4ProfEvAggregate#eventName. */
	double relativeTime;   /**< Relative time of events with name equal to CL4ProfEvAggregate#eventName. */
} CL4ProfEvAggregate;

/**
 * @brief Export options.
 * */
typedef struct cl4_prof_export_options {
	const char* separator;    /**< Field separator, defaults to tab (\\t). */
	const char* newline;      /**< Newline character, Defaults to Unix newline (\\n). */
	const char* queue_delim;  /**< Queue name delimiter, defaults to empty string. */
	const char* evname_delim; /**< Event name delimiter, defaults to empty string. */
	gboolean simple_queue_id; /**< Use simple queue IDs (0,1...n) instead of using the queue memory location as ID (defaults to TRUE). */
	gboolean zero_start;      /**< Start at instant 0 (TRUE, default), or start at oldest instant returned by OpenCL (FALSE). */
} CL4ProfExportOptions;

/**
 * @brief Associates an OpenCL event with a name.
 * */
typedef struct cl4_prof_evname {
	const char* eventName; /**< Event name. */
	cl_event event;        /**< OpenCL event. */
} CL4ProfEvName;

/** @brief Create a new OpenCL events profile. */
CL4ProfProfile* cl4_prof_profile_new();

/** @brief Free an OpenCL events profile. */
void cl4_prof_profile_free(CL4ProfProfile* profile);

/** @brief Indication that profiling sessions has started. */
void cl4_prof_profile_start(CL4ProfProfile* profile);

/** @brief Indication that profiling sessions has ended. */
void cl4_prof_profile_stop(CL4ProfProfile* profile);

/** @brief If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
gdouble cl4_prof_time_elapsed(CL4ProfProfile* profile);

/** @brief Add OpenCL event to events profile, more specifically adds
 * the start and end instants of the given event to the profile. */
int cl4_prof_profile_add(CL4ProfProfile* profile, const char* event_name, cl_event ev, GError** err);

/** @brief Add OpenCL event to events profile. Receives a CL4ProfEvName
 * instead of a separate event and name like cl4_prof_profile_add(). */
int cl4_prof_profile_add_evname(CL4ProfProfile* profile, CL4ProfEvName event_with_name, GError** err);

/** @brief Add OpenCL events to events profile, more specifically adds
 * the start of ev1 and end of ev2 to the profile. */
int cl4_prof_profile_add_composite(CL4ProfProfile* profile, const char* event_name, cl_event ev1, cl_event ev2, GError** err);

/** @brief Create new event instant. */
CL4ProfEvInst* cl4_prof_evinst_new(const char* eventName, guint id, cl_ulong instant, CL4ProfEvInstType type, cl_command_queue queue);

/** @brief Free an event instant. */
void cl4_prof_evinst_free(gpointer event_instant);

/** @brief Compares two event instants for sorting purposes. */
gint cl4_prof_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata);

/** @brief Determine overlap matrix for the given OpenCL events profile. */
int cl4_prof_profile_overmat(CL4ProfProfile* profile, GError** err);

/** @brief Determine aggregate statistics for the given OpenCL events profile. */
int cl4_prof_profile_aggregate(CL4ProfProfile* profile, GError** err);

/** @brief Create a new aggregate statistic for events of a given type. */
CL4ProfEvAggregate* cl4_prof_aggregate_new(const char* eventName);

/** @brief Free an aggregate statistic. */
void cl4_prof_aggregate_free(gpointer agg);

/** @brief Print profiling info. */
int cl4_prof_print_info(CL4ProfProfile* profile, CL4ProfEvAggDataSort evAggSortType, GError** err);

/** @brief Export profiling info to a given stream. */
int cl4_prof_export_info(CL4ProfProfile* profile, FILE* stream, GError** err);

/** @brief Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * cl4_prof_export_info() for more information. */
int cl4_prof_export_info_file(CL4ProfProfile* profile, const char* filename, GError** err);

/** @brief Set export options using a ::CL4ProfExportOptions struct. */
void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts);

/** @brief Get current export options. */
 CL4ProfExportOptions cl4_prof_export_opts_get();

#endif
