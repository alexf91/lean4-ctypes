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

/** Constructors for CValue objects. */
LEAN_EXPORT_WEAK lean_object *CValue_mk_void;
LEAN_EXPORT_WEAK lean_object *CValue_mk_int8(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_int16(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_int32(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_int64(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_uint8(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_uint16(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_uint32(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_uint64(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_float(double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_double(double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_longdouble(double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_complex_float(double, double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_complex_double(double, double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_complex_longdouble(double, double);
LEAN_EXPORT_WEAK lean_object *CValue_mk_pointer(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_mk_struct(lean_object *);

/** Forced getters. */
LEAN_EXPORT_WEAK lean_object *CValue_get_struct(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_get_complex(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_get_nat(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_get_float(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_get_pointer(lean_object *);
LEAN_EXPORT_WEAK lean_object *CValue_get_int(lean_object *);

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

    /** Get the type of the value. */
    const CType *get_type() const { return m_type.get(); }

  private:
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
    lean_obj_res box() { return CValue_mk_void; }
    std::unique_ptr<uint8_t[]> to_buffer() const { lean_internal_panic_unreachable(); }
};

/** CValue for integers and floats. */
template <typename T> class CValueScalar : public CValue {
  public:
    std::unique_ptr<uint8_t[]> to_buffer() const {
        // TODO: Implement
        return nullptr;
    }

    lean_obj_res box() {
        // TODO: Implement
        return nullptr;
    }
};

/** CValue for pointers. */
class CValuePointer : CValue {};

/** CValue for structs. */
class CValueStruct : CValue {};
