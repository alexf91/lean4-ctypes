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

#include "memory.hpp"
#include "ctype.hpp"
#include "lean/lean.h"
#include "leanvalue.hpp"
#include "utils.hpp"
#include <algorithm>
#include <complex>
#include <cstdint>

extern "C" {

/** Lean class */
lean_external_class *Memory_class = nullptr;

/** Finalize a Memory. */
static void Memory_finalize(void *p) {
    Memory *m = (Memory *)p;
    utils_log("finalizing memory %p with buffer %p", m, m->buffer);

    delete m;
}

/** Foreach for a Memory. */
static void Memory_foreach(void *mod, b_lean_obj_arg fn) {
    utils_log("NOT IMPLEMENTED");
}

/** Convert a Memory object from C to Lean. */
static inline lean_object *Memory_box(Memory *m) {
    if (Memory_class == nullptr)
        Memory_class = lean_register_external_class(Memory_finalize, Memory_foreach);
    return lean_alloc_external(Memory_class, m);
}

/**
 * Create a new Memory instance from a ByteArray.
 */
lean_obj_res Memory_fromByteArray(b_lean_obj_arg array, lean_object *unused) {
    size_t size = lean_unbox(lean_byte_array_size(array));
    uint8_t *buffer = (uint8_t *)malloc(size);
    if (buffer == nullptr)
        lean_internal_panic_out_of_memory();

    for (size_t i = 0; i < size; i++)
        buffer[i] = lean_byte_array_uget(array, i);

    Memory *m = new Memory();
    m->buffer = buffer;
    m->size = size;
    m->allocated = true;
    return lean_io_result_mk_ok(Memory_box(m));
}

/**
 * Convert a Memory region to a ByteArray.
 */
lean_obj_res Memory_toByteArray(b_lean_obj_arg memory, lean_object *unused) {
    Memory *m = Memory_unbox(memory);

    lean_object *array = lean_mk_empty_byte_array(lean_box(m->size));
    for (size_t i = 0; i < m->size; i++)
        array = lean_byte_array_push(array, ((uint8_t *)m->buffer)[i]);
    return lean_io_result_mk_ok(array);
}

/**
 * Allocate a new memory in C.
 */
lean_obj_res Memory_allocate(size_t size, lean_object *unused) {
    Memory *m = new Memory();
    m->allocated = true;
    m->buffer = calloc(size, sizeof(uint8_t));
    m->size = size;

    if (m->buffer == nullptr) {
        delete m;
        lean_internal_panic_out_of_memory();
    }
    return lean_io_result_mk_ok(Memory_box(m));
}

/**
 * Get the size of the memory.
 */
lean_obj_res Memory_size(b_lean_obj_arg memory) {
    Memory *m = Memory_unbox(memory);
    return lean_box(m->size);
}

/**
 * Check if the memory is allocated
 */
uint8_t Memory_allocated(b_lean_obj_arg memory) {
    Memory *m = Memory_unbox(memory);
    return m->allocated;
}

/**
 * Extract part of a memory view and create a new one.
 */
lean_obj_res Memory_extract(lean_obj_arg memory, b_lean_obj_arg begin,
                            b_lean_obj_arg end, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t b = lean_unbox(begin);
    size_t e = lean_unbox(end);

    // Check if we are out of bounds.
    if (b >= m->size || e >= m->size) {
        lean_dec(memory); // Release the parent memory.
        lean_object *msg = lean_mk_string("index out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    Memory *nm = new Memory();
    nm->parent = memory;
    nm->buffer = ((uint8_t *)m->buffer) + b;
    nm->size = std::max(0L, (ssize_t)e - (ssize_t)b);

    return lean_io_result_mk_ok(Memory_box(nm));
}

/**
 * Read an arbitrary value from the memory view.
 */
lean_obj_res Memory_read(b_lean_obj_arg memory, b_lean_obj_arg offset,
                         b_lean_obj_arg type, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);
    auto tp = CType::unbox(type);

    if (o + tp->get_size() > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    uint8_t *address = ((uint8_t *)m->buffer) + o;

    auto value = LeanValue::from_buffer(*tp, address);
    return lean_io_result_mk_ok(value->box(*tp));
}

/**
 * Dereference a pointer and create a new memory view.
 */
lean_obj_res Memory_dereference(b_lean_obj_arg memory, b_lean_obj_arg offset,
                                b_lean_obj_arg nsize, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);

    if (o + sizeof(void *) > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    void *address = ((uint8_t *)m->buffer) + o;

    Memory *nm = new Memory();
    nm->buffer = *((void **)address);
    nm->size = lean_unbox(nsize);

    return lean_io_result_mk_ok(Memory_box(nm));
}
}
