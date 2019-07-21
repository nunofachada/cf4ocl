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
 * Test the command queue wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>
#include "test.h"
#include "_ccl_defs.h"

/**
 * @internal
 *
 * @brief Tests creation (using "simple" constructor), getting info from and
 * destruction of command queue wrapper objects.
 * */
static void create_info_destroy_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLContext * ctx_aux = NULL;
    cl_context context = NULL;
    CCLDevice * dev = NULL;
    CCLDevice * dev_aux = NULL;
    cl_device_id device = NULL;
    CCLQueue * cq = NULL;
    cl_command_queue command_queue = NULL;
    CCLErr * err = NULL;
    cl_command_queue_properties prop_probed, prop_used;
    cl_command_queue_properties prop_base = CL_QUEUE_PROFILING_ENABLE;
    const cl_queue_properties prop_full[] =
        { CL_QUEUE_PROPERTIES, prop_base, 0 };
    cl_int ocl_status;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Test four ways to create a queue. */
    for (cl_uint i = 0; i < 4; ++i) {

        /* Create command queue wrapper. */
        switch (i) {
            case 0:
                /* The regular way. */
                prop_used = prop_base;
                cq = ccl_queue_new(ctx, dev, prop_base, &err);
                g_assert_no_error(err);
                break;
            case 1:
                /* Using the "full" constructor. */
                prop_used = prop_base;
                cq = ccl_queue_new_full(ctx, dev, prop_full, &err);
                g_assert_no_error(err);
                break;
            case 2:
                /* Using the "full" constructor with NULL properties. */
                prop_used = 0;
                cq = ccl_queue_new_full(ctx, dev, NULL, &err);
                g_assert_no_error(err);
                break;
            case 3:
                /* Using the "wrap" constructor. */
                prop_used = prop_base;
                CCL_BEGIN_IGNORE_DEPRECATIONS
                command_queue = clCreateCommandQueue(
                    ccl_context_unwrap(ctx), ccl_device_unwrap(dev),
                    prop_base, &ocl_status);
                g_assert_cmpint(ocl_status, ==, CL_SUCCESS);
                CCL_END_IGNORE_DEPRECATIONS
                cq = ccl_queue_new_wrap(command_queue);
                g_assert_cmphex(GPOINTER_TO_SIZE(command_queue), ==,
                    GPOINTER_TO_SIZE(ccl_queue_unwrap(cq)));
                break;
        }

        /* Get some info and check if the return value is as expected. */
        context = ccl_queue_get_info_scalar(
            cq, CL_QUEUE_CONTEXT, cl_context, &err);
        g_assert_no_error(err);
        g_assert_cmphex(
            GPOINTER_TO_SIZE(context),
            ==,
            GPOINTER_TO_SIZE(ccl_context_unwrap(ctx)));

        ctx_aux = ccl_queue_get_context(cq, &err);
        g_assert_no_error(err);
        g_assert_cmphex(
            GPOINTER_TO_SIZE(ctx), ==, GPOINTER_TO_SIZE(ctx_aux));

        device = ccl_queue_get_info_scalar(
            cq, CL_QUEUE_DEVICE, cl_device_id, &err);
        g_assert_no_error(err);
        g_assert_cmphex(
            GPOINTER_TO_SIZE(device),
            ==,
            GPOINTER_TO_SIZE(ccl_device_unwrap(dev)));

        dev_aux = ccl_queue_get_device(cq, &err);
        g_assert_no_error(err);
        g_assert_cmphex(
            GPOINTER_TO_SIZE(dev), ==, GPOINTER_TO_SIZE(dev_aux));

        prop_probed = ccl_queue_get_info_scalar(
            cq, CL_QUEUE_PROPERTIES, cl_command_queue_properties, &err);
        g_assert_no_error(err);
        g_assert_cmphex(prop_probed, ==, prop_used);

        /* Destroy queue. */
        ccl_queue_destroy(cq);
        cq = NULL;
    }

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert_false(ccl_wrapper_memcheck());

    /* Destroy context. */
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert_true(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests the command queue wrapper class reference counting.
 * */
static void ref_unref_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLQueue * cq = NULL;
    CCLDevice * dev = NULL;
    CCLErr * err = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Create command queue wrapper. */
    cq = ccl_queue_new(ctx, dev, 0, &err);
    g_assert_no_error(err);

    /* Increase queue reference count. */
    ccl_queue_ref(cq);

    /* Check that queue ref count is 2. */
    g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper *) cq));

    /* Unref queue. */
    ccl_queue_unref(cq);

    /* Check that queue ref count is 1. */
    g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper *) cq));

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert_false(ccl_wrapper_memcheck());

    /* Destroy stuff. */
    ccl_queue_unref(cq);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert_true(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Tests the ::ccl_enqueue_barrier() and ::ccl_enqueue_marker()
 * functions.
 * */
static void barrier_marker_test() {

    /* Test variables. */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLQueue * cq = NULL;
    CCLBuffer * buf = NULL;
    CCLEvent * evt_cq = NULL;
    CCLEvent * evt_map = NULL;
    CCLEvent * evt_unmap = NULL;
    CCLEvent * evt_barrier = NULL;
    CCLEvent * evt_marker = NULL;
    CCLErr * err = NULL;
    cl_ulong * host_buf;
    CCLEventWaitList ewl = NULL;
    cl_int exec_status = -1;
    cl_command_type ct = 0;
    cl_uint ocl_ver;
    cl_bool evt_in_cq;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* Get OpenCL version for context. */
    ocl_ver = ccl_context_get_opencl_version(ctx, &err);
    g_assert_no_error(err);

    /* Create a command queue. */
    cq = ccl_queue_new(ctx, dev, 0, &err);
    g_assert_no_error(err);

    /* Create a device buffer. */
    buf = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE, 8 * sizeof(cl_ulong), NULL, &err);
    g_assert_no_error(err);

    /* Map device buffer, get an event and analise it. */
    host_buf = ccl_buffer_enqueue_map(
        buf, cq, CL_FALSE, CL_MAP_WRITE,
        0, 8 * sizeof(cl_ulong), NULL, &evt_map, &err);
    g_assert_no_error(err);

    /* Enqueue a barrier for waiting on the map event. */
    evt_barrier = ccl_enqueue_barrier(cq, ccl_ewl(&ewl, evt_map, NULL), &err);
    g_assert_no_error(err);

    /* Check that the barrier event is CL_COMMAND_BARRIER or
     * CL_COMMAND_MARKER (this depends on the OpenCL version, as well
     * as the specific implementation). */
    ct = ccl_event_get_command_type(evt_barrier, &err);
    g_assert_no_error(err);
    g_assert_true((ct == CL_COMMAND_BARRIER) || (ct == CL_COMMAND_MARKER));

    /* Unmap buffer, get resulting event. */
    evt_unmap = ccl_buffer_enqueue_unmap(buf, cq, host_buf, NULL, &err);
    g_assert_no_error(err);

#ifdef CL_VERSION_1_2
    if (ocl_ver >= 120) {
        /* For OpenCL >= 1.2 the marker event is complete when either
         * a) all previous enqueued events have completed, or b) the
         * events specified in the event wait list have completed. */
        ccl_event_wait_list_add(
            &ewl, evt_map, evt_barrier, evt_unmap, NULL);
        evt_marker = ccl_enqueue_marker(cq, &ewl, &err);
        g_assert_no_error(err);
    } else {
#endif
        /* For OpenCL <= 1.1, the marker event is complete ONLY when all
         * previous enqueued events have completed. */
        CCL_UNUSED(ocl_ver);
        evt_marker = ccl_enqueue_marker(cq, NULL, &err);
        g_assert_no_error(err);
#ifdef CL_VERSION_1_2
    }
#endif

    /* Wait for marker event to complete (which means all previous
     * enqueued events have also completed). */
    ccl_event_wait(ccl_ewl(&ewl, evt_marker, NULL), &err);

    /* Test queue flush. */
    ccl_queue_flush(cq, &err);
    g_assert_no_error(err);

    /* Check that the unmap event is CL_COMPLETE. */
    exec_status = ccl_event_get_info_scalar(
        evt_unmap, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
    g_assert_no_error(err);
    g_assert_cmpint(exec_status, ==, CL_COMPLETE);

    /* Check that the event is a marker event. */
    ct = ccl_event_get_command_type(evt_marker, &err);
    g_assert_no_error(err);
    g_assert_cmpuint(ct, ==, CL_COMMAND_MARKER);

    /* Check that generated events are in queue. */
    CCLEvent * evts[] = {evt_map, evt_barrier, evt_unmap, evt_marker, NULL};
    for (cl_uint i = 0; evts[i] != NULL; ++i) {
        ccl_queue_iter_event_init(cq);
        evt_in_cq = CL_FALSE;
        while ((evt_cq = ccl_queue_iter_event_next(cq)) != NULL) {
            if (evt_cq == evt_map) {
                evt_in_cq = CL_TRUE;
                break;
            }
        }
        g_assert_true(evt_in_cq);
    }

    /* Clear events in queue and check that they were really destroyed. */
    ccl_queue_gc(cq);
    ccl_queue_iter_event_init(cq);
    g_assert_true(ccl_queue_iter_event_next(cq) == NULL);

    /* Confirm that memory allocated by wrappers has not yet been freed. */
    g_assert_false(ccl_wrapper_memcheck());

    /* Release wrappers. */
    ccl_buffer_destroy(buf);
    ccl_queue_destroy(cq);
    ccl_context_destroy(ctx);

    /* Confirm that memory allocated by wrappers has been properly freed. */
    g_assert_true(ccl_wrapper_memcheck());
}

/**
 * @internal
 *
 * @brief Type of wait for multiple queues / out-of-order queues test.
 */
typedef enum { CCL_TEST_WAIT_FOR_EVENTS, CCL_TEST_FINISH } WaitType;

/**
 * @internal
 *
 * @brief Perform the actual tests multiple queues and out-of-order queues.
 * */
static void do_mult_ooo(
    CCLContext * ctx, CCLQueue * cq1, CCLQueue * cq2, WaitType wait_type) {

    /* Variables. */
    CCLBuffer * buf_dev1 = NULL;
    CCLBuffer * buf_dev2 = NULL;
    CCLEvent * ew[2] = { NULL, NULL }, * er[2] = { NULL, NULL };
    CCLEventWaitList ewl = NULL;
    CCLErr * err = NULL;
    cl_uint bufsize = 512;
    cl_float * buf_host1A = (cl_float*) malloc(bufsize * sizeof(cl_float));
    cl_float * buf_host1B = (cl_float*) calloc(bufsize, sizeof(cl_float));
    cl_int * buf_host2A = (cl_int*) malloc(bufsize * sizeof(cl_int));
    cl_int * buf_host2B = (cl_int*) calloc(bufsize, sizeof(cl_int));

    /* Fill host buffers with random data. */
    for (cl_uint i = 0; i < bufsize; i++)
    {
        buf_host1A[i] = (cl_float) g_test_rand_double();
        buf_host2A[i] = (cl_int) g_test_rand_int();
    }

    /* Create device buffers. */
    buf_dev1 = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE, bufsize * sizeof(cl_float), NULL, &err);
    g_assert_no_error(err);

    buf_dev2 = ccl_buffer_new(
        ctx, CL_MEM_READ_WRITE, bufsize * sizeof(cl_int), NULL, &err);
    g_assert_no_error(err);

    /* Write something to device buffer 1 using command queue 1,
     * generate event ew[0]. */
    ew[0] = ccl_buffer_enqueue_write(
        buf_dev1, cq1, CL_FALSE, 0,
        bufsize * sizeof(cl_float), buf_host1A, NULL, &err);
    g_assert_no_error(err);

    /* Write something to device buffer 2 using command queue 2,
     * generate event ew[1]. */
    ew[1] = ccl_buffer_enqueue_write(
        buf_dev2, cq2, CL_FALSE, 0,
        bufsize * sizeof(cl_int), buf_host2A, NULL, &err);
    g_assert_no_error(err);

    /* Read from device buffer 1 using command queue 2, make it depend
     * on event ew[0] and generate event er[0]. */
    er[0] = ccl_buffer_enqueue_read(
        buf_dev1, cq2, CL_FALSE, 0, bufsize * sizeof(cl_float),
        buf_host1B, ccl_ewl(&ewl, ew[0], NULL), &err);
    g_assert_no_error(err);

    /* Read from device buffer 2 using command queue 1, make it depend
     * on event ew[1] and generate event er[1]. */
    er[1] = ccl_buffer_enqueue_read(
        buf_dev2, cq1, CL_FALSE, 0, bufsize * sizeof(cl_int),
        buf_host2B, ccl_ewl(&ewl, ew[1], NULL), &err);
    g_assert_no_error(err);

    /* Wait on host thread for work to finish. */
    switch(wait_type)
    {
        case CCL_TEST_WAIT_FOR_EVENTS:
            /* Wait on host thread for read events. */
            ccl_event_wait(ccl_ewl(&ewl, er[0], er[1], NULL), &err);
            g_assert_no_error(err);
            break;
        case CCL_TEST_FINISH:
            /* Wait on host thread for queues to be processed. */
            ccl_queue_finish(cq1, &err);
            g_assert_no_error(err);
            /* If they're not the same queue, must also wait in the
             * second queue. */
            if (cq1 != cq2)
            {
                ccl_queue_finish(cq2, &err);
                g_assert_no_error(err);
            }
            break;
        default:
            g_assert_not_reached();
    }

    /* Check results. */
    for (cl_uint i = 0; i < bufsize; i++)
    {
        g_assert_cmpfloat(buf_host1A[i], ==, buf_host1B[i]);
        g_assert_cmpint(buf_host2A[i], ==, buf_host2B[i]);
    }

    /* Release stuff. */
    ccl_buffer_destroy(buf_dev1);
    ccl_buffer_destroy(buf_dev2);
    free(buf_host1A);
    free(buf_host1B);
    free(buf_host2A);
    free(buf_host2B);
}

/**
 * @internal
 *
 * @brief Tests multiple queues and out-of-order queues.
 * */
static void mult_ooo_test() {

    /* Variables.  */
    CCLContext * ctx = NULL;
    CCLDevice * dev = NULL;
    CCLErr * err = NULL;
    CCLQueue * cq1 = NULL, * cq2 = NULL, * oocq = NULL;

    /* Get the test context with the pre-defined device. */
    ctx = ccl_test_context_new(0, &err);
    g_assert_no_error(err);

    /* Get first device in context. */
    dev = ccl_context_get_device(ctx, 0, &err);
    g_assert_no_error(err);

    /* ************************** */
    /* Test 1: Two command queues */
    /* ************************** */

    /* Create first command queue */
    cq1 = ccl_queue_new(ctx, dev, 0,  &err);
    g_assert_no_error(err);

    /* Create second command queue */
    cq2 = ccl_queue_new(ctx, dev, 0,  &err);
    g_assert_no_error(err);

    /* Test 1.1: Perform test with two different command queues and explicitly
     * waiting for events. */
    do_mult_ooo(ctx, cq1, cq2, CCL_TEST_WAIT_FOR_EVENTS);

    /* Test 1.2: Perform test with two different command queues and wait for
     * the second queue to finish.  */
    do_mult_ooo(ctx, cq1, cq2, CCL_TEST_FINISH);

    /* Release command queues. */
    ccl_queue_destroy(cq2);
    ccl_queue_destroy(cq1);

    /* ************************************** */
    /* Test 2: One out-of-order command queue */
    /* ************************************** */

    /* Does device support out-of-order queues? */
    cl_command_queue_properties qprops =
        ccl_device_get_info_scalar(
            dev, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES,
            cl_command_queue_properties, &err);
    g_assert_no_error(err);

    /* If so, test out-of-order command queues. */
    if (qprops & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
    {
        /* Create an out-of-order command queue. */
        oocq = ccl_queue_new(
            ctx, dev, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
        g_assert_no_error(err);

        /* Test 2.1: Perform test with out-of-order command queue and
         * explicitly waiting for events. */
        do_mult_ooo(ctx, oocq, oocq, CCL_TEST_WAIT_FOR_EVENTS);

        /* Test 2.2: Perform test with out-of-order command queue and wait for
         * queue to finish. */
        do_mult_ooo(ctx, oocq, oocq, CCL_TEST_FINISH);

        /* Release command queue. */
        ccl_queue_destroy(oocq);
    }

    /* ******** */
    /* Clean-up */
    /* ******** */

    /* Release context. */
    ccl_context_destroy(ctx);
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

    g_test_add_func(
        "/wrappers/queue/create-info-destroy",
        create_info_destroy_test);

    g_test_add_func(
        "/wrappers/queue/ref-unref",
        ref_unref_test);

    g_test_add_func(
        "/wrappers/queue/barrier-marker",
        barrier_marker_test);

    g_test_add_func(
        "/wrappers/queue/mult-ooo",
        mult_ooo_test);

    return g_test_run();
}
