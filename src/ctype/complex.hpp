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

#include "../leanvalue.hpp"
#include "ctype.hpp"

#include <complex>
#include <cstdint>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <stdexcept>
#include <type_traits>

/** CType for complex types. */
template <typename T> class CTypeComplex : public CType {
    static_assert(!std::is_scalar_v<T>); // TODO: Improve assertion

  public:
    CTypeComplex(ObjectTag tag) : CType(tag) {
        assert(FIRST_COMPLEX <= tag && tag <= LAST_COMPLEX);
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        T value = *((T *)buffer);
        return std::make_unique<LeanValueComplex>(value);
    }

    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[get_size()]);
        switch (value.get_tag()) {
        case LeanValue::NAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueNat &>(value).get_value();
            break;
        case LeanValue::INT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueInt &>(value).get_value();
            break;
        case LeanValue::FLOAT:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueFloat &>(value).get_value();
            break;
        case LeanValue::COMPLEX:
            *((T *)buffer.get()) =
                reinterpret_cast<const LeanValueComplex &>(value).get_value();
            break;
        default:
            throw std::runtime_error(
                "invalid cast: can't cast non-scalar or complex value to complex");
        }
        return buffer;
    }
};
