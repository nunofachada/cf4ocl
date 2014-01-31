/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/** 
 * @file
 * @brief Tests for profiling framework.
 * 
 * @author Nuno Fachada
 * @date 2013
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "test_profiler.h"
#include "clprofiler.h"

/**
 * @brief Tests the OpenCL profiling framework.
 * */
static void profilerTest() {
	
	/* Aux vars. */
	guint numEvents = 5;
	cl_event ev1, ev2, ev3, ev4, ev5, ev6, ev7, ev8;
	int status;
	cl_command_queue queue1, queue2, queue3;
	queue1 = (cl_command_queue) malloc(sizeof(int));
	queue2 = (cl_command_queue) malloc(sizeof(int));
	queue3 = (cl_command_queue) malloc(sizeof(int));
	
	/* Profiling object. */
	ProfCLProfile* profile = profcl_profile_new();
	g_assert(profile != NULL);

	/* Test with 5 unique events */
	ev1.start = 10;
	ev1.end = 15;
	ev1.queue = queue1;
	status = profcl_profile_add(profile, "Event1", ev1, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev2.start = 16;
	ev2.end = 20;
	ev2.queue = queue1;
	status = profcl_profile_add(profile, "Event2", ev2, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev3.start = 17;
	ev3.end = 30;
	ev3.queue = queue2;
	status = profcl_profile_add(profile, "Event3", ev3, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev4.start = 19;
	ev4.end = 25;
	ev4.queue = queue3;
	status = profcl_profile_add(profile, "Event4", ev4, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev5.start = 29;
	ev5.end = 40;
	ev5.queue = queue1;
	status = profcl_profile_add(profile, "Event5", ev5, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev6.start = 35;
	ev6.end = 45;
	ev6.queue = queue2;
	status = profcl_profile_add(profile, "Event1", ev6, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev7.start = 68;
	ev7.end = 69;
	ev7.queue = queue1;
	status = profcl_profile_add(profile, "Event1", ev7, NULL);
	g_assert(status == PROFCL_SUCCESS);

	ev8.start = 50;
	ev8.end = 70;
	ev8.queue = queue3;
	status = profcl_profile_add(profile, "Event1", ev8, NULL);
	g_assert(status == PROFCL_SUCCESS);

	status = profcl_profile_aggregate(profile, NULL);
	g_assert(status == PROFCL_SUCCESS);
	status = profcl_profile_overmat(profile, NULL);
	g_assert(status == PROFCL_SUCCESS);
	
	/* ************************* */
	/* Test aggregate statistics */
	/* ************************* */
	
	ProfCLEvAggregate* agg;
	
	agg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, "Event1");
	g_assert_cmpuint(agg->totalTime, ==, 36);
	g_assert_cmpfloat(agg->relativeTime - 0.51728, <, 0.0001);
	
	agg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, "Event2");
	g_assert_cmpuint(agg->totalTime, ==, 4);
	g_assert_cmpfloat(agg->relativeTime - 0.05714, <, 0.0001);

	agg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, "Event3");
	g_assert_cmpuint(agg->totalTime, ==, 13);
	g_assert_cmpfloat(agg->relativeTime - 0.18571, <, 0.0001);

	agg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, "Event4");
	g_assert_cmpuint(agg->totalTime, ==, 6);
	g_assert_cmpfloat(agg->relativeTime - 0.08571, <, 0.0001);

	agg = (ProfCLEvAggregate*) g_hash_table_lookup(profile->aggregate, "Event5");
	g_assert_cmpuint(agg->totalTime, ==, 11);
	g_assert_cmpfloat(agg->relativeTime - 0.15714, <, 0.0001);

	/* ******************* */
	/* Test overlap matrix */
	/* ******************* */
	
	/* Expected overlap matrix */
	cl_ulong expectedOvermat[5][5] = 
	{
		{1, 0, 0, 0, 5},
		{0, 0, 3, 1, 0},
		{0, 0, 0, 6, 1},
		{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0}
	};
	
	/* Test if currentOverlapMatrix is as expected */
	for (guint i = 0; i < numEvents; i++) {
		for (guint j = 0; j < numEvents; j++) {
			g_assert_cmpuint(profile->overmat[i * numEvents + j], ==, expectedOvermat[i][j]);
		}
	}
	
	/* ******************* */
	/* Test export options */
	/* ******************* */
	
	/* Set some export options. */
	ProfCLExportOptions export_options = profcl_export_opts_get();
	export_options.separator = "\t"; /* Default */
	export_options.queue_delim = ""; /* Default */
	export_options.evname_delim = ""; /* Default */
	export_options.simple_queue_id = TRUE; /* Default */
	export_options.zero_start = FALSE; /* Not default */
	profcl_export_opts_set(export_options);
	
	/* Export options. */
	gchar *name_used;
	FILE* fp = fdopen(
		g_file_open_tmp("test_profiler_XXXXXX.txt", &name_used, NULL),
		"wb"
	);
	status = profcl_export_info(profile, fp, NULL);
	g_assert(status == PROFCL_SUCCESS);
	fclose(fp);

	/* Test if output file was correctly written. */
	gchar* file_contents;
	gchar* expected_contents = "0\t10\t15\tEvent1\n0\t16\t20\tEvent2\n1\t17\t30\tEvent3\n2\t19\t25\tEvent4\n0\t29\t40\tEvent5\n1\t35\t45\tEvent1\n0\t68\t69\tEvent1\n2\t50\t70\tEvent1\n";
	gboolean read_flag = g_file_get_contents(name_used, &file_contents, NULL, NULL);
	g_assert(read_flag);
	g_assert_cmpstr(file_contents, ==, expected_contents);
	g_free(file_contents);
	g_free(name_used);
	
	/** @todo Change profcl so that it prints to a stream, then test  */
	//profcl_print_info(profile, PROFCL_AGGEVDATA_SORT_TIME);

	/* Free queue. */
	free(queue1);
	free(queue2);
	free(queue3);
	
	/* Free profile. */
	profcl_profile_free(profile);
	
}

/**
 * @brief Main function.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/profiler/profiler", profilerTest);
	return g_test_run();
}

/** 
 * @brief Stub for clGetEventProfilingInfo function. 
 * 
 * @param event Stub event.
 * @param param_name Specifies the profiling data to query.
 * @param param_value_size Ignored.
 * @param param_value A pointer to memory where the appropriate result being queried is returned.
 * @param param_value_size_ret Ignored.
 * @return Always returns CL_SUCCESS.
 * */ 
cl_int clGetEventProfilingInfo(cl_event event, cl_profiling_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) {
	/* Ignore compiler warnings. */
	param_value_size = param_value_size; param_value_size_ret = param_value_size_ret;
	/* Return start or end instants in given memory location. */
	if (param_name == CL_PROFILING_COMMAND_START)
		*((cl_ulong*) param_value) = event.start;
	else
		*((cl_ulong*) param_value) = event.end;
	/* Always return success. */
	return CL_SUCCESS;
}

/** 
 * @brief Stub for clGetEventInfo function. 
 * 
 * @param event Stub event.
 * @param param_name Ignored (assumes CL_EVENT_COMMAND_QUEUE).
 * @param param_value_size Ignored.
 * @param param_value Memory location where to place fake queue.
 * @param param_value_size_ret Ignored.
 * @return Always returns CL_SUCCESS.
 * */ 
cl_int clGetEventInfo(cl_event event, cl_event_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
	/* Ignore compiler warnings. */
	param_name = param_name; param_value_size = param_value_size; param_value_size_ret = param_value_size_ret;
	/* Return the event command queue in given memor location. */
	*((cl_command_queue*) param_value) = event.queue;
	/* Always return success. */
	return CL_SUCCESS;

}
