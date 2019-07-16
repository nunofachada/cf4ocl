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
 * Implementation of an abstract wrapper class and its methods for OpenCL
 * objects.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_abstract_wrapper.h"
#include "_ccl_abstract_wrapper.h"
#include "_ccl_kernel_wrapper.h"
#include "_ccl_defs.h"

/* Generic function pointer for OpenCL clget**Info() functions. */
typedef cl_int (*ccl_wrapper_info_fp)(void);

/* Table of all existing wrappers. */
static GHashTable * wrappers = NULL;
/* Define lock for synchronizing access to table of all existing wrappers. */
G_LOCK_DEFINE(wrappers);

/* Wrapper names ordered by their enum type. */
static const char * ccl_class_names[] = {"Buffer", "Context", "Device", "Event",
    "Image", "Kernel", "Platform", "Program", "Sampler", "Queue", "None", NULL};

/* Information functions. They must be in the same order as defined in the
 * CCLInfo enum. */
static const ccl_wrapper_info_fp info_funs[] = {
    (ccl_wrapper_info_fp) clGetContextInfo,
    (ccl_wrapper_info_fp) clGetDeviceInfo,
    (ccl_wrapper_info_fp) clGetEventInfo,
    (ccl_wrapper_info_fp) clGetEventProfilingInfo,
    (ccl_wrapper_info_fp) clGetImageInfo,
    (ccl_wrapper_info_fp) clGetKernelInfo,
#ifdef CL_VERSION_1_2
    (ccl_wrapper_info_fp) ccl_kernel_get_arg_info_adapter,
#else
    NULL,
#endif
    (ccl_wrapper_info_fp) clGetKernelWorkGroupInfo,
#ifdef CL_VERSION_2_1
    NULL /* clKernelSubGroupInfo - not implemented yet. */,
#else
    NULL,
#endif
    (ccl_wrapper_info_fp) clGetMemObjectInfo,
    (ccl_wrapper_info_fp) clGetPlatformInfo,
    (ccl_wrapper_info_fp) clGetProgramInfo,
    (ccl_wrapper_info_fp) clGetProgramBuildInfo,
    (ccl_wrapper_info_fp) clGetSamplerInfo,
    (ccl_wrapper_info_fp) clGetCommandQueueInfo,
#ifdef CL_VERSION_2_0
    NULL /* clGetPipeInfo - not implemented yet. */
#else
    NULL,
#endif
};

/**
 * Information about wrapped OpenCL objects.
 * */
struct ccl_wrapper_info_table {

    /**
     * Table containing information about the wrapped OpenCL object.
     * @private
     * */
    GHashTable * table;

    /**
     * List of replaced information about the wrapped OpenCL object.
     * @private
     * */
    GSList * old_info;

    /**
     * Mutex for controlling thread access to the OpenCL object
     * information table.
     * @private
     * */
    GMutex mutex;

};

/* ********************************* */
/* ****** Protected methods ******** */
/* ********************************* */

/**
 * Create a new ::CCLWrapper object. This function is called by the concrete
 * wrapper constructors.
 *
 * @protected @memberof ccl_wrapper
 *
 * @param[in] class Class or type of OpenCL object to wrap.
 * @param[in] cl_object OpenCL object to wrap.
 * @param[in] size Size in bytes of wrapper.
 * @return A new wrapper object.
 * */
CCLWrapper * ccl_wrapper_new(CCLClass class, void * cl_object, size_t size) {

    /* Make sure OpenCL object is not NULL. */
    g_return_val_if_fail(cl_object != NULL, NULL);

    /* The new wrapper object. */
    CCLWrapper * w;

    /* Lock access to table of all existing wrappers. */
    G_LOCK(wrappers);

    /* If table of all existing wrappers is not yet initialized,
     * initialize it. */
    if (wrappers == NULL) {
        wrappers = g_hash_table_new_full(
            g_direct_hash, g_direct_equal, NULL, NULL);
    }

    /* Check if requested wrapper already exists, and get it if so. */
    w = g_hash_table_lookup(wrappers, cl_object);

    if (w == NULL) {

        /* Wrapper doesn't yet exist, create it. */
        w = (CCLWrapper *) g_slice_alloc0(size);
        w->class = class;
        w->cl_object = cl_object;

        /* Initialize info table. */
        w->info = g_slice_new0(struct ccl_wrapper_info_table);
        g_mutex_init(&w->info->mutex);

        /* Insert newly created wrapper in table of all existing
         * wrappers. */
        g_hash_table_insert(wrappers, cl_object, w);

    }

    /* Increase reference count of wrapper. */
    ccl_wrapper_ref(w);

    /* Unlock access to table of all existing wrappers. */
    G_UNLOCK(wrappers);

    /* Return requested wrapper. */
    return w;
}

/**
 * @internal
 *
 * @brief Decrements the reference count of the wrapper object.
 * If it reaches 0, the wrapper object is destroyed.
 *
 * @protected @memberof ccl_wrapper
 *
 * @param[in] wrapper The wrapper object.
 * @param[in] size Size in bytes of wrapper object.
 * @param[in] rel_fields_fun Function for releasing specific wrapper
 * fields.
 * @param[in] rel_cl_fun Function for releasing OpenCL object.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored. The only error which may be reported by
 * this function is if some problem occurred when releasing the OpenCL
 * object.
 * @return `CL_TRUE` if wrapper was destroyed (i.e. its ref. count reached
 * zero), `CL_FALSE` otherwise.
 * */
cl_bool ccl_wrapper_unref(CCLWrapper * wrapper, size_t size,
    ccl_wrapper_release_fields rel_fields_fun,
    ccl_wrapper_release_cl_object rel_cl_fun, CCLErr ** err) {

    /* Make sure wrapper object is not NULL. */
    g_return_val_if_fail(wrapper != NULL, CL_FALSE);

    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, CL_FALSE);

    /* Flag which indicates if wrapper was destroyed or not. */
    cl_bool destroyed = CL_FALSE;

    /* OpenCL status flag. */
    cl_int ocl_status;

#ifdef CCL_DEBUG_OBJ_LIFETIME

    /* Log destruction/unreferencing of wrapper. */
    g_debug("Destroy/unref. CCL%s(%p)",
        ccl_wrapper_get_class_name(wrapper), (void *) wrapper->cl_object);

#endif

    /* Decrement reference count and check if it reaches 0. */
    if (g_atomic_int_dec_and_test(&wrapper->ref_count)) {

        /* Ref. count reached 0, so wrapper will be destroyed. */
        destroyed = CL_TRUE;

        /* Release the OpenCL wrapped object. */
        if (rel_cl_fun != NULL) {
            ocl_status = rel_cl_fun(wrapper->cl_object);
            if (ocl_status != CL_SUCCESS) {
                g_set_error(err, CCL_OCL_ERROR, ocl_status,
                "%s: unable to release OpenCL object "
                "(OpenCL error %d: %s).",
                CCL_STRD, ocl_status, ccl_err(ocl_status));
            }
        }

        /* Destroy table containing wrapped object information. */
        if (wrapper->info->table != NULL) {
            g_hash_table_destroy(wrapper->info->table);
        }
        if (wrapper->info->old_info != NULL) {
            g_slist_free_full(wrapper->info->old_info,
                (GDestroyNotify) ccl_wrapper_info_destroy);
        }
        g_mutex_clear(&wrapper->info->mutex);
        g_slice_free(struct ccl_wrapper_info_table, wrapper->info);

        /* Remove wrapper from static table, release static table if
         * empty. */
        G_LOCK(wrappers);
        g_hash_table_remove(wrappers, wrapper->cl_object);
        if (g_hash_table_size(wrappers) == 0) {
            g_hash_table_destroy(wrappers);
            wrappers = NULL;
        }
        G_UNLOCK(wrappers);

        /* Destroy remaining wrapper fields. */
        if (rel_fields_fun != NULL)
            rel_fields_fun(wrapper);

        /* Destroy wrapper. */
        g_slice_free1(size, wrapper);

    }

    /* Return flag indicating if wrapper was destroyed. */
    return destroyed;
}

/**
 * @internal
 *
 * @brief Add a ::CCLWrapperInfo object to the info table of the given wrapper.
 *
 * @protected @memberof ccl_wrapper
 *
 * @param[in] wrapper Wrapper to add info to.
 * @param[in] param_name Name of parameter which will refer to this
 * info.
 * @param[in] info Info object to add.
 * */
void ccl_wrapper_add_info(CCLWrapper * wrapper, cl_uint param_name,
    CCLWrapperInfo * info) {

    /* Make sure wrapper is not NULL. */
    g_return_if_fail(wrapper != NULL);
    /* Make sure info is not NULL. */
    g_return_if_fail(info != NULL);

    /* Lock access to info table. */
    g_mutex_lock(&wrapper->info->mutex);

    /* If information table is not yet initialized, then
     * initialize it. */
    if (wrapper->info->table == NULL) {
        wrapper->info->table = g_hash_table_new_full(
            g_direct_hash, g_direct_equal,
            NULL, (GDestroyNotify) ccl_wrapper_info_destroy);
    }

    /* Check if information with same key as already present in
     * table... */
    if (g_hash_table_contains(
            wrapper->info->table, GUINT_TO_POINTER(param_name))) {

        /* ...if so, move this information to the old information
         * table. */

        /* Get existing information... */
        CCLWrapperInfo * info_old =
            (CCLWrapperInfo *) g_hash_table_lookup(
                wrapper->info->table, GUINT_TO_POINTER(param_name));

        /* ...and put it in table of old information. */
        wrapper->info->old_info =
            g_slist_prepend(wrapper->info->old_info, info_old);

        /* Remove old info from info table without destroying it. */
        g_hash_table_steal(
            wrapper->info->table, GUINT_TO_POINTER(param_name));

    }

    /* Keep new information in information table. */
    g_hash_table_insert(wrapper->info->table,
        GUINT_TO_POINTER(param_name), info);

    /* Unlock access to info table. */
    g_mutex_unlock(&wrapper->info->mutex);
}

/**
 * @internal
 *
 * @brief Create a new ::CCLWrapperInfo object with a given value size.
 *
 * @protected @memberof ccl_wrapper_info
 *
 * @param[in] size Parameter size in bytes.
 * @return A new CCLWrapperInfo * object.
 * */
CCLWrapperInfo * ccl_wrapper_info_new(size_t size) {

    CCLWrapperInfo * info = g_slice_new(CCLWrapperInfo);

    if (size > 0)
        info->value = g_slice_alloc0(size);
    else
        info->value = NULL;
    info->size = size;

    return info;

}

/**
 * @internal
 *
 * @brief Destroy a ::CCLWrapperInfo object.
 *
 * @protected @memberof ccl_wrapper_info
 *
 * @param[in] info Object to destroy.
 * */
void ccl_wrapper_info_destroy(CCLWrapperInfo * info) {

    /* Make sure info is not NULL. */
    g_return_if_fail(info != NULL);

    if (info->size > 0)
        g_slice_free1(info->size, info->value);
    g_slice_free(CCLWrapperInfo, info);

}

/* ********************************* */
/* ******** Public methods ********* */
/* ********************************* */

/**
 * Increase the reference count of the wrapper object.
 *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper The wrapper object.
 * */
CCL_EXPORT
void ccl_wrapper_ref(CCLWrapper * wrapper) {

    /* Make sure wrapper object is not NULL. */
    g_return_if_fail(wrapper != NULL);

    /* Increment wrapper reference count. */
    g_atomic_int_inc(&wrapper->ref_count);

#ifdef CCL_DEBUG_OBJ_LIFETIME

    /* Log creation/referencing of wrapper. */
    g_debug("New/ref. CCL%s(%p)",
        ccl_wrapper_get_class_name(wrapper), (void *) wrapper->cl_object);

#endif

}

/**
 * Returns the wrapper object reference count. For debugging and
 * testing purposes only.
 *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper The wrapper object.
 * @return The wrapper object reference count or -1 if wrapper is `NULL`.
 * */
CCL_EXPORT
int ccl_wrapper_ref_count(CCLWrapper * wrapper) {

    /* Make sure wrapper is not NULL. */
    g_return_val_if_fail(wrapper != NULL, -1);

    /* Return reference count. */
    return wrapper->ref_count;
}

/**
 * Get the wrapped OpenCL object.
 *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper The wrapper object.
 * @return The wrapped OpenCL object.
 * */
CCL_EXPORT
void * ccl_wrapper_unwrap(CCLWrapper * wrapper) {

    /* Make sure wrapper is not NULL. */
    g_return_val_if_fail(wrapper != NULL, NULL);

    /* Return the OpenCL wrapped object. */
    return wrapper->cl_object;
}

/**
 * Get information about any wrapped OpenCL object.
 *
 * This function should not be directly invoked in most circumstances. Use the
 * `ccl_*_get_info_*()` macros instead.
 *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper1 The wrapper object to query.
 * @param[in] wrapper2 A second wrapper object, required in some
 * queries.
 * @param[in] param_name Name of information/parameter to get.
 * @param[in] min_size Minimum size of returned information object in
 * case of error.
 * @param[in] info_type Type of information query to perform.
 * @param[in] use_cache `CL_TRUE` if cached information is to be used,
 * `CL_FALSE` to force a new query even if information is in cache.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested information object. This object will
 * be automatically freed when the respective wrapper object is
 * destroyed. If an error occurs, either `NULL` (if `min_size == 0`), or
 * a `min_size`d information object is returned (if `min_size > 0`).
 * */
CCL_EXPORT
CCLWrapperInfo * ccl_wrapper_get_info(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err) {

    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail((err) == NULL || *(err) == NULL, NULL);

    /* Make sure wrapper1 is not NULL. */
    g_return_val_if_fail(wrapper1 != NULL, NULL);

    /* Make sure info_type has a valid value. */
    g_return_val_if_fail((info_type >= 0) && (info_type < CCL_INFO_END), NULL);

    /* Information object. */
    CCLWrapperInfo * info = NULL;

    /* Does info table cache contain requested information? */
    gboolean contains;

    /* Information function to use. */
    ccl_wrapper_info_fp info_fun = info_funs[info_type];

    /* Check if info table cache contains requested information. */
    g_mutex_lock(&wrapper1->info->mutex);
    contains = wrapper1->info->table != NULL
        ? g_hash_table_contains(
            wrapper1->info->table, GUINT_TO_POINTER(param_name))
        : FALSE;
    g_mutex_unlock(&wrapper1->info->mutex);

    /* Check if it is required to query OpenCL object, i.e. if info
     * table cache is not to be used or info table cache does not
     * contain requested info.  */
    if ((!use_cache) || (!contains)) {

        /* Let's query OpenCL object.*/
        cl_int ocl_status;
        /* Size of device information in bytes. */
        size_t size_ret = 0;

        /* Get size of information. */
        ocl_status = (wrapper2 == NULL)
            ? ((ccl_wrapper_info_fp1) info_fun)(wrapper1->cl_object,
                param_name, 0, NULL, &size_ret)
            : ((ccl_wrapper_info_fp2) info_fun)(wrapper1->cl_object,
                wrapper2->cl_object, param_name, 0, NULL, &size_ret);

        /* Avoid bug in Apple OpenCL implementation. */
#if defined(__APPLE__) || defined(__MACOSX)
        if ((ocl_status == CL_INVALID_VALUE)
            && (info_fun == (ccl_wrapper_info_fp) clGetEventProfilingInfo))
            ocl_status = CL_SUCCESS;
#endif

        g_if_err_create_goto(*err, CCL_OCL_ERROR,
            CL_SUCCESS != ocl_status, ocl_status, error_handler,
            "%s: get info [size] (OpenCL error %d: %s).",
            CCL_STRD, ocl_status, ccl_err(ocl_status));
        g_if_err_create_goto(*err, CCL_ERROR, size_ret == 0,
            CCL_ERROR_INFO_UNAVAILABLE_OCL, error_handler,
            "%s: the requested info is unavailable (info size is 0).",
            CCL_STRD);

        /* Allocate memory for information. */
        info = ccl_wrapper_info_new(size_ret);

        /* Get information. */
        ocl_status = (wrapper2 == NULL)
            ? ((ccl_wrapper_info_fp1) info_fun)(wrapper1->cl_object,
                param_name, size_ret, info->value, NULL)
            : ((ccl_wrapper_info_fp2) info_fun)(wrapper1->cl_object,
                wrapper2->cl_object, param_name, size_ret, info->value,
                NULL);
        g_if_err_create_goto(*err, CCL_OCL_ERROR,
            CL_SUCCESS != ocl_status, ocl_status, error_handler,
            "%s: get context info [info] (OpenCL error %d: %s).",
            CCL_STRD, ocl_status, ccl_err(ocl_status));

        /* Keep information in information table. */
        ccl_wrapper_add_info(wrapper1, param_name, info);

    } else {

        /* Requested info is already present in the info table,
         * retrieve it from there. */
        g_mutex_lock(&wrapper1->info->mutex);
        info = g_hash_table_lookup(
            wrapper1->info->table, GUINT_TO_POINTER(param_name));
        g_mutex_unlock(&wrapper1->info->mutex);

    }

    /* If we got here, everything is OK. */
    g_assert(err == NULL || *err == NULL);
    goto finish;

error_handler:
    /* If we got here there was an error, verify that it is so. */
    g_assert(err == NULL || *err != NULL);

    /* In case of error, return an all-zeros info if min_size is > 0. */
    if (min_size > 0) {
        if (info != NULL) ccl_wrapper_info_destroy(info);
        info = ccl_wrapper_info_new(min_size);
        ccl_wrapper_add_info(wrapper1, param_name, info);
    }

finish:

    /* Return the requested information. */
    return info;
}

/**
 * Get pointer to information value.
 *
 * This function should not be directly invoked in most circumstances. Use the
 * `ccl_*_get_info_*()` macros instead.
 *  *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper1 The wrapper object to query.
 * @param[in] wrapper2 A second wrapper object, required in some
 * queries.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] min_size Minimum size of returned value in case of error.
 * @param[in] info_type Type of information query to perform.
 * @param[in] use_cache `CL_TRUE` if cached information is to be used,
 * `CL_FALSE` to force a new query even if information is in cache.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return A pointer to the requested information value. This
 * value will be automatically freed when the wrapper object is
 * destroyed. If an error occurs, either `NULL` (if `min_size == 0`), or
 * a pointer to a `min_size`d zero value is returned (if `min_size > 0`).
 * */
CCL_EXPORT
void * ccl_wrapper_get_info_value(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err) {

    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, NULL);

    /* Make sure wrapper1 is not NULL. */
    g_return_val_if_fail(wrapper1 != NULL, NULL);

    /* Make sure info_type has a valid value. */
    g_return_val_if_fail((info_type >= 0) && (info_type < CCL_INFO_END), NULL);

    /* Get information object. */
    CCLWrapperInfo * diw = ccl_wrapper_get_info(wrapper1, wrapper2,
        param_name, min_size, info_type, use_cache, err);

    /* Return value if information object is not NULL. */
    return diw != NULL ? diw->value : NULL;
}

/**
 * Get information size.
 *
 * @public @memberof ccl_wrapper
 *
 * This function should not be directly invoked in most circumstances.
 *
 * @param[in] wrapper1 The wrapper object to query.
 * @param[in] wrapper2 A second wrapper object, required in some
 * queries.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] min_size Minimum size returned in case of error.
 * @param[in] info_type Type of information query to perform.
 * @param[in] use_cache `CL_TRUE` if cached information is to be used,
 * `CL_FALSE` to force a new query even if information is in cache.
 * @param[out] err Return location for a ::CCLErr object, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested information size. If an error occurs, a size of
 * `min_size` is returned.
 * */
CCL_EXPORT
size_t ccl_wrapper_get_info_size(CCLWrapper * wrapper1,
    CCLWrapper * wrapper2, cl_uint param_name, size_t min_size,
    CCLInfo info_type, cl_bool use_cache, CCLErr ** err) {

    /* Make sure err is NULL or it is not set. */
    g_return_val_if_fail(err == NULL || *err == NULL, 0);

    /* Make sure wrapper1 is not NULL. */
    g_return_val_if_fail(wrapper1 != NULL, 0);

    /* Make sure info_type has a valid value. */
    g_return_val_if_fail((info_type >= 0) && (info_type < CCL_INFO_END), 0);

    /* Get information object. */
    CCLWrapperInfo * diw = ccl_wrapper_get_info(wrapper1, wrapper2,
        param_name, min_size, info_type, use_cache, err);

    /* Return value if information object is not NULL. */
    return diw != NULL ? diw->size : 0;
}

/**
 * Debug function which checks if memory allocated by wrappers has been
 * properly freed.
 *
 * @public @memberof ccl_wrapper
 *
 * This function is merely a debug helper and shouldn't replace proper leak
 * checks with Valgrind or similar tool.
 *
 * @return `CL_TRUE` if memory allocated by wrappers has been properly freed,
 * `CL_FALSE` otherwise.
 */
CCL_EXPORT
cl_bool ccl_wrapper_memcheck() {

    /* Check return variable. */
    cl_bool check;

#ifndef NDEBUG

    /* Iterator over existing wrappers. */
    GHashTableIter iter;

    /* Current wrapper. */
    CCLWrapper * obj = NULL;

    /* Current wrapper address. */
    gpointer addr;

    /* Log string. */
    GString * logstr = NULL;

#endif

    /* Lock access to wrappers variable. */
    G_LOCK(wrappers);

    /* Check if wrappers variable is set. */
    check = (wrappers == NULL);

#ifndef NDEBUG

    /* In debug mode, log existing wrappers. */
    if (check) {

        /* Wrappers table is empty. */
        g_debug("Wrappers table is empty");

    } else {

        /* Wrappers table is not empty, list them. */

        /* Initialize iterator. */
        g_hash_table_iter_init(&iter, wrappers);

        /* Initialize log string. */
        logstr = g_string_new("");
        g_string_append_printf(logstr,
            "There are %u wrappers in table: ", g_hash_table_size(wrappers));

        /* Iterate over existing wrappers... */
        while(g_hash_table_iter_next(&iter, &addr, (gpointer) &obj)) {

            /*...and add their name and address to log string. */
            g_string_append_printf(logstr, "\n%s(%p) ",
                ccl_wrapper_get_class_name(obj), addr);

        }

        /* Log existing wrappers.*/
        g_debug("%s\n", logstr->str);

        /* Release string. */
        g_string_free(logstr, TRUE);

    }

#endif

    /* Unlock access to wrappers variable. */
    G_UNLOCK(wrappers);

    /* Return check. */
    return check;
}

/**
 * Get wrapper class or type name.
 *
 * @public @memberof ccl_wrapper
 *
 * @param[in] wrapper Wrapper object.
 *
 * @return Wrapper class or type name.
 * */
CCL_EXPORT
const char * ccl_wrapper_get_class_name(CCLWrapper * wrapper) {

    /* Make sure wrapper is not NULL. */
    g_return_val_if_fail(wrapper != NULL, NULL);

    /* Make sure class enum value is within bounds. */
    g_return_val_if_fail(
        (wrapper->class >= 0) && (wrapper->class < CCL_NONE), NULL);

    /* Return wrapper class name. */
    return ccl_class_names[wrapper->class];

}
