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

/** CType for struct types. */
class CTypeStruct : public CType {
  public:
    CTypeStruct(std::vector<std::unique_ptr<CType>> members)
        : CType(STRUCT), m_element_types(std::move(members)) {
        init();
    }

    CTypeStruct(b_lean_obj_arg members) : CType(STRUCT) {
        std::vector<std::unique_ptr<CType>> elements;
        for (size_t i = 0; i < lean_array_size(members); i++) {
            auto tp = CType::unbox(lean_array_get_core(members, i));
            elements.push_back(std::move(tp));
        }
        m_element_types = std::move(elements);
        init();
    }

    ~CTypeStruct() {
        assert(m_ffi_type.elements);
        delete[] m_ffi_type.elements;
    }

    std::unique_ptr<LeanValue> instance(const uint8_t *buffer) const {
        std::vector<std::unique_ptr<LeanValue>> values;
        const std::vector<size_t> offsets = get_offsets();
        for (size_t i = 0; i < get_nelements(); i++) {
            size_t off = offsets[i];
            values.push_back(m_element_types[i]->instance(buffer + off));
        }
        return std::make_unique<LeanValueStruct>(std::move(values));
    }

    std::unique_ptr<uint8_t[]> buffer(const LeanValue &value) const {
        if (value.get_tag() != LeanValue::STRUCT)
            throw std::runtime_error("invalid cast: can't cast non-struct to struct");

        auto values = reinterpret_cast<const LeanValueStruct &>(value).get_values();
        if (m_element_types.size() != values.size())
            throw std::runtime_error("invalid cast: wrong number of values");

        std::unique_ptr<uint8_t[]> buffer(new uint8_t[get_size()]);
        const std::vector<size_t> offsets = get_offsets();

        for (size_t i = 0; i < values.size(); i++) {
            auto buf = m_element_types[i]->buffer(*values[i]);
            size_t sz = m_element_types[i]->get_size();
            size_t off = offsets[i];
            memcpy(buffer.get() + off, buf.get(), sz);
        }
        return buffer;
    }

  private:
    void init() {
        m_ffi_type.type = FFI_TYPE_STRUCT;

        m_ffi_type.elements = new ffi_type *[m_element_types.size() + 1]();
        for (size_t i = 0; i < m_element_types.size(); i++)
            m_ffi_type.elements[i] = m_element_types[i]->get_ffi_type();

        // Initialize size and alignment fields.
        ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, nullptr);
    }

    std::vector<std::unique_ptr<CType>> m_element_types;
};
