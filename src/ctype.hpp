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
#include <stddef.h>
#include <type_traits>
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

        LENGTH
    };

  public:
    // Constructor for static types. Values other than the tag are taken from the
    // corresponding ffi_type.
    CType(ObjectTag tag);

    // TODO: Make this virtual without destroying our assumptions about the layout.
    //       Probably better to return a copy of the type as an ffi_type instead of
    //       doing whatever we do now.
    virtual ~CType() {}

    /** Get the size of the basic type. */
    size_t get_size() const { return m_ffi_type.size; }

    /** Get alignment. */
    size_t get_alignment() const { return m_ffi_type.alignment; }

    /** Get a string representation of the type. */
    const char *to_string() const { return name_map[m_tag]; }

    /** Convert from Lean to this class. */
    static std::unique_ptr<CType> unbox(b_lean_obj_arg obj);

    /** Check if the type is an integer type. */
    bool is_integer() const { return FIRST_INT <= m_tag && m_tag <= LAST_INT; }
    /** Check if the type is a signed integer. */
    bool is_signed() const { return FIRST_SIGNED <= m_tag && m_tag <= LAST_SIGNED; }
    /** Check if the type is an unsigned integer. */
    bool is_unsigned() const {
        return FIRST_UNSIGNED <= m_tag && m_tag <= LAST_UNSIGNED;
    }
    /** Check if the type is a floating point type. */
    bool is_float() const { return FIRST_FLOAT <= m_tag && m_tag <= LAST_FLOAT; }
    /** Check if the type is a complex floating point type. */
    bool is_complex() const { return FIRST_COMPLEX <= m_tag && m_tag <= LAST_COMPLEX; }

    /** Get the object tag. */
    ObjectTag get_tag() const { return m_tag; }

    /** Get the number of elements. */
    size_t get_nelements();

    /** Get the array of struct offsets. */
    std::vector<size_t> get_offsets();

    /** Get a pointer to the internal ffi_type. */
    ffi_type *get_ffi_type() { return &m_ffi_type; }

  private:
    // TODO: Remove this tag after implementing template classes.
    ObjectTag m_tag;

    static const ffi_type *type_map[];
    static const char *name_map[];

  protected:
    ffi_type m_ffi_type;
};

/** CType for void types. */
class CTypeVoid : public CType {
  public:
    CTypeVoid() : CType(VOID) {}
};

/** CType for integer types. */
template <typename T> class CTypeInt : public CType {
  public:
    CTypeInt(ObjectTag tag) : CType(tag) {
        assert(FIRST_INT <= tag && tag <= LAST_INT);
    }
};

/** CType for floating point types. */
template <typename T> class CTypeFloat : public CType {
  public:
    CTypeFloat(ObjectTag tag) : CType(tag) {
        assert(FIRST_FLOAT <= tag && tag <= LAST_FLOAT);
    }
};

/** CType for complex floating point types. */
template <typename T> class CTypeComplex : public CType {
  public:
    CTypeComplex(ObjectTag tag) : CType(tag) {
        assert(FIRST_COMPLEX <= tag && tag <= LAST_COMPLEX);
    }
};

/** CType for pointer types. */
class CTypePointer : public CType {
  public:
    CTypePointer() : CType(POINTER) {}
};

/** CType for array types. */
class CTypeArray : public CType {
  public:
    CTypeArray(std::unique_ptr<CType> tp, size_t length)
        : CType(ARRAY), m_element_type(std::move(tp)) {
        m_ffi_type.type = FFI_TYPE_STRUCT;

        // We reuse the unique pointer, but we consider this in the destructor.
        m_ffi_type.elements = new ffi_type *[length + 1]();
        for (size_t i = 0; i < length; i++)
            m_ffi_type.elements[i] = m_element_type->get_ffi_type();

        // Initialize size and alignment fields.
        ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, nullptr);
    }

    CTypeArray(b_lean_obj_arg type, b_lean_obj_arg length)
        : CTypeArray(CType::unbox(type), lean_unbox(length)) {}

    ~CTypeArray() {
        assert(m_ffi_type.elements);
        delete[] m_ffi_type.elements;
    }

  private:
    std::unique_ptr<CType> m_element_type;
};

/** CType for struct types. */
class CTypeStruct : public CType {
  public:
    CTypeStruct(std::vector<std::unique_ptr<CType>> members)
        : CType(STRUCT), m_element_types(std::move(members)) {
        lean_internal_panic("there");
        init();
    }

    CTypeStruct(b_lean_obj_arg members) : CType(STRUCT) {
        std::vector<std::unique_ptr<CType>> elements;
        for (size_t i = 0; i < lean_array_size(members); i++) {
            auto tp = CType::unbox(lean_array_get_core(members, i));
            elements.push_back(std::move(tp));
        }
        m_element_types = std::move(elements);
        init();
    }

    ~CTypeStruct() {
        assert(m_ffi_type.elements);
        delete[] m_ffi_type.elements;
    }

  private:
    void init() {
        m_ffi_type.type = FFI_TYPE_STRUCT;

        m_ffi_type.elements = new ffi_type *[m_element_types.size() + 1]();
        for (size_t i = 0; i < m_element_types.size(); i++)
            m_ffi_type.elements[i] = m_element_types[i]->get_ffi_type();

        // Initialize size and alignment fields.
        ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, nullptr);
    }

    std::vector<std::unique_ptr<CType>> m_element_types;
};
