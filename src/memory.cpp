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
#include <stdexcept>

/** Create a new memory. */
Memory::Memory(lean_obj_arg parent, void *buffer, size_t size, bool allocated)
    : m_parent(parent), m_buffer((uint8_t *)buffer), m_size(size),
      m_allocated(allocated) {
    if (parent != nullptr)
        lean_inc(m_parent);
}

Memory::~Memory() {
    // Views with parents can never be allocated.
    assert(!(m_allocated && m_parent));
    if (m_parent)
        lean_dec(m_parent);
    if (m_allocated)
        delete[] m_buffer;
}

/**
 * Create a new Memory instance from a ByteArray.
 */
Memory *Memory::fromByteArray(b_lean_obj_arg array) {
    size_t size = lean_unbox(lean_byte_array_size(array));
    uint8_t *buffer = new uint8_t[size];

    for (size_t i = 0; i < size; i++)
        buffer[i] = lean_byte_array_uget(array, i);

    return new Memory(nullptr, buffer, size, true);
}

/**
 * Convert a Memory region to a ByteArray.
 */
lean_obj_res Memory::toByteArray() {
    lean_object *array = lean_mk_empty_byte_array(lean_box(get_size()));
    for (size_t i = 0; i < get_size(); i++)
        array = lean_byte_array_push(array, m_buffer[i]);
    return array;
}

/** Create a memory from a type and a value. */
Memory *Memory::fromValue(const CType &type, const LeanValue &value) {
    auto buffer = type.buffer(value);
    return new Memory(nullptr, buffer.release(), type.get_size(), true);
}

/** Extract part of a memory view and create a new one. */
Memory *Memory::extract(size_t begin, size_t end) {
    // Check if we are out of bounds.
    if (begin >= get_size())
        throw std::runtime_error("begin index out of bounds");
    if (end >= get_size())
        throw std::runtime_error("end index out of bounds");

    void *new_buffer = m_buffer + begin;
    return new Memory(m_parent, new_buffer, std::max(0L, (ssize_t)end - (ssize_t)begin),
                      false);
}

/** Read a CType from the memory and create a LeanValue. */
std::unique_ptr<LeanValue> Memory::read(const CType &ct, size_t offset) {
    if (offset + ct.get_size() > get_size())
        throw std::runtime_error("reading out of bounds");

    uint8_t *address = m_buffer + offset;
    return ct.instance(address);
}

/** Dereference a pointer and create a new memory view. */
Memory *Memory::dereference(size_t offset, size_t size) {
    if (offset + sizeof(void *) > get_size())
        throw std::runtime_error("reading out of bounds");

    void *address = m_buffer + offset;

    // TODO: Does this need a parent?
    return new Memory(nullptr, address, size, false);
}

/** Create a memory view from a byte array. */
extern "C" lean_obj_res Memory_fromByteArray(b_lean_obj_arg array,
                                             lean_object *unused) {
    try {
        Memory *m = Memory::fromByteArray(array);
        return lean_io_result_mk_ok(m->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/** Create a byte array from a memory view. */
extern "C" lean_obj_res Memory_toByteArray(b_lean_obj_arg memory, lean_object *unused) {
    auto m = Memory::unbox(memory);
    try {
        lean_object *array = m->toByteArray();
        return lean_io_result_mk_ok(array);
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/** Create a memory from a value. */
extern "C" lean_obj_res Memory_fromValue(b_lean_obj_arg type, b_lean_obj_arg value,
                                         lean_object *unused) {
    try {
        Memory *m = Memory::fromValue(*CType::unbox(type), *LeanValue::unbox(value));
        return lean_io_result_mk_ok(m->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/** Get the size of the memory. */
extern "C" lean_obj_res Memory_size(b_lean_obj_arg memory) {
    auto m = Memory::unbox(memory);
    return lean_box(m->get_size());
}

/** Check if the memory is allocated. */
extern "C" uint8_t Memory_allocated(b_lean_obj_arg memory) {
    auto m = Memory::unbox(memory);
    return m->is_allocated();
}

/** Extract part of a memory view and create a new one. */
extern "C" lean_obj_res Memory_extract(lean_obj_arg memory, b_lean_obj_arg begin,
                                       b_lean_obj_arg end, lean_object *unused) {
    auto m = Memory::unbox(memory);
    size_t b = lean_unbox(begin);
    size_t e = lean_unbox(end);

    try {
        Memory *nm = m->extract(b, e);
        return lean_io_result_mk_ok(nm->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Read an arbitrary value from the memory view.
 */
extern "C" lean_obj_res Memory_read(b_lean_obj_arg memory, b_lean_obj_arg offset,
                                    b_lean_obj_arg type, lean_object *unused) {
    Memory *m = Memory::unbox(memory);
    size_t o = lean_unbox(offset);
    auto ct = CType::unbox(type);

    try {
        return lean_io_result_mk_ok(m->read(*ct, o)->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Dereference a pointer and create a new memory view.
 */
extern "C" lean_obj_res Memory_dereference(b_lean_obj_arg memory, b_lean_obj_arg offset,
                                           b_lean_obj_arg nsize, lean_object *unused) {
    Memory *m = Memory::unbox(memory);
    size_t o = lean_unbox(offset);
    size_t size = lean_unbox(nsize);

    try {
        auto nm = m->dereference(o, size);
        return lean_io_result_mk_ok(nm->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}
