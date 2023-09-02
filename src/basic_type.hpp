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

#include "utils.hpp"
#include <complex>
#include <cstdint>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>

/**
 * Basic type in C. This is a subset of all non-composite types available
 * on a regular system.
 */
class BasicType {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        INT64,
        UINT64,
        FLOAT,
        DOUBLE,
        LONGDOUBLE,
        COMPLEX_FLOAT,
        COMPLEX_DOUBLE,
        COMPLEX_LONGDOUBLE,
        LENGTH
    };

    BasicType(ObjectTag tag) {
        if (tag >= LENGTH)
            throw std::invalid_argument("invalid tag index");
        m_tag = tag;
    }

    /** Get the size of the basic type. */
    size_t size() { return size_map[m_tag]; }

    /** Get a string representation of the type. */
    const char *to_string() { return name_map[m_tag]; }

    /** Get the ffi_type of the type. */
    const ffi_type *to_ffi_type() { return type_map[m_tag]; }

    /** Convert from Lean to this class. */
    static BasicType unbox(uint8_t obj) { return BasicType((ObjectTag)obj); }

    /** Check if the type is an integer type. */
    bool is_integer() { return INT8 <= m_tag && m_tag <= UINT64; }
    /** Check if the type is a floating point type. */
    bool is_float() { return FLOAT <= m_tag && m_tag <= LONGDOUBLE; }
    /** Check if the type is a complex floating point type. */
    bool is_complex() { return COMPLEX_FLOAT <= m_tag && m_tag <= COMPLEX_LONGDOUBLE; }
    /**
     * Check if the ffi_type is one of our basic statically allocated types.
     * Note that the type might still be statically allocated even if this function
     * returns false.
     */
    bool is_static(ffi_type *tp) {
        for (size_t i = 0; i < LENGTH; i++)
            if (tp == type_map[i])
                return true;
        return false;
    }

    ObjectTag get_tag() { return m_tag; }

  private:
    ObjectTag m_tag;

    static const ffi_type *type_map[];
    static const char *name_map[];
    static const size_t size_map[];
};
