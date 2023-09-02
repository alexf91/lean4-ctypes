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
 * A type in C.
 */
class CType : public ffi_type {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        VOID,
        // Integer types
        FIRST_INT,
        INT8 = FIRST_INT,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        INT64,
        UINT64,
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
        LAST_STATIC = POINTER,

        // Composite types
        ARRAY,
        STRUCT,

        LENGTH
    };

    // Constructor for static types.
    CType(ObjectTag tag);

    /** Get the size of the basic type. */
    size_t get_size() { return size_map[m_tag]; }

    /** Get a string representation of the type. */
    const char *to_string() { return name_map[m_tag]; }

    /** Convert from Lean to this class. */
    static CType *unbox(b_lean_obj_arg obj);

    /** Check if the type is an integer type. */
    bool is_integer() { return FIRST_INT <= m_tag && m_tag <= LAST_INT; }
    /** Check if the type is a floating point type. */
    bool is_float() { return FIRST_FLOAT <= m_tag && m_tag <= LAST_FLOAT; }
    /** Check if the type is a complex floating point type. */
    bool is_complex() { return FIRST_COMPLEX <= m_tag && m_tag <= LAST_COMPLEX; }
    /**
     * Check if the ffi_type is one of our basic statically allocated types.
     * Note that the type might still be statically allocated even if this function
     * returns false.
     */
    bool is_static(ffi_type *tp) {
        for (size_t i = 0; i < LAST_STATIC; i++)
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
