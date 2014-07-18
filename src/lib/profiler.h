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

/**
 * @defgroup PROFILER The profiler module.
 *
 * @brief The profiler module provides classes and methods for
 * profiling wrapped OpenCL events and queues.
 * 
 * Todo: detailed description of module.
 * 
 * @{
 */

/** @brief Profile class, contains profiling information of OpenCL 
 * queues and events. */
typedef struct cl4_prof CL4Prof;

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
 * @brief Sorting strategy for aggregate event info instances.
 */
typedef enum {

	 /** Sort aggregate event data instances by name. */
	CL4_PROF_AGG_SORT_NAME,
	/** Sort aggregate event data instances by time. */
	CL4_PROF_AGG_SORT_TIME

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
 * @brief Sorting strategy for event profiling info instances.
 */
typedef enum {

	 /** Sort aggregate event data instances by event name. */
	CL4_PROF_INFO_SORT_NAME_EVENT,
	 /** Sort aggregate event data instances by queue name. */
	CL4_PROF_INFO_SORT_NAME_QUEUE,
	 /** Sort aggregate event data instances by queued time. */
	CL4_PROF_INFO_SORT_T_QUEUED,
	 /** Sort aggregate event data instances by submit time. */
	CL4_PROF_INFO_SORT_T_SUBMIT,
	 /** Sort aggregate event data instances by start time. */
	CL4_PROF_INFO_SORT_T_START,
	 /** Sort aggregate event data instances by end time. */
	CL4_PROF_INFO_SORT_T_END

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
	 * (::CL4ProfInstType#CL4_PROF_INST_TYPE_START or 
	 * ::CL4ProfInstType#CL4_PROF_INST_TYPE_END). */
	CL4ProfInstType type;

} CL4ProfInst;

/**
 * @brief Sorting strategy for event instants (::CL4ProfInst).
 */
typedef enum {
	
	/** Sort event instants by instant. */
	CL4_PROF_INST_SORT_INSTANT,
	/** Sort event instants by event id. */
	CL4_PROF_INST_SORT_ID

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
 * @brief Sorting strategy for overlaps (::CL4ProfOverlaps).
 */
typedef enum {
	
	/** Sort overlaps by event name. */
	CL4_PROF_OVERLAP_SORT_NAME,
	/** Sort overlaps by overlap duration. */
	CL4_PROF_OVERLAP_SORT_DURATION

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
const CL4ProfAgg const* cl4_prof_get_agg(CL4Prof* prof, const char* event_name);

/** @brief Initialize an iterator for profiled aggregate event 
 * instances. */
void cl4_prof_iter_agg_init(CL4Prof* prof, CL4ProfAggSort sort_type);

/** @brief Return the next profiled aggregate event instance. */
const CL4ProfAgg const* cl4_prof_iter_agg_next(CL4Prof* prof);

/** @brief Initialize an iterator for event profiling info instances. */
void cl4_prof_iter_info_init(CL4Prof* prof, CL4ProfInfoSort sort_type);

/** @brief Return the next event profiling info instance. */
const CL4ProfInfo const* cl4_prof_iter_info_next(CL4Prof* prof);

/** @brief Initialize an iterator for overlap instances. */
void cl4_prof_iter_overlap_init(
	CL4Prof* prof, CL4ProfOverlapSort sort_type);

/** @brief Return the next overlap instance. */
const CL4ProfOverlap const* cl4_prof_iter_overlap_next(CL4Prof* prof);

/** @brief Print profiling info. */
void cl4_prof_print_summary(CL4Prof* prof, 
	CL4ProfAggSort evAggSortType);

/** @brief Export profiling info to a given stream. */
cl_bool cl4_prof_export_info(CL4Prof* profile, FILE* stream, GError** err);

/** @brief Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * cl4_prof_export_info() for more information. */
cl_bool cl4_prof_export_info_file(
	CL4Prof* profile, const char* filename, GError** err);

/** @brief Set export options using a ::CL4ProfExportOptions struct. */
void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts);

/** @brief Get current export options. */
CL4ProfExportOptions cl4_prof_export_opts_get();

/** @} */

#endif
