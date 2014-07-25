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
 * @brief Function implementations of a profiling tool for OpenCL.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "profiler.h"

#define CL4_PROF_CMP_INT(x, y, ord) (((ord) == CL4_PROF_SORT_ASC) \
	? (((x) > (y)) ? 1 : (((x) < (y)) ? -1 : 0)) \
	: (((x) < (y)) ? 1 : (((x) > (y)) ? -1 : 0)))

#define CL4_PROF_CMP_STR(s1, s2, ord) (((ord) == CL4_PROF_SORT_ASC) \
	? g_strcmp0(s1, s2) : g_strcmp0(s2, s1))

#define cl4_prof_get_sort(userdata) \
	{0x0F & *((int*) userdata), 0xF0 & *((int*) userdata)}
	
typedef struct cl4_prof_sort_data {
	CL4ProfSortOrder order;
	int criteria;
} CL4ProfSort;

/**
 * @brief Profile class, contains profiling information of OpenCL 
 * queues and events.
 */
struct cl4_prof {
	
	/** Flag indicating if the profiling information has already been 
	 * determined. */
	gboolean calc;

	/** Hash table with keys equal to the events name, and values
	 * equal to a unique id for each event name. */
	GHashTable* event_names;
	/** Reverse of event_names in terms of key-values. */ 
	GHashTable* event_name_ids;
	/** Table of command queue wrappers. */
	GHashTable* cqueues;

	/** Total number of events. */
	guint num_events;

	/** Instants (start and end) of all events. */
	GList* instants;
	/** List of all events profiling information. */
	GList* infos;
	/** Aggregate statistics for all events in 
	 * CL4Prof#instants. */
	GList* aggs;
	/** List of event overlaps. */
	GList* overlaps;

	/** Aggregate event statistics iterator. */
	GList* agg_iter;
	/** Event info iterator. */
	GList* info_iter;
	/** Event instant iterator. */
	GList* inst_iter;
	/** Overlaps iterator. */
	GList* overlap_iter;

	/** Total time taken by all events. */
	cl_ulong total_events_time;
	/** Total time taken by all events except intervals where events overlaped. */
	cl_ulong total_events_eff_time;
	/** Time at which the first (oldest) event started. */
	cl_ulong start_time;

	/** Keeps track of time during the complete profiling session. */
	GTimer* timer;

};

/* Default export options. */
static CL4ProfExportOptions export_options = {

	.separator = "\t",
	.newline = "\n",
	.queue_delim = "",
	.evname_delim = "",
	.zero_start = CL_TRUE

};
/* Access to export_options should be thread-safe. */
G_LOCK_DEFINE_STATIC(export_options);

/** 
 * @brief Create new event instant.
 * 
 * @param event_name Name of event.
 * @param queue_name Name of command queue associated with event.
 * @param id Id of event.
 * @param instant Even instant in nanoseconds.
 * @param type Type of event instant: CL4_PROF_INST_TYPE_START or 
 * CL4_PROF_INST_TYPE_END.
 * @return A new event instant.
 */
static CL4ProfInst* cl4_prof_inst_new(const char* event_name,
	const char* queue_name, guint id, cl_ulong instant, 
	CL4ProfInstType type) {
	
	/* Allocate memory for event instant data structure. */
	CL4ProfInst* inst = g_slice_new(CL4ProfInst);

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
 * @brief Free an event instant.
 * 
 * @param instant Event instant to destroy. 
 */
static void cl4_prof_inst_destroy(gpointer instant) {
	
	g_return_if_fail(instant != NULL);
	
	g_slice_free(CL4ProfInst, instant);
}

/**
 * @brief Compares two event instants for sorting within a GList. It is
 * an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First event instant to compare.
 * @param b Second event instant to compare.
 * @param userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_inst_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
	
	/* Cast input parameters to event instant data structures. */
	CL4ProfInst* ev_inst1 = (CL4ProfInst*) a;
	CL4ProfInst* ev_inst2 = (CL4ProfInst*) b;
	CL4ProfSort sort = cl4_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CL4ProfInstSort) sort.criteria) {
		gint result;
		case CL4_PROF_INST_SORT_INSTANT:
			/* Sort by instant */
			return CL4_PROF_CMP_INT(ev_inst1->instant, ev_inst2->instant, 
				sort.order);
		case CL4_PROF_INST_SORT_ID:
			/* Sort by ID */
			result = CL4_PROF_CMP_INT(ev_inst1->id, ev_inst2->id, 
				sort.order);
			if (result != 0) return result;
			if (ev_inst1->type == CL4_PROF_INST_TYPE_START) 
				return sort.order ? 1 : -1;
			if (ev_inst1->type == CL4_PROF_INST_TYPE_END) 
				return sort.order ? -1 : 1;
		/* We shouldn't get here. */
		default:
			g_return_val_if_reached(0);
	}
}

/** 
 * @brief Create a new aggregate statistic for events of a given type.
 * 
 * @param event_name Name of event.
 * @return New aggregate statistic.
 * */
static CL4ProfAgg* cl4_prof_agg_new(const char* event_name) {
	CL4ProfAgg* agg = g_slice_new(CL4ProfAgg);
	agg->event_name = event_name;
	return agg;
}

/** 
 * @brief Free an aggregate statistic. 
 * 
 * @param agg Aggregate statistic to free.
 * */
static void cl4_prof_agg_destroy(gpointer agg) {
	g_return_if_fail(agg != NULL);
	g_slice_free(CL4ProfAgg, agg);
}

/**
 * @brief Compares two aggregate event data instances for sorting 
 * within a GList. It is an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First aggregate event data instance to compare.
 * @param b Second aggregate event data instance to compare.
 * @param userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_agg_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
	
	/* Cast input parameters to event instant data structures. */
	CL4ProfAgg* ev_agg1 = (CL4ProfAgg*) a;
	CL4ProfAgg* ev_agg2 = (CL4ProfAgg*) b;
	CL4ProfSort sort = cl4_prof_get_sort(userdata);
	
	/* Perform comparison. */
	switch ((CL4ProfAggSort) sort.criteria) {

		/* Sort by event name. */
		case CL4_PROF_AGG_SORT_NAME:
			return CL4_PROF_CMP_STR(ev_agg1->event_name, 
				ev_agg2->event_name, sort.order);

		/* Sort by absolute time. */
		case CL4_PROF_AGG_SORT_TIME:
			return CL4_PROF_CMP_INT(ev_agg1->absolute_time, 
				ev_agg2->absolute_time, sort.order);

		/* We shouldn't get here. */
		default:
			g_return_val_if_reached(0);
	}
	
}

/**
 * @brief Create a new event profiling information object.
 * 
 * @param event_name Name of event.
 * @param cq_name Name of command queue which generated this event.
 * @param t_queued Device time in nanoseconds when the command 
 * identified by event is enqueued in a command-queue by the host.
 * @param t_submit Device time counter in nanoseconds when the command 
 * identified by event that has been enqueued is submitted by the host 
 * to the device associated with the command-queue. 
 * @param t_start Device time in nanoseconds when the command identified
 * by event starts execution on the device.
 * @param t_end Device time in nanoseconds when the command identified 
 * by event has finished execution on the device. 
 * @return A new event profiling information object.
 * */
static CL4ProfInfo* cl4_prof_info_new(const char* event_name, 
	const char* queue_name, cl_ulong t_queued, cl_ulong t_submit, 
	cl_ulong t_start, cl_ulong t_end) {

	CL4ProfInfo* info = g_slice_new(CL4ProfInfo);
	
	info->event_name = event_name;
	info->queue_name = queue_name;
	info->t_queued = t_queued;
	info->t_submit = t_submit;
	info->t_start = t_start;
	info->t_end = t_end;
	
	return info;
}

/** 
 * @brief Free an event profiling information object. 
 * 
 * @param info Event profiling information object to free.
 * */
static void cl4_prof_info_destroy(CL4ProfInfo* info) {
	g_return_if_fail(info != NULL);
	g_slice_free(CL4ProfInfo, info);	
}

/**
 * @brief Compares two event profiling information instances for sorting 
 * within a GList. It is an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First event profiling information instance to compare.
 * @param b Second event profiling information instance to compare.
 * @param userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_info_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
	
	/* Cast input parameters to event instant data structures. */
	CL4ProfInfo* ev1 = (CL4ProfInfo*) a;
	CL4ProfInfo* ev2 = (CL4ProfInfo*) b;
	CL4ProfSort sort = cl4_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CL4ProfInfoSort) sort.criteria) {
		
		 /* Sort aggregate event data instances by event name. */
		case CL4_PROF_INFO_SORT_NAME_EVENT:
			return CL4_PROF_CMP_STR(ev1->event_name, ev2->event_name,
				sort.order);
		
		 /* Sort aggregate event data instances by queue name. */
		case CL4_PROF_INFO_SORT_NAME_QUEUE:
			return CL4_PROF_CMP_STR(ev1->queue_name, ev2->queue_name,
				sort.order);
		
		 /* Sort aggregate event data instances by queued time. */
		case CL4_PROF_INFO_SORT_T_QUEUED:
			return CL4_PROF_CMP_INT(ev1->t_queued, ev2->t_queued, 
				sort.order);

		 /* Sort aggregate event data instances by submit time. */
		case CL4_PROF_INFO_SORT_T_SUBMIT:
			return CL4_PROF_CMP_INT(ev1->t_submit, ev2->t_submit,
				sort.order);
		
		 /* Sort aggregate event data instances by start time. */
		case CL4_PROF_INFO_SORT_T_START:
			return CL4_PROF_CMP_INT(ev1->t_start, ev2->t_start, 
				sort.order);
		
		 /* Sort aggregate event data instances by end time. */
		case CL4_PROF_INFO_SORT_T_END:
			return CL4_PROF_CMP_INT(ev1->t_end, ev2->t_end, sort.order);
		
		/* We shouldn't get here. */
		default:
			g_return_val_if_reached(0);
	}
	
}

/**
 * @brief Create a new event overlap object.
 * 
 * @param event1_name Name of first overlapping event.
 * @param event2_name Name of second overlapping event.
 * @param duration Overlap duration in nanoseconds.
 * @return A new event overlap object.
 * */
static CL4ProfOverlap* cl4_prof_overlap_new(const char* event1_name, 
	const char* event2_name, cl_ulong duration) {

	CL4ProfOverlap* ovlp = g_slice_new(CL4ProfOverlap);
	
	ovlp->event1_name = event1_name;
	ovlp->event2_name = event2_name;
	ovlp->duration = duration;
	
	return ovlp;

}

/**
 * @brief Destroy an event overlap object.
 * 
 * @param ovlp Event overlap object to destroy.
 * */
static void cl4_prof_overlap_destroy(CL4ProfOverlap* ovlp) {
	
	g_return_if_fail(ovlp != NULL);
	g_slice_free(CL4ProfOverlap, ovlp);	
	
}

/**
 * @brief Compares two event overlap instances for sorting within a 
 * GList. It is an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First event overlap instance to compare.
 * @param b Second event overlap instance to compare.
 * @param userdata Defines the sort criteria and order.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_overlap_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
		
	gint result;

	/* Cast input parameters to event instant data structures. */
	CL4ProfOverlap* ovlp1 = (CL4ProfOverlap*) a;
	CL4ProfOverlap* ovlp2 = (CL4ProfOverlap*) b;
	CL4ProfSort sort = cl4_prof_get_sort(userdata);
	/* Perform comparison. */
	switch ((CL4ProfOverlapSort) sort.criteria) {
		
		 /* Sort overlap instances by event name. */
		case CL4_PROF_OVERLAP_SORT_NAME:
			result = CL4_PROF_CMP_STR(ovlp1->event1_name, 
				ovlp2->event1_name, sort.order);
			if (result != 0)
				return result;
			else 
				return CL4_PROF_CMP_STR(ovlp1->event2_name, 
					ovlp2->event2_name, sort.order);
		
		 /* Sort overlap instances by overlap duration. */
		case CL4_PROF_OVERLAP_SORT_DURATION:
			return CL4_PROF_CMP_INT(ovlp1->duration, ovlp2->duration, 
				sort.order);
		
		/* We shouldn't get here. */
		default:
			g_return_val_if_reached(0);
	}
	
}

/**
 * @brief Add event for profiling.
 * 
 * @param prof Profile object.
 * @param cq_name Command queue name.
 * @param evt Event wrapper object.
 * */
static void cl4_prof_add_event(CL4Prof* prof, const char* cq_name, 
	CL4Event* evt, GError** err) {

	/* Event name ID. */
	guint* event_name_id;
	/* Specific event ID. */
	guint event_id;
	/* Event instants. */
	cl_ulong instant, instant_aux;
	/* Event instant objects. */
	CL4ProfInst* evinst_start;
	CL4ProfInst* evinst_end;
	
	/* Info object. */
	CL4WrapperInfo* info;
	
	/* Event name. */
	const char* event_name;
	
	/* Get event name. */
	event_name = cl4_event_get_final_name(evt);
	
	/* Update number of events, and get an ID for the given event. */
	event_id = ++prof->num_events;
	
	/* ***************************************************** */
	/* *** 1 - Check event against table of event names. * ***/
	/* ***************************************************** */
	
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
	
	/* ******************************************************* */
	/* *** 2 - Check event against list of event instants. *** */
	/* ******************************************************* */

	/* Get event start instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_START, err); 
	if (*err != NULL)
		return;
	instant = *((cl_ulong*) info->value);
	
	/* Check if start instant is the oldest instant. If so, keep it. */
	if (instant < prof->start_time)
		prof->start_time = instant;
		
	/* Add event start instant to list of event instants. */
	evinst_start = cl4_prof_inst_new(event_name, cq_name, event_id, 
		instant, CL4_PROF_INST_TYPE_START);
	prof->instants = g_list_prepend(
		prof->instants, (gpointer) evinst_start);

	/* Get event end instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_END, err); 
	if (*err != NULL)
		return;
	instant = *((cl_ulong*) info->value);

	/* Add event end instant to list of event instants. */
	evinst_end = cl4_prof_inst_new(event_name, cq_name, event_id, 
		instant, CL4_PROF_INST_TYPE_END);
	prof->instants = g_list_prepend(
		prof->instants, (gpointer) evinst_end);

	/* *********************************************** */
	/* *** 3 - Check event against list of events. *** */
	/* *********************************************** */
	
	/* Get event queued instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_QUEUED, err); 
	if (*err != NULL)
		return;
	instant = *((cl_ulong*) info->value);

	/* Get event submit instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_SUBMIT, err); 
	if (*err != NULL)
		return;
	instant_aux = *((cl_ulong*) info->value);

	/* Add event information to list of events.*/
	prof->infos = g_list_prepend(prof->infos, 
		(gpointer) cl4_prof_info_new(event_name, cq_name,
			instant, instant_aux, evinst_start->instant, 
			evinst_end->instant));

}

/**
 * @brief Process command queues, i.e., add the respective events for 
 * profiling.
 * 
 * @param prof Profile object.
 * @param err Return location for a GError, or NULL if error reporting
 * is to be ignored.
 */ 
static void cl4_prof_process_queues(CL4Prof* prof, GError** err) {

	/* Hash table iterator. */
	GHashTableIter iter;
	/* Command queue name and wrapper. */
	gpointer cq_name;
	gpointer cq;
	
	/* Iterate over the command queues. */
	g_hash_table_iter_init(&iter, prof->cqueues);
	while (g_hash_table_iter_next(&iter, &cq_name, &cq)) {
		
		/* Iterate over the events in current command queue. */
		CL4Event* evt;
		cl4_cqueue_iter_event_init((CL4CQueue*) cq);
		while ((evt = cl4_cqueue_iter_event_next((CL4CQueue*) cq))) {

			/* Add event for profiling. */
			cl4_prof_add_event(prof, (const char*) cq_name, evt, err);
			if (*err != NULL) return;

		}
		
	}
	
}

/**
 * @brief Determine aggregate event statistics.
 * 
 * @param prof The profile object.
 * */
static void cl4_prof_calc_agg(CL4Prof* prof) {
	
	/* Hash table iterator. */
	GHashTableIter iter;
	/* Aux. hash table for aggregate statistics. */
	GHashTable* agg_table;
	/* A pointer for a event name. */
	gpointer event_name;
	/* Aggregate event info. */
	CL4ProfAgg* evagg = NULL;
	/* Type of sorting to perform on event list. */
	int sort_type;
	/* Aux. pointer for event data structure kept in a GList. */
	GList* curr_evinst_container = NULL;
	/* A pointer to a CL4ProfAgg (agg. event info) variable. */
	gpointer value_agg;
	/* Auxiliary aggregate event info variable.*/
	CL4ProfAgg* curr_agg = NULL;

	/* Create table of aggregate statistics. */
	agg_table = g_hash_table_new(g_str_hash, g_str_equal);
	
	/* Initalize table, and set aggregate values to zero. */
	g_hash_table_iter_init(&iter, prof->event_names);
	while (g_hash_table_iter_next(&iter, &event_name, NULL)) {
		evagg = cl4_prof_agg_new(event_name);
		evagg->absolute_time = 0;
		g_hash_table_insert(
			agg_table, event_name, (gpointer) evagg);
	}
	
	/* Sort event instants by eid, and then by START, END order. */
	sort_type = CL4_PROF_INST_SORT_ID | CL4_PROF_SORT_ASC;
	prof->instants = g_list_sort_with_data(
		prof->instants, cl4_prof_inst_comp, 
		(gpointer) &sort_type);

	/* Iterate through all event instants and determine total times. */
	curr_evinst_container = prof->instants;
	while (curr_evinst_container) {
		
		/* Loop aux. variables. */
		CL4ProfInst* curr_evinst = NULL;
		cl_ulong start_inst, end_inst;
		
		/* Get START event instant. */
		curr_evinst = (CL4ProfInst*) curr_evinst_container->data;
		start_inst = curr_evinst->instant;
		
		/* Get END event instant */
		curr_evinst_container = curr_evinst_container->next;
		curr_evinst = (CL4ProfInst*) curr_evinst_container->data;
		end_inst = curr_evinst->instant;
		
		/* Add new interval to respective aggregate value. */
		curr_agg = (CL4ProfAgg*) g_hash_table_lookup(
			agg_table, curr_evinst->event_name);
		curr_agg->absolute_time += end_inst - start_inst;
		prof->total_events_time += end_inst - start_inst;
		
		/* Get next START event instant. */
		curr_evinst_container = curr_evinst_container->next;
	}
	
	/* Determine relative times. */
	g_hash_table_iter_init(&iter, agg_table);
	while (g_hash_table_iter_next(&iter, &event_name, &value_agg)) {
		curr_agg = (CL4ProfAgg*) value_agg;
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
 * @brief Determine event overlaps for the given profile object.
 * 
 * @param prof Profile object.
 */ 
static void cl4_prof_calc_overlaps(CL4Prof* prof) {
	
	/* Total overlap time. */
	cl_ulong total_overlap = 0;
	/* Overlap matrix. */
	cl_ulong* overlap_matrix = NULL;
	/* Number of event names. */
	guint num_event_names;
	/* Helper table to account for all overlapping events. */
	GHashTable* overlaps = NULL;
	/* Occurring events table. */
	GHashTable* occurring_events = NULL;
	/* Type of sorting to perform. */
	CL4ProfInstSort sort_type;
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
	sort_type = CL4_PROF_INST_SORT_INSTANT | CL4_PROF_SORT_ASC;
	prof->instants = g_list_sort_with_data(prof->instants, 
		cl4_prof_inst_comp, (gpointer) &sort_type);
		
	/* Iterate through all event instants */
	curr_evinst_container = prof->instants;
	while (curr_evinst_container) {
		
		/* ** Loop aux. variables. ** */
		
		/* Current event instant. */
		CL4ProfInst* curr_evinst = NULL;
		/* Inner hash table (is value for overlap hash table). */
		GHashTable* inner_table = NULL;
		/* Hash table iterator. */
		GHashTableIter iter;
		/* Hashtable key, event name ID for current event, event 
		 * name ID for occurring event. */
		gpointer key_eid, ueid_curr_ev, ueid_occu_ev;
		/* Keys for start and end event instants. */
		guint eid_key1, eid_key2;
		/* Event overlap in nanoseconds. */
		cl_ulong eff_overlap;
		
		/* Get current event instant. */
		curr_evinst = (CL4ProfInst*) curr_evinst_container->data;
		
		/* Check if event time is START or END time */
		if (curr_evinst->type == CL4_PROF_INST_TYPE_START) { 
			/* Event START instant. */

			/* 1 - Check for overlaps with ocurring events */

			g_hash_table_iter_init(&iter, occurring_events);
			while (g_hash_table_iter_next (&iter, &key_eid, NULL)) {

				/* The first hash table key will be the smaller event id. */
				eid_key1 = curr_evinst->id <= *((guint*) key_eid) 
					? curr_evinst->id 
					: *((guint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = curr_evinst->id > *((guint*) key_eid) 
					? curr_evinst->id 
					: *((guint*) key_eid);
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
				eid_key1 = curr_evinst->id <= *((guint*) key_eid) 
					? curr_evinst->id 
					: *((guint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = curr_evinst->id > *((guint*) key_eid) 
					? curr_evinst->id 
					: *((guint*) key_eid);
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
				guint ueid_min = 
					GPOINTER_TO_UINT(ueid_curr_ev) <= GPOINTER_TO_UINT(ueid_occu_ev) 
					? GPOINTER_TO_UINT(ueid_curr_ev) 
					: GPOINTER_TO_UINT(ueid_occu_ev);
				guint ueid_max = 
					GPOINTER_TO_UINT(ueid_curr_ev) > GPOINTER_TO_UINT(ueid_occu_ev) 
					? GPOINTER_TO_UINT(ueid_curr_ev) 
					: GPOINTER_TO_UINT(ueid_occu_ev);
				overlap_matrix[ueid_min * num_event_names + ueid_max] += eff_overlap;
				total_overlap += eff_overlap;
			}	
		}
		
		/* Get next event instant. */
		curr_evinst_container = curr_evinst_container->next;
	}
	
	/* Populate list of overlaps. */
	for (guint i = 0; i < num_event_names; i++) {
		for (guint j = 0; j < num_event_names; j++) {
			if (overlap_matrix[i * num_event_names + j] > 0) {
				/* There is an overlap here, create overlap object... */
				CL4ProfOverlap* ovlp = cl4_prof_overlap_new(
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
 * @addtogroup PROFILER
 * @{
 */
 
/** 
 * @brief Create a new profile object.
 * 
 * @return A new profile object. 
 */
CL4Prof* cl4_prof_new() {
	
	/* Allocate memory for new profile data structure. */
	CL4Prof* prof = g_slice_new0(CL4Prof);

	/* Set absolute start time to maximum possible. */
	prof->start_time = CL_ULONG_MAX;

	/* Return new profile data structure */
	return prof;

}

/** 
 * @brief Destroy a profile object.
 * 
 * @param prof Profile object to destroy. 
 */
void cl4_prof_destroy(CL4Prof* prof) {
	
	/* Profile to destroy cannot be NULL. */
	g_return_if_fail(prof != NULL);
	
	/* Destroy table of event names. */
	if (prof->event_names != NULL)
		g_hash_table_destroy(prof->event_names);
	
	/* Destroy table of event IDs. */
	if (prof->event_name_ids != NULL)
		g_hash_table_destroy(prof->event_name_ids);
		
	/* Destroy table of command queue wrappers. */
	if (prof->cqueues != NULL)
		g_hash_table_destroy(prof->cqueues);
	
	/* Destroy list of all event instants. */
	if (prof->instants != NULL)
		g_list_free_full(
			prof->instants, (GDestroyNotify) cl4_prof_inst_destroy);
			
	/* Destroy list of event profiling information. */
	if (prof->infos != NULL)
		g_list_free_full(
			prof->infos, (GDestroyNotify) cl4_prof_info_destroy);
	
	/* Destroy list of aggregate statistics. */
	if (prof->aggs != NULL)
		g_list_free_full(
			prof->aggs, (GDestroyNotify) cl4_prof_agg_destroy);
	
	/* Free the overlap matrix. */
	if (prof->overlaps != NULL)
		g_list_free_full(
			prof->overlaps, (GDestroyNotify) cl4_prof_overlap_destroy);
	
	/* Destroy timer. */
	if (prof->timer != NULL)
		g_timer_destroy(prof->timer);
	
	/* Destroy profile data structure. */
	g_slice_free(CL4Prof, prof);

}

/** 
* @brief Starts the global profiler timer. Only required if client
* wishes to compare the effectively ellapsed time with the OpenCL
* kernels time.
* 
* @param prof A profile object.
* */
void cl4_prof_start(CL4Prof* prof) {
	
	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);
	
	/* Start timer. */
	prof->timer = g_timer_new();
}

/** 
 * @brief Stops the global profiler timer. Only required if 
 * cl4_prof_start() was called.
 * 
 * @param prof A profile object. 
 * */
void cl4_prof_stop(CL4Prof* prof) {
	
	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);
	
	/* Stop timer. */
	g_timer_stop(prof->timer);
}

/** 
 * @brief If profiling has started but not stopped, returns the time 
 * since the profiling started. If profiling has been stopped, returns 
 * the elapsed time between the time it started and the time it stopped.
 * 
 * @param prof A profile object. 
 * @return number of seconds elapsed, including any fractional part.
 * */
double cl4_prof_time_elapsed(CL4Prof* prof) {
	
	/* Make sure profile is not NULL. */
	g_return_val_if_fail(prof != NULL, 0.0);
	
	/* Stop timer. */
	return g_timer_elapsed(prof->timer, NULL);
}

/** 
 * @brief Add a command queue wrapper for profiling. 
 * 
 * @param prof A profile object.
 * @param cq_name Command queue name.
 * @param cq Command queue wrapper object.
 * */
void cl4_prof_add_queue(
	CL4Prof* prof, const char* cq_name, CL4CQueue* cq) {
	
	/* Make sure profile is not NULL. */
	g_return_if_fail(prof != NULL);
	/* Make sure cq is not NULL. */
	g_return_if_fail(cq != NULL);
	/* Must be added before calculations. */
	g_return_if_fail(prof->calc == FALSE);
	
	/* Check if table needs to be created first. */
	if (prof->cqueues == NULL) {
		prof->cqueues = g_hash_table_new_full(
			g_str_hash, g_direct_equal, NULL, 
			(GDestroyNotify) cl4_cqueue_destroy);
	}
	/* Warn if table already contains a queue with the specified 
	 * name. */
	if (g_hash_table_contains(prof->cqueues, cq_name))
		g_warning("Profile object already contains a queue named '%s'." \
			"The existing queue will be replaced.", cq_name);
			
	/* Add queue to queue table. */
	g_hash_table_replace(prof->cqueues, (gpointer) cq_name, cq);
	
	/* Increment queue ref. count. */
	cl4_cqueue_ref(cq);

}

/** 
 * @brief Determine aggregate statistics for the given profile object. 
 * 
 * @param prof A profile object.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return CL_TRUE if function terminates successfully, or CL_FALSE
 * otherwise.
 * */
cl_bool cl4_prof_calc(CL4Prof* prof, GError** err) {
	
	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	/* Calculations can only be performed once. */
	g_return_val_if_fail(prof->calc == FALSE, CL_FALSE);

	/* Internal error handling object. */
	GError* err_internal = NULL;
	
	/* Function return status flag. */
	cl_bool status;
	
	/* Hash table iterator. */
	GHashTableIter iter;
	
	/* Auxiliary pointers for determining the table of event_ids. */
	gpointer p_evt_name, p_id;

	/* Create table of event names. */
	prof->event_names = g_hash_table_new(g_str_hash, g_str_equal);
	
	/* Process queues and respective events. */
	cl4_prof_process_queues(prof, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* Obtain the event_ids table (by reversing the event_names table) */
	prof->event_name_ids = g_hash_table_new(g_direct_hash, g_direct_equal);
	/* Populate table. */
	g_hash_table_iter_init(&iter, prof->event_names);
	while (g_hash_table_iter_next(&iter, &p_evt_name, &p_id)) {
		g_hash_table_insert(prof->event_name_ids, p_id, p_evt_name);
	}	

	/* Calculate aggregate statistics. */
	cl4_prof_calc_agg(prof);
	
	/* Determine event overlaps. */
	cl4_prof_calc_overlaps(prof);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = CL_TRUE;
	prof->calc = TRUE;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	status = CL_FALSE;

finish:	
	
	/* Return status. */
	return status;

}

/**
 * @brief Return aggregate statistics for events with the given name.
 * 
 * @param prof Profile object.
 * @param event_name Event name.
 * @return Aggregate statistics for events with the given name.
 */
const CL4ProfAgg const* cl4_prof_get_agg(
	CL4Prof* prof, const char* event_name) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);
	
	/* Find the aggregate statistic for the given event. */
	CL4ProfAgg* agg = NULL;
	GList* agg_container = prof->aggs;
	while (agg_container != NULL) {
		const char* curr_event_name = 
			((CL4ProfAgg*) agg_container->data)->event_name;
		if (g_strcmp0(event_name, curr_event_name) == 0) {
			agg = (CL4ProfAgg*) agg_container->data;
			break;
		}
		agg_container = agg_container->next;
	}
	
	/* Return result. */
	return agg;
}

/** 
 * @brief Initialize an iterator for profiled aggregate event 
 * instances. 
 * 
 * @param prof Profile object.
 * @param sort Bitfield of ::CL4ProfAggSort OR ::CL4ProfSortOrder, for
 * example `CL4_PROF_AGG_SORT_NAME | CL4_PROF_SORT_DESC`.
 * */
void cl4_prof_iter_agg_init(CL4Prof* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of aggregate statistics as requested by client. */
	prof->aggs = g_list_sort_with_data(
		prof->aggs, cl4_prof_agg_comp, &sort);
	
	/* Set the iterator as the first element in list. */
	prof->agg_iter = prof->aggs;

}

/** 
 * @brief Return the next aggregate statistic instance.
 * 
 * @param prof Profile object.
 * @return The next aggregate statistic instance. 
 * */
const CL4ProfAgg const* cl4_prof_iter_agg_next(CL4Prof* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);
	
	/* The aggregate statistic to return. */
	CL4ProfAgg* agg;
	
	/* Check if there are any more left. */
	if (prof->agg_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		agg = (CL4ProfAgg*) prof->agg_iter->data;
		prof->agg_iter = prof->agg_iter->next;
	} else {
		/* Nothing left. */
		agg = NULL;
	}
	
	/* Return the aggregate statistic. */
	return (const CL4ProfAgg const*) agg;
}

/** 
 * @brief Initialize an iterator for event profiling info instances. 
 * 
 * @param prof Profile object.
 * @param sort Bitfield of ::CL4ProfInfoSort OR ::CL4ProfSortOrder, for
 * example `CL4_PROF_INFO_SORT_T_START | CL4_PROF_SORT_ASC`.
 * */
void cl4_prof_iter_info_init(CL4Prof* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Sort list of event prof. infos as requested by client. */
	prof->infos = g_list_sort_with_data(
		prof->infos, cl4_prof_info_comp, &sort);
	
	/* Set the iterator as the first element in list. */
	prof->info_iter = prof->infos;
}

/** 
 * @brief Return the next event profiling info instance. 
 * 
 * @param prof Profile object.
 * @return The next event profiling info instance. 
 * */
const CL4ProfInfo const* cl4_prof_iter_info_next(CL4Prof* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);
	
	/* The event profiling info instance to return. */
	CL4ProfInfo* info;
	
	/* Check if there are any more left. */
	if (prof->info_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		info = (CL4ProfInfo*) prof->info_iter->data;
		prof->info_iter = prof->info_iter->next;
	} else {
		/* Nothing left. */
		info = NULL;
	}
	
	/* Return the profiling info instance. */
	return (const CL4ProfInfo const*) info;
}

/** 
 * @brief Initialize an iterator for event instant instances. 
 * 
 * @param prof Profile object.
 * @param sort Bitfield of ::CL4ProfInstSort OR ::CL4ProfSortOrder, for
 * example `CL4_PROF_INST_SORT_INSTANT | CL4_PROF_SORT_ASC`.
 * */
void cl4_prof_iter_inst_init(CL4Prof* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);
	
	/* Sort list of event instants as requested by client. */
	prof->instants = g_list_sort_with_data(
		prof->instants, cl4_prof_inst_comp, &sort);
	
	/* Set the iterator as the first element in list. */
	prof->inst_iter = prof->instants;

}

/** 
 * @brief Return the next event instant instance. 
 * 
 * @param prof Profile object.
 * @return The next event instant instance. 
 * */
const CL4ProfInst const* cl4_prof_iter_inst_next(CL4Prof* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);
	
	/* The event profiling info instance to return. */
	CL4ProfInst* inst;
	
	/* Check if there are any more left. */
	if (prof->inst_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		inst = (CL4ProfInst*) prof->inst_iter->data;
		prof->inst_iter = prof->inst_iter->next;
	} else {
		/* Nothing left. */
		inst = NULL;
	}
	
	/* Return the profiling info instance. */
	return (const CL4ProfInst const*) inst;
}

/** 
 * @brief Initialize an iterator for overlap instances. 
 * 
 * @param prof Profile object.
 * @param sort Bitfield of ::CL4ProfOverlapSort OR ::CL4ProfSortOrder, for
 * example `CL4_PROF_OVERLAP_SORT_DURATION | CL4_PROF_SORT_DESC`.
 * */
void cl4_prof_iter_overlap_init(CL4Prof* prof, int sort) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);
	
	/* Sort list of overlaps as requested by client. */
	prof->overlaps = g_list_sort_with_data(
		prof->overlaps, cl4_prof_overlap_comp, &sort);
	
	/* Set the iterator as the first element in list. */
	prof->overlap_iter = prof->overlaps;
}

/** 
 * @brief Return the next overlap instance. 
 * 
 * @param prof Profile object.
 * @return The next overlap instance. 
 * */
const CL4ProfOverlap const* cl4_prof_iter_overlap_next(CL4Prof* prof) {

	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, NULL);
	/* This function can only be called after calculations are made. */
	g_return_val_if_fail(prof->calc == TRUE, NULL);
	
	/* The overlap instance to return. */
	CL4ProfOverlap* ovlp;
	
	/* Check if there are any more left. */
	if (prof->overlap_iter != NULL) {
		/* Yes, send current one, pass to the next. */
		ovlp = (CL4ProfOverlap*) prof->overlap_iter->data;
		prof->overlap_iter = prof->overlap_iter->next;
	} else {
		/* Nothing left. */
		ovlp = NULL;
	}
	
	/* Return the overlap instance. */
	return (const CL4ProfOverlap const*) ovlp;
}

/**
 * @brief Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics (sorted
 * by absolute time), and a table of event overlaps (sorted by overlap
 * duration).
 * 
 * For more control of where and how this summary is printed, use the
 * cl4_prof_print_summary_full() function.
 * 
 * @param prof Profile object.
 * */ 
void cl4_prof_print_summary(CL4Prof* prof) {

	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Use the full version of this function with default parameters. */
	cl4_prof_print_summary_full(prof, stdout, 
		CL4_PROF_AGG_SORT_TIME | CL4_PROF_SORT_DESC,
		CL4_PROF_OVERLAP_SORT_DURATION | CL4_PROF_SORT_DESC);
	
}

/**
 * @brief Print a summary of the profiling info. More specifically,
 * this function prints a table of aggregate event statistics and a 
 * table of event overlaps. The stream where this information is written
 * to, as well as the order of the printed information, can be specified
 * in the function arguments.
 * 
 * @param prof Profile object.
 * @param stream Stream where to output summary.
 * @param agg_sort Sorting performed on aggregate statistics (bitfield
 * of ::CL4ProfAggSort OR ::CL4ProfSortOrder).
 * @param ovlp_sort Sorting performed on event overlaps (bitfield of 
 * ::CL4ProfOverlapSort OR ::CL4ProfSortOrder).
 * */ 
void cl4_prof_print_summary_full(CL4Prof* prof, FILE* stream, 
	int agg_sort, int ovlp_sort) {
	
	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);
	/* Make sure stream is not NULL. */
	g_return_if_fail(stream != NULL);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Current aggregate statistic to print. */
	const CL4ProfAgg const* agg = NULL;
	/* Current overlap to print. */
	const CL4ProfOverlap const* ovlp = NULL;

	g_fprintf(stream, "\n   =========================== Timming/Profiling ===========================\n\n");
	
	/* Show total ellapsed time */
	if (prof->timer) {
		g_fprintf(stream, 
			"     Total ellapsed time       : %fs\n", 
			g_timer_elapsed(prof->timer, NULL));
	}
	
	/* Show total events time */
	if (prof->total_events_time > 0) {
		g_fprintf(stream, 
			"     Total of all events       : %fs\n", 
			prof->total_events_time * 1e-9);
	}
	
	/* Show aggregate event times */
	g_fprintf(stream, "     Aggregate times by event  :\n");
	g_fprintf(stream, "       ------------------------------------------------------------------\n");
	g_fprintf(stream, "       | Event name                     | Rel. time (%%) | Abs. time (s) |\n");
	g_fprintf(stream, "       ------------------------------------------------------------------\n");
	cl4_prof_iter_agg_init(prof, agg_sort);
	while ((agg = cl4_prof_iter_agg_next(prof)) != NULL) {
		g_fprintf(stream, 
			"       | %-30.30s | %13.4f | %13.4e |\n", 
			agg->event_name, 
			agg->relative_time * 100.0, 
			agg->absolute_time * 1e-9);
	}
	g_fprintf(stream, "       ------------------------------------------------------------------\n");
	
	/* *** Show overlaps *** */

	if (g_list_length(prof->overlaps) > 0) {
		/* Show total events effective time (discount overlaps) */
		g_fprintf(stream, "     Tot. of all events (eff.) : %es\n", 
			prof->total_events_eff_time * 1e-9);
		g_fprintf(stream, "                                 %es saved with overlaps\n", 
			(prof->total_events_time - prof->total_events_eff_time) * 1e-9);
		/* Title the several overlaps. */
		g_fprintf(stream, "     Event overlap times       :\n");
		g_fprintf(stream, "       ------------------------------------------------------------------\n");
		g_fprintf(stream, "       | Event 1                | Event2                 | Overlap (s)  |\n");
		g_fprintf(stream, "       ------------------------------------------------------------------\n");
		/* Show overlaps table. */
		cl4_prof_iter_overlap_init(prof, ovlp_sort);
		while ((ovlp = cl4_prof_iter_overlap_next(prof)) != NULL) {
			g_fprintf(stream, "       | %-22.22s | %-22.22s | %12.4e |\n",
				ovlp->event1_name, ovlp->event2_name, ovlp->duration * 1e-9);
		}
		g_fprintf(stream, "       ------------------------------------------------------------------\n");
	} else {
		g_fprintf(stream, "     No overlapping events.\n");
	}

}

/** 
 * @brief Export event profiling information to a given stream.
 * 
 * Each line of the exported data will have the following format:
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
 * cl4_prof_get_export_opts() and cl4_prof_set_export_opts() functions, 
 * by manipulating a ::CL4ProfExportOptions struct.
 * 
 * @param prof Profile object.
 * @param stream Stream where export info to.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return CL_TRUE if function terminates successfully, CL_FALSE
 * otherwise.
 * */ 
cl_bool cl4_prof_export_info(CL4Prof* prof, FILE* stream, GError** err) {
	
	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure stream is not NULL. */
	g_return_val_if_fail(stream != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	/* This function can only be called after calculations are made. */
	g_return_if_fail(prof->calc == TRUE);

	/* Stream write status. */
	int write_status;
	/* Return status. */
	cl_bool ret_status;
	/* Current event information. */
	const CL4ProfInfo const* curr_ev;
	
	/* Sort event information by START order, ascending. */
	cl4_prof_iter_info_init(
		prof, CL4_PROF_INFO_SORT_T_START | CL4_PROF_SORT_ASC);
	
	/* Iterate through all event information and export it to stream. */
	while ((curr_ev = cl4_prof_iter_info_next(prof)) != NULL) {

		/* Write to stream. */
		write_status = g_fprintf(stream, "%s%s%s%s%lu%s%lu%s%s%s%s%s", 
			export_options.queue_delim, 
			curr_ev->queue_name, 
			export_options.queue_delim, 
			export_options.separator, 
			(unsigned long) curr_ev->t_start, 
			export_options.separator, 
			(unsigned long) curr_ev->t_end, 
			export_options.separator, 
			export_options.evname_delim, 
			curr_ev->event_name, 
			export_options.evname_delim,
			export_options.newline);
			
		gef_if_error_create_goto(*err, CL4_ERROR, write_status < 0, 
			ret_status = CL4_ERROR_STREAM_WRITE, error_handler, 
			"Error while exporting profiling information \
			(writing to stream).");

	}
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	ret_status = CL_TRUE;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	ret_status = CL_FALSE;
	
finish:	
	
	/* Return status. */
	return ret_status;
	
}

/** 
 * @brief Helper function which exports profiling info to a given file, 
 * automatically opening and closing the file. See the 
 * cl4_prof_export_info() for more information.
 * 
 * @param prof Profile object.
 * @param filename Name of file where information will be saved to.
 * @param err Return location for a GError, or NULL if error reporting 
 * is to be ignored.
 * @return CL_TRUE if function terminates successfully, CL_FALSE
 * otherwise.
 * */ 
cl_bool cl4_prof_export_info_file(
	CL4Prof* prof, const char* filename, GError** err) {

	/* Aux. var. */
	cl_bool status;
	
	/* Internal GError object. */
	GError* err_internal = NULL;
	
	/* Open file. */
	FILE* fp = fopen(filename, "w");
	gef_if_error_create_goto(*err, CL4_ERROR, fp == NULL, 
		status = CL4_ERROR_OPENFILE, error_handler, 
		"Unable to open file '%s' for exporting.", filename);
	
	/* Export data. */
	status = cl4_prof_export_info(prof, fp, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = CL_TRUE;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);
	status = CL_FALSE;

finish:	

	/* Close file. */
	if (fp) fclose(fp);

	/* Return file contents in string form. */
	return status;

}

/**
 * @brief Set export options using a ::CL4ProfExportOptions struct.
 * 
 * @param export_opts Export options to set.
 * */
void cl4_prof_set_export_opts(CL4ProfExportOptions export_opts) {
	G_LOCK(export_options);
	export_options = export_opts;
	G_UNLOCK(export_options);
}

/**
 * @brief Get current export options.
 * 
 * @return Current export options.
 * */
 CL4ProfExportOptions cl4_prof_get_export_opts() {
	return export_options;
}

/** @}*/
