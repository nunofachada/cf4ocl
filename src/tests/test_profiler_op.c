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
 * Operation (timming and correctness) tests for the profiling
 * module. Can only be performed using the OpenCL stub.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "ocl_stub/ocl_impl.h"

#define ccl_test_prof_is_overlap(ev1, ev2) \
	( \
		(g_strcmp0(o->event1_name, ev1) == 0) \
		&& \
		(g_strcmp0(o->event2_name, ev2) == 0) \
	) || ( \
		(g_strcmp0(o->event1_name, ev2) == 0) \
		&& \
		(g_strcmp0(o->event2_name, ev1) == 0) \
	)

#ifdef G_OS_WIN32
	#define CCL_TEST_NEWLINE "\r\n"
#else
	#define CCL_TEST_NEWLINE "\n"
#endif

#define CCL_TEST_MAXBUF 512
/**
 * Tests the profiling module.
 * */
static void operation_test() {

	/* Aux vars. */
	CCLContext* ctx;
	CCLDevice* dev;
	CCLQueue *q1, *q2, *q3;
	CCLEvent *ev1, *ev2, *ev3, *ev4, *ev5, *ev6, *ev7, *ev8;
	CCLBuffer* buf;
	CCLProgram* prg;
	CCLKernel* krnl;
	CCLProf* prof;
	GError* err = NULL;
	cl_event ev_unwrapped;
	cl_int host_ptr[CCL_TEST_MAXBUF];
	const char* src = "__kernel void k1(_global int* a){}";
	void* mapped_ptr;

	/* Create OpenCL wrappers for testing. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	q1 = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	q2 = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);
	q3 = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
	g_assert_no_error(err);

	buf = ccl_buffer_new(ctx, CL_MEM_READ_ONLY,
		sizeof(cl_int) * CCL_TEST_MAXBUF, NULL, &err);
	g_assert_no_error(err);

	prg = ccl_program_new_from_source(ctx, src, &err);
	g_assert_no_error(err);

	krnl = ccl_program_get_kernel(prg, "k1", &err);
	g_assert_no_error(err);

	/* Profiling object. */
	prof = ccl_prof_new();

	/* Test with 5 different event names. */
	ev1 = ccl_buffer_enqueue_write(buf, q1, CL_TRUE, 0,
		sizeof(cl_int) * CCL_TEST_MAXBUF, host_ptr, NULL, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev1, "Event1");
	ev_unwrapped = ccl_event_unwrap(ev1);
	ev_unwrapped->t_start = 10;
	ev_unwrapped->t_end = 15;

	mapped_ptr = ccl_buffer_enqueue_map(buf, q1, CL_TRUE, CL_MAP_READ,
		0, sizeof(cl_int) * CCL_TEST_MAXBUF, NULL, &ev2, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev2, "Event2");
	ev_unwrapped = ccl_event_unwrap(ev2);
	ev_unwrapped->t_start = 16;
	ev_unwrapped->t_end = 20;

	ev3 = ccl_memobj_enqueue_unmap(
		(CCLMemObj*) buf, q1, mapped_ptr, NULL, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev3, "Event3");
	ev_unwrapped = ccl_event_unwrap(ev3);
	ev_unwrapped->t_start = 17;
	ev_unwrapped->t_end = 30;

	size_t gws = 256;
	size_t lws = 16;
	size_t gwo = 0;

	ev4 = ccl_kernel_set_args_and_enqueue_ndrange(
		krnl, q3, 1, &gwo, &gws, &lws, NULL, &err, buf, NULL);
	g_assert_no_error(err);
	ccl_event_set_name(ev4, "Event4");
	ev_unwrapped = ccl_event_unwrap(ev4);
	ev_unwrapped->t_start = 19;
	ev_unwrapped->t_end = 25;

	ev5 = ccl_buffer_enqueue_read(buf, q1, CL_TRUE, 0,
		sizeof(cl_int) * CCL_TEST_MAXBUF, host_ptr, NULL, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev5, "Event5");
	ev_unwrapped = ccl_event_unwrap(ev5);
	ev_unwrapped->t_start = 29;
	ev_unwrapped->t_end = 40;

	ev6 = ccl_buffer_enqueue_write(buf, q2, CL_TRUE, 0,
		sizeof(cl_int) * (CCL_TEST_MAXBUF / 5), host_ptr, NULL, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev6, "Event1");
	ev_unwrapped = ccl_event_unwrap(ev6);
	ev_unwrapped->t_start = 35;
	ev_unwrapped->t_end = 45;

	ev7 = ccl_kernel_set_args_and_enqueue_ndrange(krnl, q1, 1, &gwo,
		&gws, &lws, NULL, &err, buf, NULL);
	g_assert_no_error(err);
	ccl_event_set_name(ev7, "Event1");
	ev_unwrapped = ccl_event_unwrap(ev7);
	ev_unwrapped->t_start = 68;
	ev_unwrapped->t_end = 69;

	ev8 = ccl_buffer_enqueue_write(buf, q3, CL_TRUE, 0,
		sizeof(cl_int) * (CCL_TEST_MAXBUF / 5), host_ptr, NULL, &err);
	g_assert_no_error(err);
	ccl_event_set_name(ev8, "Event1");
	ev_unwrapped = ccl_event_unwrap(ev8);
	ev_unwrapped->t_start = 50;
	ev_unwrapped->t_end = 70;

	/* Add queues. */
	ccl_prof_add_queue(prof, "Q1", q1);
	ccl_prof_add_queue(prof, "Q2", q2);
	ccl_prof_add_queue(prof, "Q3", q3);

	/* Perform profiling calculations. */
	ccl_prof_calc(prof, &err);
	g_assert_no_error(err);

	/* ************************* */
	/* Test aggregate statistics */
	/* ************************* */

	/* 1) Directly. */
	const CCLProfAgg* agg;

	agg = ccl_prof_get_agg(prof, "Event1");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->absolute_time, ==, 36);
	g_assert_cmpfloat(agg->relative_time - 0.51728, <, 0.0001);

	agg = ccl_prof_get_agg(prof, "Event2");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->absolute_time, ==, 4);
	g_assert_cmpfloat(agg->relative_time - 0.05714, <, 0.0001);

	agg = ccl_prof_get_agg(prof, "Event3");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->absolute_time, ==, 13);
	g_assert_cmpfloat(agg->relative_time - 0.18571, <, 0.0001);

	agg = ccl_prof_get_agg(prof, "Event4");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->absolute_time, ==, 6);
	g_assert_cmpfloat(agg->relative_time - 0.08571, <, 0.0001);

	agg = ccl_prof_get_agg(prof, "Event5");
	g_assert(agg != NULL);
	g_assert_cmpuint(agg->absolute_time, ==, 11);
	g_assert_cmpfloat(agg->relative_time - 0.15714, <, 0.0001);

	/* 2) By cycling all agg. stats. */
	const char* prev_name = "zzzz";

	ccl_prof_iter_agg_init(
		prof, CCL_PROF_AGG_SORT_NAME | CCL_PROF_SORT_DESC);

	while ((agg = ccl_prof_iter_agg_next(prof)) != NULL) {

		/* Just check that the event names are ordered properly. */
		g_assert_cmpstr(agg->event_name, <=, prev_name);
		prev_name = agg->event_name;
	}

	/* **************** */
	/* Test event infos */
	/* **************** */

	prev_name = "0000";
	const CCLProfInfo* info;

	/* Test ordering by event name. */
	ccl_prof_iter_info_init(
		prof, CCL_PROF_INFO_SORT_NAME_EVENT | CCL_PROF_SORT_ASC);

	while ((info = ccl_prof_iter_info_next(prof)) != NULL) {

		/* Check that the event names are ordered properly. */
		g_assert_cmpstr(info->event_name, >=, prev_name);
		prev_name = info->event_name;

	}

	/* ******************* */
	/* Test event instants */
	/* ******************* */

	const CCLProfInst* pi;
	cl_ulong prev_inst = 0;
	ccl_prof_iter_inst_init(
		prof, CCL_PROF_INST_SORT_INSTANT | CCL_PROF_SORT_ASC);

	while ((pi = ccl_prof_iter_inst_next(prof)) != NULL) {

		/* Check that previous instant occured before current one. */
		g_assert_cmpuint(prev_inst, <=, pi->instant);
		prev_inst = pi->instant;
	}

	/* ************* */
	/* Test overlaps */
	/* ************* */

	const CCLProfOverlap* o;
	ccl_prof_iter_overlap_init(prof, CCL_PROF_OVERLAP_SORT_DURATION |
		CCL_PROF_SORT_DESC);
	while ((o = ccl_prof_iter_overlap_next(prof)) != NULL) {
		if (ccl_test_prof_is_overlap("Event3", "Event4")) {
			g_assert_cmpuint(o->duration, ==, 6);
		} else if (ccl_test_prof_is_overlap("Event1", "Event5")) {
			g_assert_cmpuint(o->duration, ==, 5);
		} else if (ccl_test_prof_is_overlap("Event2", "Event3")) {
			g_assert_cmpuint(o->duration, ==, 3);
		} else if (ccl_test_prof_is_overlap("Event3", "Event5")) {
			g_assert_cmpuint(o->duration, ==, 1);
		} else if (ccl_test_prof_is_overlap("Event2", "Event4")) {
			g_assert_cmpuint(o->duration, ==, 1);
		} else if (ccl_test_prof_is_overlap("Event1", "Event1")) {
			g_assert_cmpuint(o->duration, ==, 1);
		} else {
			g_assert_not_reached();
		}
	}

	/* ******************* */
	/* Test export options */
	/* ******************* */

	/* Set some export options. */
	CCLProfExportOptions export_options = ccl_prof_get_export_opts();
	export_options.separator = "\t"; /* Default */
	export_options.queue_delim = ""; /* Default */
	export_options.evname_delim = ""; /* Default */
	export_options.zero_start = FALSE; /* Not default */
	ccl_prof_set_export_opts(export_options);

	/* Export options. */
	gchar *tmp_dir_name, *tmp_file_name;
	tmp_dir_name = g_dir_make_tmp("test_op_profiler_XXXXXX", &err);
	g_assert_no_error(err);
	tmp_file_name = g_strconcat(
		tmp_dir_name, G_DIR_SEPARATOR_S, "export.tsv", NULL);

	cl_bool export_status = ccl_prof_export_info_file(
		prof, tmp_file_name, &err);
	g_assert_no_error(err);
	g_assert(export_status);

	/* Test if output file was correctly written. */
	gchar* file_contents;
	gchar* expected_contents =
		"Q1\t10\t15\tEvent1" CCL_TEST_NEWLINE \
		"Q1\t16\t20\tEvent2" CCL_TEST_NEWLINE \
		"Q1\t17\t30\tEvent3" CCL_TEST_NEWLINE \
		"Q3\t19\t25\tEvent4" CCL_TEST_NEWLINE \
		"Q1\t29\t40\tEvent5" CCL_TEST_NEWLINE \
		"Q2\t35\t45\tEvent1" CCL_TEST_NEWLINE \
		"Q3\t50\t70\tEvent1" CCL_TEST_NEWLINE \
		"Q1\t68\t69\tEvent1" CCL_TEST_NEWLINE;
	gboolean read_flag = g_file_get_contents(
		tmp_file_name, &file_contents, NULL, NULL);
	g_assert(read_flag);
	g_assert_cmpstr(file_contents, ==, expected_contents);
	g_free(file_contents);
	g_free(tmp_dir_name);
	g_free(tmp_file_name);

	/* Print summary to debug output. */
	const char* summary = ccl_prof_get_summary(prof,
		CCL_PROF_AGG_SORT_TIME | CCL_PROF_SORT_DESC,
		CCL_PROF_OVERLAP_SORT_DURATION | CCL_PROF_SORT_DESC);

	g_debug("\n%s", summary);

	/* Free profile. */
	ccl_prof_destroy(prof);

	/* Free program wrapper. */
	ccl_program_destroy(prg);

	/* Free buffer wrapper. */
	ccl_buffer_destroy(buf);

	/* Free queue wrappers. */
	ccl_queue_destroy(q3);
	ccl_queue_destroy(q2);
	ccl_queue_destroy(q1);

	/* Free context. */
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/profiler/operation", operation_test);

	return g_test_run();

}

