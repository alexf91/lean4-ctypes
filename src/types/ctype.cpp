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

#include "ctype.hpp"
#include "common.hpp"
#include <ffi.h>
#include <memory>

/******************************************************************************
 * Shared methods for the base class and primitive types.
 ******************************************************************************/

/** Constructor for primitive types. */
CType::CType(ObjectTag tag) : m_tag(tag) {
    m_ffi_type = {0};
    if (tag < STRUCT) {
        const ffi_type *tp = ffi_type_map[tag];
        m_ffi_type.size = tp->size;
        m_ffi_type.alignment = tp->alignment;
        m_ffi_type.type = tp->type;
        m_ffi_type.elements = tp->elements;
    }
}

/** Unbox the type into a CType class. */
std::unique_ptr<CType> CType::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    if (tag < STRUCT) {
        return std::make_unique<CTypePrimitive>(tag);
    } else if (tag == STRUCT) {
        return std::make_unique<CTypeStruct>(lean_ctor_get(obj, 0));
    } else {
        lean_internal_panic("unknown type");
    }
}

/** Get the number of elements. */
size_t CType::nelements() const {
    if (m_ffi_type.elements) {
        size_t n = 0;
        while (m_ffi_type.elements[n])
            n++;
        return n;
    }
    return 0;
}

/** Get the array of struct offsets. */
const std::vector<size_t> CType::offsets() const {
    size_t n = nelements();
    std::vector<size_t> offsets;

    size_t offs[n];
    ffi_status status =
        ffi_get_struct_offsets(FFI_DEFAULT_ABI, (ffi_type *)&m_ffi_type, offs);

    if (status == FFI_BAD_TYPEDEF)
        return offsets;

    if (status != FFI_OK)
        lean_internal_panic("ffi_get_struct_offsets failed");

    for (size_t i = 0; i < n; i++)
        offsets.push_back(offs[i]);
    return offsets;
}

/******************************************************************************
 * Methods for the CTypeStruct type.
 ******************************************************************************/

/** Create type from already existing and initialized types. */
CTypeStruct::CTypeStruct(std::vector<std::unique_ptr<CType>> members)
    : CType(STRUCT), m_element_types(std::move(members)) {
    populate_ffi_type();
}

/** Create type from a Lean array of CType types. */
CTypeStruct::CTypeStruct(b_lean_obj_arg members) : CType(STRUCT) {
    std::vector<std::unique_ptr<CType>> elements;
    for (size_t i = 0; i < lean_array_size(members); i++) {
        auto tp = CType::unbox(lean_array_get_core(members, i));
        elements.push_back(std::move(tp));
    }
    m_element_types = std::move(elements);
    populate_ffi_type();
}

CTypeStruct::~CTypeStruct() {
    assert(m_ffi_type.elements);
    delete[] m_ffi_type.elements;
}

/** Initialize the FFI type. */
void CTypeStruct::populate_ffi_type() {
    m_ffi_type.type = FFI_TYPE_STRUCT;
    m_ffi_type.elements = new ffi_type *[m_element_types.size() + 1]();
    for (size_t i = 0; i < m_element_types.size(); i++)
        m_ffi_type.elements[i] = m_element_types[i]->ffitype();

    // Initialize size and alignment fields.
    ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, nullptr);
}
