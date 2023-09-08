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
    case FLOAT:
        return std::make_unique<LeanValueFloat>(obj);
    case COMPLEX:
        return std::make_unique<LeanValueComplex>(obj);
    default:
        lean_internal_panic_unreachable();
    }
}

/** Convert from a buffer and a CType back to a LeanValue object.  */
std::unique_ptr<LeanValue> LeanValue::from_buffer(const CType &ct,
                                                  const uint8_t *buffer) {
    switch (ct.get_tag()) {
    case CType::VOID:
        return std::make_unique<LeanValueUnit>();
    case CType::INT8:
        return std::make_unique<LeanValueInt>((ssize_t)(*((const int8_t *)buffer)));
    case CType::INT16:
        return std::make_unique<LeanValueInt>((ssize_t)(*((const int16_t *)buffer)));
    case CType::INT32:
        return std::make_unique<LeanValueInt>((ssize_t)(*((const int32_t *)buffer)));
    case CType::INT64:
        return std::make_unique<LeanValueInt>((ssize_t)(*((const int64_t *)buffer)));
    case CType::UINT8:
        return std::make_unique<LeanValueInt>((size_t)(*((const uint8_t *)buffer)));
    case CType::UINT16:
        return std::make_unique<LeanValueInt>((size_t)(*((const uint16_t *)buffer)));
    case CType::UINT32:
        return std::make_unique<LeanValueInt>((size_t)(*((const uint32_t *)buffer)));
    case CType::UINT64:
        return std::make_unique<LeanValueInt>((size_t)(*((const uint64_t *)buffer)));
    case CType::FLOAT:
        return std::make_unique<LeanValueFloat>(*((const float *)buffer));
    case CType::DOUBLE:
        return std::make_unique<LeanValueFloat>(*((const double *)buffer));
    case CType::LONGDOUBLE:
        return std::make_unique<LeanValueFloat>(*((const long double *)buffer));
    case CType::COMPLEX_FLOAT:
        return std::make_unique<LeanValueComplex>(
            (std::complex<double>)(*((const std::complex<float> *)buffer)));
    case CType::COMPLEX_DOUBLE:
        return std::make_unique<LeanValueComplex>(
            (std::complex<double>)(*((const std::complex<double> *)buffer)));
    case CType::COMPLEX_LONGDOUBLE:
        return std::make_unique<LeanValueComplex>(
            (std::complex<double>)(*((const std::complex<long double> *)buffer)));
    case CType::POINTER:
        lean_internal_panic("POINTER not supported");
    case CType::ARRAY:
        lean_internal_panic("ARRAY not supported");
    case CType::STRUCT:
        lean_internal_panic("STRUCT not supported");
    case CType::UNION:
        lean_internal_panic("UNION not supported");
    default:
        lean_internal_panic_unreachable();
    }
}

/******************************************************************************
 * UNIT TYPE
 ******************************************************************************/

/** Constructor for the unit type. */
LeanValueUnit::LeanValueUnit() : LeanValue(UNIT) {}

/**
 * We can't create a buffer from the unit data type, as it can't be used as an
 * argument.
 */
std::unique_ptr<uint8_t[]> LeanValueUnit::to_buffer(const CType &ct) {
    lean_internal_panic("can't create buffer from unit type");
}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueUnit::box(const CType &ct) {
    if (ct.get_tag() != CType::VOID)
        lean_internal_panic("can't convert unit to other than void");
    return LeanValue_mkUnit(lean_box(0));
}

/******************************************************************************
 * INTEGER TYPE
 ******************************************************************************/

/** Constructor for integer types from a value. */
LeanValueInt::LeanValueInt(size_t value) : LeanValue(INT) { m_value = (uint64_t)value; }
LeanValueInt::LeanValueInt(ssize_t value) : LeanValue(INT) {
    m_value = (uint64_t)value;
}

/** Constructor for integer types from an object. */
LeanValueInt::LeanValueInt(b_lean_obj_arg obj)
    : LeanValueInt(lean_uint64_of_nat(lean_ctor_get(obj, 0))) {}

std::unique_ptr<uint8_t[]> LeanValueInt::to_buffer(const CType &ct) {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[ct.get_size()]);
    switch (ct.get_tag()) {
    case CType::INT8:
        *((int8_t *)buffer.get()) = (int8_t)m_value;
        break;
    case CType::UINT8:
        *((uint8_t *)buffer.get()) = (uint8_t)m_value;
        break;
    case CType::INT16:
        *((int16_t *)buffer.get()) = (int16_t)m_value;
        break;
    case CType::UINT16:
        *((uint16_t *)buffer.get()) = (uint16_t)m_value;
        break;
    case CType::INT32:
        *((int32_t *)buffer.get()) = (int32_t)m_value;
        break;
    case CType::UINT32:
        *((uint32_t *)buffer.get()) = (uint32_t)m_value;
        break;
    case CType::INT64:
        *((int64_t *)buffer.get()) = (int64_t)m_value;
        break;
    case CType::UINT64:
        *((uint64_t *)buffer.get()) = (uint64_t)m_value;
        break;
    default:
        lean_internal_panic("invalid type");
    }
    return buffer;
}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueInt::box(const CType &ct) {
    if (!ct.is_integer())
        lean_internal_panic("can't convert integer to non-integer type");

    int shift = (sizeof(uint64_t) - ct.get_size()) * 8;
    assert(shift >= 0);
    if (ct.is_signed()) {
        // Shift left and then back to sign extend the value.
        int64_t value = ((int64_t)m_value << shift) >> shift;
        return LeanValue_mkInt(lean_int64_to_int(value));
    } else {
        // Shift left and then back to crop the value.
        uint64_t value = (m_value << shift) >> shift;
        return LeanValue_mkInt(lean_uint64_to_nat(value));
    }
}

/******************************************************************************
 * FLOATING POINT TYPE
 ******************************************************************************/

/** Constructor for floating point types from a value. */
LeanValueFloat::LeanValueFloat(double value) : LeanValue(FLOAT) { m_value = value; }

/** Constructor for floating point types from an object. */
LeanValueFloat::LeanValueFloat(b_lean_obj_arg obj)
    : LeanValueFloat(lean_unbox_float(obj)) {}

std::unique_ptr<uint8_t[]> LeanValueFloat::to_buffer(const CType &ct) {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[ct.get_size()]);
    switch (ct.get_tag()) {
    case CType::FLOAT:
        *((float *)buffer.get()) = (float)m_value;
        break;
    case CType::DOUBLE:
        *((double *)buffer.get()) = (double)m_value;
        break;
    case CType::LONGDOUBLE:
        *((long double *)buffer.get()) = (long double)m_value;
        break;
    default:
        lean_internal_panic("invalid type");
    }
    return buffer;
}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueFloat::box(const CType &ct) {
    if (!ct.is_float())
        lean_internal_panic("can't convert float to non-float type");
    return LeanValue_mkFloat(m_value);
}

/******************************************************************************
 * COMPLEX FLOATING POINT TYPE
 ******************************************************************************/

/** Constructor for complex floating point types from a value. */
LeanValueComplex::LeanValueComplex(std::complex<double> value) : LeanValue(COMPLEX) {
    m_value = value;
}

/** Constructor for complex floating point types from an object. */
LeanValueComplex::LeanValueComplex(b_lean_obj_arg obj) : LeanValue(COMPLEX) {
    using namespace std::complex_literals;
    double real = lean_ctor_get_float(obj, 0);
    double imag = lean_ctor_get_float(obj, sizeof(double));
    m_value = real + imag * 1i;
}

std::unique_ptr<uint8_t[]> LeanValueComplex::to_buffer(const CType &ct) {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[ct.get_size()]);
    switch (ct.get_tag()) {
    case CType::COMPLEX_FLOAT:
        *((std::complex<float> *)buffer.get()) = (std::complex<float>)m_value;
        break;
    case CType::COMPLEX_DOUBLE:
        *((std::complex<double> *)buffer.get()) = (std::complex<double>)m_value;
        break;
    case CType::COMPLEX_LONGDOUBLE:
        *((std::complex<long double> *)buffer.get()) =
            (std::complex<long double>)m_value;
        break;
    default:
        lean_internal_panic("invalid type");
    }
    return buffer;
}

/** Convert the type to a Lean object. */
lean_obj_res LeanValueComplex::box(const CType &ct) {
    if (!ct.is_complex())
        lean_internal_panic("can't convert complex to non-complex type");
    return LeanValue_mkComplex(std::real(m_value), std::imag(m_value));
}
