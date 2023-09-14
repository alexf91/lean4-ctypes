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

#include "leanvalue.hpp"
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

    virtual ~CType() {}

    /** Get the size of the basic type. */
    size_t get_size() const { return m_ffi_type.size; }

    /** Get alignment. */
    size_t get_alignment() const { return m_ffi_type.alignment; }

    /** Convert from Lean to this class. */
    static std::unique_ptr<CType> unbox(b_lean_obj_arg obj);

    /** Get the number of elements. */
    size_t get_nelements();

    /** Get the array of struct offsets. */
    std::vector<size_t> get_offsets();

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

/** CType for void types. */
class CTypeVoid : public CType {
  public:
    CTypeVoid() : CType(VOID) {}
    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        return std::make_unique<LeanValueUnit>();
    }
    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        // TODO: This is a user error (void not allowed as argument), so it should
        // be an exception.
        lean_internal_panic("can't cast void to buffer");
    }
};

/** CType for scalar types. */
template <typename T> class CTypeScalar : public CType {
    static_assert(std::is_scalar_v<T>);

  public:
    // TODO: assertion is not correct.
    CTypeScalar(ObjectTag tag) : CType(tag) {
        assert((FIRST_INT <= tag && tag <= LAST_INT) ||
               (FIRST_FLOAT <= tag && tag <= LAST_FLOAT) || tag == POINTER);
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        if (std::is_integral_v<T> && std::is_signed_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueInt>(value);
        } else if (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueNat>(value);
        } else if (std::is_floating_point_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueFloat>(value);
        } else {
            lean_internal_panic_unreachable();
        }
    }

    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[get_size()]);
        switch (value.get_tag()) {
        case LeanValue::NAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueNat &>(value).get_value();
            break;
        case LeanValue::INT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueInt &>(value).get_value();
            break;
        case LeanValue::FLOAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueFloat &>(value).get_value();
            break;
        default:
            // TODO: This is a user error (wrong argument type of value), so it should
            // be an exception.
            lean_internal_panic("cast not supported");
        }
        return buffer;
    }
};

/** CType for complex types. */
template <typename T> class CTypeComplex : public CType {
    static_assert(!std::is_scalar_v<T>); // TODO: Improve assertion

  public:
    CTypeComplex(ObjectTag tag) : CType(tag) {
        assert(FIRST_COMPLEX <= tag && tag <= LAST_COMPLEX);
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        T value = *((T *)buffer);
        return std::make_unique<LeanValueComplex>(value);
    }

    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[get_size()]);
        switch (value.get_tag()) {
        case LeanValue::NAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueNat &>(value).get_value();
            break;
        case LeanValue::INT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueInt &>(value).get_value();
            break;
        case LeanValue::FLOAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueFloat &>(value).get_value();
            break;
        case LeanValue::COMPLEX:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueComplex &>(value).get_value();
            break;
        default:
            // TODO: This is a user error (wrong argument type of value), so it should
            // be an exception.
            lean_internal_panic("cast not supported");
        }
        return buffer;
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
