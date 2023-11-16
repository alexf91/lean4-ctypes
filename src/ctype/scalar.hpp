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

#include <cstdint>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <stdexcept>
#include <type_traits>

/** CType for scalar types. */
template <typename T> class CTypeScalar : public CType {
    // static_assert(std::is_scalar_v<T>);

  public:
    CTypeScalar(ObjectTag tag) : CType(tag) {
        assert((FIRST_INT <= tag && tag <= LAST_INT) ||
               (FIRST_FLOAT <= tag && tag <= LAST_FLOAT) || tag == POINTER);
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        if (std::is_integral_v<T> && std::is_signed_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueInt>(value);
        } else if (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueNat>(value);
        } else if (std::is_floating_point_v<T>) {
            T value = *((T *)buffer);
            return std::make_unique<LeanValueFloat>(value);
        } else {
            lean_internal_panic_unreachable();
        }
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
        default:
            throw std::runtime_error(
                "invalid cast: can't cast non-scalar value to scalar");
        }
        return buffer;
    }
};
