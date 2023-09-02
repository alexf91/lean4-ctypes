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

#include "basic_type.hpp"

/**
 * Primitive types defined in libffi. They are in the same order as the CType
 * enum. They are statically allocated.
 */
const ffi_type *BasicType::type_map[] = {
    &ffi_type_sint8,          &ffi_type_uint8,
    &ffi_type_sint16,         &ffi_type_uint16,
    &ffi_type_sint32,         &ffi_type_uint32,
    &ffi_type_sint64,         &ffi_type_uint64,
    &ffi_type_float,          &ffi_type_double,
    &ffi_type_longdouble,     &ffi_type_complex_float,
    &ffi_type_complex_double, &ffi_type_complex_longdouble,
};

/** Names of primitive types for debugging. */
const char *BasicType::name_map[] = {
    "int8",           "uint8",
    "int16",          "uint16",
    "int32",          "uint32",
    "int64",          "uint64",
    "float",          "double",
    "longdouble",     "complex_float",
    "complex_double", "complex_longdouble",
};

/** Size of basic types. */
const size_t BasicType::size_map[] = {
    sizeof(int8_t),
    sizeof(uint8_t),
    sizeof(int16_t),
    sizeof(uint16_t),
    sizeof(int32_t),
    sizeof(uint32_t),
    sizeof(int64_t),
    sizeof(uint64_t),
    sizeof(float),
    sizeof(double),
    sizeof(long double),
    sizeof(std::complex<float>),
    sizeof(std::complex<double>),
    sizeof(std::complex<long double>),
};

/** Get the size of a type. */
extern "C" lean_obj_res BasicType_sizeof(uint8_t type) {
    BasicType tp = BasicType::unbox(type);
    return lean_box(tp.size());
}
