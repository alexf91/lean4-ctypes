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
#include <lean/lean.h>

/** CType for void types. */
class CTypeVoid : public CType {
  public:
    CTypeVoid() : CType(VOID) {}
    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        return std::make_unique<LeanValueUnit>();
    }
    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        throw std::runtime_error("invalid cast: can't cast value to void type");
    }
};