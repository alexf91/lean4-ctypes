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

#include "memory.h"
#include "utils.h"
#include <complex.h>
#include <stdlib.h>

/** Size of basic types. */
size_t BasicType_sizeof(uint8_t type) {
    switch (type) {
    case BASIC_TYPE_INT8:
        return sizeof(int8_t);
    case BASIC_TYPE_UINT8:
        return sizeof(uint8_t);
    case BASIC_TYPE_INT16:
        return sizeof(int16_t);
    case BASIC_TYPE_UINT16:
        return sizeof(uint16_t);
    case BASIC_TYPE_INT32:
        return sizeof(int32_t);
    case BASIC_TYPE_UINT32:
        return sizeof(uint32_t);
    case BASIC_TYPE_INT64:
        return sizeof(int64_t);
    case BASIC_TYPE_UINT64:
        return sizeof(uint64_t);
    case BASIC_TYPE_FLOAT:
        return sizeof(float);
    case BASIC_TYPE_DOUBLE:
        return sizeof(double);
    case BASIC_TYPE_LONGDOUBLE:
        return sizeof(long double);
    case BASIC_TYPE_COMPLEX_FLOAT:
        return sizeof(float complex);
    case BASIC_TYPE_COMPLEX_DOUBLE:
        return sizeof(double complex);
    case BASIC_TYPE_COMPLEX_LONGDOUBLE:
        return sizeof(long double complex);
    case BASIC_TYPE_POINTER:
        return sizeof(void *);
    }
    lean_internal_panic_unreachable();
}

lean_obj_res BasicType_sizeof_Nat(uint8_t type) {
    return lean_box(BasicType_sizeof(type));
}

/** Lean class */
lean_external_class *Memory_class = NULL;

/** Finalize a Memory. */
static void Memory_finalize(void *p) {
    Memory *m = (Memory *)p;
    utils_log("finalizing memory %p with buffer %p", m, m->buffer);

    // Views with parents can never be allocated.
    assert(!(m->allocated && m->parent));

    if (m->parent)
        lean_dec(m->parent);

    if (m->allocated)
        free(m->buffer);
    free(m);
}

/** Foreach for a Memory. */
static void Memory_foreach(void *mod, b_lean_obj_arg fn) {
    utils_log("NOT IMPLEMENTED");
}

/** Convert a Memory object from C to Lean. */
static inline lean_object *Memory_box(Memory *m) {
    if (Memory_class == NULL)
        Memory_class = lean_register_external_class(Memory_finalize, Memory_foreach);
    return lean_alloc_external(Memory_class, m);
}

/**
 * Create a new Memory instance from a ByteArray.
 */
lean_obj_res Memory_fromByteArray(b_lean_obj_arg array, lean_object *unused) {
    size_t size = lean_unbox(lean_byte_array_size(array));
    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        lean_internal_panic_out_of_memory();

    for (size_t i = 0; i < size; i++)
        buffer[i] = lean_byte_array_uget(array, i);

    Memory *m = calloc(1, sizeof(Memory));
    if (m == NULL) {
        free(buffer);
        lean_internal_panic_out_of_memory();
    }

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
    Memory *m = calloc(1, sizeof(Memory));
    if (m == NULL)
        lean_internal_panic_out_of_memory();

    m->allocated = true;
    m->buffer = calloc(size, sizeof(uint8_t));
    m->size = size;

    if (m->buffer == NULL) {
        free(m);
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

    Memory *nm = calloc(1, sizeof(Memory));
    nm->parent = memory;
    nm->buffer = ((uint8_t *)m->buffer) + b;
    nm->size = max(0, e - b);

    return lean_io_result_mk_ok(Memory_box(nm));
}

/**
 * Read an integer from the memory view.
 */
lean_obj_res Memory_readInt(b_lean_obj_arg memory, b_lean_obj_arg offset, uint8_t type,
                            lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);
    size_t size = BasicType_sizeof(type);

    if (o + size > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    void *address = ((uint8_t *)m->buffer) + o;

    switch (type) {
    case BASIC_TYPE_INT8:
        return lean_io_result_mk_ok(lean_int64_to_int(*((int8_t *)address)));
    case BASIC_TYPE_UINT8:
        return lean_io_result_mk_ok(lean_int64_to_int(*((uint8_t *)address)));
    case BASIC_TYPE_INT16:
        return lean_io_result_mk_ok(lean_int64_to_int(*((int16_t *)address)));
    case BASIC_TYPE_UINT16:
        return lean_io_result_mk_ok(lean_int64_to_int(*((uint16_t *)address)));
    case BASIC_TYPE_INT32:
        return lean_io_result_mk_ok(lean_int64_to_int(*((int32_t *)address)));
    case BASIC_TYPE_UINT32:
        return lean_io_result_mk_ok(lean_int64_to_int(*((uint32_t *)address)));
    case BASIC_TYPE_INT64:
        return lean_io_result_mk_ok(lean_int64_to_int(*((int64_t *)address)));
    case BASIC_TYPE_UINT64:
        return lean_io_result_mk_ok(lean_big_uint64_to_nat(*((uint64_t *)address)));
    }
    lean_object *msg = lean_mk_string("not an integer type");
    return lean_io_result_mk_error(lean_mk_io_user_error(msg));
}

/**
 * Read a floating point value from the memory view.
 */
lean_obj_res Memory_readFloat(b_lean_obj_arg memory, b_lean_obj_arg offset,
                              uint8_t type, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);
    size_t size = BasicType_sizeof(type);

    if (o + size > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    void *address = ((uint8_t *)m->buffer) + o;

    switch (type) {
    case BASIC_TYPE_FLOAT:
        return lean_io_result_mk_ok(lean_box_float(*((float *)address)));
    case BASIC_TYPE_DOUBLE:
        return lean_io_result_mk_ok(lean_box_float(*((double *)address)));
    case BASIC_TYPE_LONGDOUBLE:
        return lean_io_result_mk_ok(lean_box_float(*((long double *)address)));
    }
    lean_object *msg = lean_mk_string("not a floating point type");
    return lean_io_result_mk_error(lean_mk_io_user_error(msg));
}

/**
 * Read a complex floating point value from the memory view.
 */
lean_obj_res Memory_readComplex(b_lean_obj_arg memory, b_lean_obj_arg offset,
                                uint8_t type, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);
    size_t size = BasicType_sizeof(type);

    if (o + size > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    void *address = ((uint8_t *)m->buffer) + o;
    double complex result;

    switch (type) {
    case BASIC_TYPE_COMPLEX_FLOAT:
        result = *((float complex *)address);
        break;
    case BASIC_TYPE_COMPLEX_DOUBLE:
        result = *((double complex *)address);
        break;
    case BASIC_TYPE_COMPLEX_LONGDOUBLE:
        result = *((long double complex *)address);
        break;
    default:
        lean_object *msg = lean_mk_string("not a complex type");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    lean_object *array = lean_mk_empty_float_array(lean_box(2));
    array = lean_float_array_push(array, creal(result));
    array = lean_float_array_push(array, cimag(result));

    return lean_io_result_mk_ok(array);
}

/**
 * Dereference a pointer and create a new memory view.
 */
lean_obj_res Memory_dereference(b_lean_obj_arg memory, b_lean_obj_arg offset,
                                b_lean_obj_arg nsize, lean_object *unused) {
    Memory *m = Memory_unbox(memory);
    size_t o = lean_unbox(offset);
    size_t size = BasicType_sizeof(BASIC_TYPE_POINTER);

    if (o + size > m->size) {
        lean_object *msg = lean_mk_string("reading out of bounds");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }
    void *address = ((uint8_t *)m->buffer) + o;

    Memory *nm = calloc(1, sizeof(Memory));
    nm->buffer = *((void **)address);
    nm->size = lean_unbox(nsize);

    return lean_io_result_mk_ok(Memory_box(nm));
}
