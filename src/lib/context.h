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
 * @brief OpenCL context wrapper.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef CL4_CONTEXT_H
#define CL4_CONTEXT_H

#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif
#include "devsel.h"
#include "gerrorf.h"
#include "common.h"
#include "errors.h"

/**
 * @brief The scene object encompasses the OpenCL context, associated
 * devices, programs, queues and kernels. Should be accessed using
 * the cl4_context_* group of functions.
 * */
typedef struct cl4_context CL4Context;

// Filters key: a devsel function
// Filters value: the devsel function arguments (or data)
CL4Context* cl4_context_new(GHashTable* filters, GError **err);

CL4Context* cl4_context_new_cpu(GError **err);

CL4Context* cl4_context_new_gpu(GError **err);

void cl4_context_destroy(CL4Context* scene);

#endif
