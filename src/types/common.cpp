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

#include "common.hpp"
#include <ffi.h>

/**
 * Primitive types defined in libffi. They are in the same order as the CType
 * enum. They are statically allocated.
 */
const ffi_type *ffi_type_map[] = {
    &ffi_type_void,          &ffi_type_sint8,          &ffi_type_sint16,
    &ffi_type_sint32,        &ffi_type_sint64,         &ffi_type_uint8,
    &ffi_type_uint16,        &ffi_type_uint32,         &ffi_type_uint64,
    &ffi_type_float,         &ffi_type_double,         &ffi_type_longdouble,
    &ffi_type_complex_float, &ffi_type_complex_double, &ffi_type_complex_longdouble,
    &ffi_type_pointer,
};
