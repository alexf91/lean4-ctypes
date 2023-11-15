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

#include "ctype.hpp"
#include "external_type.hpp"
#include "leanvalue.hpp"
#include <cassert>
#include <cstdlib>
#include <lean/lean.h>

class Pointer final : public ExternalType<Pointer> {
  public:
    Pointer(uint8_t *pointer) : m_pointer(pointer) {}

    ~Pointer() {}

    /** Read a CType from the memory, creating a LeanValue. */
    std::unique_ptr<LeanValue> read(const CType &type) {
        return type.instance(m_pointer);
    }

    /** Write a value to the memory location. */
    void write(const CType &type, const LeanValue &value) {
        memcpy(m_pointer, type.buffer(value).get(), type.get_size());
    }

    /** Get the address of the buffer. */
    uint8_t *get_pointer() const { return m_pointer; }

  private:
    // Address of the pointer.
    uint8_t *m_pointer;
};
