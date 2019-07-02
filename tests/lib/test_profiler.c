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
 * @internal
 *
 * @file
 * Test the profiling module.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"

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

#define CCL_TEST_MAXBUF 512

/**
 * @internal
 *
 * @brief Tests creation, getting info from and destruction of
 * profiler objects, and their relationship with context, device and
 * queue wrapper objects.
 * */
static void create_add_destroy_test() {

    /* Test variables. */
    CCLErr * err = NULL;
    CCLBuffer * buf1 = NULL;
    CCLBuffer * buf2 = NULL;
    CCLProf * prof = NULL;
    CCLContext * ctx = NULL;
    CCLDevice * d = NULL;
    CCLQueue * cq1 = NULL;
    CCLQueue * cq2 = NULL;
    CCLEvent * evt = NULL;
    CCLEventWaitList ewl = NULL;
    size_t buf_size = 8 * sizeof(cl_short);
    cl_short hbuf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    cl_ulong duration, eff_duration;
    double time_elapsed;

    /* Create a new profile object. */
    prof = ccl_prof_new();

    /* Get a context and a device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    d = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create two command queue wrappers. */
    cq1 = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
    g_assert_no_error(err);

    cq2 = ccl_queue_new(ctx, d, CL_QUEUE_PROFILING_ENABLE, &err);
    g_assert_no_error(err);

    /* Create device buffers. */
    buf1 = ccl_buffer_new(ctx, CL_MEM_READ_ONLY, buf_size, NULL, &err);
    g_assert_no_error(err);
    buf2 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE, buf_size, NULL, &err);
    g_assert_no_error(err);

    /* Start profile object timer. */
    ccl_prof_start(prof);

    /* Transfer data to buffer. */
    evt = ccl_buffer_enqueue_write(
        buf1, cq1, CL_FALSE, 0, buf_size, hbuf, NULL, &err);
    g_assert_no_error(err);

    /* Transfer data from one buffer to another. */
    evt = ccl_buffer_enqueue_copy(
        buf1, buf2, cq2, 0, 0, buf_size, ccl_ewl(&ewl, evt, NULL), &err);
    g_assert_no_error(err);

    /* Wait for copy. */
    ccl_event_wait(ccl_ewl(&ewl, evt, NULL), &err);
    g_assert_no_error(err);

    /* Stop profile object timer. */
    ccl_prof_stop(prof);

    /* Add both queues to profile object. */
    ccl_prof_add_queue(prof, "A Queue", cq1);
    ccl_prof_add_queue(prof, "Another Queue", cq2);

    /* Process queues. */
    ccl_prof_calc(prof, &err);
    g_assert_no_error(err);

    /* Request some profiling information. */
    time_elapsed = ccl_prof_time_elapsed(prof);
    duration = ccl_prof_get_duration(prof);
    eff_duration = ccl_prof_get_eff_duration(prof);

    g_debug("Profiling time elapsed: %lf", time_elapsed);
    g_debug("Profiling duration: %d", (cl_int) duration);
    g_debug("Profiling eff. duration: %d", (cl_int) eff_duration);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Destroy buffers. */
    ccl_buffer_destroy(buf1);
    ccl_buffer_destroy(buf2);

    /* Unref cq1, which should not be destroyed because it is held
     * by the profile object. */
    ccl_queue_destroy(cq1);

    /* Destroy the profile object, which will also destroy cq1. cq2
     * will me merely unrefed and must still be explicitly destroyed. */
    ccl_prof_destroy(prof);

    /* Destroy cq2. */
    ccl_queue_destroy(cq2);

    /* Destroy the context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests profiler features.
 * */
static void features_test() {

    /* Aux vars. */
    CCLContext * ctx;
    CCLDevice * dev;
    CCLQueue * q1, * q2;
    CCLBuffer * buf1, * buf2;
    CCLEvent * ev1, * ev2, * ev3, * ev4;
    CCLEventWaitList ewl = NULL;
    CCLProf * prof;
    CCLErr * err = NULL;
    cl_int h_buf1[CCL_TEST_MAXBUF], h_buf2[CCL_TEST_MAXBUF];

    /* Put random stuff in host buffers. */
    for (guint i = 0; i < CCL_TEST_MAXBUF; ++i)
    {
        h_buf1[i] = g_test_rand_int();
        h_buf2[i] = g_test_rand_int();
    }

    /* Create OpenCL wrappers for testing. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    q1 = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    g_assert_no_error(err);
    q2 = ccl_queue_new(ctx, dev, CL_QUEUE_PROFILING_ENABLE, &err);
    g_assert_no_error(err);

    buf1 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE,
        sizeof(cl_int) * CCL_TEST_MAXBUF, NULL, &err);
    g_assert_no_error(err);

    buf2 = ccl_buffer_new(ctx, CL_MEM_READ_WRITE,
        sizeof(cl_int) * CCL_TEST_MAXBUF, NULL, &err);
    g_assert_no_error(err);

    /* Profiling object. */
    prof = ccl_prof_new();

    /* Write stuff from host buffers to device buffers using different
     * queues. These are non-blocking writes, so they may overlap in time. */
    ev1 = ccl_buffer_enqueue_write(
        buf1, q1, CL_FALSE, 0,
        sizeof(cl_int) * CCL_TEST_MAXBUF, h_buf1, NULL, &err);
    g_assert_no_error(err);
    ccl_event_set_name(ev1, "Event1");

    ev2 = ccl_buffer_enqueue_write(
        buf2, q2, CL_FALSE, 0,
        sizeof(cl_int) * CCL_TEST_MAXBUF, h_buf2, NULL, &err);
    g_assert_no_error(err);
    ccl_event_set_name(ev2, "Event2");

    /* Add write events to the event wait list and wait for writes to finish. */
    ccl_event_wait_list_add(&ewl, ev1, ev2, NULL);
    ccl_event_wait(&ewl, &err);
    g_assert_no_error(err);

    /* Now do some reads, swaping data in the host buffers. The reads
     * will only start when the writes are finished. */
    ev3 = ccl_buffer_enqueue_read(
        buf1, q1, CL_FALSE, 0,
        sizeof(cl_int) * CCL_TEST_MAXBUF, h_buf2, NULL, &err);
    g_assert_no_error(err);
    ccl_event_set_name(ev3, "Event3");

    ev4 = ccl_buffer_enqueue_read(
        buf2, q2, CL_FALSE, 0,
        sizeof(cl_int) * CCL_TEST_MAXBUF, h_buf1, NULL, &err);
    g_assert_no_error(err);
    ccl_event_set_name(ev4, "Event4");

    /* Add read events to the event wait list and wait for reads to finish. */
    ccl_event_wait_list_add(&ewl, ev3, ev4, NULL);
    ccl_event_wait(&ewl, &err);
    g_assert_no_error(err);

    /* Add queues for profiling. */
    ccl_prof_add_queue(prof, "Q1", q1);
    ccl_prof_add_queue(prof, "Q2", q2);

    /* Perform profiling calculations. */
    ccl_prof_calc(prof, &err);
    g_assert_no_error(err);

    /* **************** */
    /* Test event names */
    /* **************** */

    g_assert_cmpstr(ccl_event_get_name(ev1), ==, "Event1");
    g_assert_cmpstr(ccl_event_get_name(ev2), ==, "Event2");
    g_assert_cmpstr(ccl_event_get_name(ev3), ==, "Event3");
    g_assert_cmpstr(ccl_event_get_name(ev4), ==, "Event4");

    /* ************************* */
    /* Test aggregate statistics */
    /* ************************* */

    /* 1) Directly. */
    const CCLProfAgg * agg;

    agg = ccl_prof_get_agg(prof, "Event1");
    g_assert(agg != NULL);
    g_assert_cmpuint(agg->absolute_time, >=, 0);
    g_assert_cmpfloat(agg->relative_time, >=, 0);

    agg = ccl_prof_get_agg(prof, "Event2");
    g_assert(agg != NULL);
    g_assert_cmpuint(agg->absolute_time, >=, 0);
    g_assert_cmpfloat(agg->relative_time, >=, 0);

    agg = ccl_prof_get_agg(prof, "Event3");
    g_assert(agg != NULL);
    g_assert_cmpuint(agg->absolute_time, >=, 0);
    g_assert_cmpfloat(agg->relative_time, >=, 0);

    agg = ccl_prof_get_agg(prof, "Event4");
    g_assert(agg != NULL);
    g_assert_cmpuint(agg->absolute_time, >=, 0);
    g_assert_cmpfloat(agg->relative_time, >=, 0);

    /* 2) By cycling all aggregate stats. */
    const char * prev_name = "zzzz";

    ccl_prof_iter_agg_init(prof, CCL_PROF_AGG_SORT_NAME | CCL_PROF_SORT_DESC);

    while ((agg = ccl_prof_iter_agg_next(prof)) != NULL) {

        /* Just check that the event names are ordered properly. */
        g_assert_cmpstr(agg->event_name, <=, prev_name);
        prev_name = agg->event_name;
    }

    /* **************** */
    /* Test event infos */
    /* **************** */

    prev_name = "0000";
    const CCLProfInfo * info;

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

    const CCLProfInst * pi;
    cl_ulong prev_inst = 0;
    ccl_prof_iter_inst_init(
        prof, CCL_PROF_INST_SORT_INSTANT | CCL_PROF_SORT_ASC);

    while ((pi = ccl_prof_iter_inst_next(prof)) != NULL) {

        /* Check that previous instant occurred before current one. */
        g_assert_cmpuint(prev_inst, <=, pi->instant);
        prev_inst = pi->instant;
    }

     /* ************* */
    /* Test overlaps */
    /* ************* */

    const CCLProfOverlap * o;
    ccl_prof_iter_overlap_init(prof, CCL_PROF_OVERLAP_SORT_DURATION |
        CCL_PROF_SORT_DESC);
    while ((o = ccl_prof_iter_overlap_next(prof)) != NULL) {

        /* Check for impossible overlaps. */
        g_assert(!ccl_test_prof_is_overlap("Event1", "Event3"));
        g_assert(!ccl_test_prof_is_overlap("Event1", "Event4"));
        g_assert(!ccl_test_prof_is_overlap("Event2", "Event3"));
        g_assert(!ccl_test_prof_is_overlap("Event2", "Event4"));

        /* Check for possible overlaps. */
        if (ccl_test_prof_is_overlap("Event1", "Event2")) {
            g_assert_cmpuint(o->duration, >=, 0);
        } else if (ccl_test_prof_is_overlap("Event3", "Event4")) {
            g_assert_cmpuint(o->duration, >=, 0);
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
    gchar * tmp_dir_name, * tmp_file_name;
    tmp_dir_name = g_dir_make_tmp("test_profiler_XXXXXX", &err);
    g_assert_no_error(err);
    tmp_file_name = g_strconcat(
        tmp_dir_name, G_DIR_SEPARATOR_S, "export.tsv", NULL);

    cl_bool export_status = ccl_prof_export_info_file(
        prof, tmp_file_name, &err);
    g_assert_no_error(err);
    g_assert(export_status);

    /* Test if output file was correctly written. */
    gchar * file_contents;
    gboolean read_flag = g_file_get_contents(
        tmp_file_name, &file_contents, NULL, NULL);
    g_assert(read_flag);
    g_assert(g_strrstr(file_contents, "Event1"));
    g_assert(g_strrstr(file_contents, "Event2"));
    g_assert(g_strrstr(file_contents, "Event3"));
    g_assert(g_strrstr(file_contents, "Event4"));
    g_free(file_contents);
    g_free(tmp_dir_name);
    g_free(tmp_file_name);

    /* Print summary to debug output. */
    const char * summary = ccl_prof_get_summary(prof,
        CCL_PROF_AGG_SORT_TIME | CCL_PROF_SORT_DESC,
        CCL_PROF_OVERLAP_SORT_DURATION | CCL_PROF_SORT_DESC);

    g_debug("\n%s", summary);

    /* ******************* */
    /* Clean and terminate */
    /* ******************* */

    /* Free profiler object. */
    ccl_prof_destroy(prof);

    /* Free buffer wrappers. */
    ccl_buffer_destroy(buf2);
    ccl_buffer_destroy(buf1);

    /* Free queue wrappers. */
    ccl_queue_destroy(q2);
    ccl_queue_destroy(q1);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert(!ccl_wrapper_memcheck());

    /* Free context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char ** argv) {

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/profiler/create-add-destroy", create_add_destroy_test);
    g_test_add_func("/profiler/features", features_test);

    return g_test_run();

}
