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

/**
 * @brief Profile class, contains profiling information of OpenCL 
 * queues and events.
 */
struct cl4_prof {
	
	/** Hash table with keys equal to the events name, and values
	 * equal to a unique id for each event name. */
	GHashTable* event_names;
	
	/** Table of command queue wrappers. */
	GHashTable* cqueues;
	
	/** Instants (start and end) of all events occuring in an OpenCL
	 * application. */
	GList* event_instants;
	
	/** Total number of event instants in CL4Prof#event_instants. */
	guint num_event_instants;
	
	/** Aggregate statistics for all events in 
	 * CL4Prof#event_instants. */
	GHashTable* aggregate;
	
	/** Overlap matrix for all events in event_instants. */
	cl_ulong* overmat;
	
	/** Total time taken by all events. */
	cl_ulong total_events_time;
	
	/** Total time taken by all events except intervals where events overlaped. */
	cl_ulong total_events_eff_time;
	
	/** Time at which the first (oldest) event started. */
	cl_ulong start_time;
	
	/** Keeps track of time during the complete profiling session. */
	GTimer* timer;

};

/**
 * @brief Type of event instant (::CL4ProfEvInst).
 */
typedef enum {
	
	/** CL4ProfEvInst is a start event instant. */
	CL4_PROF_EV_START,
	/** CL4ProfEvInst is an end event instant. */
	CL4_PROF_EV_END
	
} CL4ProfEvInstType;

/**
 * @brief Event instant.
 */
typedef struct cl4_prof_evinst {

	 /** Name of event which the instant refers to. */
	const char* event_name;
	/** Name of command queue associated with event. */
	const char* queue_name;
	/** Event instant ID. */
	guint id;
	/** Event instant in nanoseconds from current device time counter. */
	cl_ulong instant;
	/** Type of event instant (::CL4ProfEvInstType#CL4_PROF_EV_START or 
	 * ::CL4ProfEvInstType#CL4_PROF_EV_END). */
	CL4ProfEvInstType type;

} CL4ProfEvInst;

/**
 * @brief Aggregate event info.
 */
typedef struct cl4_prof_ev_aggregate {

	/** Name of event which the instant refers to. */
	const char* event_name;
	/** Total (absolute) time of events with name equal to 
	 * ::CL4ProfEvAgg#event_name. */
	cl_ulong absolute_time;
	/** Relative time of events with name equal to 
	 * ::CL4ProfEvAgg#event_name. */
	double relative_time;
	
} CL4ProfEvAgg;

/**
 * @brief Sorting strategy for event instants (CL4ProfEvInst).
 */
typedef enum {
	
	/** Sort event instants by instant. */
	CL4_PROF_EV_SORT_INSTANT,
	/** Sort event instants by event id. */
	CL4_PROF_EV_SORT_ID

} CL4ProfEvSort;

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
 * @param type Type of event instant: CL4_PROF_EV_START or 
 * CL4_PROF_EV_END.
 * @return A new event instant.
 */
static CL4ProfEvInst* cl4_prof_evinst_new(const char* event_name,
	const char* queue_name, guint id, cl_ulong instant, 
	CL4ProfEvInstType type) {
	
	/* Allocate memory for event instant data structure. */
	CL4ProfEvInst* event_instant = g_slice_new(CL4ProfEvInst);

	/* Initialize structure fields. */
	event_instant->event_name = event_name;
	event_instant->queue_name = queue_name;
	event_instant->id = id;
	event_instant->instant = instant;
	event_instant->type = type;

	/* Return event instant data structure. */
	return event_instant;	
}

/**
 * @brief Free an event instant.
 * 
 * @param event_instant Event instant to destroy. 
 */
static void cl4_prof_evinst_destroy(gpointer event_instant) {
	
	g_return_if_fail(event_instant != NULL);
	
	g_slice_free(CL4ProfEvInst, event_instant);
}

/**
 * @brief Compares two event instants for sorting within a GList. It is
 * an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First event instant to compare.
 * @param b Second event instant to compare.
 * @param userdata Defines what type of sorting to do.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_evinst_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
	
	/* Cast input parameters to event instant data structures. */
	CL4ProfEvInst* ev_inst1 = (CL4ProfEvInst*) a;
	CL4ProfEvInst* ev_inst2 = (CL4ProfEvInst*) b;
	CL4ProfEvSort* sort_type = (CL4ProfEvSort*) userdata;
	/* Perform comparison. */
	if (*sort_type == CL4_PROF_EV_SORT_INSTANT) {
		/* Sort by instant */
		if (ev_inst1->instant > ev_inst2->instant) return  1;
		if (ev_inst1->instant < ev_inst2->instant) return -1;
	} else if (*sort_type == CL4_PROF_EV_SORT_ID) {
		/* Sort by ID */
		if (ev_inst1->id > ev_inst2->id) return 1;
		if (ev_inst1->id < ev_inst2->id) return -1;
		if (ev_inst1->type == CL4_PROF_EV_END) return 1;
		if (ev_inst1->type == CL4_PROF_EV_START) return -1;
	}
	return 0;
}

/**
 * @brief Compares two aggregate event data instances for sorting 
 * within a GList. It is an implementation of GCompareDataFunc from GLib.
 * 
 * @param a First aggregate event data instance to compare.
 * @param b Second aggregate event data instance to compare.
 * @param userdata Defines what type of sorting to do.
 * @return Negative value if a < b; zero if a = b; positive value if a > b.
 */
static gint cl4_prof_evagg_comp(
	gconstpointer a, gconstpointer b, gpointer userdata) {
	
	/* Cast input parameters to event instant data structures. */
	CL4ProfEvAgg* ev_agg1 = (CL4ProfEvAgg*) a;
	CL4ProfEvAgg* ev_agg2 = (CL4ProfEvAgg*) b;
	CL4ProfEvAggDataSort* sort_type = (CL4ProfEvAggDataSort*) userdata;
	/* Perform comparison. */
	if (*sort_type == CL4_PROF_AGGEVDATA_SORT_NAME) {
		/* Sort by event name */
		return strcmp(ev_agg1->event_name, ev_agg2->event_name);
	} else if (*sort_type == CL4_PROF_AGGEVDATA_SORT_TIME) {
		/* Sort by event time period */
		if (ev_agg1->absolute_time < ev_agg2->absolute_time) return 1;
		if (ev_agg1->absolute_time > ev_agg2->absolute_time) return -1;
	}
	return 0;
	
}

/** 
 * @brief Create a new aggregate statistic for events of a given type.
 * 
 * @param event_name Name of event.
 * @return New aggregate statistic.
 * */
static CL4ProfEvAgg* cl4_prof_agg_new(const char* event_name) {
	CL4ProfEvAgg* agg = g_slice_new(CL4ProfEvAgg);
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
	g_slice_free(CL4ProfEvAgg, agg);
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
	/* Event instant. */
	cl_ulong instant;
	/* Event instant objects. */
	CL4ProfEvInst* evinst_start;
	CL4ProfEvInst* evinst_end;
	
	/* Info object. */
	CL4WrapperInfo* info;
	
	/* Event name. */
	const char* event_name;
	
	/* Get event name. */
	event_name = cl4_event_get_final_name(evt);
	
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
	
	/* Update number of event instants, and get an ID for the given event. */
	event_id = ++prof->num_event_instants;
	
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
	evinst_start = cl4_prof_evinst_new(event_name, cq_name, event_id, 
		instant, CL4_PROF_EV_START);
	prof->event_instants = g_list_prepend(
		prof->event_instants, (gpointer) evinst_start);

	/* Get event end instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_END, err); 
	if (*err != NULL)
		return;
	instant = *((cl_ulong*) info->value);

	/* Add event end instant to list of event instants. */
	evinst_end = cl4_prof_evinst_new(event_name, cq_name, event_id, 
		instant, CL4_PROF_EV_END);
	prof->event_instants = g_list_prepend(
		prof->event_instants, (gpointer) evinst_end);

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
	GHashTableIter iter_qs;
	/* Command queue name and wrapper. */
	gpointer cq_name;
	gpointer cq;

	/* Iterate over the command queues. */
	g_hash_table_iter_init(&iter_qs, prof->cqueues);
	while (g_hash_table_iter_next(&iter_qs, &cq_name, &cq)) {
		
		/* Iterate over the events in current command queue. */
		CL4Event* evt;
		CL4Iterator iter = 
			cl4_cqueue_get_event_iterator((CL4CQueue*) cq);
		while ((evt = cl4_cqueue_get_next_event(iter))) {

			/* Add event for profiling. */
			cl4_prof_add_event(prof, (const char*) cq_name, evt, err);
			if (*err != NULL) return;

		}
		
	}
}

static void cl4_prof_calc_agg(CL4Prof* prof) {
	
	/* Hash table iterator. */
	GHashTableIter iter;
	/* A pointer for a event name. */
	gpointer event_name;
	/* Aggregate event info. */
	CL4ProfEvAgg* evagg = NULL;
	/* Type of sorting to perform on event list. */
	CL4ProfEvSort sort_type;
	/* Aux. pointer for event data structure kept in a GList. */
	GList* curr_evinst_container = NULL;
	/* A pointer to a CL4ProfEvAgg (agg. event info) variable. */
	gpointer value_agg;
	/* Auxiliary aggregate event info variable.*/
	CL4ProfEvAgg* curr_agg = NULL;
	
	/* Initalize table, and set aggregate values to zero. */
	g_hash_table_iter_init(&iter, prof->event_names);
	while (g_hash_table_iter_next(&iter, &event_name, NULL)) {
		evagg = cl4_prof_agg_new(event_name);
		evagg->absolute_time = 0;
		g_hash_table_insert(
			prof->aggregate, event_name, (gpointer) evagg);
	}
	
	/* Sort event instants by eid, and then by START, END order. */
	sort_type = CL4_PROF_EV_SORT_ID;
	prof->event_instants = g_list_sort_with_data(
		prof->event_instants, cl4_prof_evinst_comp, 
		(gpointer) &sort_type);

	/* Iterate through all event instants and determine total times. */
	curr_evinst_container = prof->event_instants;
	while (curr_evinst_container) {
		
		/* Loop aux. variables. */
		CL4ProfEvInst* curr_evinst = NULL;
		cl_ulong start_inst, end_inst;
		
		/* Get START event instant. */
		curr_evinst = (CL4ProfEvInst*) curr_evinst_container->data;
		start_inst = curr_evinst->instant;
		
		/* Get END event instant */
		curr_evinst_container = curr_evinst_container->next;
		curr_evinst = (CL4ProfEvInst*) curr_evinst_container->data;
		end_inst = curr_evinst->instant;
		
		/* Add new interval to respective aggregate value. */
		curr_agg = (CL4ProfEvAgg*) g_hash_table_lookup(
			prof->aggregate, curr_evinst->event_name);
		curr_agg->absolute_time += end_inst - start_inst;
		prof->total_events_time += end_inst - start_inst;
		
		/* Get next START event instant. */
		curr_evinst_container = curr_evinst_container->next;
	}
	
	/* Determine relative times. */
	g_hash_table_iter_init(&iter, prof->aggregate);
	while (g_hash_table_iter_next(&iter, &event_name, &value_agg)) {
		curr_agg = (CL4ProfEvAgg*) value_agg;
		curr_agg->relative_time = 
			((double) curr_agg->absolute_time) 
			/ 
			((double) prof->total_events_time);
	}

}

/**
 * @brief Determine overlap matrix for the given profile object.
 * 
 * @param profile An OpenCL events profile.
 */ 
static void cl4_prof_calc_overmat(CL4Prof* prof) {
	
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
	CL4ProfEvSort sort_type;
	/* Container for current event instants. */
	GList* curr_evinst_container;
	
	/* Determine number of event names. */
	num_event_names = g_hash_table_size(prof->event_names);
	
	/* Initialize overlap matrix. */
	overlap_matrix = g_new0(cl_ulong, num_event_names * num_event_names);
		
	/* Initialize helper table to account for all overlapping events. */
	overlaps = g_hash_table_new_full(g_direct_hash, g_direct_equal, 
		NULL, (GDestroyNotify) g_hash_table_destroy);
	
	/* Setup ocurring events table (key: eventID, value: uniqueEventID) */
	occurring_events = g_hash_table_new(g_int_hash, g_int_equal);
		
	/* Sort all event instants. */
	sort_type = CL4_PROF_EV_SORT_INSTANT;
	prof->event_instants = g_list_sort_with_data(prof->event_instants, 
		cl4_prof_evinst_comp, (gpointer) &sort_type);
	
	/* Iterate through all event instants */
	curr_evinst_container = prof->event_instants;
	while (curr_evinst_container) {
		
		/* ** Loop aux. variables. ** */
		
		/* Current event instant. */
		CL4ProfEvInst* curr_evinst = NULL;
		/* Inner hash table (is value for overlap hash table). */
		GHashTable* inner_table = NULL;
		/* Hash table iterator. */
		GHashTableIter iter;
		/* Hashtable key, unique event ID for current event, unique 
		 * event ID for occurring event. */
		gpointer key_eid, ueid_curr_ev, ueid_occu_ev;
		/* Keys for start and end event instants. */
		guint eid_key1, eid_key2;
		/* Event overlap in nanoseconds. */
		cl_ulong eff_overlap;
		
		/* Get current event instant. */
		curr_evinst = (CL4ProfEvInst*) curr_evinst_container->data;
		
		/* Check if event time is START or END time */
		if (curr_evinst->type == CL4_PROF_EV_START) { 
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

	/* Add a pointer to overlap matrix to the profile. */
	prof->overmat = overlap_matrix;
	
	/* Determine and save effective events time. */
	prof->total_events_eff_time = prof->total_events_time - total_overlap;
	
	/* Free overlaps hash table. */
	if (overlaps) g_hash_table_destroy(overlaps);
	
	/* Free occurring_events hash table. */
	if (occurring_events) g_hash_table_destroy(occurring_events);

}

/** 
 * @brief Create a new profile object.
 * 
 * @return A new profile object. 
 */
CL4Prof* cl4_prof_new() {
	
	/* Allocate memory for new profile data structure. */
	CL4Prof* prof = g_slice_new(CL4Prof);

	/* Create table of event names. */
	prof->event_names = g_hash_table_new(g_str_hash, g_str_equal);
		
	/* Create table of command queue wrappers. */
	prof->cqueues = g_hash_table_new_full(
		g_str_hash, g_direct_equal, NULL, 
		(GDestroyNotify) cl4_cqueue_destroy);
		
	/* Create list of all event instants... */
	prof->event_instants = NULL;
	
	/* ...and set number of event instants to zero. */
	prof->num_event_instants = 0;
	
	/* Create table of aggregate statistics. */
	prof->aggregate = g_hash_table_new_full(
		g_str_hash, 
		g_str_equal, 
		NULL, 
		cl4_prof_agg_destroy);
	
	/* Set overlap matrix to NULL. */
	prof->overmat = NULL;
	
	/* Set timer structure to NULL. */
	prof->timer = NULL;
	
	/* Set total times to 0. */
	prof->total_events_time = 0;
	prof->total_events_eff_time = 0;

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
	g_hash_table_destroy(prof->event_names);
	
	/* Destroy table of command queue wrappers. */
	g_hash_table_destroy(prof->cqueues);
	
	/* Destroy list of all event instants. */
	g_list_free_full(prof->event_instants, cl4_prof_evinst_destroy);
	
	/* Destroy table of aggregate statistics. */
	g_hash_table_destroy(prof->aggregate);
	
	/* Free the overlap matrix. */
	if (prof->overmat != NULL)
		g_free(prof->overmat);
	
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

	/* Internal error handling object. */
	GError* err_internal = NULL;
	
	/* Function return status flag. */
	cl_bool status;
	
	/* Process queues and respective events. */
	cl4_prof_process_queues(prof, &err_internal);
	gef_if_err_propagate_goto(err, err_internal, error_handler);

	/* Calculate aggregate statistics. */
	cl4_prof_calc_agg(prof);
	
	/* Calculate overlap matrix. */
	cl4_prof_calc_overmat(prof);
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = CL_TRUE;
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
 * @brief Print profiling info.
 * 
 * Client code can redirect the output using the g_set_print_handler()
 * function from GLib.
 * 
 * @param prof Profile object.
 * @param ev_aggSortType Sorting strategy for aggregate event data 
 * instances.
 * */ 
void cl4_prof_print_info(CL4Prof* prof,
	CL4ProfEvAggDataSort ev_aggSortType) {
	
	/* Make sure prof is not NULL. */
	g_return_if_fail(prof != NULL);

	/* Auxiliary hash table. */
	GHashTable* tmp = NULL;
	/* Auxiliary hash table iterator. */
	GHashTableIter iter;
	/* Auxiliary pointers for data within hash tables and lists. */
	gpointer p_evt_name, p_id;
	/* List of aggregate events. */
	GList* ev_agg_list = NULL;
	/* List of aggregate events (traversing pointer). */
	GList* ev_agg_container = NULL;
	/* Aggregate event info. */
	CL4ProfEvAgg* ev_agg = NULL;
	/* Number of unique event names. */
	guint num_event_names;
	/* String containing description of overlapping events. */
	GString* overlap_string = NULL;
	
	g_print("\n   =========================== Timming/Profiling ===========================\n\n");
	
	/* Show total ellapsed time */
	if (prof->timer) {
		g_print(
			"     Total ellapsed time       : %fs\n", 
			g_timer_elapsed(prof->timer, NULL));
	}
	
	/* Show total events time */
	if (prof->total_events_time > 0) {
		g_print(
			"     Total of all events       : %fs\n", 
			prof->total_events_time * 1e-9);
	}
	
	/* Show aggregate event times */
	if (g_hash_table_size(prof->aggregate) > 0) {
		g_print("     Aggregate times by event  :\n");
		ev_agg_list = g_hash_table_get_values(prof->aggregate);
		ev_agg_list = g_list_sort_with_data(
			ev_agg_list, cl4_prof_evagg_comp, &ev_aggSortType);
		ev_agg_container = ev_agg_list;
		g_print("       ------------------------------------------------------------------\n");
		g_print("       | Event name                     | Rel. time (%%) | Abs. time (s) |\n");
		g_print("       ------------------------------------------------------------------\n");
		while (ev_agg_container) {
			ev_agg = (CL4ProfEvAgg*) ev_agg_container->data;
			g_assert(ev_agg != NULL);
			g_print(
				"       | %-30.30s | %13.4f | %13.4e |\n", 
				ev_agg->event_name, 
				ev_agg->relative_time * 100.0, 
				ev_agg->absolute_time * 1e-9);
			ev_agg_container = ev_agg_container->next;
		}
		g_print("       ------------------------------------------------------------------\n");
	}
	
	/* Show overlaps */
	if (prof->overmat) {
		/* Create new temporary hash table which is the reverse in 
		 * terms of key-values of prof->event_names. */
		tmp = g_hash_table_new(g_direct_hash, g_direct_equal);
		/* Populate temporary hash table. */
		g_hash_table_iter_init(&iter, prof->event_names);
		while (g_hash_table_iter_next(&iter, &p_evt_name, &p_id)) {
			g_hash_table_insert(tmp, p_id, p_evt_name);
		}
		/* Get number of unique events. */
		num_event_names = g_hash_table_size(prof->event_names);
		/* Show overlaps. */
		overlap_string = g_string_new("");
		for (guint i = 0; i < num_event_names; i++) {
			for (guint j = 0; j < num_event_names; j++) {
				if (prof->overmat[i * num_event_names + j] > 0) {
					g_string_append_printf(
						overlap_string,
						"       | %-22.22s | %-22.22s | %12.4e |\n", 
						(const char*) g_hash_table_lookup(
							tmp, GUINT_TO_POINTER(i)),
						(const char*) g_hash_table_lookup(
							tmp, GUINT_TO_POINTER(j)),
						prof->overmat[i * num_event_names + j] * 1e-9
					);
				}
			}
		}
		if (strlen(overlap_string->str) > 0) {
			/* Show total events effective time (discount overlaps) */
			g_print("     Tot. of all events (eff.) : %es\n", 
				prof->total_events_eff_time * 1e-9);
			g_print("                                 %es saved with overlaps\n", 
				(prof->total_events_time - prof->total_events_eff_time) * 1e-9);
			/* Title the several overlaps. */
			g_print("     Event overlap times       :\n");
			g_print("       ------------------------------------------------------------------\n");
			g_print("       | Event 1                | Event2                 | Overlap (s)  |\n");
			g_print("       ------------------------------------------------------------------\n");
			/* Show overlaps table. */
			g_print("%s", overlap_string->str);
			g_print("       ------------------------------------------------------------------\n");
		}
	}
	
	/* Free stuff. */
	if (ev_agg_list) g_list_free(ev_agg_list);
	if (overlap_string) g_string_free(overlap_string, TRUE);
	if (tmp) g_hash_table_destroy(tmp);

}

/** 
 * @brief Export profiling info to a given stream.
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
 * cl4_prof_export_opts_get() and cl4_prof_export_opts_set() functions, by
 * manipulating a ::CL4ProfExportOptions struct.
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

	/* Return status. */
	int ret_status, write_status;
	/* Type of sorting to perform on event list. */
	CL4ProfEvSort sort_type;
	/* List of event instants (traversing pointer). */
	GList* ev_inst_container = NULL;
	
	/* Sort event instants by eid, and then by START, END order. */
	sort_type = CL4_PROF_EV_SORT_ID;
	prof->event_instants = g_list_sort_with_data(prof->event_instants, 
		cl4_prof_evinst_comp, (gpointer) &sort_type);
	
	/* Iterate through all event instants, determine complete event
	 * information and export it to stream. */
	ev_inst_container = prof->event_instants;
	while (ev_inst_container) {

		/* Loop aux. variables. */
		CL4ProfEvInst* curr_evinst = NULL;
		const char* cq_name;
		cl_ulong start_inst, end_inst;
		const char *ev1_name, *ev2_name;
		
		/* Get information from start instant. */
		curr_evinst = (CL4ProfEvInst*) ev_inst_container->data;
		start_inst = export_options.zero_start 
			? curr_evinst->instant - prof->start_time 
			: curr_evinst->instant;
		cq_name = curr_evinst->queue_name;
		ev1_name = curr_evinst->event_name;
		
		/* Get information from end instant. */
		ev_inst_container = ev_inst_container->next;
		curr_evinst = (CL4ProfEvInst*) ev_inst_container->data;
		end_inst = export_options.zero_start 
			? curr_evinst->instant - prof->start_time 
			: curr_evinst->instant;
		ev2_name = curr_evinst->event_name;
		
		/* Make sure both event instants correspond to the same event. */
		g_assert_cmpstr(ev1_name, ==, ev2_name);
		/* Make sure start instant occurs before end instant. */
		g_assert_cmpint(start_inst, <, end_inst);
		
		/* Write to stream. */
		write_status = fprintf(stream, "%s%s%s%s%lu%s%lu%s%s%s%s%s", 
			export_options.queue_delim, 
			cq_name, 
			export_options.queue_delim, 
			export_options.separator, 
			(unsigned long) start_inst, 
			export_options.separator, 
			(unsigned long) end_inst, 
			export_options.separator, 
			export_options.evname_delim, 
			ev1_name, 
			export_options.evname_delim,
			export_options.newline);
			
		gef_if_error_create_goto(
			*err, 
			CL4_ERROR, 
			write_status < 0, 
			ret_status = CL4_ERROR_STREAM_WRITE, 
			error_handler, 
			"Error while exporting profiling information (writing to stream).");
		
		/* Get next START event instant. */
		ev_inst_container = ev_inst_container->next;
		
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
void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts) {
	G_LOCK(export_options);
	export_options = export_opts;
	G_UNLOCK(export_options);
}

/**
 * @brief Get current export options.
 * 
 * @return Current export options.
 * */
 CL4ProfExportOptions cl4_prof_export_opts_get() {
	return export_options;
}
