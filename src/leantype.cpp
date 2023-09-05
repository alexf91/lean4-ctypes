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

#include "leantype.hpp"
#include "ctype.hpp"
#include <lean/lean.h>

LeanType *LeanType::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    switch (tag) {
    case UNIT:
        return new LeanTypeUnit();
    case INT:
        return new LeanTypeInt(lean_ctor_get(obj, 0));
    case FLOAT:
        return new LeanTypeFloat(obj);
    default:
        lean_internal_panic_unreachable();
    }
    return nullptr;
}

/******************************************************************************
 * UNIT TYPE
 ******************************************************************************/

/** Constructor for the unit type. */
LeanTypeUnit::LeanTypeUnit() : LeanType(UNIT) {}

/**
 * We can't create a buffer from the unit data type, as it can't be used as an
 * argument.
 */
void *LeanTypeUnit::to_buffer(CType *ct) {
    lean_internal_panic("can't create buffer from unit type");
}

/** Convert the type to a Lean object. */
lean_obj_res LeanTypeUnit::box(CType *ct) {
    assert(ct != nullptr);
    if (ct->get_tag() != CType::VOID)
        lean_internal_panic("can't convert unit to other than void");
    return LeanType_mkUnit(lean_box(0));
}

/******************************************************************************
 * INTEGER TYPE
 ******************************************************************************/

/** Constructor for integer types from a value. */
LeanTypeInt::LeanTypeInt(size_t value) : LeanType(INT) { m_value = (uint64_t)value; }
LeanTypeInt::LeanTypeInt(ssize_t value) : LeanType(INT) { m_value = (uint64_t)value; }

/** Constructor for integer types from an object. */
LeanTypeInt::LeanTypeInt(b_lean_obj_arg obj) : LeanTypeInt(lean_uint64_of_nat(obj)) {}

void *LeanTypeInt::to_buffer(CType *ct) {
    switch (ct->get_tag()) {
    case CType::INT8:
        return new int8_t(m_value);
    case CType::UINT8:
        return new uint8_t(m_value);
    case CType::INT16:
        return new int16_t(m_value);
    case CType::UINT16:
        return new uint16_t(m_value);
    case CType::INT32:
        return new int32_t(m_value);
    case CType::UINT32:
        return new uint32_t(m_value);
    case CType::INT64:
        return new int64_t(m_value);
    case CType::UINT64:
        return new uint64_t(m_value);
    default:
        lean_internal_panic("invalid type");
    }
}

/** Convert the type to a Lean object. */
lean_obj_res LeanTypeInt::box(CType *ct) {
    assert(ct != nullptr);
    if (!ct->is_integer())
        lean_internal_panic("can't convert integer to non-integer type");

    int shift = (sizeof(uint64_t) - ct->get_size()) * 8;
    assert(shift >= 0);
    if (ct->is_signed()) {
        // Shift left and then back to sign extend the value.
        int64_t value = ((int64_t)m_value << shift) >> shift;
        return LeanType_mkInt(lean_int64_to_int(value));
    } else {
        // Shift left and then back to crop the value.
        uint64_t value = (m_value << shift) >> shift;
        return LeanType_mkInt(lean_uint64_to_nat(value));
    }
}

/******************************************************************************
 * FLOATING POINT TYPE
 ******************************************************************************/

/** Constructor for floating point types from a value. */
LeanTypeFloat::LeanTypeFloat(double value) : LeanType(FLOAT) { m_value = value; }

/** Constructor for floating point types from an object. */
LeanTypeFloat::LeanTypeFloat(b_lean_obj_arg obj)
    : LeanTypeFloat(lean_unbox_float(obj)) {}

void *LeanTypeFloat::to_buffer(CType *ct) {
    switch (ct->get_tag()) {
    case CType::FLOAT:
        return new float(m_value);
    case CType::DOUBLE:
        return new double(m_value);
    case CType::LONGDOUBLE:
        return new long double(m_value);
    default:
        lean_internal_panic("invalid type");
    }
}

/** Convert the type to a Lean object. */
lean_obj_res LeanTypeFloat::box(CType *ct) { return LeanType_mkFloat(m_value); }

/******************************************************************************
 * EXPOSED C FUNCTIONS
 ******************************************************************************/

/** Testing of LeanType. */
extern "C" lean_obj_res LeanType_test(b_lean_obj_arg type, lean_object *unused) {
    LeanType *tp = LeanType::unbox(type);
    delete tp;
    return lean_io_result_mk_ok(lean_box(0));
}
