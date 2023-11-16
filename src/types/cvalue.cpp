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

#include "cvalue.hpp"
#include <ffi.h>
#include <memory>

using namespace std::complex_literals;

/** Unbox the type into a CType class. */
std::unique_ptr<CValue> CValue::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    // TODO: Reference counting?
    auto type = CType::unbox(CValue_type(obj));

    assert(type->get_tag() == tag);

    // TODO: Get rid of this mess, or at least simplify it.
    switch (tag) {
    case VOID:
        return std::make_unique<CValueVoid>(type);
    case INT8:
        return std::make_unique<CValueScalar<INT8>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case INT16:
        return std::make_unique<CValueScalar<INT16>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case INT32:
        return std::make_unique<CValueScalar<INT32>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case INT64:
        return std::make_unique<CValueScalar<INT64>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case UINT8:
        return std::make_unique<CValueScalar<UINT8>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case UINT16:
        return std::make_unique<CValueScalar<UINT16>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case UINT32:
        return std::make_unique<CValueScalar<UINT32>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case UINT64:
        return std::make_unique<CValueScalar<UINT64>>(
            type, lean_uint64_of_nat(lean_ctor_get(obj, 0)));
    case FLOAT:
        return std::make_unique<CValueScalar<FLOAT>>(type, lean_unbox_float(obj));
    case DOUBLE:
        return std::make_unique<CValueScalar<DOUBLE>>(type, lean_unbox_float(obj));
    case LONGDOUBLE:
        return std::make_unique<CValueScalar<LONGDOUBLE>>(type, lean_unbox_float(obj));
    case COMPLEX_FLOAT: {
        auto real = lean_ctor_get_float(obj, 0);
        auto imag = lean_ctor_get_float(obj, sizeof(double));
        return std::make_unique<CValueScalar<COMPLEX_FLOAT>>(
            type, (std::complex<float>)(real + imag * 1i));
    }
    case COMPLEX_DOUBLE: {
        auto real = lean_ctor_get_float(obj, 0);
        auto imag = lean_ctor_get_float(obj, sizeof(double));
        return std::make_unique<CValueScalar<COMPLEX_DOUBLE>>(
            type, (std::complex<double>)(real + imag * 1i));
    }
    case COMPLEX_LONGDOUBLE: {
        auto real = lean_ctor_get_float(obj, 0);
        auto imag = lean_ctor_get_float(obj, sizeof(double));
        return std::make_unique<CValueScalar<COMPLEX_LONGDOUBLE>>(
            type, (std::complex<long double>)(real + imag * 1i));
    }
    case POINTER:
        break;
    case STRUCT:
        break;
    default:
        lean_internal_panic_unreachable();
    }
    lean_internal_panic_unreachable();
}

/** Create a value from a type and a buffer. */
std::unique_ptr<CValue> CValue::from_buffer(std::unique_ptr<CType> &type,
                                            const uint8_t *buffer) {
    switch (type->get_tag()) {
    case VOID:
        return std::make_unique<CValueVoid>(type);
    case INT8:
        break;
    case INT16:
        break;
    case INT32:
        break;
    case INT64:
        break;
    case UINT8:
        break;
    case UINT16:
        break;
    case UINT32:
        break;
    case UINT64:
        break;
    case FLOAT:
        break;
    case DOUBLE:
        break;
    case LONGDOUBLE:
        break;
    case COMPLEX_FLOAT:
        break;
    case COMPLEX_DOUBLE:
        break;
    case COMPLEX_LONGDOUBLE:
        break;
    case POINTER:
        break;
    case STRUCT:
        break;
    default:
        lean_internal_panic_unreachable();
    }
    lean_internal_panic_unreachable();
}
