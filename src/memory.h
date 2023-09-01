/*
 * Copyright 2023 Alexander Fasching
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <lean/lean.h>

/** Basic types enum */
enum BasicType {
    BASIC_TYPE_INT8,
    BASIC_TYPE_UINT8,
    BASIC_TYPE_INT16,
    BASIC_TYPE_UINT16,
    BASIC_TYPE_INT32,
    BASIC_TYPE_UINT32,
    BASIC_TYPE_INT64,
    BASIC_TYPE_UINT64,
    BASIC_TYPE_FLOAT,
    BASIC_TYPE_DOUBLE,
    BASIC_TYPE_LONG_DOUBLE,
};

/**
 * Internal representation of a memory region.
 * If the memory is allocated in the constructor, then it has to be freed when
 * it is finalized.
 */
typedef struct {
    lean_object *parent;
    void *buffer;
    size_t size;
    bool allocated;
} Memory;

/** Convert a Memory object from Lean to C. */
static inline Memory *Memory_unbox(b_lean_obj_arg m) {
    return (Memory *)(lean_get_external_data(m));
}
