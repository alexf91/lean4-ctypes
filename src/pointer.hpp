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

#include "ctype.hpp"
#include "external_type.hpp"
#include "leanvalue.hpp"
#include <cassert>
#include <cstdlib>
#include <lean/lean.h>

class Pointer final : public ExternalType<Pointer> {
  public:
    Pointer(lean_obj_arg parent, void *buffer, size_t size, bool allocated);
    ~Pointer();

    /** Create a memory view and initialize it from the byte array. */
    static Pointer *fromByteArray(b_lean_obj_arg array);

    /** Create a Lean ByteArray from the memory view. */
    lean_obj_res toByteArray();

    /** Create a memory from a type and a value. */
    static Pointer *fromValue(const CType &type, const LeanValue &value);

    /** Extract part of a memory view and create a new one. */
    Pointer *extract(size_t begin, size_t end);

    /** Read a CType from the memory, creating a LeanValue. */
    std::unique_ptr<LeanValue> read(const CType &type, size_t offset);

    /** Dereference a pointer and create a new memory view. */
    Pointer *dereference(size_t offset, size_t size);

    /** Get the size of the memory. */
    size_t get_size() { return m_size; }

    /** Check if the memory is allocated. */
    bool is_allocated() { return m_allocated; }

    /** Get the address of the buffer. */
    void *get_address() const { return m_buffer; }

  private:
    lean_object *m_parent;
    uint8_t *m_buffer;
    size_t m_size;
    bool m_allocated;
};
