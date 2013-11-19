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
#ifndef PREDPREYPROFILER_H
#define PREDPREYPROFILER_H

#ifdef CLPROF_TEST
	#include "tests/test_profiler.h"
#else
	#include <CL/cl.h>
#endif

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gerrorf.h"

/**
 * @brief Error codes.
 * */ 
enum profcl_error_codes {
	PROFCL_SUCCESS = 0,        /**< Successful operation. */
	PROFCL_ALLOC_ERROR = 1,    /**< Error code thrown when no memory allocation is possible. */
	PROFCL_ERROR_OPENFILE = 2, /**< Error code thrown when it's not possible to open file. */
	PROFCL_OCL_ERROR = 10      /**< An OpenCL error ocurred. */
};

/** Resolves to error category identifying string, in this case an error in the OpenCL profiler library. */
#define PROFCL_ERROR profcl_error_quark()

/** 
 * @brief Contains the profiling info of an OpenCL application.
 */
typedef struct profcl_profile { 
	GHashTable* unique_events;	/**< Hash table with keys equal to the unique events name, and values equal to a unique event id. */
	GList* event_instants;		/**< Instants (start and end) of all events occuring in an OpenCL application. */
	guint num_event_instants;	/**< Total number of event instants in ProfCLProfile#event_instants. */
	GHashTable* aggregate;		/**< Aggregate statistics for all events in ProfCLProfile#event_instants. */
	cl_ulong* overmat;			/**< Overlap matrix for all events in event_instants. */
	cl_ulong totalEventsTime;	/**< Total time taken by all events. */
	cl_ulong totalEventsEffTime;/**< Total time taken by all events except intervals where events overlaped. */
	GTimer* timer;				/**< Keeps track of time during the complete profiling session. */
} ProfCLProfile;

/**
 * @brief Type of event instant (ProfCLEvInst).
 */
typedef enum {
	PROFCL_EV_START,	/**< ProfCLEvInst is a start event instant. */
	PROFCL_EV_END		/**< ProfCLEvInst is a end event instant. */
} ProfCLEvInstType;

/**
 * @brief Sorting strategy for event instants (ProfCLEvInst).
 */
typedef enum {
	PROFCL_EV_SORT_INSTANT,	/**< Sort event instants by instant. */
	PROFCL_EV_SORT_ID		/**< Sort event instants by event id. */
} ProfCLEvSort;

/**
 * @brief Sorting strategy for aggregate event data instances. 
 */
typedef enum {
	PROFCL_AGGEVDATA_SORT_NAME,	/**< Sort aggregate event data instances by name. */
	PROFCL_AGGEVDATA_SORT_TIME	/**< Sort aggregate event data instances by time. */
}  ProfCLEvAggDataSort;

/**
 * @brief Event instant. 
 */
typedef struct profcl_evinst { 
	const char* eventName;	/**< Name of event which the instant refers to (ProfCLEvInfo#eventName). */
	guint id;				/**< Event instant ID. */
	cl_ulong instant;		/**< Event instant in nanoseconds from current device time counter. */
	ProfCLEvInstType type;	/**< Type of event instant (ProfCLEvInstType#PROFCL_EV_START or ProfCLEvInstType#PROFCL_EV_END). */
	cl_command_queue queue; /**< The command queue where the event took place. */
} ProfCLEvInst;

/**
 * @brief Aggregate event info.
 */
typedef struct profcl_ev_aggregate {
	const char* eventName;	/**< Name of event which the instant refers to (ProfCLEvInfo#eventName). */
	cl_ulong totalTime;		/**< Total time of events with name equal to ProfCLEvAggregate#eventName. */
	double relativeTime;	/**< Relative time of events with name equal to ProfCLEvAggregate#eventName. */
} ProfCLEvAggregate;

/** @brief Create a new OpenCL events profile. */
ProfCLProfile* profcl_profile_new();

/** @brief Free an OpenCL events profile. */
void profcl_profile_free(ProfCLProfile* profile);

/** @brief Indication that profiling sessions has started. */
void profcl_profile_start(ProfCLProfile* profile);

/** @brief Indication that profiling sessions has ended. */
void profcl_profile_stop(ProfCLProfile* profile);

/** @brief If profiling has started but not stopped, returns the time 
 * since the profiling started. If profiling has been stopped, returns 
 * the elapsed time between the time it started and the time it stopped. */
gdouble profcl_time_elapsed(ProfCLProfile* profile);

/** @brief Add OpenCL event to events profile, more specifically adds 
 * the start and end instants of the given event to the profile. */
int profcl_profile_add(ProfCLProfile* profile, const char* event_name, cl_event ev, GError** err);

/** @brief Add OpenCL events to events profile, more specifically adds 
 * the start of ev1 and end of ev2 to the profile. */ 
int profcl_profile_add_composite(ProfCLProfile* profile, const char* event_name, cl_event ev1, cl_event ev2, GError** err);

/** @brief Create new event instant. */
ProfCLEvInst* profcl_evinst_new(const char* eventName, guint id, cl_ulong instant, ProfCLEvInstType type, cl_command_queue queue);

/** @brief Free an event instant. */
void profcl_evinst_free(gpointer event_instant);

/** @brief Compares two event instants for sorting purposes. */
gint profcl_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata);

/** @brief Determine overlap matrix for the given OpenCL events profile. */
int profcl_profile_overmat(ProfCLProfile* profile, GError** err);

/** @brief Determine aggregate statistics for the given OpenCL events profile. */
int profcl_profile_aggregate(ProfCLProfile* profile, GError** err);

/** @brief Create a new aggregate statistic for events of a given type. */
ProfCLEvAggregate* profcl_aggregate_new(const char* eventName);

/** @brief Free an aggregate statistic. */
void profcl_aggregate_free(gpointer agg);

/** @brief Print profiling info. */
int profcl_print_info(ProfCLProfile* profile, ProfCLEvAggDataSort evAggSortType, GError** err);

/** @brief Resolves to error category identifying string, in this case an error in the OpenCL profiler library. */
GQuark profcl_error_quark(void);

#endif


