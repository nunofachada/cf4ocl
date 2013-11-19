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
 * @date 2013
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "clprofiler.h"

/** 
 * @brief Create a new OpenCL events profile.
 * 
 * @return A new profile or NULL if operation failed. 
 */
ProfCLProfile* profcl_profile_new() {
	
	/* Allocate memory for new profile data structure. */
	ProfCLProfile* profile = (ProfCLProfile*) malloc(sizeof(ProfCLProfile));

	/* If allocation successful... */
	if (profile != NULL) {
		/* ... create table of unique events, ... */
		profile->unique_events = g_hash_table_new(g_str_hash, g_str_equal);
		/* ... create list of all event instants, ... */
		profile->event_instants = NULL;
		/* ... and set number of event instants to zero. */
		profile->num_event_instants = 0;
		/* ... and create table of aggregate statistics. */
		profile->aggregate = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, profcl_aggregate_free);
		/* ... and set overlap matrix to NULL. */
		profile->overmat = NULL;
		/* ... and set timer structure to NULL. */
		profile->timer = NULL;
		/* ... and set total times to 0. */
		profile->totalEventsTime = 0;
		profile->totalEventsEffTime = 0;
	}

	/* Return new profile data structure */
	return profile;
}


/** 
 * @brief Free an OpenCL events profile.
 * 
 * @param profile OpenCL events profile to destroy. 
 */
void profcl_profile_free(ProfCLProfile* profile) {
	/* Profile to free cannot be NULL. */
	g_assert(profile != NULL);
	/* Destroy table of unique events. */
	g_hash_table_destroy(profile->unique_events);
	/* Destroy list of all event instants. */
	g_list_free_full(profile->event_instants, profcl_evinst_free);
	/* Destroy table of aggregate statistics. */
	g_hash_table_destroy(profile->aggregate);
	/* Free the overlap matrix. */
	if (profile->overmat != NULL)
		free(profile->overmat);
	/* Destroy timer. */
	if (profile->timer != NULL)
		g_timer_destroy(profile->timer);
	/* Destroy profile data structure. */
	free(profile);
}

/**
 * @brief Add OpenCL event to events profile, more specifically adds 
 * the start and end instants of the given event to the profile.
 * 
 * @param profile OpenCL events profile.
 * @param event_name Event name.
 * @param ev Event to get information of.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link profcl_error_codes::PROFCL_SUCCESS @endlink if event 
 * successfully added, or another value of #profcl_error_codes if an 
 * error occurs.
 */ 
int profcl_profile_add(ProfCLProfile* profile, const char* event_name, cl_event ev, GError** err) {
	/* Just call profcl_profile_add_composite sending the same event as start and end event. */
	return profcl_profile_add_composite(profile, event_name, ev, ev, err);
}


/**
 * @brief Add OpenCL events to events profile, more specifically adds 
 * the start of ev1 and end of ev2 to the profile.
 * 
 * @param profile OpenCL events profile.
 * @param event_name Event name.
 * @param ev1 Event to get start information of.
 * @param ev2 Event to get end information of.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link profcl_error_codes::PROFCL_SUCCESS @endlink if event 
 * successfully added, or another value of #profcl_error_codes if an 
 * error occurs.
 */ 
int profcl_profile_add_composite(ProfCLProfile* profile, const char* event_name, cl_event ev1, cl_event ev2, GError** err) {
	
	/* OpenCL return status. */
	cl_int ocl_status;
	/* Function return status. */
	int ret_status; 
	/* Unique event ID. */
	guint* unique_event_id;
	/* Specific event ID. */
	guint event_id;
	/* Event instant. */
	cl_ulong instant;
	/* Event instant objects. */
	ProfCLEvInst* evinst_start;
	ProfCLEvInst* evinst_end;
	
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	
	/* Check if event is already registered in the unique events table... */
	if (!g_hash_table_contains(profile->unique_events, event_name)) {
		/* ...if not, register it. */
		unique_event_id = GUINT_TO_POINTER(g_hash_table_size(profile->unique_events));
		g_hash_table_insert(profile->unique_events, (gpointer) event_name, (gpointer) unique_event_id);
	}
	
	/* Update number of event instants, and get an ID for the given event. */
	event_id = ++profile->num_event_instants;
	
	/* Get event start instant. */
	ocl_status = clGetEventProfilingInfo(ev1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &instant, NULL);
	gef_if_error_create_goto(*err, PROFCL_ERROR, CL_SUCCESS != ocl_status, ret_status = PROFCL_OCL_ERROR, error_handler, "Get event start instant: OpenCL error %d.", ocl_status);
		
	/* Add event start instant to list of event instants. */
	evinst_start = profcl_evinst_new(event_name, event_id, instant, PROFCL_EV_START);
	gef_if_error_create_goto(*err, PROFCL_ERROR, evinst_start == NULL, ret_status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for ProfCLEvInst object for start of event '%s' with ID %d.", event_name, event_id);
	profile->event_instants = g_list_prepend(profile->event_instants, (gpointer) evinst_start);

	/* Get event end instant. */
	ocl_status = clGetEventProfilingInfo(ev2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &instant, NULL);
	gef_if_error_create_goto(*err, PROFCL_ERROR, CL_SUCCESS != ocl_status, ret_status = PROFCL_OCL_ERROR, error_handler, "Get event end instant: OpenCL error %d.", ocl_status)

	/* Add event end instant to list of event instants. */
	evinst_end = profcl_evinst_new(event_name, event_id, instant, PROFCL_EV_END);
	gef_if_error_create_goto(*err, PROFCL_ERROR, evinst_end == NULL, ret_status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for ProfCLEvInst object for end of event '%s' with ID %d.", event_name, event_id);
	profile->event_instants = g_list_prepend(profile->event_instants, (gpointer) evinst_end);

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	ret_status = PROFCL_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	/* Return status. */
	return ret_status;
	
}

/** 
 * @brief Create new event instant.
 * 
 * @param eventName Name of event.
 * @param id Id of event.
 * @param instant Even instant in nanoseconds.
 * @param type Type of event instant: PROFCL_EV_START or PROFCL_EV_END
 * @return A new event instant or NULL if operation failed.
 */
ProfCLEvInst* profcl_evinst_new(const char* eventName, guint id, cl_ulong instant, ProfCLEvInstType type) {
	
	/* Allocate memory for event instant data structure. */
	ProfCLEvInst* event_instant = (ProfCLEvInst*) malloc(sizeof(ProfCLEvInst));
	/* If allocation successful... */
	if (event_instant != NULL) {
		/* ...initialize structure fields. */
		event_instant->eventName = eventName;
		event_instant->id = id;
		event_instant->instant = instant;
		event_instant->type = type;
	}
	/* Return event instant data structure. */
	return event_instant;	
}


/**
 * @brief Free an event instant.
 * 
 * @param event_instant Event instant to destroy. 
 */
void profcl_evinst_free(gpointer event_instant) {
	free(event_instant);
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
gint profcl_evinst_comp(gconstpointer a, gconstpointer b, gpointer userdata) {
	/* Cast input parameters to event instant data structures. */
	ProfCLEvInst* evInst1 = (ProfCLEvInst*) a;
	ProfCLEvInst* evInst2 = (ProfCLEvInst*) b;
	ProfCLEvSort* sortType = (ProfCLEvSort*) userdata;
	/* Perform comparison. */
	if (*sortType == PROFCL_EV_SORT_INSTANT) {
		/* Sort by instant */
		if (evInst1->instant > evInst2->instant) return  1;
		if (evInst1->instant < evInst2->instant) return -1;
	} else if (*sortType == PROFCL_EV_SORT_ID) {
		/* Sort by ID */
		if (evInst1->id > evInst2->id) return 1;
		if (evInst1->id < evInst2->id) return -1;
		if (evInst1->type == PROFCL_EV_END) return 1;
		if (evInst1->type == PROFCL_EV_START) return -1;
	}
	return 0;
}


/**
 * @brief Determine overlap matrix for the given OpenCL events profile.
 *         Must be called after profcl_profile_aggregate.
 * 
 * @param profile An OpenCL events profile.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link profcl_error_codes::PROFCL_SUCCESS @endlink if function
 * terminates successfully, or another value of #profcl_error_codes if 
 * an error occurs.
 */ 
int profcl_profile_overmat(ProfCLProfile* profile, GError** err) {
	
	/* Return status. */
	int status;
	/* Total overlap time. */
	cl_ulong totalOverlap = 0;
	/* Overlap matrix. */
	cl_ulong* overlapMatrix = NULL;
	/* Number of unique events. */
	guint numUniqEvts;
	/* Helper table to account for all overlapping events. */
	GHashTable* overlaps = NULL;
	/* Occurring events table. */
	GHashTable* eventsOccurring = NULL;
	/* Type of sorting to perform. */
	ProfCLEvSort sortType;
	/* Container for current event instants. */
	GList* currEvInstContainer;
	
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	
	/* Determine number of unique events. */
	numUniqEvts = g_hash_table_size(profile->unique_events);
	
	/* Initialize overlap matrix. */
	overlapMatrix = (cl_ulong*) malloc(numUniqEvts * numUniqEvts * sizeof(cl_ulong));
	gef_if_error_create_goto(*err, PROFCL_ERROR, overlapMatrix == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for overlapMatrix.");
	for (guint i = 0; i < numUniqEvts * numUniqEvts; i++)
		overlapMatrix[i] = 0;
		
	/* Initialize helper table to account for all overlapping events. */
	overlaps = g_hash_table_new(g_direct_hash, g_direct_equal);
	gef_if_error_create_goto(*err, PROFCL_ERROR, overlaps == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for overlaps helper table: g_hash_table_new function returned NULL.");
	
	/* Setup ocurring events table (key: eventID, value: uniqueEventID) */
	eventsOccurring = g_hash_table_new(g_int_hash, g_int_equal);
	gef_if_error_create_goto(*err, PROFCL_ERROR, eventsOccurring == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for occurring events table: g_hash_table_new function returned NULL.");
		
	/* Sort all event instants. */
	sortType = PROFCL_EV_SORT_INSTANT;
	profile->event_instants = g_list_sort_with_data(profile->event_instants, profcl_evinst_comp, (gpointer) &sortType);
	
	/* Iterate through all event instants */
	currEvInstContainer = profile->event_instants;
	while (currEvInstContainer) {
		
		/* Loop aux. variables. */
		ProfCLEvInst* currEvInst = NULL;            /* Current event instant. */
		GHashTable* innerTable = NULL;                /* Inner hash table (is value for overlap hash table). */
		GHashTableIter iter;                          /* Hash table iterator. */
		gpointer key_eid, ueid_curr_ev, ueid_occu_ev; /* Hashtable key, unique event ID for current event, unique event ID for occurring event. */
		guint eid_key1, eid_key2;                     /* Keys for start and end event instants. */
		cl_ulong effOverlap;                          /* Event overlap in nanoseconds. */
		
		/* Get current event instant. */
		currEvInst = (ProfCLEvInst*) currEvInstContainer->data;
		
		/* Check if event time is START or END time */
		if (currEvInst->type == PROFCL_EV_START) { 
			/* Event START instant. */
			
			/* 1 - Check for overlaps with ocurring events */

			g_hash_table_iter_init(&iter, eventsOccurring);
			while (g_hash_table_iter_next (&iter, &key_eid, NULL)) {

				/* The first hash table key will be the smaller event id. */
				eid_key1 = currEvInst->id <= *((guint*) key_eid) ? currEvInst->id : *((guint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = currEvInst->id > *((guint*) key_eid) ? currEvInst->id : *((guint*) key_eid);
				/* Check if the first key (smaller id) is already in the hash table... */
				if (!g_hash_table_lookup_extended(overlaps, GUINT_TO_POINTER(eid_key1), NULL, (gpointer) &innerTable)) {
					/* ...if not in table, add it to table, creating a new 
					 * inner table as value. Inner table will be initalized 
					 * with second key (larger id) as key and event start 
					 * instant as value. */
					innerTable = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify) g_hash_table_destroy);
					gef_if_error_create_goto(*err, PROFCL_ERROR, innerTable == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for events inner table: g_hash_table_new_full function returned NULL.");
					g_hash_table_insert(overlaps, GUINT_TO_POINTER(eid_key1), innerTable);
				}
				/* Add second key (larger id) to inner tabler, setting the 
				 * start instant as the value. */
				g_hash_table_insert(innerTable, GUINT_TO_POINTER(eid_key2), &(currEvInst->instant));
			}

			/* 2 - Add event to occurring events. */
			g_hash_table_insert(
				eventsOccurring, 
				&(currEvInst->id), /* eid */
				g_hash_table_lookup(profile->unique_events, currEvInst->eventName) /* ueid */
			); 
			
		} else {
			/* Event END instant. */
			
			/* 1 - Remove event from ocurring events */
			g_hash_table_remove(eventsOccurring, &(currEvInst->id));
			
			/* 2 - Check for overlap termination with current events */
			g_hash_table_iter_init(&iter, eventsOccurring);
			while (g_hash_table_iter_next(&iter, &key_eid, &ueid_occu_ev)) {
				/* The first hash table key will be the smaller event id. */
				eid_key1 = currEvInst->id <= *((guint*) key_eid) ? currEvInst->id : *((guint*) key_eid);
				/* The second hash table key will be the larger event id. */
				eid_key2 = currEvInst->id > *((guint*) key_eid) ? currEvInst->id : *((guint*) key_eid);
				/* Get effective overlap in nanoseconds. */
				innerTable = g_hash_table_lookup(overlaps, GUINT_TO_POINTER(eid_key1));
				effOverlap = currEvInst->instant - *((cl_ulong*) g_hash_table_lookup(innerTable, GUINT_TO_POINTER(eid_key2)));
				/* Add overlap to overlap matrix. */
				ueid_curr_ev = g_hash_table_lookup(profile->unique_events, currEvInst->eventName);
				guint ueid_min = GPOINTER_TO_UINT(ueid_curr_ev) <= GPOINTER_TO_UINT(ueid_occu_ev) 
					? GPOINTER_TO_UINT(ueid_curr_ev) 
					: GPOINTER_TO_UINT(ueid_occu_ev);
				guint ueid_max = GPOINTER_TO_UINT(ueid_curr_ev) > GPOINTER_TO_UINT(ueid_occu_ev) 
					? GPOINTER_TO_UINT(ueid_curr_ev) 
					: GPOINTER_TO_UINT(ueid_occu_ev);
				overlapMatrix[ueid_min * numUniqEvts + ueid_max] += effOverlap;
				totalOverlap += effOverlap;
			}	
		}
		
		/* Get next event instant. */
		currEvInstContainer = currEvInstContainer->next;
	}

	/* Add a pointer to overlap matrix to the profile. */
	profile->overmat = overlapMatrix;
	
	/* Determine and save effective events time. */
	profile->totalEventsEffTime = profile->totalEventsTime - totalOverlap;
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = PROFCL_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	
	/* Free overlaps hash table. */
	if (overlaps) g_hash_table_destroy(overlaps);
	
	/* Free eventsOccurring hash table. */
	if (eventsOccurring) g_hash_table_destroy(eventsOccurring);
	
	/* Return status. */
	return status;	
}

/** 
 * @brief Determine aggregate statistics for the given OpenCL events profile. 
 * 
 * @param profile An OpenCL events profile.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link profcl_error_codes::PROFCL_SUCCESS @endlink if function
 * terminates successfully, or another value of #profcl_error_codes if 
 * an error occurs.
 * */
int profcl_profile_aggregate(ProfCLProfile* profile, GError** err) {
	
	/* Return status. */
	int status;
	/* Hash table iterator. */
	GHashTableIter iter;
	/* A pointer for a event name. */
	gpointer eventName;
	/* Aggregate event info. */
	ProfCLEvAggregate* evagg = NULL;
	/* Type of sorting to perform on event list. */
	ProfCLEvSort sortType;
	/* Aux. pointer for event data structure kept in a GList. */
	GList* currEvInstContainer = NULL;
	/* A pointer to a ProfCLEvAggregate (agg. event info) variable. */
	gpointer valueAgg;
	/* Auxiliary aggregate event info variable.*/
	ProfCLEvAggregate* currAgg = NULL;
	
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);

	/* Initalize table, and set aggregate values to zero. */
	g_hash_table_iter_init(&iter, profile->unique_events);
	while (g_hash_table_iter_next(&iter, &eventName, NULL)) {
		evagg = profcl_aggregate_new(eventName);
		gef_if_error_create_goto(*err, PROFCL_ERROR, evagg == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for ProfCLEvAggregate object.");
		evagg->totalTime = 0;
		g_hash_table_insert(profile->aggregate, eventName, (gpointer) evagg);
	}
	
	/* Sort event instants by eid, and then by START, END order. */
	sortType = PROFCL_EV_SORT_ID;
	profile->event_instants = g_list_sort_with_data(profile->event_instants, profcl_evinst_comp, (gpointer) &sortType);

	/* Iterate through all event instants and determine total times. */
	currEvInstContainer = profile->event_instants;
	while (currEvInstContainer) {
		
		/* Loop aux. variables. */
		ProfCLEvInst* currEvInst = NULL;
		cl_ulong startInst, endInst;
		
		/* Get START event instant. */
		currEvInst = (ProfCLEvInst*) currEvInstContainer->data;
		startInst = currEvInst->instant;
		
		/* Get END event instant */
		currEvInstContainer = currEvInstContainer->next;
		currEvInst = (ProfCLEvInst*) currEvInstContainer->data;
		endInst = currEvInst->instant;
		
		/* Add new interval to respective aggregate value. */
		currAgg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, currEvInst->eventName);
		currAgg->totalTime += endInst - startInst;
		profile->totalEventsTime += endInst - startInst;
		
		/* Get next START event instant. */
		currEvInstContainer = currEvInstContainer->next;
	}
	
	/* Determine relative times. */
	g_hash_table_iter_init(&iter, profile->aggregate);
	while (g_hash_table_iter_next(&iter, &eventName, &valueAgg)) {
		currAgg = (ProfCLEvAggregate*) valueAgg;
		currAgg->relativeTime = ((double) currAgg->totalTime) / ((double) profile->totalEventsTime);
	}

	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = PROFCL_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	
	/* Return status. */
	return status;		
	
}

/** 
 * @brief Create a new aggregate statistic for events of a given type.
 * 
 * @param eventName Name of event.
 * @return New aggregate statistic or NULL if allocation unsuccessful.
 * */
ProfCLEvAggregate* profcl_aggregate_new(const char* eventName) {
	ProfCLEvAggregate* agg = (ProfCLEvAggregate*) malloc(sizeof(ProfCLEvAggregate));
	if (agg != NULL) {
		agg->eventName = eventName;
	}
	return agg;
}

/** 
 * @brief Free an aggregate statistic. 
 * 
 * @param agg Aggregate statistic to free.
 * */
void profcl_aggregate_free(gpointer agg) {
	free(agg);
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
gint profcl_evagg_comp(gconstpointer a, gconstpointer b, gpointer userdata) {
	/* Cast input parameters to event instant data structures. */
	ProfCLEvAggregate* evAgg1 = (ProfCLEvAggregate*) a;
	ProfCLEvAggregate* evAgg2 = (ProfCLEvAggregate*) b;
	ProfCLEvAggDataSort* sortType = (ProfCLEvAggDataSort*) userdata;
	/* Perform comparison. */
	if (*sortType == PROFCL_AGGEVDATA_SORT_NAME) {
		/* Sort by event name */
		return strcmp(evAgg1->eventName, evAgg2->eventName);
	} else if (*sortType == PROFCL_AGGEVDATA_SORT_TIME) {
		/* Sort by event time period */
		if (evAgg1->totalTime < evAgg2->totalTime) return 1;
		if (evAgg1->totalTime > evAgg2->totalTime) return -1;
	}
	return 0;
}

/** 
* @brief Indication that profiling sessions has started. Starts the
* global profiler timer. 
* 
* @param profile An OpenCL events profile.
* */
void profcl_profile_start(ProfCLProfile* profile) {
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	/* Start timer. */
	profile->timer = g_timer_new();
}

/** 
 * @brief Indication that profiling sessions has ended. Stops the 
 * global profiler timer.
 * 
 * @param profile An OpenCL events profile. 
 * */
void profcl_profile_stop(ProfCLProfile* profile) {
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	/* Stop timer. */
	g_timer_stop(profile->timer);
}

/** 
 * @brief If profiling has started but not stopped, returns the time 
 * since the profiling started. If profiling has been stopped, returns 
 * the elapsed time between the time it started and the time it stopped.
 * 
 * @param profile An OpenCL events profile. 
 * @return number of seconds elapsed, including any fractional part.
 * */
gdouble profcl_time_elapsed(ProfCLProfile* profile) {
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	/* Stop timer. */
	return g_timer_elapsed(profile->timer, NULL);
}

/**
 * @brief Print profiling info.
 * 
 * @param profile An OpenCL events profile.
 * @param evAggSortType Sorting strategy for aggregate event data instances.
 * @param err Error structure, to be populated if an error occurs.
 * @return @link profcl_error_codes::PROFCL_SUCCESS @endlink if function
 * terminates successfully, or another value of #profcl_error_codes if 
 * an error occurs.
 * */ 
int profcl_print_info(ProfCLProfile* profile, ProfCLEvAggDataSort evAggSortType, GError** err) {
	
	/* Function return status. */
	int status;
	/* Auxiliary hash table. */
	GHashTable* tmp = NULL;
	/* Auxiliary hash table iterator. */
	GHashTableIter iter;
	/* Auxiliary pointers for data within hash tables and lists. */
	gpointer pEventName, pId;
	/* List of aggregate events. */
	GList* evAggList = NULL;
	/* List of aggregate events (traversing pointer). */
	GList* evAggContainer = NULL;
	/* Aggregate event info. */
	ProfCLEvAggregate* evAgg = NULL;
	/* Number of unique event names. */
	guint numUniqEvts;
	/* String containing description of overlapping events. */
	GString* overlapString = NULL;
	
	/* Make sure profile is not NULL. */
	g_assert(profile != NULL);
	
	printf("\n   =========================== Timming/Profiling ===========================\n\n");
	
	/* Show total ellapsed time */
	if (profile->timer) {
		printf("     Total ellapsed time       : %fs\n", g_timer_elapsed(profile->timer, NULL));
	}
	
	/* Show total events time */
	if (profile->totalEventsTime > 0) {
		printf("     Total of all events       : %fs\n", profile->totalEventsTime * 1e-9);
	}
	
	/* Show aggregate event times */
	if (g_hash_table_size(profile->aggregate) > 0) {
		printf("     Aggregate times by event  :\n");
		evAggList = g_hash_table_get_values(profile->aggregate);
		gef_if_error_create_goto(*err, PROFCL_ERROR, evAggList == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for GList object 'evAggList'.");
		evAggList = g_list_sort_with_data(evAggList, profcl_evagg_comp, &evAggSortType);
		evAggContainer = evAggList;
		printf("       ------------------------------------------------------------------\n");
		printf("       | Event name                     | Rel. time (%%) | Abs. time (s) |\n");
		printf("       ------------------------------------------------------------------\n");
		while (evAggContainer) {
			evAgg = (ProfCLEvAggregate*) evAggContainer->data;
			g_assert(evAgg != NULL);
			printf("       | %-30.30s | %13.4f | %13.4e |\n", evAgg->eventName, evAgg->relativeTime * 100.0, evAgg->totalTime * 1e-9);
			evAggContainer = evAggContainer->next;
		}
		printf("       ------------------------------------------------------------------\n");
	}
	
	/* Show overlaps */
	if (profile->overmat) {
		/* Create new temporary hash table which is the reverse in 
		 * terms of key-values of profile->unique_events. */
		tmp = g_hash_table_new(g_direct_hash, g_direct_equal);
		gef_if_error_create_goto(*err, PROFCL_ERROR, tmp == NULL, status = PROFCL_ALLOC_ERROR, error_handler, "Unable to allocate memory for 'tmp' table: g_hash_table_new function returned NULL.");
		/* Populate temporary hash table. */
		g_hash_table_iter_init(&iter, profile->unique_events);
		while (g_hash_table_iter_next(&iter, &pEventName, &pId)) {
			g_hash_table_insert(tmp, pId, pEventName);
		}
		/* Get number of unique events. */
		numUniqEvts = g_hash_table_size(profile->unique_events);
		/* Show overlaps. */
		overlapString = g_string_new("");
		for (guint i = 0; i < numUniqEvts; i++) {
			for (guint j = 0; j < numUniqEvts; j++) {
				if (profile->overmat[i * numUniqEvts + j] > 0) {
					g_string_append_printf(
						overlapString,
						"       | %-22.22s | %-22.22s | %12.4e |\n", 
						(const char*) g_hash_table_lookup(tmp, GUINT_TO_POINTER(i)),
						(const char*) g_hash_table_lookup(tmp, GUINT_TO_POINTER(j)),
						profile->overmat[i * numUniqEvts + j] * 1e-9
					);
				}
			}
		}
		if (strlen(overlapString->str) > 0) {
			/* Show total events effective time (discount overlaps) */
			printf("     Tot. of all events (eff.) : %es\n", profile->totalEventsEffTime * 1e-9);
			printf("                                 %es saved with overlaps\n", (profile->totalEventsTime - profile->totalEventsEffTime) * 1e-9);
			/* Title the several overlaps. */
			printf("     Event overlap times       :\n");
			printf("       ------------------------------------------------------------------\n");
			printf("       | Event 1                | Event2                 | Overlap (s)  |\n");
			printf("       ------------------------------------------------------------------\n");
			/* Show overlaps table. */
			printf("%s", overlapString->str);
			printf("       ------------------------------------------------------------------\n");
		}
	}
	
	/* If we got here, everything is OK. */
	g_assert (err == NULL || *err == NULL);
	status = PROFCL_SUCCESS;
	goto finish;
	
error_handler:
	/* If we got here there was an error, verify that it is so. */
	g_assert (err == NULL || *err != NULL);

finish:	
	
	/* Free stuff. */
	if (evAggList) g_list_free(evAggList);
	if (overlapString) g_string_free(overlapString, TRUE);
	if (tmp) g_hash_table_destroy(tmp);

	/* Return status. */
	return status;			

}

/** 
 * @brief Resolves to error category identifying string, in this case an error in the OpenCL profiler library.
 * 
 * @return A GQuark structure defined by category identifying string, which identifies the error as a CL profiler generated error.
 */
GQuark profcl_error_quark() {
	return g_quark_from_static_string("profcl-error-quark");
}
