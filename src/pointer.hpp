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

#include "external_type.hpp"
#include "types.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

/** A pointer in C. */
class Pointer final : public ExternalType<Pointer> {
  public:
    Pointer(uint8_t *pointer) : m_pointer(pointer) {}

    ~Pointer() {}

    /** Read a CType from the memory, creating a CValue. */
    std::unique_ptr<CValue> read(const CType &type) {
        return CValue::from_buffer(type, m_pointer);
    }

    /** Write a value to the memory location. */
    void write(const CValue &value) {
        memcpy(m_pointer, value.to_buffer().get(), value.type()->size());
    }

    /** Call the pointer as a function. */
    std::unique_ptr<CValue> call(CType &rtype,
                                 std::vector<std::unique_ptr<CValue>> &args,
                                 std::vector<std::unique_ptr<CValue>> &vargs);

    /** Get the address of the buffer. */
    uint8_t *pointer() const { return m_pointer; }

    /** No children. */
    const std::vector<lean_object *> children() { return {}; }

  private:
    // Address of the pointer.
    uint8_t *m_pointer;
};
