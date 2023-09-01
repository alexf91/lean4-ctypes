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

extern "C" {

/** Basic types enum */
enum BasicType {
    BASIC_TYPE_INT8,
    BASIC_TYPE_UINT8,
    BASIC_TYPE_INT16,
    BASIC_TYPE_UINT16,
    BASIC_TYPE_INT32,
    BASIC_TYPE_UINT32,
    BASIC_TYPE_INT64,
    BASIC_TYPE_UINT64,
    BASIC_TYPE_FLOAT,
    BASIC_TYPE_DOUBLE,
    BASIC_TYPE_LONGDOUBLE,
    BASIC_TYPE_COMPLEX_FLOAT,
    BASIC_TYPE_COMPLEX_DOUBLE,
    BASIC_TYPE_COMPLEX_LONGDOUBLE,
};

/** CType enum (object tags). */
enum CType {
    CTYPE_VOID,
    CTYPE_BASIC,
    CTYPE_POINTER,
    CTYPE_ARRAY,
    CTYPE_STRUCT,
};

/** Size of basic types. */
size_t BasicType_sizeof(uint8_t type);

/** Free an ffi_type. */
void ffi_type_free(ffi_type *tp);
}
