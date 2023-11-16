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

// #include "ctype/ctype.hpp"
// #include "ctype/pointer.hpp"
// #include "ctype/scalar.hpp"
// #include "ctype/struct.hpp"
// #include "ctype/void.hpp"

#include <lean/lean.h>

struct CType {
    static CType *unbox(...) { return nullptr; }
    lean_obj_res box() { return nullptr; }
};

struct CTypeScalar : CType {};

struct CTypePointer : CType {};

struct CTypeStruct : CType {};

struct CValue {
    static CValue *unbox(...) { return nullptr; }
    static CValue *from_buffer(...) { return nullptr; }
    lean_obj_res box() { return nullptr; }
    void *to_buffer() { return nullptr; }
};

template <typename T> struct CValueScalar : CValue {};

struct CValuePointer : CValue {};

struct CValueStruct : CValue {};
