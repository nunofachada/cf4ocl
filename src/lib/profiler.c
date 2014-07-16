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
	 * ::CL4ProfEvAggregate#event_name. */
	cl_ulong absolute_time;
	/** Relative time of events with name equal to 
	 * ::CL4ProfEvAggregate#event_name. */
	double relative_time;
	
} CL4ProfEvAggregate;

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

/* Default export options. */
static CL4ProfExportOptions export_options = {

	.separator = "\t",
	.newline = "\n",
	.queue_delim = "",
	.evname_delim = "",
	.simple_queue_id = TRUE,
	.zero_start = TRUE

};

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
static void cl4_prof_evinst_free(gpointer event_instant) {
	
	g_return_if_fail(event_instant != NULL);
	
	g_slice_free(CL4ProfEvInst, event_instant);
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
	/* Command queues ID. */
	guint* queue_id;
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
	profile->event_instants = g_list_prepend(
		profile->event_instants, (gpointer) evinst_start);

	/* Get event end instant. */
	info = cl4_event_get_profiling_info(
		evt, CL_PROFILING_COMMAND_END, err); 
	if (*err != NULL)
		return;
	instant = *((cl_ulong*) info->value);

	/* Add event end instant to list of event instants. */
	evinst_end = cl4_prof_evinst_new(event_name, cq_name, event_id, 
		instant, CL4_PROF_EV_END);
	profile->event_instants = g_list_prepend(
		profile->event_instants, (gpointer) evinst_end);

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
	GHashTableIter iter_qs, iter_evs;
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
		while (evt = cl4_cqueue_get_next_event((CL4CQueue*) cq, iter)) {

			/* Add event for profiling. */
			cl4_prof_add_event(prof, (CL4CQueue*) cq, 
				(const char*) cq_name, evt, err);
			if (*err != NULL) return;

		}
		
	}
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
		cl4_prof_aggregate_free);
	
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
	g_list_free_full(prof->event_instants, cl4_prof_evinst_free);
	
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
cl_bool cl4_prof_aggregate(CL4Prof* prof, GError** err) {
	
	/* Make sure prof is not NULL. */
	g_return_val_if_fail(prof != NULL, CL_FALSE);
	/* Make sure err is NULL or it is not set. */
	g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);
	
	/* Process queues and respective events. */
	cl4_prof_process_queues(CL4Prof* prof, GError** err);
	
	/* Return flag. */
	cl_bool status;
	/* Hash table iterator. */
	GHashTableIter iter;
	/* A pointer for a event name. */
	gpointer eventName;
	/* Aggregate event info. */
	CL4ProfEvAggregate* evagg = NULL;
	/* Type of sorting to perform on event list. */
	CL4ProfEvSort sortType;
	/* Aux. pointer for event data structure kept in a GList. */
	GList* currEvInstContainer = NULL;
	/* A pointer to a CL4ProfEvAggregate (agg. event info) variable. */
	gpointer valueAgg;
	/* Auxiliary aggregate event info variable.*/
	CL4ProfEvAggregate* currAgg = NULL;
	
	/* Initalize table, and set aggregate values to zero. */
	g_hash_table_iter_init(&iter, profile->unique_events);
	while (g_hash_table_iter_next(&iter, &eventName, NULL)) {
		evagg = cl4_prof_aggregate_new(eventName);
		evagg->absolute_time = 0;
		g_hash_table_insert(profile->aggregate, eventName, (gpointer) evagg);
	}
	
	/* Sort event instants by eid, and then by START, END order. */
	sortType = CL4_PROF_EV_SORT_ID;
	profile->event_instants = g_list_sort_with_data(
		profile->event_instants, cl4_prof_evinst_comp, (gpointer) &sortType);

	/* Iterate through all event instants and determine total times. */
	currEvInstContainer = profile->event_instants;
	while (currEvInstContainer) {
		
		/* Loop aux. variables. */
		CL4ProfEvInst* currEvInst = NULL;
		cl_ulong startInst, endInst;
		
		/* Get START event instant. */
		currEvInst = (CL4ProfEvInst*) currEvInstContainer->data;
		startInst = currEvInst->instant;
		
		/* Get END event instant */
		currEvInstContainer = currEvInstContainer->next;
		currEvInst = (CL4ProfEvInst*) currEvInstContainer->data;
		endInst = currEvInst->instant;
		
		/* Add new interval to respective aggregate value. */
		currAgg = (CL4ProfEvAggregate*) g_hash_table_lookup(
			profile->aggregate, currEvInst->event_name);
		currAgg->absolute_time += endInst - startInst;
		profile->total_events_time += endInst - startInst;
		
		/* Get next START event instant. */
		currEvInstContainer = currEvInstContainer->next;
	}
	
	/* Determine relative times. */
	g_hash_table_iter_init(&iter, profile->aggregate);
	while (g_hash_table_iter_next(&iter, &eventName, &valueAgg)) {
		currAgg = (CL4ProfEvAggregate*) valueAgg;
		currAgg->relative_time = 
			((double) currAgg->absolute_time) 
			/ 
			((double) profile->total_events_time);
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = CL4_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	
	/* Return status. */
	return status;		
	
}

//~ /**
 //~ * @brief Compares two event instants for sorting within a GList. It is
 //~ * an implementation of GCompareDataFunc from GLib.
 //~ * 
 //~ * @param a First event instant to compare.
 //~ * @param b Second event instant to compare.
 //~ * @param userdata Defines what type of sorting to do.
 //~ * @return Negative value if a < b; zero if a = b; positive value if a > b.
 //~ */
//~ gint cl4_prof_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata) {
	//~ /* Cast input parameters to event instant data structures. */
	//~ CL4ProfEvInst* evInst1 = (CL4ProfEvInst*) a;
	//~ CL4ProfEvInst* evInst2 = (CL4ProfEvInst*) b;
	//~ CL4ProfEvSort* sortType = (CL4ProfEvSort*) userdata;
	//~ /* Perform comparison. */
	//~ if (*sortType == CL4_PROF_EV_SORT_INSTANT) {
		//~ /* Sort by instant */
		//~ if (evInst1->instant > evInst2->instant) return  1;
		//~ if (evInst1->instant < evInst2->instant) return -1;
	//~ } else if (*sortType == CL4_PROF_EV_SORT_ID) {
		//~ /* Sort by ID */
		//~ if (evInst1->id > evInst2->id) return 1;
		//~ if (evInst1->id < evInst2->id) return -1;
		//~ if (evInst1->type == CL4_PROF_EV_END) return 1;
		//~ if (evInst1->type == CL4_PROF_EV_START) return -1;
	//~ }
	//~ return 0;
//~ }
//~ 
//~ /**
 //~ * @brief Determine overlap matrix for the given OpenCL events profile.
 //~ *         Must be called after cl4_prof_aggregate.
 //~ * 
 //~ * @param profile An OpenCL events profile.
 //~ * @param err Error structure, to be populated if an error occurs.
 //~ * @return @link cl4_error_codes::CL4_SUCCESS @endlink if function
 //~ * terminates successfully, or another value of #cl4_error_codes if 
 //~ * an error occurs.
 //~ */ 
//~ int cl4_prof_overmat(CL4Prof* profile, GError** err) {
	//~ 
	//~ /* Return status. */
	//~ int status;
	//~ /* Total overlap time. */
	//~ cl_ulong totalOverlap = 0;
	//~ /* Overlap matrix. */
	//~ cl_ulong* overlapMatrix = NULL;
	//~ /* Number of unique events. */
	//~ guint numUniqEvts;
	//~ /* Helper table to account for all overlapping events. */
	//~ GHashTable* overlaps = NULL;
	//~ /* Occurring events table. */
	//~ GHashTable* eventsOccurring = NULL;
	//~ /* Type of sorting to perform. */
	//~ CL4ProfEvSort sortType;
	//~ /* Container for current event instants. */
	//~ GList* currEvInstContainer;
	//~ 
	//~ /* Make sure profile is not NULL. */
	//~ g_assert(profile != NULL);
	//~ 
	//~ /* Determine number of unique events. */
	//~ numUniqEvts = g_hash_table_size(profile->unique_events);
	//~ 
	//~ /* Initialize overlap matrix. */
	//~ overlapMatrix = 
		//~ (cl_ulong*) malloc(numUniqEvts * numUniqEvts * sizeof(cl_ulong));
	//~ gef_if_error_create_goto(
		//~ *err, 
		//~ CL4_ERROR, 
		//~ overlapMatrix == NULL, 
		//~ status = CL4_ERROR_NOALLOC, 
		//~ error_handler, 
		//~ "Unable to allocate memory for overlapMatrix.");
	//~ for (guint i = 0; i < numUniqEvts * numUniqEvts; i++)
		//~ overlapMatrix[i] = 0;
		//~ 
	//~ /* Initialize helper table to account for all overlapping events. */
	//~ overlaps = g_hash_table_new_full(g_direct_hash, g_direct_equal, 
		//~ NULL, (GDestroyNotify) g_hash_table_destroy);
	//~ gef_if_error_create_goto(
		//~ *err, 
		//~ CL4_ERROR, 
		//~ overlaps == NULL, 
		//~ status = CL4_ERROR_NOALLOC, 
		//~ error_handler, 
		//~ "Unable to allocate memory for overlaps helper table: g_hash_table_new function returned NULL.");
	//~ 
	//~ /* Setup ocurring events table (key: eventID, value: uniqueEventID) */
	//~ eventsOccurring = g_hash_table_new(g_int_hash, g_int_equal);
	//~ gef_if_error_create_goto(
		//~ *err, CL4_ERROR, 
		//~ eventsOccurring == NULL, 
		//~ status = CL4_ERROR_NOALLOC, 
		//~ error_handler, 
		//~ "Unable to allocate memory for occurring events table: g_hash_table_new function returned NULL.");
		//~ 
	//~ /* Sort all event instants. */
	//~ sortType = CL4_PROF_EV_SORT_INSTANT;
	//~ profile->event_instants = g_list_sort_with_data(
		//~ profile->event_instants, 
		//~ cl4_prof_evinst_comp, 
		//~ (gpointer) &sortType);
	//~ 
	//~ /* Iterate through all event instants */
	//~ currEvInstContainer = profile->event_instants;
	//~ while (currEvInstContainer) {
		//~ 
		//~ /* Loop aux. variables. */
		//~ CL4ProfEvInst* currEvInst = NULL;            /* Current event instant. */
		//~ GHashTable* innerTable = NULL;                /* Inner hash table (is value for overlap hash table). */
		//~ GHashTableIter iter;                          /* Hash table iterator. */
		//~ gpointer key_eid, ueid_curr_ev, ueid_occu_ev; /* Hashtable key, unique event ID for current event, unique event ID for occurring event. */
		//~ guint eid_key1, eid_key2;                     /* Keys for start and end event instants. */
		//~ cl_ulong effOverlap;                          /* Event overlap in nanoseconds. */
		//~ 
		//~ /* Get current event instant. */
		//~ currEvInst = (CL4ProfEvInst*) currEvInstContainer->data;
		//~ 
		//~ /* Check if event time is START or END time */
		//~ if (currEvInst->type == CL4_PROF_EV_START) { 
			//~ /* Event START instant. */
			//~ 
			//~ /* 1 - Check for overlaps with ocurring events */
//~ 
			//~ g_hash_table_iter_init(&iter, eventsOccurring);
			//~ while (g_hash_table_iter_next (&iter, &key_eid, NULL)) {
//~ 
				//~ /* The first hash table key will be the smaller event id. */
				//~ eid_key1 = currEvInst->id <= *((guint*) key_eid) 
					//~ ? currEvInst->id 
					//~ : *((guint*) key_eid);
				//~ /* The second hash table key will be the larger event id. */
				//~ eid_key2 = currEvInst->id > *((guint*) key_eid) 
					//~ ? currEvInst->id 
					//~ : *((guint*) key_eid);
				//~ /* Check if the first key (smaller id) is already in the hash table... */
				//~ if (!g_hash_table_lookup_extended(overlaps, 
					//~ GUINT_TO_POINTER(eid_key1), NULL, 
					//~ (gpointer) &innerTable)) {
					//~ /* ...if not in table, add it to table, creating a new 
					 //~ * inner table as value. Inner table will be initalized 
					 //~ * with second key (larger id) as key and event start 
					 //~ * instant as value. */
					//~ innerTable = g_hash_table_new(
						//~ g_direct_hash, g_direct_equal);
					//~ gef_if_error_create_goto(
						//~ *err, 
						//~ CL4_ERROR, 
						//~ innerTable == NULL, 
						//~ status = CL4_ERROR_NOALLOC, 
						//~ error_handler, 
						//~ "Unable to allocate memory for events inner table: g_hash_table_new_full function returned NULL.");
					//~ g_hash_table_insert(
						//~ overlaps, GUINT_TO_POINTER(eid_key1), innerTable);
				//~ }
				//~ /* Add second key (larger id) to inner tabler, setting the 
				 //~ * start instant as the value. */
				//~ g_hash_table_insert(
					//~ innerTable, 
					//~ GUINT_TO_POINTER(eid_key2), 
					//~ &(currEvInst->instant));
			//~ }
//~ 
			//~ /* 2 - Add event to occurring events. */
			//~ g_hash_table_insert(
				//~ eventsOccurring, 
				//~ &(currEvInst->id), /* eid */
				//~ g_hash_table_lookup(profile->unique_events, currEvInst->event_name) /* ueid */
			//~ ); 
			//~ 
		//~ } else {
			//~ /* Event END instant. */
			//~ 
			//~ /* 1 - Remove event from ocurring events */
			//~ g_hash_table_remove(eventsOccurring, &(currEvInst->id));
			//~ 
			//~ /* 2 - Check for overlap termination with current events */
			//~ g_hash_table_iter_init(&iter, eventsOccurring);
			//~ while (g_hash_table_iter_next(&iter, &key_eid, &ueid_occu_ev)) {
				//~ /* The first hash table key will be the smaller event id. */
				//~ eid_key1 = currEvInst->id <= *((guint*) key_eid) 
					//~ ? currEvInst->id 
					//~ : *((guint*) key_eid);
				//~ /* The second hash table key will be the larger event id. */
				//~ eid_key2 = currEvInst->id > *((guint*) key_eid) 
					//~ ? currEvInst->id 
					//~ : *((guint*) key_eid);
				//~ /* Get effective overlap in nanoseconds. */
				//~ innerTable = g_hash_table_lookup(
					//~ overlaps, GUINT_TO_POINTER(eid_key1));
				//~ effOverlap = 
					//~ currEvInst->instant 
					//~ - 
					//~ *((cl_ulong*) g_hash_table_lookup(
						//~ innerTable, GUINT_TO_POINTER(eid_key2)));
				//~ /* Add overlap to overlap matrix. */
				//~ ueid_curr_ev = g_hash_table_lookup(
					//~ profile->unique_events, currEvInst->event_name);
				//~ guint ueid_min = GPOINTER_TO_UINT(ueid_curr_ev) <= GPOINTER_TO_UINT(ueid_occu_ev) 
					//~ ? GPOINTER_TO_UINT(ueid_curr_ev) 
					//~ : GPOINTER_TO_UINT(ueid_occu_ev);
				//~ guint ueid_max = GPOINTER_TO_UINT(ueid_curr_ev) > GPOINTER_TO_UINT(ueid_occu_ev) 
					//~ ? GPOINTER_TO_UINT(ueid_curr_ev) 
					//~ : GPOINTER_TO_UINT(ueid_occu_ev);
				//~ overlapMatrix[ueid_min * numUniqEvts + ueid_max] += effOverlap;
				//~ totalOverlap += effOverlap;
			//~ }	
		//~ }
		//~ 
		//~ /* Get next event instant. */
		//~ currEvInstContainer = currEvInstContainer->next;
	//~ }
//~ 
	//~ /* Add a pointer to overlap matrix to the profile. */
	//~ profile->overmat = overlapMatrix;
	//~ 
	//~ /* Determine and save effective events time. */
	//~ profile->total_events_eff_time = profile->total_events_time - totalOverlap;
	//~ 
	//~ /* If we got here, everything is OK. */
	//~ g_assert (err == NULL || *err == NULL);
	//~ status = CL4_SUCCESS;
	//~ goto finish;
	//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert (err == NULL || *err != NULL);
//~ 
//~ finish:	
	//~ 
	//~ /* Free overlaps hash table. */
	//~ if (overlaps) g_hash_table_destroy(overlaps);
	//~ 
	//~ /* Free eventsOccurring hash table. */
	//~ if (eventsOccurring) g_hash_table_destroy(eventsOccurring);
	//~ 
	//~ /* Return status. */
	//~ return status;	
//~ }
//~ 
//~ /** 
 //~ * @brief Create a new aggregate statistic for events of a given type.
 //~ * 
 //~ * @param eventName Name of event.
 //~ * @return New aggregate statistic or NULL if allocation unsuccessful.
 //~ * */
//~ CL4ProfEvAggregate* cl4_prof_aggregate_new(const char* eventName) {
	//~ CL4ProfEvAggregate* agg = (CL4ProfEvAggregate*) malloc(sizeof(CL4ProfEvAggregate));
	//~ if (agg != NULL) {
		//~ agg->event_name = eventName;
	//~ }
	//~ return agg;
//~ }
//~ 
//~ /** 
 //~ * @brief Free an aggregate statistic. 
 //~ * 
 //~ * @param agg Aggregate statistic to free.
 //~ * */
//~ void cl4_prof_aggregate_free(gpointer agg) {
	//~ free(agg);
//~ }
//~ 
//~ /**
 //~ * @brief Compares two aggregate event data instances for sorting 
 //~ * within a GList. It is an implementation of GCompareDataFunc from GLib.
 //~ * 
 //~ * @param a First aggregate event data instance to compare.
 //~ * @param b Second aggregate event data instance to compare.
 //~ * @param userdata Defines what type of sorting to do.
 //~ * @return Negative value if a < b; zero if a = b; positive value if a > b.
 //~ */
//~ gint cl4_prof_evagg_comp(gconstpointer a, gconstpointer b, gpointer userdata) {
	//~ /* Cast input parameters to event instant data structures. */
	//~ CL4ProfEvAggregate* evAgg1 = (CL4ProfEvAggregate*) a;
	//~ CL4ProfEvAggregate* evAgg2 = (CL4ProfEvAggregate*) b;
	//~ CL4ProfEvAggDataSort* sortType = (CL4ProfEvAggDataSort*) userdata;
	//~ /* Perform comparison. */
	//~ if (*sortType == CL4_PROF_AGGEVDATA_SORT_NAME) {
		//~ /* Sort by event name */
		//~ return strcmp(evAgg1->event_name, evAgg2->event_name);
	//~ } else if (*sortType == CL4_PROF_AGGEVDATA_SORT_TIME) {
		//~ /* Sort by event time period */
		//~ if (evAgg1->absolute_time < evAgg2->absolute_time) return 1;
		//~ if (evAgg1->absolute_time > evAgg2->absolute_time) return -1;
	//~ }
	//~ return 0;
//~ }
//~ 
//~ /**
 //~ * @brief Print profiling info.
 //~ * 
 //~ * @param profile An OpenCL events profile.
 //~ * @param evAggSortType Sorting strategy for aggregate event data instances.
 //~ * @param err Error structure, to be populated if an error occurs.
 //~ * @return @link cl4_error_codes::CL4_SUCCESS @endlink if function
 //~ * terminates successfully, or another value of #cl4_error_codes if 
 //~ * an error occurs.
 //~ * */ 
//~ int cl4_prof_print_info(CL4Prof* profile, CL4ProfEvAggDataSort evAggSortType, GError** err) {
	//~ 
	//~ /* Function return status. */
	//~ int status;
	//~ /* Auxiliary hash table. */
	//~ GHashTable* tmp = NULL;
	//~ /* Auxiliary hash table iterator. */
	//~ GHashTableIter iter;
	//~ /* Auxiliary pointers for data within hash tables and lists. */
	//~ gpointer pEventName, pId;
	//~ /* List of aggregate events. */
	//~ GList* evAggList = NULL;
	//~ /* List of aggregate events (traversing pointer). */
	//~ GList* evAggContainer = NULL;
	//~ /* Aggregate event info. */
	//~ CL4ProfEvAggregate* evAgg = NULL;
	//~ /* Number of unique event names. */
	//~ guint numUniqEvts;
	//~ /* String containing description of overlapping events. */
	//~ GString* overlapString = NULL;
	//~ 
	//~ /* Make sure profile is not NULL. */
	//~ g_assert(profile != NULL);
	//~ 
	//~ printf("\n   =========================== Timming/Profiling ===========================\n\n");
	//~ 
	//~ /* Show total ellapsed time */
	//~ if (profile->timer) {
		//~ printf(
			//~ "     Total ellapsed time       : %fs\n", 
			//~ g_timer_elapsed(profile->timer, NULL));
	//~ }
	//~ 
	//~ /* Show total events time */
	//~ if (profile->total_events_time > 0) {
		//~ printf(
			//~ "     Total of all events       : %fs\n", 
			//~ profile->total_events_time * 1e-9);
	//~ }
	//~ 
	//~ /* Show aggregate event times */
	//~ if (g_hash_table_size(profile->aggregate) > 0) {
		//~ printf("     Aggregate times by event  :\n");
		//~ evAggList = g_hash_table_get_values(profile->aggregate);
		//~ gef_if_error_create_goto(
			//~ *err, 
			//~ CL4_ERROR, 
			//~ evAggList == NULL, 
			//~ status = CL4_ERROR_NOALLOC, 
			//~ error_handler, 
			//~ "Unable to allocate memory for GList object 'evAggList'.");
		//~ evAggList = g_list_sort_with_data(
			//~ evAggList, cl4_prof_evagg_comp, &evAggSortType);
		//~ evAggContainer = evAggList;
		//~ printf("       ------------------------------------------------------------------\n");
		//~ printf("       | Event name                     | Rel. time (%%) | Abs. time (s) |\n");
		//~ printf("       ------------------------------------------------------------------\n");
		//~ while (evAggContainer) {
			//~ evAgg = (CL4ProfEvAggregate*) evAggContainer->data;
			//~ g_assert(evAgg != NULL);
			//~ printf(
				//~ "       | %-30.30s | %13.4f | %13.4e |\n", 
				//~ evAgg->event_name, 
				//~ evAgg->relative_time * 100.0, 
				//~ evAgg->absolute_time * 1e-9);
			//~ evAggContainer = evAggContainer->next;
		//~ }
		//~ printf("       ------------------------------------------------------------------\n");
	//~ }
	//~ 
	//~ /* Show overlaps */
	//~ if (profile->overmat) {
		//~ /* Create new temporary hash table which is the reverse in 
		 //~ * terms of key-values of profile->unique_events. */
		//~ tmp = g_hash_table_new(g_direct_hash, g_direct_equal);
		//~ gef_if_error_create_goto(
			//~ *err, 
			//~ CL4_ERROR, 
			//~ tmp == NULL, 
			//~ status = CL4_ERROR_NOALLOC, 
			//~ error_handler, 
			//~ "Unable to allocate memory for 'tmp' table: g_hash_table_new function returned NULL.");
		//~ /* Populate temporary hash table. */
		//~ g_hash_table_iter_init(&iter, profile->unique_events);
		//~ while (g_hash_table_iter_next(&iter, &pEventName, &pId)) {
			//~ g_hash_table_insert(tmp, pId, pEventName);
		//~ }
		//~ /* Get number of unique events. */
		//~ numUniqEvts = g_hash_table_size(profile->unique_events);
		//~ /* Show overlaps. */
		//~ overlapString = g_string_new("");
		//~ for (guint i = 0; i < numUniqEvts; i++) {
			//~ for (guint j = 0; j < numUniqEvts; j++) {
				//~ if (profile->overmat[i * numUniqEvts + j] > 0) {
					//~ g_string_append_printf(
						//~ overlapString,
						//~ "       | %-22.22s | %-22.22s | %12.4e |\n", 
						//~ (const char*) g_hash_table_lookup(
							//~ tmp, GUINT_TO_POINTER(i)),
						//~ (const char*) g_hash_table_lookup(
							//~ tmp, GUINT_TO_POINTER(j)),
						//~ profile->overmat[i * numUniqEvts + j] * 1e-9
					//~ );
				//~ }
			//~ }
		//~ }
		//~ if (strlen(overlapString->str) > 0) {
			//~ /* Show total events effective time (discount overlaps) */
			//~ printf("     Tot. of all events (eff.) : %es\n", 
				//~ profile->total_events_eff_time * 1e-9);
			//~ printf("                                 %es saved with overlaps\n", 
				//~ (profile->total_events_time - profile->total_events_eff_time) * 1e-9);
			//~ /* Title the several overlaps. */
			//~ printf("     Event overlap times       :\n");
			//~ printf("       ------------------------------------------------------------------\n");
			//~ printf("       | Event 1                | Event2                 | Overlap (s)  |\n");
			//~ printf("       ------------------------------------------------------------------\n");
			//~ /* Show overlaps table. */
			//~ printf("%s", overlapString->str);
			//~ printf("       ------------------------------------------------------------------\n");
		//~ }
	//~ }
	//~ 
	//~ /* If we got here, everything is OK. */
	//~ g_assert (err == NULL || *err == NULL);
	//~ status = CL4_SUCCESS;
	//~ goto finish;
	//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert (err == NULL || *err != NULL);
//~ 
//~ finish:	
	//~ 
	//~ /* Free stuff. */
	//~ if (evAggList) g_list_free(evAggList);
	//~ if (overlapString) g_string_free(overlapString, TRUE);
	//~ if (tmp) g_hash_table_destroy(tmp);
//~ 
	//~ /* Return status. */
	//~ return status;			
//~ 
//~ }
//~ 
//~ 
//~ /** 
 //~ * @brief Export profiling info to a given stream.
 //~ * 
 //~ * Each line of the exported data will have the following format:
 //~ * 
 //~ *     queue start-time end-time event-name
 //~ * 
 //~ * For example:
 //~ * 
 //~ *     0    100    120    load_data1
 //~ *     1    100    132    load_data2
 //~ *     0    121    159    process_data1
 //~ *     1    133    145    process_data2
 //~ *     0    146    157    read_result
 //~ * 
 //~ * Several export parameters can be configured with the 
 //~ * cl4_prof_export_opts_get() and cl4_prof_export_opts_set() functions, by
 //~ * manipulating a ::CL4ProfExportOptions struct.
 //~ * 
 //~ * @param profile An OpenCL events profile.
 //~ * @param stream Stream where export info to.
 //~ * @param err Error structure, to be populated if an error occurs.
 //~ * @return @link cl4_error_codes::CL4_SUCCESS @endlink if function
 //~ * terminates successfully, or another value of #cl4_error_codes if 
 //~ * an error occurs.
 //~ * */ 
//~ int cl4_prof_export_info(CL4Prof* profile, FILE* stream, GError** err) {
	//~ 
	//~ /* Return status. */
	//~ int ret_status, write_status;
	//~ /* Type of sorting to perform on event list. */
	//~ CL4ProfEvSort sortType;
	//~ /* List of event instants (traversing pointer). */
	//~ GList* evInstContainer = NULL;
	//~ 
	//~ /* Make sure profile is not NULL. */
	//~ g_assert(profile != NULL);
	//~ 
	//~ /* Sort event instants by eid, and then by START, END order. */
	//~ sortType = CL4_PROF_EV_SORT_ID;
	//~ profile->event_instants = g_list_sort_with_data(
		//~ profile->event_instants, cl4_prof_evinst_comp, (gpointer) &sortType);
	//~ 
	//~ /* Iterate through all event instants, determine complete event
	 //~ * information and export it to stream. */
	//~ evInstContainer = profile->event_instants;
	//~ while (evInstContainer) {
//~ 
		//~ /* Loop aux. variables. */
		//~ CL4ProfEvInst* currEvInst = NULL;
		//~ gulong q1, q2;
		//~ gulong qId;
		//~ cl_ulong startInst, endInst;
		//~ const char *ev1Name, *ev2Name;
		//~ 
		//~ /* Get information from start instant. */
		//~ currEvInst = (CL4ProfEvInst*) evInstContainer->data;
		//~ startInst = export_options.zero_start 
			//~ ? currEvInst->instant - profile->start_time 
			//~ : currEvInst->instant;
		//~ q1 = export_options.simple_queue_id 
			//~ ? (gulong) GPOINTER_TO_UINT(
				//~ g_hash_table_lookup(
					//~ profile->command_queues, currEvInst->queue)) 
			//~ : (gulong) currEvInst->queue;
		//~ ev1Name = currEvInst->event_name;
		//~ 
		//~ /* Get information from end instant. */
		//~ evInstContainer = evInstContainer->next;
		//~ currEvInst = (CL4ProfEvInst*) evInstContainer->data;
		//~ endInst = export_options.zero_start 
			//~ ? currEvInst->instant - profile->start_time 
			//~ : currEvInst->instant;
		//~ q2 = export_options.simple_queue_id 
			//~ ? (gulong) GPOINTER_TO_UINT(
				//~ g_hash_table_lookup(
					//~ profile->command_queues, currEvInst->queue)) 
			//~ : (gulong) currEvInst->queue;
		//~ ev2Name = currEvInst->event_name;
		//~ 
		//~ /* Make sure both event instants correspond to the same event. */
		//~ g_assert_cmpstr(ev1Name, ==, ev2Name);
		//~ /* Make sure start instant occurs before end instant. */
		//~ g_assert_cmpint(startInst, <, endInst);
		//~ 
		//~ /* Determine queue Id. */
		//~ qId = (q1 == q2) ? q1 : (q1 + q2);
		//~ 
		//~ /* Write to stream. */
		//~ write_status = fprintf(stream, "%s%lu%s%s%lu%s%lu%s%s%s%s%s", 
			//~ export_options.queue_delim, 
			//~ qId, 
			//~ export_options.queue_delim, 
			//~ export_options.separator, 
			//~ (unsigned long) startInst, 
			//~ export_options.separator, 
			//~ (unsigned long) endInst, 
			//~ export_options.separator, 
			//~ export_options.evname_delim, 
			//~ ev1Name, 
			//~ export_options.evname_delim,
			//~ export_options.newline);
			//~ 
		//~ gef_if_error_create_goto(
			//~ *err, 
			//~ CL4_ERROR, 
			//~ write_status < 0, 
			//~ ret_status = CL4_ERROR_STREAM_WRITE, 
			//~ error_handler, 
			//~ "Error while exporting profiling information (writing to stream).");
		//~ 
		//~ /* Get next START event instant. */
		//~ evInstContainer = evInstContainer->next;
		//~ 
	//~ }
	//~ 
	//~ /* If we got here, everything is OK. */
	//~ g_assert (err == NULL || *err == NULL);
	//~ ret_status = CL4_SUCCESS;
	//~ goto finish;
	//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert (err == NULL || *err != NULL);
//~ 
//~ finish:	
	//~ 
	//~ /* Return status. */
	//~ return ret_status;		
	//~ 
//~ }
//~ 
//~ /** 
 //~ * @brief Helper function which exports profiling info to a given file, 
 //~ * automatically opening and closing the file. Check the 
 //~ * cl4_prof_export_info() for more information.
 //~ * 
 //~ * @param profile An OpenCL events profile.
 //~ * @param filename Name of file where information will be saved to.
 //~ * @param err Error structure, to be populated if an error occurs.
 //~ * @return @link cl4_error_codes::CL4_SUCCESS @endlink if function
 //~ * terminates successfully, or another value of #cl4_error_codes if 
 //~ * an error occurs.
 //~ * */ 
//~ int cl4_prof_export_info_file(CL4Prof* profile, const char* filename, GError** err) {
//~ 
	//~ /* Aux. var. */
	//~ int status;
	//~ 
	//~ /* Internal GError object. */
	//~ GError* errInt = NULL;
	//~ 
	//~ /* Open file. */
	//~ FILE* fp = fopen(filename, "w");
	//~ gef_if_error_create_goto(
		//~ *err, 
		//~ CL4_ERROR, 
		//~ fp == NULL, 
		//~ status = CL4_ERROR_OPENFILE, 
		//~ error_handler, 
		//~ "Unable to open file '%s' for exporting.", 
		//~ filename);
	//~ 
	//~ /* Export data. */
	//~ status = cl4_prof_export_info(profile, fp, &errInt);
	//~ gef_if_error_propagate_goto(
		//~ err, errInt, GEF_USE_GERROR, status, error_handler);
	//~ 
	//~ /* If we got here, everything is OK. */
	//~ g_assert (errInt == NULL);
	//~ status = CL4_SUCCESS;
	//~ goto finish;
	//~ 
//~ error_handler:
	//~ /* If we got here there was an error, verify that it is so. */
	//~ g_assert (err == NULL || *err != NULL);
	//~ /* Free internal GError object. */
	//~ g_error_free(errInt);
//~ 
//~ finish:	
//~ 
	//~ /* Close file. */
	//~ if (fp) fclose(fp);
//~ 
	//~ /* Return file contents in string form. */
	//~ return status;
//~ 
//~ }
//~ 
//~ /**
 //~ * @brief Set export options using a ::CL4ProfExportOptions struct.
 //~ * 
 //~ * @param export_opts Export options to set.
 //~ * */
//~ void cl4_prof_export_opts_set(CL4ProfExportOptions export_opts) {
	//~ export_options = export_opts;
//~ }
//~ 
//~ /**
 //~ * @brief Get current export options.
 //~ * 
 //~ * @return Current export options.
 //~ * */
 //~ CL4ProfExportOptions cl4_prof_export_opts_get() {
	//~ return export_options;
//~ }
