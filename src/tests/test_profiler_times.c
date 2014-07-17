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
 * @brief Timming and correctness tests for the profiling module. Can
 * only be performed using the OpenCL stub.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include "profiler.h"
#include "ocl_stub/ocl_impl.h"

/**
 * @brief Tests the profiling module.
 * */
static void timesTest() {
	
	/* Aux vars. */
	guint numEvents = 5;
	struct _cl_event ev1, ev2, ev3, ev4, ev5, ev6, ev7, ev8;
	int status;
	cl_command_queue queue1, queue2, queue3;
	queue1 = clCreateCommandQueue(NULL, NULL, 0, NULL);
	queue2 = clCreateCommandQueue(NULL, NULL, 0, NULL);
	queue3 = clCreateCommandQueue(NULL, NULL, 0, NULL);
	
	/* Profiling object. */
	CL4Prof* profile = cl4_prof_new();
	g_assert(profile != NULL);

	/* Test with 5 unique events */
	ev1.start = 10;
	ev1.end = 15;
	ev1.queue = queue1;
	status = cl4_prof_add(profile, "Event1", &ev1, NULL);
	g_assert(status == CL4_SUCCESS);

	ev2.start = 16;
	ev2.end = 20;
	ev2.queue = queue1;
	status = cl4_prof_add(profile, "Event2", &ev2, NULL);
	g_assert(status == CL4_SUCCESS);

	ev3.start = 17;
	ev3.end = 30;
	ev3.queue = queue2;
	status = cl4_prof_add(profile, "Event3", &ev3, NULL);
	g_assert(status == CL4_SUCCESS);

	ev4.start = 19;
	ev4.end = 25;
	ev4.queue = queue3;
	status = cl4_prof_add(profile, "Event4", &ev4, NULL);
	g_assert(status == CL4_SUCCESS);

	ev5.start = 29;
	ev5.end = 40;
	ev5.queue = queue1;
	status = cl4_prof_add(profile, "Event5", &ev5, NULL);
	g_assert(status == CL4_SUCCESS);

	ev6.start = 35;
	ev6.end = 45;
	ev6.queue = queue2;
	status = cl4_prof_add(profile, "Event1", &ev6, NULL);
	g_assert(status == CL4_SUCCESS);

	ev7.start = 68;
	ev7.end = 69;
	ev7.queue = queue1;
	status = cl4_prof_add(profile, "Event1", &ev7, NULL);
	g_assert(status == CL4_SUCCESS);

	ev8.start = 50;
	ev8.end = 70;
	ev8.queue = queue3;
	status = cl4_prof_add(profile, "Event1", &ev8, NULL);
	g_assert(status == CL4_SUCCESS);

	status = cl4_prof_aggregate(profile, NULL);
	g_assert(status == CL4_SUCCESS);
	status = cl4_prof_overmat(profile, NULL);
	g_assert(status == CL4_SUCCESS);
	
	/* ************************* */
	/* Test aggregate statistics */
	/* ************************* */
	
	CL4ProfEvAgg* agg;
	
	agg = (CL4ProfEvAgg*) g_hash_table_lookup(profile->aggregate, "Event1");
	g_assert_cmpuint(agg->totalTime, ==, 36);
	g_assert_cmpfloat(agg->relativeTime - 0.51728, <, 0.0001);
	
	agg = (CL4ProfEvAgg*) g_hash_table_lookup(profile->aggregate, "Event2");
	g_assert_cmpuint(agg->totalTime, ==, 4);
	g_assert_cmpfloat(agg->relativeTime - 0.05714, <, 0.0001);

	agg = (CL4ProfEvAgg*) g_hash_table_lookup(profile->aggregate, "Event3");
	g_assert_cmpuint(agg->totalTime, ==, 13);
	g_assert_cmpfloat(agg->relativeTime - 0.18571, <, 0.0001);

	agg = (CL4ProfEvAgg*) g_hash_table_lookup(profile->aggregate, "Event4");
	g_assert_cmpuint(agg->totalTime, ==, 6);
	g_assert_cmpfloat(agg->relativeTime - 0.08571, <, 0.0001);

	agg = (CL4ProfEvAgg*) g_hash_table_lookup(profile->aggregate, "Event5");
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
	CL4ProfExportOptions export_options = cl4_prof_export_opts_get();
	export_options.separator = "\t"; /* Default */
	export_options.queue_delim = ""; /* Default */
	export_options.evname_delim = ""; /* Default */
	export_options.simple_queue_id = TRUE; /* Default */
	export_options.zero_start = FALSE; /* Not default */
	cl4_prof_export_opts_set(export_options);
	
	/* Export options. */
	gchar *name_used;
	FILE* fp = fdopen(
		g_file_open_tmp("test_profiler_XXXXXX.txt", &name_used, NULL),
		"wb"
	);
	status = cl4_prof_export_info(profile, fp, NULL);
	g_assert(status == CL4_SUCCESS);
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
	//cl4_prof_print_info(profile, CL4_PROF_AGGEVDATA_SORT_TIME);

	/* Free queue. */
	clReleaseCommandQueue(queue1);
	clReleaseCommandQueue(queue2);
	clReleaseCommandQueue(queue3);
	
	/* Free profile. */
	cl4_prof_destroy(profile);
	
}

/**
 * @brief Main function.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/profiler/times", timesTest);
	return g_test_run();
}

