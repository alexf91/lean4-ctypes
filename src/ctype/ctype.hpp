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

#include "../leanvalue.hpp"
#include "../utils.hpp"

#include <cstdint>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

/**
 * A type in C.
 */
class CType {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        VOID,
        // Integer types
        FIRST_INT,
        FIRST_SIGNED = FIRST_INT,
        INT8 = FIRST_INT,
        INT16,
        INT32,
        INT64,
        LAST_SIGNED = INT64,

        FIRST_UNSIGNED,
        UINT8 = FIRST_UNSIGNED,
        UINT16,
        UINT32,
        UINT64,
        LAST_UNSIGNED = UINT64,
        LAST_INT = UINT64,

        // Floating point types
        FIRST_FLOAT,
        FLOAT = FIRST_FLOAT,
        DOUBLE,
        LONGDOUBLE,
        LAST_FLOAT = LONGDOUBLE,

        // Complex types
        FIRST_COMPLEX,
        COMPLEX_FLOAT = FIRST_COMPLEX,
        COMPLEX_DOUBLE,
        COMPLEX_LONGDOUBLE,
        LAST_COMPLEX = COMPLEX_LONGDOUBLE,

        // Pointer types
        POINTER,
        LAST_PRIMITIVE = POINTER,

        // Composite types
        ARRAY,
        STRUCT,
        UNION,

        // Aliased types
        FIRST_ALIASED,
        CHAR = FIRST_ALIASED,
        SHORT,
        INT,
        LONG,
        LONGLONG,
        SSIZE_T,
        UCHAR,
        USHORT,
        UINT,
        ULONG,
        ULONGLONG,
        SIZE_T,
        TIME_T,
        LAST_ALIASED = TIME_T,

        LENGTH
    };

  public:
    // Constructor for static types. Values other than the tag are taken from the
    // corresponding ffi_type.
    CType(ObjectTag tag);

    virtual ~CType() {}

    /** Get the size of the basic type. */
    size_t get_size() const { return m_ffi_type.size; }

    /** Get alignment. */
    size_t get_alignment() const { return m_ffi_type.alignment; }

    /** Convert from Lean to this class. */
    static std::unique_ptr<CType> unbox(b_lean_obj_arg obj);

    /** Get the number of elements. */
    size_t get_nelements() const;

    /** Get the array of struct offsets. */
    const std::vector<size_t> get_offsets() const;

    /** Get a pointer to the internal ffi_type. */
    ffi_type *get_ffi_type() { return &m_ffi_type; }

    /** Create the corresponding LeanValue instance from a buffer. */
    virtual std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        lean_internal_panic("CType::instance() not implemented");
    }

    /** Create a corresponding buffer from a LeanValue. */
    virtual std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        lean_internal_panic("CType::buffer() not implemented");
    }

  private:
    static const ffi_type *type_map[];

  protected:
    ffi_type m_ffi_type;
};
