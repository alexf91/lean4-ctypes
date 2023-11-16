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

#include <complex>
#include <cstdint>
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

/**
 * Type trait for scalar values.
 * This way we can bundle a few things together if needed.
 */
template <ObjectTag T> struct TagToType;

template <> struct TagToType<INT8> {
    using type = int8_t;
};
template <> struct TagToType<INT16> {
    using type = int16_t;
};
template <> struct TagToType<INT32> {
    using type = int32_t;
};
template <> struct TagToType<INT64> {
    using type = int64_t;
};
template <> struct TagToType<UINT8> {
    using type = uint8_t;
};
template <> struct TagToType<UINT16> {
    using type = uint16_t;
};
template <> struct TagToType<UINT32> {
    using type = uint32_t;
};
template <> struct TagToType<UINT64> {
    using type = uint64_t;
};
template <> struct TagToType<FLOAT> {
    using type = float;
};
template <> struct TagToType<DOUBLE> {
    using type = double;
};
template <> struct TagToType<LONGDOUBLE> {
    using type = long double;
};
template <> struct TagToType<COMPLEX_FLOAT> {
    using type = std::complex<float>;
};
template <> struct TagToType<COMPLEX_DOUBLE> {
    using type = std::complex<double>;
};
template <> struct TagToType<COMPLEX_LONGDOUBLE> {
    using type = std::complex<long double>;
};
