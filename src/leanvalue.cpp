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

#include "leanvalue.hpp"
#include "ctype.hpp"
#include <lean/lean.h>

std::unique_ptr<LeanValue> LeanValue::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    switch (tag) {
    case UNIT:
        return std::make_unique<LeanValueUnit>();
    case INT:
        return std::make_unique<LeanValueInt>(obj);
    case NAT:
        return std::make_unique<LeanValueNat>(obj);
    case FLOAT:
        return std::make_unique<LeanValueFloat>(obj);
    case COMPLEX:
        return std::make_unique<LeanValueComplex>(obj);
    case STRUCT:
        return std::make_unique<LeanValueStruct>(obj);
    default:
        lean_internal_panic_unreachable();
    }
}

/******************************************************************************
 * UNIT TYPE
 ******************************************************************************/

/** Constructor for the unit type. */
LeanValueUnit::LeanValueUnit() : LeanValue(UNIT) {}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueUnit::box() { return LeanValue_mkUnit(lean_box(0)); }

/******************************************************************************
 * SIGNED INTEGER TYPE
 ******************************************************************************/

/** Constructor for integer types from a value. */
LeanValueInt::LeanValueInt(int64_t value) : LeanValue(INT), m_value(value) {}

/** Constructor for integer types from an object. */
LeanValueInt::LeanValueInt(b_lean_obj_arg obj)
    : LeanValueInt(lean_uint64_of_nat(lean_ctor_get(obj, 0))) {}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueInt::box() { return LeanValue_mkInt(lean_int64_to_int(m_value)); }

/******************************************************************************
 * UNSIGNED INTEGER TYPE
 ******************************************************************************/

/** Constructor for integer types from a value. */
LeanValueNat::LeanValueNat(uint64_t value) : LeanValue(NAT), m_value(value) {}

/** Constructor for integer types from an object. */
LeanValueNat::LeanValueNat(b_lean_obj_arg obj)
    : LeanValueNat(lean_uint64_of_nat(lean_ctor_get(obj, 0))) {}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueNat::box() {
    return LeanValue_mkNat(lean_uint64_to_nat(m_value));
}

/******************************************************************************
 * FLOATING POINT TYPE
 ******************************************************************************/

/** Constructor for floating point types from a value. */
LeanValueFloat::LeanValueFloat(long double value) : LeanValue(FLOAT), m_value(value) {}

/** Constructor for floating point types from an object. */
LeanValueFloat::LeanValueFloat(b_lean_obj_arg obj)
    : LeanValueFloat(lean_unbox_float(obj)) {}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueFloat::box() { return LeanValue_mkFloat(m_value); }

/******************************************************************************
 * COMPLEX FLOATING POINT TYPE
 ******************************************************************************/

/** Constructor for complex floating point types from a value. */
LeanValueComplex::LeanValueComplex(std::complex<long double> value)
    : LeanValue(COMPLEX), m_value(value) {}

/** Constructor for complex floating point types from an object. */
LeanValueComplex::LeanValueComplex(b_lean_obj_arg obj) : LeanValue(COMPLEX) {
    using namespace std::complex_literals;
    double real = lean_ctor_get_float(obj, 0);
    double imag = lean_ctor_get_float(obj, sizeof(double));
    m_value = real + imag * 1i;
}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueComplex::box() {
    return LeanValue_mkComplex(std::real(m_value), std::imag(m_value));
}

/******************************************************************************
 * STRUCT TYPE
 ******************************************************************************/

/** Constructor for struct values. */
LeanValueStruct::LeanValueStruct(std::vector<std::unique_ptr<LeanValue>> values)
    : LeanValue(STRUCT), m_values(std::move(values)) {}

/** Constructor for struct objects. */
LeanValueStruct::LeanValueStruct(b_lean_obj_arg obj) : LeanValue(STRUCT) {
    lean_object *values = lean_ctor_get(obj, 0);
    for (size_t i = 0; i < lean_array_size(values); i++) {
        lean_object *o = lean_array_get_core(values, i);
        m_values.push_back(std::move(LeanValue::unbox(o)));
    }
}

LeanValueStruct::~LeanValueStruct() {}

lean_obj_res LeanValueStruct::box() {
    lean_object *values = lean_alloc_array(m_values.size(), m_values.size());
    for (size_t i = 0; i < m_values.size(); i++)
        lean_array_set_core(values, i, m_values[i]->box());
    return LeanValue_mkStruct(values);
}
