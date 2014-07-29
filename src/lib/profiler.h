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
 
#ifndef CL4_PROFILER_H
#define CL4_PROFILER_H

#include <glib.h>
#include <string.h>
#include "oclversions.h"
#include "gerrorf.h"
#include "errors.h"
#include "common.h"
#include "queue_wrapper.h"

/**
 * @defgroup PROFILER The profiler module.
 *
 * @brief The profiler module provides classes and methods for
 * profiling wrapped OpenCL events and queues.
 * 
 * @warning The functions in this module are not thread-safe.
 * 
 * Todo: detailed description of module.
 * 
 * @{
 */

/** @brief Profile class, contains profiling information of OpenCL 
 * queues and events. */
typedef struct cl4_prof CL4Prof;

/**
 * @brief Sort order for the profile module iterators.
 * */
typedef enum {
	/** Sort ascending (default). */
	CL4_PROF_SORT_ASC  = 0x0,
	/** Sort descending. */
	CL4_PROF_SORT_DESC = 0x1
} CL4ProfSortOrder;


/**
 * @brief Aggregate event info.
 */
typedef struct cl4_prof_agg {

	/** Name of event which the instant refers to. */
	const char* event_name;
	/** Total (absolute) time of events with name equal to 
	 * ::CL4ProfAgg#event_name. */
	cl_ulong absolute_time;
	/** Relative time of events with name equal to 
	 * ::CL4ProfAgg#event_name. */
	double relative_time;
	
} CL4ProfAgg;


/**
 * @brief Sort criteria for aggregate event info instances.
 */
typedef enum {

	 /** Sort aggregate event data instances by name. */
	CL4_PROF_AGG_SORT_NAME = 0x00,
	/** Sort aggregate event data instances by time. */
	CL4_PROF_AGG_SORT_TIME = 0x10

} CL4ProfAggSort;

/**
 * @brief Event profiling info. 
 * */
typedef struct cl4_prof_info {

	/** Name of event. */
	const char* event_name;
	/** Name of command queue which generated this event. */
	const char* queue_name;
	/** Device time in nanoseconds when the command identified by event 
	 * is enqueued in a command-queue by the host. */
	cl_ulong t_queued;
	/** Device time counter in nanoseconds when the command identified 
	 * by event that has been enqueued is submitted by the host to the 
	 * device associated with the command-queue. */
	cl_ulong t_submit;
	/** Device time in nanoseconds when the command identified by event
	 * starts execution on the device. */
	cl_ulong t_start;
	/** Device time in nanoseconds when the command identified by event
	 * has finished execution on the device. */
	cl_ulong t_end;

} CL4ProfInfo;

/**
 * @brief Sort criteria for event profiling info instances.
 */
typedef enum {

	 /** Sort event profiling info instances by event name. */
	CL4_PROF_INFO_SORT_NAME_EVENT = 0x20,
	 /** Sort event profiling info instances by queue name. */
	CL4_PROF_INFO_SORT_NAME_QUEUE = 0x30,
	 /** Sort event profiling info instances by queued time. */
	CL4_PROF_INFO_SORT_T_QUEUED   = 0x40,
	 /** Sort event profiling info instances by submit time. */
	CL4_PROF_INFO_SORT_T_SUBMIT   = 0x50,
	 /** Sort event profiling info instances by start time. */
	CL4_PROF_INFO_SORT_T_START    = 0x60,
	 /** Sort event profiling info instances by end time. */
	CL4_PROF_INFO_SORT_T_END      = 0x70

} CL4ProfInfoSort;


/**
 * @brief Type of event instant (::CL4ProfInst).
 */
typedef enum {
	
	/** Start event instant. */
	CL4_PROF_INST_TYPE_START,
	/** End event instant. */
	CL4_PROF_INST_TYPE_END
	
} CL4ProfInstType;

/**
 * @brief Event instant.
 */
typedef struct cl4_prof_inst {

	 /** Name of event which the instant refers to. */
	const char* event_name;
	/** Name of command queue associated with event. */
	const char* queue_name;
	/** Event instant ID. */
	guint id;
	/** Event instant in nanoseconds from current device time counter. */
	cl_ulong instant;
	/** Type of event instant 
	 * (CL4ProfInstType::CL4_PROF_INST_TYPE_START or 
	 * CL4ProfInstType::CL4_PROF_INST_TYPE_END). */
	CL4ProfInstType type;

} CL4ProfInst;

/**
 * @brief Sort criteria for event instants (::CL4ProfInst).
 */
typedef enum {
	
	/** Sort event instants by instant. */
	CL4_PROF_INST_SORT_INSTANT = 0x80,
	/** Sort event instants by event id. */
	CL4_PROF_INST_SORT_ID      = 0x90

} CL4ProfInstSort;

/**
 * @brief Representation of an overlap of events.
 */
typedef struct cl4_prof_overlap {

	 /** Name of first overlapping event. */
	const char* event1_name;
	 /** Name of second overlapping event. */
	const char* event2_name;
	/** Overlap duration in nanoseconds. */
	cl_ulong duration;

} CL4ProfOverlap;

/**
 * @brief Sort criteria for overlaps (::CL4ProfOverlap).
 */
typedef enum {
	
	/** Sort overlaps by event name. */
	CL4_PROF_OVERLAP_SORT_NAME     = 0xa0,
	/** Sort overlaps by overlap duration. */
	CL4_PROF_OVERLAP_SORT_DURATION = 0xb0

} CL4ProfOverlapSort;

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
	/** Start at instant 0 (TRUE, default), or start at oldest instant 
	 * returned by OpenCL (FALSE). */
	cl_bool zero_start;
	
}  CL4ProfExportOptions;

/** @brief Create a new profile object. */
CL4Prof* cl4_prof_new();

/** @brief Destroy a profile object. */
void cl4_prof_destroy(CL4Prof* prof);

/** @brief Starts the global profiler timer. Only required if client
* wishes to compare the effectively ellapsed time with the OpenCL
* kernels time. */
void cl4_prof_start(CL4Prof* prof);

/** @brief Stops the global profiler timer. Only required if 
 * cl4_prof_start() was called. */
void cl4_prof_stop(CL4Prof* prof);

/** @brief If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
double cl4_prof_time_elapsed(CL4Prof* prof);

/** @brief Add a command queue wrapper for profiling. */
void cl4_prof_add_queue(
	CL4Prof* prof, const char* cq_name, CL4CQueue* cq);

/** @brief Determine aggregate statistics for the given profile object. */
cl_bool cl4_prof_calc(CL4Prof* prof, GError** err);

/** @brief Return aggregate statistics for events with the given name. */
const CL4ProfAgg const* cl4_prof_get_agg(
	CL4Prof* prof, const char* event_name);

/** @brief Initialize an iterator for profiled aggregate event 
 * instances. */
void cl4_prof_iter_agg_init(CL4Prof* prof, int sort);

/** @brief Return the next profiled aggregate event instance. */
const CL4ProfAgg const* cl4_prof_iter_agg_next(CL4Prof* prof);

/** @brief Initialize an iterator for event profiling info instances. */
void cl4_prof_iter_info_init(CL4Prof* prof, int sort);

/** @brief Return the next event profiling info instance. */
const CL4ProfInfo const* cl4_prof_iter_info_next(CL4Prof* prof);

/** @brief Initialize an iterator for event instant instances. */
void cl4_prof_iter_inst_init(CL4Prof* prof, int sort);

/** @brief Return the next event instant instance. */
const CL4ProfInst const* cl4_prof_inst_info_next(CL4Prof* prof);

/** @brief Initialize an iterator for overlap instances. */
void cl4_prof_iter_overlap_init(CL4Prof* prof, int sort);

/** @brief Return the next overlap instance. */
const CL4ProfOverlap const* cl4_prof_iter_overlap_next(CL4Prof* prof);

/** @brief Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics (sorted
 * by absolute time), and a table of event overlaps (sorted by overlap
 * duration). */ 
void cl4_prof_print_summary(CL4Prof* prof);

/** @brief Get a summary with the profiling info. More specifically,
 * this function returns a string containing a table of aggregate event 
 * statistics and a table of event overlaps. The order of the returned
 * information can be specified in the function arguments. The returned
 * string must be freed with the `g_free()` function from GLib. */ 
gchar* cl4_prof_get_summary(
	CL4Prof* prof, int agg_sort, int ovlp_sort);

/** @brief Export profiling info to a given stream. */
cl_bool cl4_prof_export_info(CL4Prof* profile, FILE* stream, GError** err);

/** @brief Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * cl4_prof_export_info() for more information. */
cl_bool cl4_prof_export_info_file(
	CL4Prof* profile, const char* filename, GError** err);

/** @brief Set export options using a ::CL4ProfExportOptions struct. */
void cl4_prof_set_export_opts(CL4ProfExportOptions export_opts);

/** @brief Get current export options. */
CL4ProfExportOptions cl4_prof_get_export_opts();

/** @} */

#endif
