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

/** Unbox the type into a CType class. */
std::unique_ptr<CValue> CValue::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    // TODO: Reference counting?
    auto type = CType::unbox(CValue_type(obj));

    // This is not necessarily true in case we don't keep the CValue and CType enum
    // in sync. But it is for now and will probably stay this way.
    assert(type->get_tag() == tag);

    switch (tag) {
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
