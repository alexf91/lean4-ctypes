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

#include "types.hpp"
#include <ccomplex>
#include <stdlib.h>

extern "C" {

/**
 * Primitive types defined in libffi. They are in the same order as the CType
 * enum. They are statically allocated.
 */
static const ffi_type *BasicType_ffi_types[] = {
    &ffi_type_sint8,          &ffi_type_uint8,
    &ffi_type_sint16,         &ffi_type_uint16,
    &ffi_type_sint32,         &ffi_type_uint32,
    &ffi_type_sint64,         &ffi_type_uint64,
    &ffi_type_float,          &ffi_type_double,
    &ffi_type_longdouble,     &ffi_type_complex_float,
    &ffi_type_complex_double, &ffi_type_complex_longdouble,
};

/** Names of primitive types for debugging. */
__attribute__((unused)) static const char *BasicType_ffi_types_name[] = {
    "ffi_type_sint8",          "ffi_type_uint8",
    "ffi_type_sint16",         "ffi_type_uint16",
    "ffi_type_sint32",         "ffi_type_uint32",
    "ffi_type_sint64",         "ffi_type_uint64",
    "ffi_type_float",          "ffi_type_double",
    "ffi_type_longdouble",     "ffi_type_complex_float",
    "ffi_type_complex_double", "ffi_type_complex_longdouble",
};

/** Size of basic types. */
size_t BasicType_sizeof(uint8_t type) {
    switch (type) {
    case BASIC_TYPE_INT8:
        return sizeof(int8_t);
    case BASIC_TYPE_UINT8:
        return sizeof(uint8_t);
    case BASIC_TYPE_INT16:
        return sizeof(int16_t);
    case BASIC_TYPE_UINT16:
        return sizeof(uint16_t);
    case BASIC_TYPE_INT32:
        return sizeof(int32_t);
    case BASIC_TYPE_UINT32:
        return sizeof(uint32_t);
    case BASIC_TYPE_INT64:
        return sizeof(int64_t);
    case BASIC_TYPE_UINT64:
        return sizeof(uint64_t);
    case BASIC_TYPE_FLOAT:
        return sizeof(float);
    case BASIC_TYPE_DOUBLE:
        return sizeof(double);
    case BASIC_TYPE_LONGDOUBLE:
        return sizeof(long double);
    case BASIC_TYPE_COMPLEX_FLOAT:
        return sizeof(std::complex<float>);
    case BASIC_TYPE_COMPLEX_DOUBLE:
        return sizeof(std::complex<double>);
    case BASIC_TYPE_COMPLEX_LONGDOUBLE:
        return sizeof(std::complex<long double>);
    }
    lean_internal_panic_unreachable();
}

lean_obj_res BasicType_sizeof_Nat(uint8_t type) {
    return lean_box(BasicType_sizeof(type));
}

/** Check if a type is statically allocated. */
static bool is_static(ffi_type *tp) {
    for (size_t i = 0; i < sizeof(BasicType_ffi_types) / sizeof(BasicType_ffi_types[0]);
         i++)
        if (tp == BasicType_ffi_types[i])
            return true;
    return tp == &ffi_type_void || tp == &ffi_type_pointer;
}

/**
 * Free an ffi_type.
 */
void ffi_type_free(ffi_type *tp) {
    if (is_static(tp))
        return;

    // For arrays and struct types.
    // TODO: Let's hope there is nothing static in there that we don't have in the
    //       primitive_types array.
    for (size_t i = 0; tp->elements && tp->elements[i]; i++)
        ffi_type_free(tp->elements[i]);
    free(tp->elements);
    free(tp);
}
}
