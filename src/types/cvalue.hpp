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

#include "../utils.hpp"
#include "common.hpp"
#include "ctype.hpp"
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

/** Get the CType of a CValue. */
LEAN_EXPORT_WEAK lean_object *CValue_type(lean_object *);

/**
 * Representation of a C value in Lean.
 */
class CValue {
  public:
    CValue(std::unique_ptr<CType> &type) : m_type(std::move(type)) {}

    virtual ~CValue() {}

    /** Convert from Lean to this class. */
    static std::unique_ptr<CValue> unbox(b_lean_obj_arg obj);

    /** Create a value from a type and a buffer. */
    static std::unique_ptr<CValue> from_buffer(std::unique_ptr<CType> &type,
                                               const uint8_t *buffer);

    /** Convert this class to a Lean object. */
    virtual lean_obj_res box() = 0;

    /** Create a buffer for the value. */
    virtual std::unique_ptr<uint8_t[]> to_buffer() const = 0;

    /** Get the type of the value.
     *
     * TODO: Remove this. It leaks the address of the unique pointer.
     */
    const CType *get_type() const { return m_type.get(); }

  protected:
    std::unique_ptr<CType> m_type;
};

/**
 * CValue for the void type.
 *
 * Can't be packed to a buffer and unpacks regardless of the buffer content.
 */
class CValueVoid : public CValue {
  public:
    CValueVoid(std::unique_ptr<CType> &type) : CValue(type) {}
    lean_obj_res box() { return lean_box(0); }
    std::unique_ptr<uint8_t[]> to_buffer() const {
        lean_internal_panic("can't convert void to buffer");
    }
};

/** CValue for integers, floats and complex floats. */
template <ObjectTag Tag> class CValueScalar : public CValue {
    using T = TagToType<Tag>::type;

  public:
    CValueScalar(std::unique_ptr<CType> &type, T value) : CValue(type), m_value(value) {
        assert(Tag == m_type->get_tag());
    }

    std::unique_ptr<uint8_t[]> to_buffer() const {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[m_type->get_size()]);
        *((T *)buffer.get()) = m_value;
        return buffer;
    }

  protected:
    T m_value;
};

/** CValue with a single Nat constructor (i.e. uint types). */
template <ObjectTag Tag> class CValueNat : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a type and a CValue object. */
    CValueNat(std::unique_ptr<CType> &type, b_lean_obj_arg obj)
        : CValueNat<Tag>(type, (T)lean_uint64_of_nat(lean_ctor_get(obj, 0))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueNat(std::unique_ptr<CType> &type, const uint8_t *buffer)
        : CValueNat(type, *((T *)buffer)) {}

    /** Create directly from value. */
    CValueNat(std::unique_ptr<CType> &type, T value) : CValueScalar<Tag>(type, value) {}

    lean_obj_res box() {
        auto o = lean_alloc_ctor(Tag, 1, 0);
        lean_ctor_set(o, 0, lean_uint64_to_nat(this->m_value));
        return o;
    }
};

/** CValue with a single Int constructor (i.e. int types). */
template <ObjectTag Tag> class CValueInt : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a type and a CValue object. */
    // TODO: Is lean_uint64_of_nat() correct here?
    CValueInt(std::unique_ptr<CType> &type, b_lean_obj_arg obj)
        : CValueInt<Tag>(type, (T)lean_uint64_of_nat(lean_ctor_get(obj, 0))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueInt(std::unique_ptr<CType> &type, const uint8_t *buffer)
        : CValueInt(type, *((T *)buffer)) {}

    /** Create directly from value. */
    CValueInt(std::unique_ptr<CType> &type, T value) : CValueScalar<Tag>(type, value) {}

    lean_obj_res box() {
        auto o = lean_alloc_ctor(Tag, 1, 0);
        lean_ctor_set(o, 0, lean_int64_to_int(this->m_value));
        return o;
    }
};

/** CValue with a single Float constructor. */
template <ObjectTag Tag> class CValueFloat : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a type and a CValue object. */
    CValueFloat(std::unique_ptr<CType> &type, b_lean_obj_arg obj)
        : CValueFloat<Tag>(type, (T)lean_unbox_float(obj)) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueFloat(std::unique_ptr<CType> &type, const uint8_t *buffer)
        : CValueFloat(type, *((T *)buffer)) {}

    /** Create directly from value. */
    CValueFloat(std::unique_ptr<CType> &type, T value)
        : CValueScalar<Tag>(type, value) {}

    lean_obj_res box() {
        auto o = lean_alloc_ctor(Tag, 0, sizeof(double));
        lean_ctor_set_float(o, 0, (double)this->m_value);
        return o;
    }
};

/** CValue with a double Float constructor. */
template <ObjectTag Tag> class CValueComplex : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a type and a CValue object. */
    CValueComplex(std::unique_ptr<CType> &type, b_lean_obj_arg obj)
        : CValueScalar<Tag>(type, lean_ctor_get_float(obj, 0) +
                                      1i * lean_ctor_get_float(obj, sizeof(double))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueComplex(std::unique_ptr<CType> &type, const uint8_t *buffer)
        : CValueComplex(type, *((T *)buffer)) {}

    /** Create directly from value. */
    CValueComplex(std::unique_ptr<CType> &type, T value)
        : CValueScalar<Tag>(type, value) {}

    lean_obj_res box() {
        auto o = lean_alloc_ctor(Tag, 0, 2 * sizeof(double));
        lean_ctor_set_float(o, 0, (double)this->m_value.real());
        lean_ctor_set_float(o, sizeof(double), (double)this->m_value.imag());
        return o;
    }
};

/** CValue for pointers. */
class CValuePointer : CValue {};

/** CValue for structs. */
class CValueStruct : CValue {};
