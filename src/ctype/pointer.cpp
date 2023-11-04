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

#include "pointer.hpp"
#include "../leanvalue.hpp"
#include "../pointer.hpp"

#include <cstdint>
#include <memory>

std::unique_ptr<uint8_t[]> CTypePointer::buffer(const LeanValue &value) const {
    if (value.get_tag() != LeanValue::POINTER)
        throw std::runtime_error(
            "invalid cast: can't cast non-pointer type to pointer");

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[sizeof(void *)]);
    // TODO: This is a forward declared type...
    auto m = reinterpret_cast<const LeanValuePointer &>(value).get_memory();

    *((void **)buffer.get()) = m->get_address();

    return buffer;
}
