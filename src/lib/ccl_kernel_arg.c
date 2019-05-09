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
 * Implementation of a wrapper type and related functions for a OpenCL kernel
 * arguments.
 *
 * @author Nuno Fachada
 * @date 2019
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#include "ccl_kernel_arg.h"
#include "_ccl_abstract_wrapper.h"

/**
 * Determine if argument is local/private.
 *
 * @internal
 *
 * @param[in] arg Kernel argument.
 * @return True if argument is local or private, false if argument is
 * a real ::CCLWrapper object.
 * */
#define ccl_arg_is_local(arg) \
     (arg->info == (void *) &arg_local_marker)

/**
 * Marker which determines if argument is local/private or a
 * real ::CCLWrapper object.
 *
 * @internal
 * */
static char arg_local_marker;

/**
 * This variables defines a kernel argument to be skiped in
 * ::ccl_kernel_set_args() and ::ccl_kernel_set_args_v() functions.
 * Client code should use the ::ccl_arg_skip global variable.
 *
 * @internal
 * */
static const CCLArg arg_skip = { CCL_NONE, NULL, NULL, 0 };

/* Use this constant to skip kernel arguments in ::ccl_kernel_set_args()
 * and ::ccl_kernel_set_args_v() functions. */
CCL_EXPORT
const CCLArg * ccl_arg_skip = &arg_skip;

/**
 * Create a new kernel argument.
 *
 * Arguments created with this function can local, private or NULL.
 * Client code shouldn't directly use this function, but use instead
 * ccl_arg_priv(), ccl_arg_local() or ccl_arg_full().
 *
 * @param[in] value Argument value.
 * @param[in] size Argument size.
 * @return A new kernel argument.
 * */
CCL_EXPORT
CCLArg * ccl_arg_new(void * value, size_t size) {

    /* Make sure size is > 0. */
    g_return_val_if_fail(size > 0, NULL);

    CCLArg * arg = g_slice_new(CCLArg);

    arg->cl_object = g_memdup((const void *) value, (guint) size);
    arg->info = (void *) &arg_local_marker;
    arg->ref_count = (gint) size;

    return arg;
}

/**
 * Destroy a kernel argument.
 *
 * Client code shouldn't directly use this function.
 *
 * @internal
 *
 * @param[in] arg Argument to destroy.
 * */
CCL_EXPORT
void ccl_arg_destroy(CCLArg * arg) {

    /* Make sure arg is not NULL. */
    g_return_if_fail(arg != NULL);

    if ccl_arg_is_local(arg) {
        g_free(arg->cl_object);
        g_slice_free(CCLArg, arg);
    }
}

/**
 * Get size in bytes of kernel argument.
 *
 * Client code shouldn't directly use this function.
 *
 * @internal
 *
 * @param[in] arg Argument to get size of.
 * @return Argument size in bytes.
 * */
CCL_EXPORT
size_t ccl_arg_size(CCLArg * arg) {

    /* Make sure arg is not NULL. */
    g_return_val_if_fail(arg != NULL, 0);

    return ccl_arg_is_local(arg)
        ? (size_t) arg->ref_count
        : sizeof(void *);
}

/**
 * Get value of kernel argument.
 *
 * Client code shouldn't directly use this function.
 *
 * @internal
 *
 * @param[in] arg Argument to get value of.
 * @return Argument value.
 * */
CCL_EXPORT
void* ccl_arg_value(CCLArg * arg) {

    /* Make sure arg is not NULL. */
    g_return_val_if_fail(arg != NULL, NULL);

    return ccl_arg_is_local(arg)
        ? arg->cl_object
        : &arg->cl_object;
}
