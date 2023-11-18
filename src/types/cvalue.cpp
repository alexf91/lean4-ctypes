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
#include "../pointer.hpp"
#include <ffi.h>
#include <memory>

/** Unbox the type into a CValue class. */
std::unique_ptr<CValue> CValue::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);

    // TODO: Simplify this?
    switch (tag) {
    case VOID:
        return std::make_unique<CValueVoid>();
    case INT8:
        return std::make_unique<CValueInt<INT8>>(obj);
    case INT16:
        return std::make_unique<CValueInt<INT16>>(obj);
    case INT32:
        return std::make_unique<CValueInt<INT32>>(obj);
    case INT64:
        return std::make_unique<CValueInt<INT64>>(obj);
    case UINT8:
        return std::make_unique<CValueNat<UINT8>>(obj);
    case UINT16:
        return std::make_unique<CValueNat<UINT16>>(obj);
    case UINT32:
        return std::make_unique<CValueNat<UINT32>>(obj);
    case UINT64:
        return std::make_unique<CValueNat<UINT64>>(obj);
    case FLOAT:
        return std::make_unique<CValueFloat<FLOAT>>(obj);
    case DOUBLE:
        return std::make_unique<CValueFloat<DOUBLE>>(obj);
    case LONGDOUBLE:
        return std::make_unique<CValueFloat<LONGDOUBLE>>(obj);
    case COMPLEX_FLOAT:
        return std::make_unique<CValueComplex<COMPLEX_FLOAT>>(obj);
    case COMPLEX_DOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_DOUBLE>>(obj);
    case COMPLEX_LONGDOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_LONGDOUBLE>>(obj);
    case POINTER:
        return std::make_unique<CValuePointer>(obj);
    case STRUCT:
        return std::make_unique<CValueStruct>(obj);
    default:
        lean_internal_panic("unknown tag");
    }
    lean_internal_panic_unreachable();
}

/** Create a value from a type and a buffer. */
std::unique_ptr<CValue> CValue::from_buffer(std::unique_ptr<CType> type,
                                            const uint8_t *buffer) {
    switch (type->get_tag()) {
    case VOID:
        return std::make_unique<CValueVoid>();
    case INT8:
        return std::make_unique<CValueInt<INT8>>(buffer);
    case INT16:
        return std::make_unique<CValueInt<INT16>>(buffer);
    case INT32:
        return std::make_unique<CValueInt<INT32>>(buffer);
    case INT64:
        return std::make_unique<CValueInt<INT64>>(buffer);
    case UINT8:
        return std::make_unique<CValueNat<UINT8>>(buffer);
    case UINT16:
        return std::make_unique<CValueNat<UINT16>>(buffer);
    case UINT32:
        return std::make_unique<CValueNat<UINT32>>(buffer);
    case UINT64:
        return std::make_unique<CValueNat<UINT64>>(buffer);
    case FLOAT:
        return std::make_unique<CValueFloat<FLOAT>>(buffer);
    case DOUBLE:
        return std::make_unique<CValueFloat<DOUBLE>>(buffer);
    case LONGDOUBLE:
        return std::make_unique<CValueFloat<LONGDOUBLE>>(buffer);
    case COMPLEX_FLOAT:
        return std::make_unique<CValueComplex<COMPLEX_FLOAT>>(buffer);
    case COMPLEX_DOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_DOUBLE>>(buffer);
    case COMPLEX_LONGDOUBLE:
        return std::make_unique<CValueComplex<COMPLEX_LONGDOUBLE>>(buffer);
    case POINTER:
        return std::make_unique<CValuePointer>(buffer);
    case STRUCT:
        return std::make_unique<CValueStruct>(std::move(type), buffer);
    default:
        lean_internal_panic_unreachable();
    }
    lean_internal_panic_unreachable();
}

/** Create the value from a CValue object. */
CValuePointer::CValuePointer(b_lean_obj_arg obj)
    : CValue(std::make_unique<CType>(POINTER)), m_pointer(lean_ctor_get(obj, 0)) {
    assert(lean_obj_tag(obj) == POINTER);
    lean_inc(m_pointer);
}

CValuePointer::CValuePointer(const uint8_t *buffer)
    : CValue(std::make_unique<CType>(POINTER)),
      m_pointer((new Pointer(*((uint8_t **)buffer)))->box()) {
    lean_inc(m_pointer);
}

std::unique_ptr<uint8_t[]> CValuePointer::to_buffer() const {
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[m_type->get_size()]);
    *((uint8_t **)buffer.get()) = Pointer::unbox(m_pointer)->get_pointer();
    return buffer;
}
