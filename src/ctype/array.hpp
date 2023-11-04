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
#include <cstring>
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <stdexcept>
#include <vector>

/** CType for array types. */
class CTypeArray : public CType {
  public:
    CTypeArray(std::unique_ptr<CType> tp, size_t length)
        : CType(ARRAY), m_element_type(std::move(tp)), m_length(length) {
        m_ffi_type.type = FFI_TYPE_STRUCT;

        // We reuse the unique pointer, but we consider this in the destructor.
        m_ffi_type.elements = new ffi_type *[length + 1]();
        for (size_t i = 0; i < length; i++)
            m_ffi_type.elements[i] = m_element_type->get_ffi_type();

        // Initialize size and alignment fields.
        ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, nullptr);
    }

    CTypeArray(b_lean_obj_arg type, b_lean_obj_arg length)
        : CTypeArray(CType::unbox(type), lean_unbox(length)) {}

    ~CTypeArray() {
        assert(m_ffi_type.elements);
        delete[] m_ffi_type.elements;
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        std::vector<std::unique_ptr<LeanValue>> values;
        size_t size = m_element_type->get_size();
        for (size_t i = 0; i < m_length; i++)
            values.push_back(m_element_type->instance(buffer + i * size));
        return std::make_unique<LeanValueArray>(std::move(values));
    }

    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        if (value.get_tag() != LeanValue::ARRAY)
            throw std::runtime_error(
                "invalid cast: can't cast non-array type to array");

        auto values = reinterpret_cast<const LeanValueArray &>(value).get_values();
        if (m_length != values.size())
            throw std::runtime_error("invalid cast: wrong number of values");

        std::unique_ptr<uint8_t[]> buffer(new uint8_t[get_size()]);
        const std::vector<size_t> offsets = get_offsets();

        for (size_t i = 0; i < values.size(); i++) {
            auto buf = m_element_type->buffer(*values[i]);
            size_t sz = m_element_type->get_size();
            size_t off = offsets[i];
            memcpy(buffer.get() + off, buf.get(), sz);
        }
        return buffer;
    }

  private:
    std::unique_ptr<CType> m_element_type;
    size_t m_length;
};
