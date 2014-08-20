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
 * 
 * Definition of classes and methods for profiling OpenCL events.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */
 
#ifndef _CCL_PROFILER_H_
#define _CCL_PROFILER_H_

#include <glib.h>
#include <string.h>
#include "oclversions.h"
#include "errors.h"
#include "common.h"
#include "queue_wrapper.h"

/**
 * @defgroup PROFILER Profiler
 *
 * The profiler module provides classes and methods for
 * profiling wrapped OpenCL events and queues.
 * 
 * @warning The functions in this module are not thread-safe.
 * 
 * @todo detailed description of module.
 * 
 * @{
 */

/** 
 * Profile class, contains profiling information of OpenCL 
 * queues and events.
 * 
 * @warning Instances of this class are not thread-safe.
 * 
 * */
typedef struct ccl_prof CCLProf;

/**
 * Sort order for the profile module iterators.
 * */
typedef enum {
	/** Sort ascending (default). */
	CCL_PROF_SORT_ASC  = 0x0,
	/** Sort descending. */
	CCL_PROF_SORT_DESC = 0x1
} CCLProfSortOrder;


/**
 * Aggregate event info.
 */
typedef struct ccl_prof_agg {

	/** 
	 * Name of event which the instant refers to. 
	 * @public
	 * */
	const char* event_name;
	
	/** 
	 * Total (absolute) time of events with name equal to 
	 * ::CCLProfAgg::event_name. 
	 * @public
	 * */
	cl_ulong absolute_time;
	
	/** 
	 * Relative time of events with name equal to 
	 * ::CCLProfAgg::event_name. 
	 * @public
	 * */
	double relative_time;
	
} CCLProfAgg;


/**
 * Sort criteria for aggregate event info instances.
 */
typedef enum {

	 /** Sort aggregate event data instances by name. */
	CCL_PROF_AGG_SORT_NAME = 0x00,
	
	/** Sort aggregate event data instances by time. */
	CCL_PROF_AGG_SORT_TIME = 0x10

} CCLProfAggSort;

/**
 * Event profiling info. 
 * */
typedef struct ccl_prof_info {

	/** 
	 * Name of event. 
	 * @public
	 * */
	const char* event_name;
	
	/** 
	 * Name of command queue which generated this event. 
	 * @public
	 * */
	const char* queue_name;

	/** 
	 * Device time in nanoseconds when the command identified by event 
	 * is enqueued in a command-queue by the host. 
	 * @public
	 * */
	cl_ulong t_queued;

	/** 
	 * Device time counter in nanoseconds when the command identified 
	 * by event that has been enqueued is submitted by the host to the 
	 * device associated with the command-queue. 
	 * @public
	 * */
	cl_ulong t_submit;

	/** 
	 * Device time in nanoseconds when the command identified by event
	 * starts execution on the device. 
	 * @public
	 * */
	cl_ulong t_start;

	/** 
	 * Device time in nanoseconds when the command identified by event
	 * has finished execution on the device. 
	 * @public
	 * */
	cl_ulong t_end;

} CCLProfInfo;

/**
 * Sort criteria for event profiling info instances.
 */
typedef enum {

	 /** Sort event profiling info instances by event name. */
	CCL_PROF_INFO_SORT_NAME_EVENT = 0x20,

	 /** Sort event profiling info instances by queue name. */
	CCL_PROF_INFO_SORT_NAME_QUEUE = 0x30,

	 /** Sort event profiling info instances by queued time. */
	CCL_PROF_INFO_SORT_T_QUEUED   = 0x40,

	 /** Sort event profiling info instances by submit time. */
	CCL_PROF_INFO_SORT_T_SUBMIT   = 0x50,

	 /** Sort event profiling info instances by start time. */
	CCL_PROF_INFO_SORT_T_START    = 0x60,

	 /** Sort event profiling info instances by end time. */
	CCL_PROF_INFO_SORT_T_END      = 0x70

} CCLProfInfoSort;


/**
 * Type of event instant (::CCLProfInst).
 */
typedef enum {
	
	/** Start event instant. */
	CCL_PROF_INST_TYPE_START,

	/** End event instant. */
	CCL_PROF_INST_TYPE_END
	
} CCLProfInstType;

/**
 * Event instant.
 */
typedef struct ccl_prof_inst {

	 /** 
	  * Name of event which the instant refers to. 
	 * @public
	  * */
	const char* event_name;

	/** 
	 * Name of command queue associated with event. 
	 * @public
	 * */
	const char* queue_name;

	/** 
	 * Event instant ID. 
	 * @public
	 * */
	guint id;

	/** 
	 * Event instant in nanoseconds from current device time counter. 
	 * @public
	 * */
	cl_ulong instant;

	/** 
	 * Type of event instant 
	 * (CCLProfInstType::CCL_PROF_INST_TYPE_START or 
	 * CCLProfInstType::CCL_PROF_INST_TYPE_END). 
	 * @public
	 * */
	CCLProfInstType type;

} CCLProfInst;

/**
 * Sort criteria for event instants (::CCLProfInst).
 */
typedef enum {
	
	/** Sort event instants by instant. */
	CCL_PROF_INST_SORT_INSTANT = 0x80,

	/** Sort event instants by event id. */
	CCL_PROF_INST_SORT_ID      = 0x90

} CCLProfInstSort;

/**
 * Representation of an overlap of events.
 */
typedef struct ccl_prof_overlap {

	/** 
	 * Name of first overlapping event. 
	 * @public
	 * */
	const char* event1_name;

	/** 
	 * Name of second overlapping event. 
	 * @public
	 * */
	const char* event2_name;

	/** 
	 * Overlap duration in nanoseconds. 
	 * @public
	 * */
	cl_ulong duration;

} CCLProfOverlap;

/**
 * Sort criteria for overlaps (::CCLProfOverlap).
 */
typedef enum {
	
	/** Sort overlaps by event name. */
	CCL_PROF_OVERLAP_SORT_NAME     = 0xa0,

	/** Sort overlaps by overlap duration. */
	CCL_PROF_OVERLAP_SORT_DURATION = 0xb0

} CCLProfOverlapSort;

/**
 * Export options.
 * */
typedef struct ccl_prof_export_options {

	/** 
	 * Field separator, defaults to tab (\\t). 
	 * @public
	 * */
	const char* separator;

	/** 
	 * Newline character, Defaults to Unix newline (\\n). 
	 * @public
	 * */
	const char* newline;

	/** 
	 * Queue name delimiter, defaults to empty string. 
	 * @public
	 * */
	const char* queue_delim;

	/** 
	 * Event name delimiter, defaults to empty string. 
	 * @public
	 * */
	const char* evname_delim;

	/** Start at instant 0 (TRUE, default), or start at oldest instant 
	 * returned by OpenCL (FALSE). 
	 * @public
	 * */
	cl_bool zero_start;
	
}  CCLProfExportOptions;

/* Create a new profile object. */
CCLProf* ccl_prof_new();

/* Destroy a profile object. */
void ccl_prof_destroy(CCLProf* prof);

/* Starts the global profiler timer. Only required if client
* wishes to compare the effectively ellapsed time with the OpenCL
* kernels time. */
void ccl_prof_start(CCLProf* prof);

/* Stops the global profiler timer. Only required if 
 * ccl_prof_start() was called. */
void ccl_prof_stop(CCLProf* prof);

/* If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
double ccl_prof_time_elapsed(CCLProf* prof);

/* Add a command queue wrapper for profiling. */
void ccl_prof_add_queue(
	CCLProf* prof, const char* cq_name, CCLQueue* cq);

/* Determine aggregate statistics for the given profile object. */
cl_bool ccl_prof_calc(CCLProf* prof, GError** err);

/* Return aggregate statistics for events with the given name. */
const CCLProfAgg* ccl_prof_get_agg(
	CCLProf* prof, const char* event_name);

/* Initialize an iterator for profiled aggregate event 
 * instances. */
void ccl_prof_iter_agg_init(CCLProf* prof, int sort);

/* Return the next profiled aggregate event instance. */
const CCLProfAgg* ccl_prof_iter_agg_next(CCLProf* prof);

/* Initialize an iterator for event profiling info instances. */
void ccl_prof_iter_info_init(CCLProf* prof, int sort);

/* Return the next event profiling info instance. */
const CCLProfInfo* ccl_prof_iter_info_next(CCLProf* prof);

/* Initialize an iterator for event instant instances. */
void ccl_prof_iter_inst_init(CCLProf* prof, int sort);

/* Return the next event instant instance. */
const CCLProfInst* ccl_prof_iter_inst_next(CCLProf* prof);

/* Initialize an iterator for overlap instances. */
void ccl_prof_iter_overlap_init(CCLProf* prof, int sort);

/* Return the next overlap instance. */
const CCLProfOverlap* ccl_prof_iter_overlap_next(CCLProf* prof);

/* Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics (sorted
 * by absolute time), and a table of event overlaps (sorted by overlap
 * duration). */ 
void ccl_prof_print_summary(CCLProf* prof);

/* Get a summary with the profiling info. More specifically,
 * this function returns a string containing a table of aggregate event 
 * statistics and a table of event overlaps. The order of the returned
 * information can be specified in the function arguments. */ 
const char* ccl_prof_get_summary(
	CCLProf* prof, int agg_sort, int ovlp_sort);

/* Export profiling info to a given stream. */
cl_bool ccl_prof_export_info(CCLProf* profile, FILE* stream, GError** err);

/* Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * ccl_prof_export_info() for more information. */
cl_bool ccl_prof_export_info_file(
	CCLProf* profile, const char* filename, GError** err);

/* Set export options using a ::CCLProfExportOptions struct. */
void ccl_prof_set_export_opts(CCLProfExportOptions export_opts);

/* Get current export options. */
CCLProfExportOptions ccl_prof_get_export_opts();

/** @} */

#endif
