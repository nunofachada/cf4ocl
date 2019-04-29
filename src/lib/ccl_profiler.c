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
 * Implementation of classes and methods for profiling OpenCL events.
 *
 * @author Nuno Fachada
 * @date 2017
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_profiler.h"
#include "_ccl_defs.h"

/**
 * @internal
 * Compare two integers depending on the sort order.
 *
 * @param[in] x First integer to compare.
 * @param[in] y Second integer to compare.
 * @param[in] ord Sort order (ascending or descending).
 * @return 1, 0 or -1.
 * */
#define CCL_PROF_CMP_INT(x, y, ord) (((ord) == CCL_PROF_SORT_ASC) \
	? (((x) > (y)) ? 1 : (((x) < (y)) ? -1 : 0)) \
	: (((x) < (y)) ? 1 : (((x) > (y)) ? -1 : 0)))

/**
 * @internal
 * Compare two strings depending on the sort order.
 *
 * @param[in] s1 First string to compare.
 * @param[in] s2 Second string to compare.
 * @param[in] ord Sort order (ascending or descending).
 * @return 1, 0 or -1.
 * */
#define CCL_PROF_CMP_STR(s1, s2, ord) (((ord) == CCL_PROF_SORT_ASC) \
	? g_strcmp0(s1, s2) : g_strcmp0(s2, s1))

/**
 * @internal
 * Get a ::CCLProfSort object from the given user data.
 *
 * @param[in] userdata An integer representing a bitfield containing
 * the sort order and the sort criteria.
 * @return A ::CCLProfSort object containing the sort order and the
 * sort criteria separately.
 * */
#define ccl_prof_get_sort(userdata) \
	{0x0F & *((int*) userdata), 0xF0 & *((int*) userdata)}

/**
 * @internal
 * Class representing a sort order (ascending or descending) and
 * the a sort criteria (context dependent).
 * */
typedef struct ccl_prof_sort_data {

	/** Sort order. */
	CCLProfSortOrder order;

	/** Sort criteria. */
	int criteria;

} CCLProfSort;

/**
 * Profile class, contains profiling information of OpenCL
 * queues and events.
 *
 * @warning Instances of this class are not thread-safe.
 *
 * */
struct ccl_prof {

	/**
	 * Flag indicating if the profiling information has already been
	 * determined.
	 * @private
	 * */
	gboolean calc;

	/**
	 * Hash table with keys equal to the events name, and values
	 * equal to a unique id for each event name.
	 * @private
	 * */
	GHashTable* event_names;

	/**
	 * Reverse of event_names in terms of key-values.
	 * @private
	 * */
	GHashTable* event_name_ids;

	/**
	 * Table of command queue wrappers.
	 * @private
	 * */
	GHashTable* queues;

	/**
	 * Total number of events.
	 * @private
	 * */
	cl_uint num_events;

	/**
	 * Instants (start and end) of all events.
	 * @private
	 * */
	GList* instants;

	/**
	 * List of all events profiling information.
	 * @private
	 * */
	GList* infos;

	/**
	 * Aggregate statistics for all events in
	 * ::CCLProf::instants.
	 * @private
	 * */
	GList* aggs;

	/**
	 * List of event overlaps.
	 * @private
	 * */
	GList* overlaps;

	/**
	 * Aggregate event statistics iterator.
	 * @private
	 * */
	GList* agg_iter;

	/**
	 * Event info iterator.
	 * @private
	 * */
	GList* info_iter;

	/**
	 * Event instant iterator.
	 * @private
	 * */
	GList* inst_iter;

	/**
	 * Overlaps iterator.
	 * @private
	 * */
	GList* overlap_iter;

	/**
	 * Total time taken by all events.
	 * @private
	 * */
	cl_ulong total_events_time;

	/**
	 * Total time taken by all events except intervals where events
	 * overlaped.
	 * @private
	 * */
	cl_ulong total_events_eff_time;

	/**
	 * Time at which the first (oldest) event started.
	 * @private
	 * */
	cl_ulong t_start;

	/**
	 * Summary string.
	 * @private
	 * */
	gchar* summary;

	/**
	 * Keeps track of time during the complete profiling session.
	 * @private
	 * */
	GTimer* timer;

};

/* Default export options. */
static CCLProfExportOptions export_options = {

	.separator = "\t",
	.newline = "\n",
	.queue_delim = "",
	.evname_delim = "",
	.zero_start = CL_TRUE

};

/**
 * @internal
 * Create new event instant.
 *
 * @private @memberof ccl_prof_inst
 *
 * @param[in] event_name Name of event.
 * @param[in] queue_name Name of command queue associated with event.
 * @param[in] id Id of event.
 * @param[in] instant Even instant in nanoseconds.
 * @param[in] type Type of event instant: CCL_PROF_INST_TYPE_START or
 * CCL_PROF_INST_TYPE_END.
 * @return A new event instant.
 */
static CCLProfInst* ccl_prof_inst_new(const char* event_name,
	const char* queue_name, cl_uint id, cl_ulong instant,
	CCLProfInstType type) {

	/* Allocate memory for event instant data structure. */
	CCLProfInst* inst = g_slice_new(CCLProfInst);

	/* Initialize structure fields. */
	inst->event_name = event_name;
	inst->queue_name = queue_name;
	inst->id = id;
	inst->instant = instant;
	inst->type = type;

	/* Return event instant data structure. */
	return inst;
}

/**
 * @internal
 * Free an event instant.
 *
 * @private @memberof ccl_prof_inst
 *
 * @param[in] instant Event instant to destroy.
 */
static void ccl_prof_inst_destroy(CCLProfInst* instant) {

	g_return_if_fail(instant != NULL);

	g_slice_free(CCLProfInst, instant);
}

/**
 * @internal
 * Compares two event instants for sorting within a GList. It is
 * an implementation of GCompareDataFunc from GLib.
 *
 * @private @memberof ccl_prof_inst
 *
 * @param[in] a First event instant to compare.
 * @param[in] b Second event instant to compare.
 * @param[in] userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if
 * a > b.
 */
static gint ccl_prof_inst_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {

	/* Cast input parameters to event instant data structures. */
	CCLProfInst* ev_inst1 = (CCLProfInst*) a;
	CCLProfInst* ev_inst2 = (CCLProfInst*) b;
	CCLProfSort sort = ccl_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CCLProfInstSort) sort.criteria) {
		gint result;
		case CCL_PROF_INST_SORT_INSTANT:
			/* Sort by instant */
			return CCL_PROF_CMP_INT(ev_inst1->instant, ev_inst2->instant,
				sort.order);
		case CCL_PROF_INST_SORT_ID:
			/* Sort by ID */
			result = CCL_PROF_CMP_INT(ev_inst1->id, ev_inst2->id,
				sort.order);
			if (result != 0) return result;
			if (ev_inst1->type == CCL_PROF_INST_TYPE_START)
				return sort.order ? 1 : -1;
			if (ev_inst1->type == CCL_PROF_INST_TYPE_END)
				return sort.order ? -1 : 1;
			g_warning("Expecting sort criteria by ID to be START or END.");
			return 0;
		/* We shouldn't get here. */
		default:
			g_warning("Unknown PROF_INST sort criteria/order.");
			return 0;
	}
}

/**
 * @internal
 * Create a new aggregate statistic for events of a given type.
 *
 * @private @memberof ccl_prof_agg
 *
 * @param[in] event_name Name of event.
 * @return New aggregate statistic.
 * */
static CCLProfAgg* ccl_prof_agg_new(const char* event_name) {
	CCLProfAgg* agg = g_slice_new(CCLProfAgg);
	agg->event_name = event_name;
	return agg;
}

/**
 * @internal
 * Free an aggregate statistic.
 *
 * @private @memberof ccl_prof_agg
 *
 * @param[in] agg Aggregate statistic to free.
 * */
static void ccl_prof_agg_destroy(CCLProfAgg* agg) {
	g_return_if_fail(agg != NULL);
	g_slice_free(CCLProfAgg, agg);
}

/**
 * @internal
 * Compares two aggregate event data instances for sorting
 * within a GList. It is an implementation of GCompareDataFunc from GLib.
 *
 * @private @memberof ccl_prof_agg
 *
 * @param[in] a First aggregate event data instance to compare.
 * @param[in] b Second aggregate event data instance to compare.
 * @param[in] userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if
 * a > b.
 */
static gint ccl_prof_agg_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {

	/* Cast input parameters to event instant data structures. */
	CCLProfAgg* ev_agg1 = (CCLProfAgg*) a;
	CCLProfAgg* ev_agg2 = (CCLProfAgg*) b;
	CCLProfSort sort = ccl_prof_get_sort(userdata);

	/* Perform comparison. */
	switch ((CCLProfAggSort) sort.criteria) {

		/* Sort by event name. */
		case CCL_PROF_AGG_SORT_NAME:
			return CCL_PROF_CMP_STR(ev_agg1->event_name,
				ev_agg2->event_name, sort.order);

		/* Sort by absolute time. */
		case CCL_PROF_AGG_SORT_TIME:
			return CCL_PROF_CMP_INT(ev_agg1->absolute_time,
				ev_agg2->absolute_time, sort.order);

		/* We shouldn't get here. */
		default:
			g_warning("Unknown PROF_AGG sort criteria/order.");
			return 0;
	}

}

/**
 * @internal
 * Create a new event profiling information object.
 *
 * @private @memberof ccl_prof_info
 *
 * @param[in] event_name Name of event.
 * @param[in] command_type Type of command which produced the event.
 * @param[in] queue_name Name of command queue which generated this
 * event.
 * @param[in] t_queued Device time in nanoseconds when the command
 * identified by event is enqueued in a command-queue by the host.
 * @param[in] t_submit Device time counter in nanoseconds when the
 * command identified by event that has been enqueued is submitted by
 * the host to the device associated with the command-queue.
 * @param[in] t_start Device time in nanoseconds when the command
 * identified by event starts execution on the device.
 * @param[in] t_end Device time in nanoseconds when the command
 * identified by event has finished execution on the device.
 * @return A new event profiling information object.
 * */
static CCLProfInfo* ccl_prof_info_new(const char* event_name,
	cl_command_type command_type, const char* queue_name,
	cl_ulong t_queued, cl_ulong t_submit, cl_ulong t_start,
	cl_ulong t_end) {

	CCLProfInfo* info = g_slice_new(CCLProfInfo);

	info->event_name = event_name;
	info->command_type = command_type;
	info->queue_name = queue_name;
	info->t_queued = t_queued;
	info->t_submit = t_submit;
	info->t_start = t_start;
	info->t_end = t_end;

	return info;
}

/**
 * @internal
 * Free an event profiling information object.
 *
 * @private @memberof ccl_prof_info
 *
 * @param[in] info Event profiling information object to free.
 * */
static void ccl_prof_info_destroy(CCLProfInfo* info) {
	g_return_if_fail(info != NULL);
	g_slice_free(CCLProfInfo, info);
}

/**
 * @internal
 * Compares two event profiling information instances for sorting
 * within a GList. It is an implementation of GCompareDataFunc from GLib.
 *
 * @private @memberof ccl_prof_info
 *
 * @param[in] a First event profiling information instance to compare.
 * @param[in] b Second event profiling information instance to compare.
 * @param[in] userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if
 * a > b.
 */
static gint ccl_prof_info_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {

	/* Cast input parameters to event instant data structures. */
	CCLProfInfo* ev1 = (CCLProfInfo*) a;
	CCLProfInfo* ev2 = (CCLProfInfo*) b;
	CCLProfSort sort = ccl_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CCLProfInfoSort) sort.criteria) {

		 /* Sort aggregate event data instances by event name. */
		case CCL_PROF_INFO_SORT_NAME_EVENT:
			return CCL_PROF_CMP_STR(ev1->event_name, ev2->event_name,
				sort.order);

		 /* Sort aggregate event data instances by queue name. */
		case CCL_PROF_INFO_SORT_NAME_QUEUE:
			return CCL_PROF_CMP_STR(ev1->queue_name, ev2->queue_name,
				sort.order);

		 /* Sort aggregate event data instances by queued time. */
		case CCL_PROF_INFO_SORT_T_QUEUED:
			return CCL_PROF_CMP_INT(ev1->t_queued, ev2->t_queued,
				sort.order);

		 /* Sort aggregate event data instances by submit time. */
		case CCL_PROF_INFO_SORT_T_SUBMIT:
			return CCL_PROF_CMP_INT(ev1->t_submit, ev2->t_submit,
				sort.order);

		 /* Sort aggregate event data instances by start time. */
		case CCL_PROF_INFO_SORT_T_START:
			return CCL_PROF_CMP_INT(ev1->t_start, ev2->t_start,
				sort.order);

		 /* Sort aggregate event data instances by end time. */
		case CCL_PROF_INFO_SORT_T_END:
			return CCL_PROF_CMP_INT(ev1->t_end, ev2->t_end, sort.order);

		/* We shouldn't get here. */
		default:
			g_warning("Unknown PROF_INFO sort criteria/order.");
			return 0;
	}

}

/**
 * @internal
 * Create a new event overlap object.
 *
 * @private @memberof ccl_prof_overlap
 *
 * @param[in] event1_name Name of first overlapping event.
 * @param[in] event2_name Name of second overlapping event.
 * @param[in] duration Overlap duration in nanoseconds.
 * @return A new event overlap object.
 * */
static CCLProfOverlap* ccl_prof_overlap_new(const char* event1_name,
	const char* event2_name, cl_ulong duration) {

	CCLProfOverlap* ovlp = g_slice_new(CCLProfOverlap);

	ovlp->event1_name = event1_name;
	ovlp->event2_name = event2_name;
	ovlp->duration = duration;

	return ovlp;

}

/**
 * @internal
 * Destroy an event overlap object.
 *
 * @private @memberof ccl_prof_overlap
 *
 * @param[in] ovlp Event overlap object to destroy.
 * */
static void ccl_prof_overlap_destroy(CCLProfOverlap* ovlp) {

	g_return_if_fail(ovlp != NULL);
	g_slice_free(CCLProfOverlap, ovlp);

}

/**
 * @internal
 * Compares two event overlap instances for sorting within a
 * GList. It is an implementation of GCompareDataFunc from GLib.
 *
 * @private @memberof ccl_prof_overlap
 *
 * @param[in] a First event overlap instance to compare.
 * @param[in] b Second event overlap instance to compare.
 * @param[in] userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint ccl_prof_overlap_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {

	gint result;

	/* Cast input parameters to event instant data structures. */
	CCLProfOverlap* ovlp1 = (CCLProfOverlap*) a;
	CCLProfOverlap* ovlp2 = (CCLProfOverlap*) b;
	CCLProfSort sort = ccl_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CCLProfOverlapSort) sort.criteria) {

		 /* Sort overlap instances by event name. */
		case CCL_PROF_OVERLAP_SORT_NAME:
			result = CCL_PROF_CMP_STR(ovlp1->event1_name,
				ovlp2->event1_name, sort.order);
			if (result != 0)
				return result;
			else
				return CCL_PROF_CMP_STR(ovlp1->event2_name,
					ovlp2->event2_name, sort.order);

		 /* Sort overlap instances by overlap duration. */
		case CCL_PROF_OVERLAP_SORT_DURATION:
			return CCL_PROF_CMP_INT(ovlp1->duration, ovlp2->duration,
				sort.order);

		/* We shouldn't get here. */
		default:
			g_warning("Unknown PROF_OVERLAP sort criteria/order.");
			return 0;
	}

}

/**
 * @internal
 * Add event for profiling.
 *
 * @private @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] cq_name Command queue name.
 * @param[in] evt Event wrapper object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * */
static void ccl_prof_add_event(CCLProf* prof, const char* cq_name,
	CCLEvent* evt, CCLErr** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail(err == NULL || *err == NULL);
	/* Make sure profile object is not NULL. */
	g_return_if_fail(prof != NULL);
	/* Make sure command queue name is not NULL. */
	g_return_if_fail(cq_name != NULL);
	/* Make sure event wrapper is not NULL. */
	g_return_if_fail(evt != NULL);

	/* Event name ID. */
	cl_uint* event_name_id;
	/* Specific event ID. */
	cl_uint event_id;
	/* Event instants. */
	cl_ulong instant_queued, instant_submit, instant_start, instant_end;
	/* Type of command which produced the event. */
	cl_command_type command_type;
	/* Event instant objects. */
	CCLProfInst* evinst_start;
	CCLProfInst* evinst_end;
	/* Internal error handling object. */
	CCLErr* err_internal = NULL;

	/* Event name. */
	const char* event_name;

	/* Get event name. */
	event_name = ccl_event_get_final_name(evt);

	/* Get event queued instant. */
	instant_queued = ccl_event_get_profiling_info_scalar(
		evt, CL_PROFILING_COMMAND_QUEUED, cl_ulong, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get event submit instant. */
	instant_submit = ccl_event_get_profiling_info_scalar(
		evt, CL_PROFILING_COMMAND_SUBMIT, cl_ulong, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get event start instant. */
	instant_start = ccl_event_get_profiling_info_scalar(
		evt, CL_PROFILING_COMMAND_START, cl_ulong, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get event end instant. */
	instant_end = ccl_event_get_profiling_info_scalar(
		evt, CL_PROFILING_COMMAND_END, cl_ulong, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* Get command type. */
	command_type = ccl_event_get_info_scalar(
		evt, CL_EVENT_COMMAND_TYPE, cl_command_type, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we get here, update number of profilable events, and get an ID
	 * for the given event. */
	event_id = ++prof->num_events;

	/* Check if event name is already registered in the table of event
	 * names... */
	if (!g_hash_table_contains(prof->event_names, event_name)) {
		/* ...if not, register it. */
		event_name_id =
			GUINT_TO_POINTER(g_hash_table_size(prof->event_names));
		g_hash_table_insert(
			prof->event_names,
			(gpointer) event_name,
			(gpointer) event_name_id);
	}

	/* If end instant occurs after start instant... */
	if (instant_end > instant_start) {

		/* Add event start instant to list of event instants. */
		evinst_start = ccl_prof_inst_new(event_name, cq_name, event_id,
			instant_start, CCL_PROF_INST_TYPE_START);
		prof->instants = g_list_prepend(
			prof->instants, (gpointer) evinst_start);

		/* Add event end instant to list of event instants. */
		evinst_end = ccl_prof_inst_new(event_name, cq_name, event_id,
			instant_end, CCL_PROF_INST_TYPE_END);
		prof->instants = g_list_prepend(
			prof->instants, (gpointer) evinst_end);

		/* Check if start instant is the oldest instant. If so, keep it. */
		if (instant_start < prof->t_start)
			prof->t_start = instant_start;

	} else {

		g_info("Event '%s' did not use device time. As such its "\
			"start and end instants will not be added to the list of "\
			"event instants.", event_name);

	}

	/* Add event information to list of event information..*/
	prof->infos = g_list_prepend(prof->infos,
		(gpointer) ccl_prof_info_new(event_name, command_type, cq_name,
			instant_queued, instant_submit, instant_start, instant_end));

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return. */
	return;
}

/**
 * @internal
 * Process command queues, i.e., add the respective events for
 * profiling.
 *
 * @private @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 */
static void ccl_prof_process_queues(CCLProf* prof, CCLErr** err) {

	/* Make sure err is NULL or it is not set. */
	g_return_if_fail(err == NULL || *err == NULL);
	/* Make sure profile object is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Hash table iterator. */
	GHashTableIter iter;
	/* Command queue name and wrapper. */
	gpointer cq_name;
	gpointer cq;
	/* Queue properties. */
	cl_command_queue_properties qprop;
	/* Internal error reporting object. */
	CCLErr* err_internal = NULL;

	/* Iterate over the command queues. */
	g_hash_table_iter_init(&iter, prof->queues);
	while (g_hash_table_iter_next(&iter, &cq_name, &cq)) {

		/* Check that queue has profiling enabled. */
		qprop = ccl_queue_get_info_scalar(cq, CL_QUEUE_PROPERTIES,
			cl_command_queue_properties, &err_internal);
		g_if_err_propagate_goto(err, err_internal, error_handler);
		g_if_err_create_goto(*err, CCL_ERROR,
			(qprop & CL_QUEUE_PROFILING_ENABLE) == 0, CCL_ERROR_OTHER,
			error_handler,
			"%s: the '%s' queue does not have profiling enabled.",
			CCL_STRD, (char*) cq_name);

		/* Iterate over the events in current command queue. */
		CCLEvent* evt;
		ccl_queue_iter_event_init((CCLQueue*) cq);
		while ((evt = ccl_queue_iter_event_next((CCLQueue*) cq))) {

			/* Add event for profiling. */
			ccl_prof_add_event(
				prof, (const char*) cq_name, evt, &err_internal);
			if ((err_internal != NULL) &&
				(((err_internal->domain == CCL_OCL_ERROR) &&
                 (err_internal->code == CL_PROFILING_INFO_NOT_AVAILABLE))
                ||
                 ((err_internal->domain == CCL_ERROR) &&
                 (err_internal->code == CCL_ERROR_INFO_UNAVAILABLE_OCL)))) {

				/* Some types of events in certain platforms don't
				 * provide profiling info. Don't stop profiling,
				 * ignore this specific event, but log a message
				 * saying so. */
				g_info("The '%s' event does not have profiling info",
					ccl_event_get_final_name(evt));
				g_clear_error(&err_internal);
				continue;
			}
			g_if_err_propagate_goto(err, err_internal, error_handler);

		}

		/* Release queue events. */
		ccl_queue_gc((CCLQueue*) cq);
	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);

finish:

	/* Return. */
	return;

}

/**
 * @internal
 * Determine aggregate event statistics.
 *
 * @private @memberof ccl_prof
 *
 * @param[in] prof The profile object.
 * */
static void ccl_prof_calc_agg(CCLProf* prof) {

	/* Make sure profile object is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Hash table iterator. */
	GHashTableIter iter;
	/* Aux. hash table for aggregate statistics. */
	GHashTable* agg_table;
	/* A pointer for a event name. */
	gpointer event_name;
	/* Aggregate event info. */
	CCLProfAgg* evagg = NULL;
	/* Type of sorting to perform on event list. */
	int sort_type;
	/* Aux. pointer for event data structure kept in a GList. */
	GList* curr_evinst_container = NULL;
	/* A pointer to a CCLProfAgg (agg. event info) variable. */
	gpointer value_agg;
	/* Auxiliary aggregate event info variable.*/
	CCLProfAgg* curr_agg = NULL;

	/* Create table of aggregate statistics. */
	agg_table = g_hash_table_new(g_str_hash, g_str_equal);

	/* Initalize table, and set aggregate values to zero. */
	g_hash_table_iter_init(&iter, prof->event_names);
	while (g_hash_table_iter_next(&iter, &event_name, NULL)) {
		evagg = ccl_prof_agg_new(event_name);
		evagg->absolute_time = 0;
		g_hash_table_insert(
			agg_table, event_name, (gpointer) evagg);
	}

	/* Sort event instants by eid, and then by START, END order. */
	sort_type = CCL_PROF_INST_SORT_ID | CCL_PROF_SORT_ASC;
	prof->instants = g_list_sort_with_data(
		prof->instants, ccl_prof_inst_comp,
		(gpointer) &sort_type);

	/* Iterate through all event instants and determine total times. */
	curr_evinst_container = prof->instants;
	while (curr_evinst_container) {

		/* Loop aux. variables. */
		CCLProfInst* curr_evinst = NULL;
		cl_ulong start_inst, end_inst;

		/* Get START event instant. */
		curr_evinst = (CCLProfInst*) curr_evinst_container->data;
		start_inst = curr_evinst->instant;

		/* Get END event instant */
		curr_evinst_container = curr_evinst_container->next;
		curr_evinst = (CCLProfInst*) curr_evinst_container->data;
		end_inst = curr_evinst->instant;

		/* Add new interval to respective aggregate value. */
		curr_agg = (CCLProfAgg*) g_hash_table_lookup(
			agg_table, curr_evinst->event_name);
		curr_agg->absolute_time += end_inst - start_inst;
		prof->total_events_time += end_inst - start_inst;

		/* Get next START event instant. */
		curr_evinst_container = curr_evinst_container->next;
	}

	/* Determine relative times. */
	g_hash_table_iter_init(&iter, agg_table);
	while (g_hash_table_iter_next(&iter, &event_name, &value_agg)) {
		curr_agg = (CCLProfAgg*) value_agg;
		curr_agg->relative_time =
			((double) curr_agg->absolute_time)
			/
			((double) prof->total_events_time);
	}

	/* Keep a list of aggregates. */
	prof->aggs = g_hash_table_get_values(agg_table);

	/* Release aux. hash table. */
	g_hash_table_destroy(agg_table);

}

/**
 * @internal
 * Determine event overlaps for the given profile object.
 *
 * @private @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 */
static void ccl_prof_calc_overlaps(CCLProf* prof) {

	/* Make sure profile object is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Total overlap time. */
	cl_ulong total_overlap = 0;
	/* Overlap matrix. */
	cl_ulong* overlap_matrix = NULL;
	/* Number of event names. */
	cl_uint num_event_names;
	/* Helper table to account for all overlapping events. */
	GHashTable* overlaps = NULL;
	/* Occurring events table. */
	GHashTable* occurring_events = NULL;
	/* Type of sorting to perform. */
	CCLProfInstSort sort_type;
	/* Container for current event instants. */
	GList* curr_evinst_container;

	/* Determine number of event names. */
	num_event_names = g_hash_table_size(prof->event_names);

	/* Initialize overlap matrix. */
	overlap_matrix = g_slice_alloc0(
		sizeof(cl_ulong) * num_event_names * num_event_names);

	/* Initialize helper table to account for all overlapping events. */
	overlaps = g_hash_table_new_full(g_direct_hash, g_direct_equal,
		NULL, (GDestroyNotify) g_hash_table_destroy);

	/* Setup ocurring events table (key: eventID, value: eventNameID) */
	occurring_events = g_hash_table_new(g_int_hash, g_int_equal);

	/* Sort all event instants. */
	sort_type = CCL_PROF_INST_SORT_INSTANT | CCL_PROF_SORT_ASC;
	prof->instants = g_list_sort_with_data(prof->instants,
		ccl_prof_inst_comp, (gpointer) &sort_type);

	/* Iterate through all event instants */
	curr_evinst_container = prof->instants;
	while (curr_evinst_container) {

		/* ** Loop aux. variables. ** */

		/* Current event instant. */
		CCLProfInst* curr_evinst = NULL;
		/* Inner hash table (is value for overlap hash table). */
		GHashTable* inner_table = NULL;
		/* Hash table iterator. */
		GHashTableIter iter;
		/* Hashtable key, event name ID for current event, event
		 * name ID for occurring event. */
		gpointer key_eid, ueid_curr_ev, ueid_occu_ev;
		/* Keys for start and end event instants. */
		cl_uint eid_key1, eid_key2;
		/* Event overlap in nanoseconds. */
		cl_ulong eff_overlap;

		/* Get current event instant. */
		curr_evinst = (CCLProfInst*) curr_evinst_container->data;

		/* Check if event time is START or END time */
		if (curr_evinst->type == CCL_PROF_INST_TYPE_START) {
			/* Event START instant. */

			/* 1 - Check for overlaps with ocurring events */

			g_hash_table_iter_init(&iter, occurring_events);
			while (g_hash_table_iter_next (&iter, &key_eid, NULL)) {

				/* The first hash table key will be the smaller event id. */
				eid_key1 = curr_evinst->id <= *((cl_uint*) key_eid)
					? curr_evinst->id
					: *((cl_uint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = curr_evinst->id > *((cl_uint*) key_eid)
					? curr_evinst->id
					: *((cl_uint*) key_eid);
				/* Check if the first key (smaller id) is already in the
				 * hash table... */
				if (!g_hash_table_lookup_extended(overlaps,
					GUINT_TO_POINTER(eid_key1), NULL,
					(gpointer) &inner_table)) {
					/* ...if not in table, add it to table, creating a new
					 * inner table as value. Inner table will be initalized
					 * with second key (larger id) as key and event start
					 * instant as value. */
					inner_table = g_hash_table_new(
						g_direct_hash, g_direct_equal);
					g_hash_table_insert(
						overlaps, GUINT_TO_POINTER(eid_key1), inner_table);
				}
				/* Add second key (larger id) to inner tabler, setting the
				 * start instant as the value. */
				g_hash_table_insert(
					inner_table,
					GUINT_TO_POINTER(eid_key2),
					&(curr_evinst->instant));
			}

			/* 2 - Add event to occurring events. */
			g_hash_table_insert(
				occurring_events,
				&(curr_evinst->id), /* eid */
				g_hash_table_lookup(prof->event_names,
					curr_evinst->event_name) /* ueid */
			);

		} else {
			/* Event END instant. */

			/* 1 - Remove event from ocurring events */
			g_hash_table_remove(occurring_events, &(curr_evinst->id));

			/* 2 - Check for overlap termination with current events */
			g_hash_table_iter_init(&iter, occurring_events);
			while (g_hash_table_iter_next(&iter, &key_eid, &ueid_occu_ev)) {
				/* The first hash table key will be the smaller event id. */
				eid_key1 = curr_evinst->id <= *((cl_uint*) key_eid)
					? curr_evinst->id
					: *((cl_uint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = curr_evinst->id > *((cl_uint*) key_eid)
					? curr_evinst->id
					: *((cl_uint*) key_eid);
				/* Get effective overlap in nanoseconds. */
				inner_table = g_hash_table_lookup(
					overlaps, GUINT_TO_POINTER(eid_key1));
				eff_overlap =
					curr_evinst->instant
					-
					*((cl_ulong*) g_hash_table_lookup(
						inner_table, GUINT_TO_POINTER(eid_key2)));
				/* Add overlap to overlap matrix. */
				ueid_curr_ev = g_hash_table_lookup(
					prof->event_names, curr_evinst->event_name);
				cl_uint ueid_min =
					GPOINTER_TO_UINT(ueid_curr_ev)
						<= GPOINTER_TO_UINT(ueid_occu_ev)
					? GPOINTER_TO_UINT(ueid_curr_ev)
					: GPOINTER_TO_UINT(ueid_occu_ev);
				cl_uint ueid_max =
					GPOINTER_TO_UINT(ueid_curr_ev)
						> GPOINTER_TO_UINT(ueid_occu_ev)
					? GPOINTER_TO_UINT(ueid_curr_ev)
					: GPOINTER_TO_UINT(ueid_occu_ev);
				overlap_matrix[ueid_min * num_event_names + ueid_max] +=
					eff_overlap;
				total_overlap += eff_overlap;
			}
		}

		/* Get next event instant. */
		curr_evinst_container = curr_evinst_container->next;
	}

	/* Populate list of overlaps. */
	for (cl_uint i = 0; i < num_event_names; i++) {
		for (cl_uint j = 0; j < num_event_names; j++) {
			if (overlap_matrix[i * num_event_names + j] > 0) {
				/* There is an overlap here, create overlap object... */
				CCLProfOverlap* ovlp = ccl_prof_overlap_new(
					(const char*) g_hash_table_lookup(
						prof->event_name_ids, GUINT_TO_POINTER(i)),
					(const char*) g_hash_table_lookup(
						prof->event_name_ids, GUINT_TO_POINTER(j)),
					overlap_matrix[i * num_event_names + j]);
				/*  ...and add it to list of overlaps. */
				prof->overlaps = g_list_prepend(
					prof->overlaps, (gpointer) ovlp);
			}
		}
	}

	/* Determine and save effective events time. */
	prof->total_events_eff_time = prof->total_events_time - total_overlap;

	/* Free the overlaps matrix. */
	g_slice_free1(sizeof(cl_ulong) * num_event_names * num_event_names,
		overlap_matrix);

	/* Free overlaps hash table. */
	g_hash_table_destroy(overlaps);

	/* Free occurring_events hash table. */
	g_hash_table_destroy(occurring_events);

}

/**
 * @addtogroup CCL_PROFILER
 * @{
 */

/**
 * Create a new profile object.
 *
 * @public @memberof ccl_prof
 *
 * @return A new profile object.
 * */
CCL_EXPORT
CCLProf* ccl_prof_new() {

	/* Allocate memory for new profile data structure. */
	CCLProf* prof = g_slice_new0(CCLProf);

	/* Set absolute start time to maximum possible. */
	prof->t_start = CL_ULONG_MAX;

	/* Return new profile data structure */
	return prof;

}

/**
 * Destroy a profile object.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object to destroy.
 * */
CCL_EXPORT
void ccl_prof_destroy(CCLProf* prof) {

	/* Profile to destroy cannot be NULL. */
	g_return_if_fail(prof != NULL);

	/* Destroy table of event names. */
	if (prof->event_names != NULL)
		g_hash_table_destroy(prof->event_names);

	/* Destroy table of event IDs. */
	if (prof->event_name_ids != NULL)
		g_hash_table_destroy(prof->event_name_ids);

	/* Destroy table of command queue wrappers. */
	if (prof->queues != NULL)
		g_hash_table_destroy(prof->queues);

	/* Destroy list of all event instants. */
	if (prof->instants != NULL)
		g_list_free_full(
			prof->instants, (GDestroyNotify) ccl_prof_inst_destroy);

	/* Destroy list of event profiling information. */
	if (prof->infos != NULL)
		g_list_free_full(
			prof->infos, (GDestroyNotify) ccl_prof_info_destroy);

	/* Destroy list of aggregate statistics. */
	if (prof->aggs != NULL)
		g_list_free_full(
			prof->aggs, (GDestroyNotify) ccl_prof_agg_destroy);

	/* Free the overlap matrix. */
	if (prof->overlaps != NULL)
		g_list_free_full(
			prof->overlaps, (GDestroyNotify) ccl_prof_overlap_destroy);

	/* Free the summary string. */
	if (prof->summary != NULL)
		g_free(prof->summary);

	/* Destroy timer. */
	if (prof->timer != NULL)
		g_timer_destroy(prof->timer);

	/* Destroy profile data structure. */
	g_slice_free(CCLProf, prof);

}

/**
 * Starts the global profiler timer. Only required if client
 * wishes to compare the effectively elapsed time with the OpenCL
 * kernels time.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof A profile object.
 * */
CCL_EXPORT
void ccl_prof_start(CCLProf* prof) {

	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Start timer. */
	prof->timer = g_timer_new();
}

/**
 * Stops the global profiler timer. Only required if
 * ccl_prof_start() was called.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof A profile object.
 * */
CCL_EXPORT
void ccl_prof_stop(CCLProf* prof) {

	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Stop timer. */
	g_timer_stop(prof->timer);
}

/**
 * If profiling has started but not stopped, returns the time
 * since the profiling started. If profiling has been stopped, returns
 * the elapsed time between the time it started and the time it stopped.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof A profile object.
 * @return number of seconds elapsed, including any fractional part.
 * */
CCL_EXPORT
double ccl_prof_time_elapsed(CCLProf* prof) {

	/* Make sure profile is not NULL. */
	g_return_val_if_fail(prof != NULL, 0.0);

	/* Stop timer. */
	return g_timer_elapsed(prof->timer, NULL);
}

/**
 * Add a command queue wrapper for profiling.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof A profile object.
 * @param[in] cq_name Command queue name.
 * @param[in] cq Command queue wrapper object.
 * */
CCL_EXPORT
void ccl_prof_add_queue(
	CCLProf* prof, const char* cq_name, CCLQueue* cq) {

	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);
	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);
	/* Must be added before calculations. */
	g_return_if_fail(prof->calc == FALSE);

	/* Check if table needs to be created first. */
	if (prof->queues == NULL) {
		prof->queues = g_hash_table_new_full(
			g_str_hash, g_direct_equal, NULL,
			(GDestroyNotify) ccl_queue_destroy);
	}
	/* Warn if table already contains a queue with the specified
	 * name. */
	if (g_hash_table_contains(prof->queues, cq_name))
		g_warning("Profile object already contains a queue named '%s'." \
			"The existing queue will be replaced.", cq_name);

	/* Add queue to queue table. */
	g_hash_table_replace(prof->queues, (gpointer) cq_name, cq);

	/* Increment queue ref. count. */
	ccl_queue_ref(cq);

}

/**
 * Determine aggregate statistics for the given profile object.
 *
 * The command queues to be profiled will have their events garbage
 * collected with ::ccl_queue_gc(). As such, they can be reused and
 * re-added for profiling to a new profile object.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof A profile object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return CL_TRUE if function terminates successfully, or CL_FALSE
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_prof_calc(CCLProf* prof, CCLErr** err) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	/* Calculations can only be performed once. */
	g_return_val_if_fail(prof->calc == FALSE, CL_FALSE);
	/* There must be some queues to process. */
	g_return_val_if_fail(prof->queues != NULL, CL_FALSE);

	/* Internal error handling object. */
	CCLErr* err_internal = NULL;

	/* Function return status flag. */
	cl_bool status;

	/* Hash table iterator. */
	GHashTableIter iter;

	/* Auxiliary pointers for determining the table of event_ids. */
	gpointer p_evt_name, p_id;

	/* Create table of event names. */
	prof->event_names = g_hash_table_new(g_str_hash, g_str_equal);

	/* Process queues and respective events. */
	ccl_prof_process_queues(prof, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* Obtain the event_ids table (by reversing the event_names table) */
	prof->event_name_ids = g_hash_table_new(g_direct_hash, g_direct_equal);
	/* Populate table. */
	g_hash_table_iter_init(&iter, prof->event_names);
	while (g_hash_table_iter_next(&iter, &p_evt_name, &p_id)) {
		g_hash_table_insert(prof->event_name_ids, p_id, p_evt_name);
	}

	/* Calculate aggregate statistics. */
	ccl_prof_calc_agg(prof);

	/* Determine event overlaps. */
	ccl_prof_calc_overlaps(prof);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	prof->calc = TRUE;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	status = CL_FALSE;

finish:

	/* Return status. */
	return status;

}

/**
 * Return aggregate statistics for events with the given name.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] event_name Event name.
 * @return Aggregate statistics for events with the given name.
 */
CCL_EXPORT
const CCLProfAgg* ccl_prof_get_agg(
	CCLProf* prof, const char* event_name) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* Make sure event name is not NULL. */
	g_return_val_if_fail(event_name != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* Find the aggregate statistic for the given event. */
	CCLProfAgg* agg = NULL;
	GList* agg_container = prof->aggs;
	while (agg_container != NULL) {
		const char* curr_event_name =
			((CCLProfAgg*) agg_container->data)->event_name;
		if (g_strcmp0(event_name, curr_event_name) == 0) {
			agg = (CCLProfAgg*) agg_container->data;
			break;
		}
		agg_container = agg_container->next;
	}

	/* Return result. */
	return agg;
}

/**
 * Initialize an iterator for profiled aggregate event
 * instances.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] sort Bitfield of ::CCLProfAggSort OR ::CCLProfSortOrder,
 * for example `CCL_PROF_AGG_SORT_NAME | CCL_PROF_SORT_DESC`.
 * */
CCL_EXPORT
void ccl_prof_iter_agg_init(CCLProf* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of aggregate statistics as requested by client. */
	prof->aggs = g_list_sort_with_data(
		prof->aggs, ccl_prof_agg_comp, &sort);

	/* Set the iterator as the first element in list. */
	prof->agg_iter = prof->aggs;

}

/**
 * Return the next aggregate statistic instance.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The next aggregate statistic instance or `NULL` if no more instances
 * are left.
 * */
CCL_EXPORT
const CCLProfAgg* ccl_prof_iter_agg_next(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* The aggregate statistic to return. */
	CCLProfAgg* agg;

	/* Check if there are any more left. */
	if (prof->agg_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		agg = (CCLProfAgg*) prof->agg_iter->data;
		prof->agg_iter = prof->agg_iter->next;
	} else {
		/* Nothing left. */
		agg = NULL;
	}

	/* Return the aggregate statistic. */
	return (const CCLProfAgg*) agg;
}

/**
 * Initialize an iterator for event profiling info instances.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] sort Bitfield of ::CCLProfInfoSort OR ::CCLProfSortOrder,
 * for example `CCL_PROF_INFO_SORT_T_START | CCL_PROF_SORT_ASC`.
 * */
CCL_EXPORT
void ccl_prof_iter_info_init(CCLProf* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of event prof. infos as requested by client. */
	prof->infos = g_list_sort_with_data(
		prof->infos, ccl_prof_info_comp, &sort);

	/* Set the iterator as the first element in list. */
	prof->info_iter = prof->infos;
}

/**
 * Return the next event profiling info instance.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The next event profiling info instance or `NULL` if no more instances
 * are left.
 * */
CCL_EXPORT
const CCLProfInfo* ccl_prof_iter_info_next(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* The event profiling info instance to return. */
	CCLProfInfo* info;

	/* Check if there are any more left. */
	if (prof->info_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		info = (CCLProfInfo*) prof->info_iter->data;
		prof->info_iter = prof->info_iter->next;
	} else {
		/* Nothing left. */
		info = NULL;
	}

	/* Return the profiling info instance. */
	return (const CCLProfInfo*) info;
}

/**
 * Initialize an iterator for event instant instances.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] sort Bitfield of ::CCLProfInstSort OR ::CCLProfSortOrder,
 * for example `CCL_PROF_INST_SORT_INSTANT | CCL_PROF_SORT_ASC`.
 * */
CCL_EXPORT
void ccl_prof_iter_inst_init(CCLProf* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of event instants as requested by client. */
	prof->instants = g_list_sort_with_data(
		prof->instants, ccl_prof_inst_comp, &sort);

	/* Set the iterator as the first element in list. */
	prof->inst_iter = prof->instants;

}

/**
 * Return the next event instant instance.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The next event instant instance or `NULL` if no more instances
 * are left.
 * */
CCL_EXPORT
const CCLProfInst* ccl_prof_iter_inst_next(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* The event profiling info instance to return. */
	CCLProfInst* inst;

	/* Check if there are any more left. */
	if (prof->inst_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		inst = (CCLProfInst*) prof->inst_iter->data;
		prof->inst_iter = prof->inst_iter->next;
	} else {
		/* Nothing left. */
		inst = NULL;
	}

	/* Return the profiling info instance. */
	return (const CCLProfInst*) inst;
}

/**
 * Initialize an iterator for overlap instances.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] sort Bitfield of ::CCLProfOverlapSort OR
 * ::CCLProfSortOrder, for example
 * `CCL_PROF_OVERLAP_SORT_DURATION | CCL_PROF_SORT_DESC`.
 * */
CCL_EXPORT
void ccl_prof_iter_overlap_init(CCLProf* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of overlaps as requested by client. */
	prof->overlaps = g_list_sort_with_data(
		prof->overlaps, ccl_prof_overlap_comp, &sort);

	/* Set the iterator as the first element in list. */
	prof->overlap_iter = prof->overlaps;
}

/**
 * Return the next overlap instance.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The next overlap instance or `NULL` if no more instances
 * are left.
 * */
CCL_EXPORT
const CCLProfOverlap* ccl_prof_iter_overlap_next(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* The overlap instance to return. */
	CCLProfOverlap* ovlp;

	/* Check if there are any more left. */
	if (prof->overlap_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		ovlp = (CCLProfOverlap*) prof->overlap_iter->data;
		prof->overlap_iter = prof->overlap_iter->next;
	} else {
		/* Nothing left. */
		ovlp = NULL;
	}

	/* Return the overlap instance. */
	return (const CCLProfOverlap*) ovlp;
}

/**
 * Get duration of all events in nanoseconds.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The duration of all events in nanoseconds.
 * */
CCL_EXPORT
cl_ulong ccl_prof_get_duration(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, 0);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, 0);

	/* Return requested data. */
	return prof->total_events_time;

}

/**
 * Get effective duration of all events in nanoseconds, i.e. the
 * duration of all events minus event overlaps.
 *
 * If no overlaps occur, this function will return the same value as
 * ::ccl_prof_get_duration().
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @return The effective duration of all events in nanoseconds, i.e. the
 * duration of all events minus event overlaps.
 * */
CCL_EXPORT
cl_ulong ccl_prof_get_eff_duration(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, 0);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, 0);

	/* Return requested data. */
	return prof->total_events_eff_time;
}

/**
 * Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics (sorted
 * by absolute time), and a table of event overlaps (sorted by overlap
 * duration).
 *
 * For more control of where and how this summary is printed, use the
 * ccl_prof_get_summary() function.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * */
CCL_EXPORT
void ccl_prof_print_summary(CCLProf* prof) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Summary to print. */
	const char* summary;

	/* Get the summary. */
	summary = ccl_prof_get_summary(prof,
		CCL_PROF_AGG_SORT_TIME | CCL_PROF_SORT_DESC,
		CCL_PROF_OVERLAP_SORT_DURATION | CCL_PROF_SORT_DESC);

	/* Print summary. */
	g_printf("%s", summary);

}

/**
 * Get a summary with the profiling info. More specifically,
 * this function returns a string containing a table of aggregate event
 * statistics and a table of event overlaps. The order of the returned
 * information can be specified in the function arguments.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] agg_sort Sorting performed on aggregate statistics
 * (bitfield of ::CCLProfAggSort ORed with ::CCLProfSortOrder).
 * @param[in] ovlp_sort Sorting performed on event overlaps (bitfield of
 * ::CCLProfOverlapSort ORed with ::CCLProfSortOrder).
 * @return A string containing the summary.
 * */
CCL_EXPORT
const char* ccl_prof_get_summary(
	CCLProf* prof, int agg_sort, int ovlp_sort) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);

	/* Current aggregate statistic to print. */
	const CCLProfAgg* agg = NULL;
	/* Current overlap to print. */
	const CCLProfOverlap* ovlp = NULL;
	/* The summary string. */
	GString* str_obj = g_string_new("\n");

	/* Show aggregate event times */
	g_string_append_printf(str_obj,
		" Aggregate times by event  :\n");
	g_string_append_printf(str_obj,
		"   ------------------------------------------------------------------\n");
	g_string_append_printf(str_obj,
		"   | Event name                     | Rel. time (%%) | Abs. time (s) |\n");
	g_string_append_printf(str_obj,
		"   ------------------------------------------------------------------\n");
	ccl_prof_iter_agg_init(prof, agg_sort);
	while ((agg = ccl_prof_iter_agg_next(prof)) != NULL) {
		g_string_append_printf(str_obj,
			"   | %-30.30s | %13.4f | %13.4e |\n",
			agg->event_name,
			agg->relative_time * 100.0,
			agg->absolute_time * 1e-9);
	}
	g_string_append_printf(str_obj,
		"   ------------------------------------------------------------------\n");

	/* Show total events time */
	if (prof->total_events_time > 0) {
		g_string_append_printf(str_obj,
			"                                    |         Total | %13.4e |\n",
			prof->total_events_time * 1e-9);
		g_string_append_printf(str_obj,
			"                                    ---------------------------------\n");
	}

	/* *** Show overlaps *** */

	if (g_list_length(prof->overlaps) > 0) {
		/* Title the several overlaps. */
		g_string_append_printf(str_obj,
			" Event overlaps            :\n");
		g_string_append_printf(str_obj,
			"   ------------------------------------------------------------------\n");
		g_string_append_printf(str_obj,
			"   | Event 1                | Event2                 | Overlap (s)  |\n");
		g_string_append_printf(str_obj,
			"   ------------------------------------------------------------------\n");
		/* Show overlaps table. */
		ccl_prof_iter_overlap_init(prof, ovlp_sort);
		while ((ovlp = ccl_prof_iter_overlap_next(prof)) != NULL) {
			g_string_append_printf(str_obj, "   | %-22.22s | %-22.22s | %12.4e |\n",
				ovlp->event1_name, ovlp->event2_name, ovlp->duration * 1e-9);
		}
		g_string_append_printf(str_obj,
			"   ------------------------------------------------------------------\n");
		/* Show total events effective time (discount overlaps) */
		g_string_append_printf(str_obj,
			"                            |                  Total | %12.4e |\n",
			(prof->total_events_time - prof->total_events_eff_time) * 1e-9);
		g_string_append_printf(str_obj,
			"                            -----------------------------------------\n");
		g_string_append_printf(str_obj,
			" Tot. of all events (eff.) : %es\n",
			prof->total_events_eff_time * 1e-9);
	} else {
		g_string_append_printf(str_obj,
			" Event overlaps            : None\n");
	}

	/* Show total elapsed time */
	if (prof->timer) {
		double t_elapsed = g_timer_elapsed(prof->timer, NULL);
		g_string_append_printf(str_obj,
			" Total elapsed time        : %es\n", t_elapsed);
		g_string_append_printf(str_obj,
			" Time spent in device      : %.2f%%\n",
			prof->total_events_eff_time * 1e-9 * 100 / t_elapsed);
		g_string_append_printf(str_obj,
			" Time spent in host        : %.2f%%\n",
			100 - prof->total_events_eff_time * 1e-9 * 100 / t_elapsed);
	}
	g_string_append_printf(str_obj, "\n");

	/* If a summary already exists, free it before keeping a new one. */
	if (prof->summary != NULL)
		g_free(prof->summary);

	/* Free String object and keep underlying string in prof struct. */
	prof->summary = g_string_free(str_obj, FALSE);

	/* Return summary string. */
	return (const char*) prof->summary;

}

/**
 * Export event profiling information to a given stream.
 *
 * Each line of the exported data will have the following format,
 * ordered by event start time:
 *
 *     queue start-time end-time event-name
 *
 * For example:
 *
 *     q0    100    120    load_data1
 *     q1    100    132    load_data2
 *     q0    121    159    process_data1
 *     q1    133    145    process_data2
 *     q0    146    157    read_result
 *
 * Several export parameters can be configured with the
 * ccl_prof_get_export_opts() and ccl_prof_set_export_opts() functions,
 * by manipulating a ::CCLProfExportOptions struct.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[out] stream Stream where export info to.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return CL_TRUE if function terminates successfully, CL_FALSE
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_prof_export_info(CCLProf* prof, FILE* stream, CCLErr** err) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure stream is not NULL. */
	g_return_val_if_fail(stream != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, CL_FALSE);

	/* Stream write status. */
	int write_status;
	/* Return status. */
	cl_bool ret_status;
	/* Current event information. */
	const CCLProfInfo* curr_ev;
	/* Start time. */
	cl_ulong t_start = 0;

	/* Sort event information by START order, ascending. */
	ccl_prof_iter_info_init(
		prof, CCL_PROF_INFO_SORT_T_START | CCL_PROF_SORT_ASC);

	/* If zero start is set, use the start time of the first event
	 * as zero time. */
	if (export_options.zero_start)
		t_start = prof->t_start;

	/* Iterate through all event information and export it to stream. */
	while ((curr_ev = ccl_prof_iter_info_next(prof)) != NULL) {

		/* Write to stream. */
		write_status = fprintf(stream, "%s%s%s%s%lu%s%lu%s%s%s%s%s",
			export_options.queue_delim,
			curr_ev->queue_name,
			export_options.queue_delim,
			export_options.separator,
			(unsigned long) (curr_ev->t_start - t_start),
			export_options.separator,
			(unsigned long) (curr_ev->t_end - t_start),
			export_options.separator,
			export_options.evname_delim,
			curr_ev->event_name,
			export_options.evname_delim,
			export_options.newline);

		g_if_err_create_goto(*err, CCL_ERROR, write_status < 0,
			CCL_ERROR_STREAM_WRITE, error_handler,
			"Error while exporting profiling information" \
			"(writing to stream).");

	}

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	ret_status = CL_TRUE;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	ret_status = CL_FALSE;

finish:

	/* Return status. */
	return ret_status;

}

/**
 * Helper function which exports profiling info to a given file,
 * automatically opening and closing the file. See the
 * ccl_prof_export_info() for more information.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] prof Profile object.
 * @param[in] filename Name of file where information will be saved to.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return CL_TRUE if function terminates successfully, CL_FALSE
 * otherwise.
 * */
CCL_EXPORT
cl_bool ccl_prof_export_info_file(
	CCLProf* prof, const char* filename, CCLErr** err) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure filename is not NULL. */
	g_return_val_if_fail(filename != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, CL_FALSE);

	/* Aux. var. */
	cl_bool status;

	/* Internal CCLErr object. */
	CCLErr* err_internal = NULL;

	/* Open file. */
	FILE* fp = fopen(filename, "w");
	g_if_err_create_goto(*err, CCL_ERROR, fp == NULL,
		CCL_ERROR_OPENFILE, error_handler,
		"Unable to open file '%s' for exporting.", filename);

	/* Export data. */
	status = ccl_prof_export_info(prof, fp, &err_internal);
	g_if_err_propagate_goto(err, err_internal, error_handler);

	/* If we got here, everything is OK. */
	g_assert(err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;

error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert(err == NULL || *err != NULL);
	status = CL_FALSE;

finish:

	/* Close file. */
	if (fp) fclose(fp);

	/* Return file contents in string form. */
	return status;

}

/**
 * Set export options using a ::CCLProfExportOptions struct.
 *
 * @public @memberof ccl_prof
 *
 * @param[in] export_opts Export options to set.
 * */
CCL_EXPORT
void ccl_prof_set_export_opts(CCLProfExportOptions export_opts) {
	export_options = export_opts;
}

/**
 * Get current export options.
 *
 * @public @memberof ccl_prof
 *
 * @return Current export options.
 * */
CCL_EXPORT
 CCLProfExportOptions ccl_prof_get_export_opts() {
	return export_options;
}

/** @}*/
