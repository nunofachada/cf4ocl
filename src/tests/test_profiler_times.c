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
	guint num_event_names = 5;
	CL4Context* ctx;
	CL4CQueue *q1, *q2, *q3;
	CL4Event *ev1, *ev2, *ev3, *ev4, *ev5, *ev6, *ev7, *ev8;
	CL4Buffer* buf;
	CL4Program* prg;
	CL4Kernel* krnl;
	GError* err = NULL;
	cl_int buf_host[100];
	cl_event ev_unwrapped;
	void* a_bug = NULL;
	
	/* Create OpenCL wrappers for testing. */
	ctx = cl4_context_new_any(&err);
	g_assert_no_error(err);
	
	dev = cl4_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);
	
	q1 = cl4_cqueue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	q2 = cl4_cqueue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	q3 = cl4_cqueue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	
	buf = cl4_buffer_new(ctx, CL_MEM_READ_ONLY, sizeof(cl_int) * 100, NULL, &err);
	g_assert_no_error(err);
	
	prg = cl4_program_new_from_source(ctx, "__kernel void k1(_global int* a){}", &err);
	g_assert_no_error(err);

	krnl = cl4_program_get_kernel(prg, "k1", &err);
	g_assert_no_error(err);
	
	/* Profiling object. */
	CL4Prof* profile = cl4_prof_new();

	/* Test with 5 different event names. */
	ev1 = cl4_buffer_write(q1, buf, CL_TRUE, 0, sizeof(cl_int) * 100, a_bug, NULL, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev1, "Event1");
	ev_unwrapped = cl4_event_unwrap(ev1);
	ev_unwrapped.t_start = 10;
	ev_unwrapped.t_end = 15;

	cl4_buffer_map(q1, buf, CL_TRUE, CL_MAP_READ, 0, sizeof(cl_int) * 100, NULL, &ev2, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev2, "Event2");
	ev_unwrapped = cl4_event_unwrap(ev2);
	ev_unwrapped.t_start = 16;
	ev_unwrapped.t_end = 20;

	ev3 = cl4_memobj_unmap((CL4MemObj*) buf, q1, a_bug, NULL, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev3, "Event3");
	ev_unwrapped = cl4_event_unwrap(ev3);
	ev_unwrapped.t_start = 17;
	ev_unwrapped.t_end = 30;

	size_t gws = 256;
	size_t lws = 16;
	size_t gwo = 0;

	ev4 = cl4_kernel_set_args_and_run(krnl, q3, 1, &gwo, &gws, &lws, NULL, &err, buf, NULL);
	g_assert_no_error(err);
	cl4_event_set_name(ev4, "Event4");
	ev_unwrapped = cl4_event_unwrap(ev4);
	ev_unwrapped.t_start = 19;
	ev_unwrapped.t_end = 25;

	ev5 = cl4_buffer_read(q1, buf, CL_TRUE, 0, sizeof(cl_int) * 100, a_bug, NULL, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev5, "Event5");
	ev_unwrapped = cl4_event_unwrap(ev5);
	ev_unwrapped.t_start = 29;
	ev_unwrapped.t_end = 40;

	ev6 = cl4_buffer_write(q2, buf, CL_TRUE, 0, sizeof(cl_int) * 20, a_bug, NULL, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev6, "Event1");
	ev_unwrapped = cl4_event_unwrap(ev6);
	ev_unwrapped.t_start = 35;
	ev_unwrapped.t_end = 45;
	ev6.queue = queue2;

	ev7 = cl4_kernel_set_args_and_run(krnl, q1, 1, &gwo, &gws, &lws, NULL, &err, buf, NULL);
	g_assert_no_error(err);
	cl4_event_set_name(ev7, "Event1");
	ev_unwrapped = cl4_event_unwrap(ev7);
	ev_unwrapped.t_start = 68;
	ev_unwrapped.t_end = 69;

	ev8 = cl4_buffer_write(q3, buf, CL_TRUE, 0, sizeof(cl_int) * 20, a_bug, NULL, &err);
	g_assert_no_error(err);
	cl4_event_set_name(ev8, "Event1");
	ev_unwrapped = cl4_event_unwrap(ev8);
	ev_unwrapped.t_start = 50;
	ev_unwrapped.t_end = 70;

	/* Add queues. */
	cl4_prof_add_queue(prof, "Q1", q1);
	cl4_prof_add_queue(prof, "Q2", q2);
	cl4_prof_add_queue(prof, "Q3", q3);

	/* Perform profiling calculations. */
	cl4_prof_calc(prof, &err);
	g_assert_no_error(err);
	
	/* ************************* */
	/* Test aggregate statistics */
	/* ************************* */
	
	CL4ProfEventAgg* agg;
	
	agg = cl4_prof_get_agg(profile, "Event1");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->totalTime, ==, 36);
	g_assert_cmpfloat(agg->relativeTime - 0.51728, <, 0.0001);
	
	agg = cl4_prof_get_agg(profile, "Event2");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->totalTime, ==, 4);
	g_assert_cmpfloat(agg->relativeTime - 0.05714, <, 0.0001);

	agg = (CL4ProfEventAgg*) cl4_prof_get_agg(profile, "Event3");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->totalTime, ==, 13);
	g_assert_cmpfloat(agg->relativeTime - 0.18571, <, 0.0001);

	agg = (CL4ProfEventAgg*) cl4_prof_get_agg(profile, "Event4");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->totalTime, ==, 6);
	g_assert_cmpfloat(agg->relativeTime - 0.08571, <, 0.0001);

	agg = (CL4ProfEventAgg*) cl4_prof_get_agg(profile, "Event5");
	g_assert(agg != NULL);
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
	//cl4_prof_print_info(profile, CL4_PROF_EVENTAGG_SORT_TIME);

	
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

