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

extern "C" LEAN_EXPORT_WEAK lean_object *CValue_type__(lean_object *);
/** Get the CType of a CValue. */
static inline lean_object *CValue_type(lean_object *o) {
    lean_inc(o);
    return CValue_type__(o);
}

/** A pointer in C. */
class Pointer;

/**
 * Representation of a C value in Lean.
 */
class CValue {
  public:
    virtual ~CValue() {}

    /** Convert from Lean to this class. */
    static std::unique_ptr<CValue> unbox(b_lean_obj_arg obj);

    /** Create a value from a type and a buffer. */
    static std::unique_ptr<CValue> from_buffer(std::unique_ptr<CType> type,
                                               const uint8_t *buffer);

    /** Convert this class to a Lean object. */
    virtual lean_obj_res box() const = 0;

    /** Create a buffer for the value. */
    virtual std::unique_ptr<uint8_t[]> to_buffer() const = 0;

    /** Get the type of the value. */
    virtual std::unique_ptr<CType> type() const {
        return CType::unbox(CValue_type(box()));
    }
};

/**
 * CValue for the void type.
 *
 * Can't be packed to a buffer and unpacks regardless of the buffer content.
 */
class CValueVoid : public CValue {
  public:
    lean_obj_res box() const override { return lean_box(0); }
    std::unique_ptr<uint8_t[]> to_buffer() const override {
        lean_internal_panic("can't convert void to buffer");
    }
};

/** CValue for integers, floats and complex floats. */
template <ObjectTag Tag> class CValueScalar : public CValue {
    using T = TagToType<Tag>::type;

  public:
    CValueScalar(T value) : m_value(value) {}

    std::unique_ptr<uint8_t[]> to_buffer() const override {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[type()->get_size()]);
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
    /** Create the value from a CValue object. */
    CValueNat(b_lean_obj_arg obj)
        : CValueNat<Tag>((T)lean_uint64_of_nat(lean_ctor_get(obj, 0))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueNat(const uint8_t *buffer) : CValueNat(*((T *)buffer)) {}

    /** Create directly from value. */
    CValueNat(T value) : CValueScalar<Tag>(value) {}

    lean_obj_res box() const override {
        auto o = lean_alloc_ctor(Tag, 1, 0);
        lean_ctor_set(o, 0, lean_uint64_to_nat(this->m_value));
        return o;
    }
};

/** CValue with a single Int constructor (i.e. int types). */
template <ObjectTag Tag> class CValueInt : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a CValue object. */
    CValueInt(b_lean_obj_arg obj)
        : CValueInt<Tag>((T)lean_scalar_to_int64(lean_ctor_get(obj, 0))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueInt(const uint8_t *buffer) : CValueInt(*((T *)buffer)) {}

    /** Create directly from value. */
    CValueInt(T value) : CValueScalar<Tag>(value) {}

    lean_obj_res box() const override {
        auto o = lean_alloc_ctor(Tag, 1, 0);
        lean_ctor_set(o, 0, lean_int64_to_int(this->m_value));
        return o;
    }
};

/** CValue with a single Float constructor. */
template <ObjectTag Tag> class CValueFloat : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a CValue object. */
    CValueFloat(b_lean_obj_arg obj) : CValueFloat<Tag>((T)lean_unbox_float(obj)) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueFloat(const uint8_t *buffer) : CValueFloat(*((T *)buffer)) {}

    /** Create directly from value. */
    CValueFloat(T value) : CValueScalar<Tag>(value) {}

    lean_obj_res box() const override {
        auto o = lean_alloc_ctor(Tag, 0, sizeof(double));
        lean_ctor_set_float(o, 0, (double)this->m_value);
        return o;
    }
};

/** CValue with a double Float constructor. */
template <ObjectTag Tag> class CValueComplex : public CValueScalar<Tag> {
    using T = TagToType<Tag>::type;

  public:
    /** Create the value from a CValue object. */
    CValueComplex(b_lean_obj_arg obj)
        : CValueScalar<Tag>(lean_ctor_get_float(obj, 0) +
                            1i * lean_ctor_get_float(obj, sizeof(double))) {
        assert(lean_obj_tag(obj) == Tag);
    }

    /** Create from buffer. */
    CValueComplex(const uint8_t *buffer) : CValueComplex(*((T *)buffer)) {}

    /** Create directly from value. */
    CValueComplex(T value) : CValueScalar<Tag>(value) {}

    lean_obj_res box() const override {
        auto o = lean_alloc_ctor(Tag, 0, 2 * sizeof(double));
        lean_ctor_set_float(o, 0, (double)this->m_value.real());
        lean_ctor_set_float(o, sizeof(double), (double)this->m_value.imag());
        return o;
    }
};

/** CValue for pointers. */
class CValuePointer : public CValue {
  public:
    /** Create the value from a CValue object. */
    CValuePointer(b_lean_obj_arg obj);

    /**
     * Create from buffer.
     *
     * Note that `buffer` this is not the address of the pointer. The actual address is
     * found by treating the value in the buffer as a `void` pointer.
     */
    CValuePointer(const uint8_t *buffer);

    /** Just decrease the reference count, it is freed in the finalizer. */
    ~CValuePointer() { lean_dec(m_pointer); }

    lean_obj_res box() const override {
        // TODO: Do we need to change the reference count of m_pointer?
        auto o = lean_alloc_ctor(POINTER, 1, 0);
        lean_ctor_set(o, 0, m_pointer);
        return o;
    }

    std::unique_ptr<uint8_t[]> to_buffer() const override;

  private:
    // CValuePointer objects are different than scalars, because Pointer is an external
    // type. We have to keep track of its reference count, so we reference the object
    // itself.
    lean_object *m_pointer;
};

/** CValue for structs. */
class CValueStruct : public CValue {
  public:
    /** Create the value from a CValue object. */
    CValueStruct(b_lean_obj_arg obj) {
        assert(lean_obj_tag(obj) == STRUCT);
        auto values = lean_ctor_get(obj, 0);
        for (size_t i = 0; i < lean_array_size(values); i++) {
            auto o = lean_array_uget(values, i); // Calls lean_inc()
            auto value = CValue::unbox(o);
            m_values.push_back(std::move(value));
        }
    }

    /** Use type description to read a value from a buffer. */
    CValueStruct(std::unique_ptr<CType> type, const uint8_t *buffer) {
        // assert(type->get_tag() == STRUCT);
        // auto elements = dynamic_cast<CTypeStruct &>(*type()).elements();
        // auto offsets = type->get_offsets();
        // assert(elements.size() == offsets.size());

        // TODO: Implement the rest.
        // for (size_t i; i < elements.size(); i++) {
        //    auto offset = offsets[i];
        //    m_values.push_back(CValue::from_buffer(elements[i], buffer[offset]));
        //}
    }

    lean_obj_res box() const override {
        // TODO: Implement
        return nullptr;
    }

    std::unique_ptr<uint8_t[]> to_buffer() const override {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[type()->get_size()]);
        // TODO: Implement
        return buffer;
    }

  private:
    /** Values in the struct. */
    std::vector<std::unique_ptr<CValue>> m_values;
};
