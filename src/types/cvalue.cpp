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
    // TODO: Move at least lean_ctor_get() etc. to constructor?
    switch (tag) {
    case VOID:
        return std::make_unique<CValueVoid>(type);
    case INT8:
        return std::make_unique<CValueInt<INT8>>(type, obj);
    case INT16:
        return std::make_unique<CValueInt<INT16>>(type, obj);
    case INT32:
        return std::make_unique<CValueInt<INT32>>(type, obj);
    case INT64:
        return std::make_unique<CValueInt<INT64>>(type, obj);
    case UINT8:
        return std::make_unique<CValueNat<UINT8>>(type, obj);
    case UINT16:
        return std::make_unique<CValueNat<UINT16>>(type, obj);
    case UINT32:
        return std::make_unique<CValueNat<UINT32>>(type, obj);
    case UINT64:
        return std::make_unique<CValueNat<UINT64>>(type, obj);
    case FLOAT:
        return std::make_unique<CValueFloat<FLOAT>>(type, obj);
    case DOUBLE:
        return std::make_unique<CValueFloat<DOUBLE>>(type, obj);
    case LONGDOUBLE:
        return std::make_unique<CValueFloat<LONGDOUBLE>>(type, obj);
    case COMPLEX_FLOAT:
        return std::make_unique<CValueComplex<COMPLEX_FLOAT>>(type, obj);
    case COMPLEX_DOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_DOUBLE>>(type, obj);
    case COMPLEX_LONGDOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_LONGDOUBLE>>(type, obj);
    case POINTER:
        lean_internal_panic("pointer boxing not implemented");
    case STRUCT:
        lean_internal_panic("struct boxing not implemented");
    default:
        lean_internal_panic("unknown tag");
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
        return std::make_unique<CValueInt<INT8>>(type, buffer);
    case INT16:
        return std::make_unique<CValueInt<INT16>>(type, buffer);
    case INT32:
        return std::make_unique<CValueInt<INT32>>(type, buffer);
    case INT64:
        return std::make_unique<CValueInt<INT64>>(type, buffer);
    case UINT8:
        return std::make_unique<CValueNat<UINT8>>(type, buffer);
    case UINT16:
        return std::make_unique<CValueNat<UINT16>>(type, buffer);
    case UINT32:
        return std::make_unique<CValueNat<UINT32>>(type, buffer);
    case UINT64:
        return std::make_unique<CValueNat<UINT64>>(type, buffer);
    case FLOAT:
        return std::make_unique<CValueFloat<FLOAT>>(type, buffer);
    case DOUBLE:
        return std::make_unique<CValueFloat<DOUBLE>>(type, buffer);
    case LONGDOUBLE:
        return std::make_unique<CValueFloat<LONGDOUBLE>>(type, buffer);
    case COMPLEX_FLOAT:
        return std::make_unique<CValueComplex<COMPLEX_FLOAT>>(type, buffer);
    case COMPLEX_DOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_DOUBLE>>(type, buffer);
    case COMPLEX_LONGDOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_LONGDOUBLE>>(type, buffer);
    case POINTER:
        break;
    case STRUCT:
        break;
    default:
        lean_internal_panic_unreachable();
    }
    lean_internal_panic_unreachable();
}
