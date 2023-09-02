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

#include "basic_type.hpp"
#include <complex>
#include <ffi.h>
#include <lean/lean.h>

/**
 * Basic and composite types in C.
 */
class CType : public ffi_type {
  public:
    CType() {}
    ~CType() {}
    CType *unbox(b_lean_obj_arg obj);

  private:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        CTYPE_VOID,
        CTYPE_BASIC,
        CTYPE_POINTER,
        CTYPE_ARRAY,
        CTYPE_STRUCT,
    };
};

/** Free an ffi_type. */
void ffi_type_free(ffi_type *tp);
