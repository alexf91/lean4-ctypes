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

/** Map from ObjectTag to primitive FFI type. */
extern const ffi_type *ffi_type_map[];

/**
 * Constructor tags for the CType and CValue types in Lean.
 * Keep them in sync.
 */
enum ObjectTag {
    VOID,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT,
    DOUBLE,
    LONGDOUBLE,
    COMPLEX_FLOAT,
    COMPLEX_DOUBLE,
    COMPLEX_LONGDOUBLE,
    POINTER,
    STRUCT,
    LENGTH
};
