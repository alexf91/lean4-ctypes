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

#include "common.hpp"
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

class CType {
  public:
    /** Basic constructor for the base type. */
    CType(ObjectTag tag);

    virtual ~CType() {}

    /** Convert from Lean to this class. */
    static std::unique_ptr<CType> unbox(b_lean_obj_arg obj);

    /** Get the size of the basic type. */
    size_t size() const { return m_ffi_type.size; }

    /** Get alignment. */
    size_t alignment() const { return m_ffi_type.alignment; }

    /** Get the number of elements. */
    size_t nelements() const;

    /** Get the array of struct offsets. */
    const std::vector<size_t> offsets() const;

    /** Get a pointer to the internal ffi_type. */
    ffi_type *ffitype() { return &m_ffi_type; }

    /** Get the tag of the CType. */
    ObjectTag tag() const { return m_tag; }

  protected:
    ffi_type m_ffi_type;

  private:
    ObjectTag m_tag;
};

/** Primitive types that are already defined in ffi.h. */
class CTypePrimitive : public CType {
  public:
    /** Basic constructor for the base type. */
    CTypePrimitive(ObjectTag tag) : CType(tag) {}
};

/** Composite types that require manual construction. */
class CTypeStruct : public CType {
  public:
    /** Create type from already existing and initialized types. */
    CTypeStruct(std::vector<std::unique_ptr<CType>> members);

    /** Create type from a Lean array of CType types. */
    CTypeStruct(b_lean_obj_arg members);

    ~CTypeStruct();

    /** Get elements in the struct. */
    const std::vector<CType *> elements() const {
        std::vector<CType *> elements;
        for (auto &e : m_element_types)
            elements.push_back(e.get());
        return elements;
    }

  private:
    /** Initialize the FFI type. */
    void populate_ffi_type();

    std::vector<std::unique_ptr<CType>> m_element_types;
};
