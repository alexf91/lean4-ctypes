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

#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

struct CType {
    /** Convert from Lean to this class. */
    static std::unique_ptr<CType> unbox(b_lean_obj_arg obj);

    /** Get the size of the basic type. */
    size_t get_size() const { return 0; }

    /** Get alignment. */
    size_t get_alignment() const { return 0; }

    /** Get the number of elements. */
    size_t get_nelements() const { return 0; }

    /** Get the array of struct offsets. */
    const std::vector<size_t> get_offsets() const;

    /** Get a pointer to the internal ffi_type. */
    ffi_type *get_ffi_type() { return nullptr; }
};

struct CTypeScalar : CType {};

struct CTypePointer : CType {};

struct CTypeStruct : CType {};

struct CValue {
    static CValue *unbox(...) { return nullptr; }
    static std::unique_ptr<CValue> from_buffer(...) { return nullptr; }
    lean_obj_res box() { return nullptr; }
    std::unique_ptr<uint8_t *> to_buffer() const { return nullptr; }
    const CType *get_type() const { return nullptr; }
};

template <typename T> struct CValueScalar : CValue {};

struct CValuePointer : CValue {};

struct CValueStruct : CValue {};
