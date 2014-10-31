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

#include <string.h>
#include "ccl_errors.h"
#include "ccl_common.h"
#include "ccl_queue_wrapper.h"

/**
 * @defgroup CCL_PROFILER Profiler
 *
 * The profiler module provides classes and methods for
 * profiling wrapped OpenCL events and queues.
 *
 * @warning The functions in this module are not thread-safe.
 *
 * The profiling module offers two methods for obtaining information
 * about the performed computations:
 *
 * 1. Detailed profiling of OpenCL events using the
 * ::ccl_prof_add_queue() function.
 * 2. Simple (and optional) timming of the performed computations using
 * the ::ccl_prof_start() and ::ccl_prof_stop() functions. If this
 * timming is measured it will be taken into account by the
 * `ccl_prof_*_summary()` functions.
 *
 * In order to use the first method, the `CL_QUEUE_PROFILING_ENABLE`
 * property should be specified when creating command queue wrappers
 * with ::ccl_queue_new().
 *
 * After the computations take place, all the (finished) queue wrappers
 * are passed to the profiler using the ::ccl_prof_add_queue()
 * function. The ccl_prof_calc() function can then be called to
 * perform the required analysis.
 *
 * At this stage, different types of profiling information become
 * available, and can be iterated over:
 *
 * 1. _Aggregate event information_: absolute and relative durations of
 * all events with same name, represented by the ::CCLProfAgg* class. If
 * an event name is not set during the course of the computation, the
 * aggregation is performed by event type, i.e., by events which
 * represent the same command. A sequence of ::CCLProfAgg* objects can
 * be iterated over using the ::ccl_prof_iter_agg_init() and
 * ::ccl_prof_iter_agg_next() functions. A specific aggregate event
 * can be obtained by name using the ::ccl_prof_get_agg() function.
 * 2. _Non-aggregate event information_: event-specific information,
 * represented by the ::CCLProfInfo* class, such as event name (or type,
 * if no name is given), the queue the event is associated with, and
 * submit, queued, start and end instants. A sequence of ::CCLProfInfo*
 * objects can be iterated over using the ::ccl_prof_iter_info_init()
 * and ::ccl_prof_iter_info_next() functions.
 * 3. _Event instants_: specific start and end event instants, represented
 * by the ::CCLProfInst* class. A sequence of ::CCLProfInst* objects can
 * be iterated over using the ::ccl_prof_iter_inst_init() and
 * ::ccl_prof_iter_inst_next() functions.
 * 4. _Event overlaps_: information about event overlaps, represented by
 * the ::CCLProfOverlap* class. Event overlaps can only occur when more
 * than one queue is used on the same device. A sequence of
 * ::CCLProfOverlap* objects can be iterated over using the
 * ::ccl_prof_iter_overlap_init() and ::ccl_prof_iter_overlap_next()
 * functions.
 *
 * While this information can be subject to different types of
 * examination by client code, the profiler module also offers some
 * functionality which allows for a more instant interpretation of
 * results:
 *
 * 1. A summary of the profiling analysis can be obtained or printed
 * with the ::ccl_prof_get_summary() or ::ccl_prof_print_summary()
 * functions, respectively.
 * 2. An exported list of ::CCLProfInfo* data, namely queue name, start
 * instant, end instant and event name, sorted by start instant, can be
 * opened by the @ref ccl_plot_events "plot events" script to plot a
 * Gantt-like chart of the performed computation. Such list can be
 * exported with the ::ccl_prof_export_info() or
 * ::ccl_prof_export_info_file() functions, using the default export
 * options.
 *
 * _Example: Conway's game of life using double-buffered images_
 * (@ref ca.c "complete example")
 *
 * @dontinclude ca.c
 * @skipline Wrappers for
 * @skipline queue_exec
 * @until queue_comm
 * @skipline CCLProf
 * @skipline Error handling
 * @until GError*
 *
 * @skipline Create command queues
 * @skipline queue_exec
 * @skipline queue_comm
 *
 * @skipline Start profiling
 * @until origin,
 *
 * @skipline Run CA_ITERS
 * @until origin,
 *
 * @skipline Execute
 * @until NULL)
 *
 * @skipline }
 * @until origin,
 *
 * @skipline Stop profiling
 * @until queue_exec);
 *
 * @skipline Process profiling
 * @until ccl_prof_calc
 *
 * @skipline Print
 * @until ccl_prof_export_info
 *
 * @skipline Release wrappers
 * @skipline ccl_queue_destroy(queue_comm)
 * @until ccl_queue_destroy(queue_exec)
 *
 * @skipline Destroy profiler
 * @until ccl_prof_destroy
 *
 * The output of ::ccl_prof_print_summary() will be something like:
 *
 * @verbatim
    =========================== Timming/Profiling ===========================

    Aggregate times by event  :
      ------------------------------------------------------------------
      | Event name                     | Rel. time (%) | Abs. time (s) |
      ------------------------------------------------------------------
      | NDRANGE_KERNEL                 |       97.2742 |    3.7468e-02 |
      | READ_IMAGE                     |        2.6747 |    1.0303e-03 |
      | WRITE_IMAGE                    |        0.0511 |    1.9690e-05 |
      ------------------------------------------------------------------
                                       |         Total |    3.8518e-02 |
                                       ---------------------------------
    Event overlaps            :
      ------------------------------------------------------------------
      | Event 1                | Event2                 | Overlap (s)  |
      ------------------------------------------------------------------
      | READ_IMAGE             | NDRANGE_KERNEL         |   1.3618e-04 |
      ------------------------------------------------------------------
                               |                  Total |   1.3618e-04 |
                               -----------------------------------------
    Tot. of all events (eff.) : 3.838198e-02s
    Total ellapsed time       : 4.295200e-02s
    Time spent in device      : 89.36%
    Time spent in host        : 10.64%
 @endverbatim
 *
 * Instead of the default command-based event names such as
 * `NDRANGE_KERNEL`, specific names can be set with the
 * ::ccl_event_set_name() function. This allows to: (a) separate the
 * aggregation of events of the same type (e.g., differentiate between
 * the execution of two different kernels); and, (b) aggregate events
 * of different types (e.g., aggregate reads and writes into a single
 * "comms" event).
 *
 * The @ref ccl_plot_events script can be used to plot a Gantt-like
 * chart of the events which took place in the queues. Running the
 * following command...
 *
 *     $ python ccl_plot_events.py prof.tsv
 *
 * ...will produce the following image:
 *
 * @image html gantt_ca.png
 * @image latex gantt_ca.png "Gantt-like chart of events which took place in the queues" width=0.7\textwidth
 * @manonly
 * Image not available in man pages.
 * @endmanonly
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
	 * Type of command which produced the event.
	 * @public
	 * */
	cl_command_type command_type;

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
CCL_EXPORT
CCLProf* ccl_prof_new();

/* Destroy a profile object. */
CCL_EXPORT
void ccl_prof_destroy(CCLProf* prof);

/* Starts the global profiler timer. Only required if client
* wishes to compare the effectively ellapsed time with the OpenCL
* kernels time. */
CCL_EXPORT
void ccl_prof_start(CCLProf* prof);

/* Stops the global profiler timer. Only required if
 * ccl_prof_start() was called. */
CCL_EXPORT
void ccl_prof_stop(CCLProf* prof);

/* If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped. */
CCL_EXPORT
double ccl_prof_time_elapsed(CCLProf* prof);

/* Add a command queue wrapper for profiling. */
CCL_EXPORT
void ccl_prof_add_queue(
	CCLProf* prof, const char* cq_name, CCLQueue* cq);

/* Determine aggregate statistics for the given profile object. */
CCL_EXPORT
cl_bool ccl_prof_calc(CCLProf* prof, GError** err);

/* Return aggregate statistics for events with the given name. */
CCL_EXPORT
const CCLProfAgg* ccl_prof_get_agg(
	CCLProf* prof, const char* event_name);

/* Initialize an iterator for profiled aggregate event
 * instances. */
CCL_EXPORT
void ccl_prof_iter_agg_init(CCLProf* prof, int sort);

/* Return the next profiled aggregate event instance. */
CCL_EXPORT
const CCLProfAgg* ccl_prof_iter_agg_next(CCLProf* prof);

/* Initialize an iterator for event profiling info instances. */
CCL_EXPORT
void ccl_prof_iter_info_init(CCLProf* prof, int sort);

/* Return the next event profiling info instance. */
CCL_EXPORT
const CCLProfInfo* ccl_prof_iter_info_next(CCLProf* prof);

/* Initialize an iterator for event instant instances. */
CCL_EXPORT
void ccl_prof_iter_inst_init(CCLProf* prof, int sort);

/* Return the next event instant instance. */
CCL_EXPORT
const CCLProfInst* ccl_prof_iter_inst_next(CCLProf* prof);

/* Initialize an iterator for overlap instances. */
CCL_EXPORT
void ccl_prof_iter_overlap_init(CCLProf* prof, int sort);

/* Return the next overlap instance. */
CCL_EXPORT
const CCLProfOverlap* ccl_prof_iter_overlap_next(CCLProf* prof);

/* Get duration of all events in nanoseconds. */
CCL_EXPORT
cl_ulong ccl_prof_get_duration(CCLProf* prof);

/* Get effective duration of all events in nanoseconds, i.e. the
 * duration of all events minus event overlaps. */
CCL_EXPORT
cl_ulong ccl_prof_get_eff_duration(CCLProf* prof);

/* Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics (sorted
 * by absolute time), and a table of event overlaps (sorted by overlap
 * duration). */
CCL_EXPORT
void ccl_prof_print_summary(CCLProf* prof);

/* Get a summary with the profiling info. More specifically,
 * this function returns a string containing a table of aggregate event
 * statistics and a table of event overlaps. The order of the returned
 * information can be specified in the function arguments. */
CCL_EXPORT
const char* ccl_prof_get_summary(
	CCLProf* prof, int agg_sort, int ovlp_sort);

/* Export profiling info to a given stream. */
CCL_EXPORT
cl_bool ccl_prof_export_info(CCLProf* profile, FILE* stream, GError** err);

/* Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. Check the
 * ccl_prof_export_info() for more information. */
CCL_EXPORT
cl_bool ccl_prof_export_info_file(
	CCLProf* profile, const char* filename, GError** err);

/* Set export options using a ::CCLProfExportOptions struct. */
CCL_EXPORT
void ccl_prof_set_export_opts(CCLProfExportOptions export_opts);

/* Get current export options. */
CCL_EXPORT
CCLProfExportOptions ccl_prof_get_export_opts();

/** @} */

#endif
