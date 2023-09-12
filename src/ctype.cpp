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

#include "ctype.hpp"
#include <lean/lean.h>
#include <stdexcept>

/**
 * Primitive types defined in libffi. They are in the same order as the CType
 * enum. They are statically allocated.
 */
const ffi_type *CType::type_map[] = {
    &ffi_type_void,          &ffi_type_sint8,          &ffi_type_sint16,
    &ffi_type_sint32,        &ffi_type_sint64,         &ffi_type_uint8,
    &ffi_type_uint16,        &ffi_type_uint32,         &ffi_type_uint64,
    &ffi_type_float,         &ffi_type_double,         &ffi_type_longdouble,
    &ffi_type_complex_float, &ffi_type_complex_double, &ffi_type_complex_longdouble,
    &ffi_type_pointer,
};

/** Names of primitive types for debugging. */
const char *CType::name_map[] = {
    "void",    "int8",       "int16",         "int32",          "int64",
    "uint8",   "uint16",     "uint32",        "uint64",         "float",
    "double",  "longdouble", "complex_float", "complex_double", "complex_longdouble",
    "pointer", "array",      "struct",        "union"};

/** Constructor for primitive types. */
CType::CType(ObjectTag tag) : m_tag(tag) {
    m_ffi_type = {0};
    if (tag <= LAST_PRIMITIVE) {
        const ffi_type *tp = type_map[tag];
        m_ffi_type.size = tp->size;
        m_ffi_type.alignment = tp->alignment;
        m_ffi_type.type = tp->type;
        m_ffi_type.elements = tp->elements;
    }
}

/** Unbox the type into a CType class. */
std::unique_ptr<CType> CType::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    switch (tag) {
    case VOID:
        return std::make_unique<CTypeVoid>();
    case INT8:
        return std::make_unique<CTypeInt<int8_t>>(tag);
    case INT16:
        return std::make_unique<CTypeInt<int16_t>>(tag);
    case INT32:
        return std::make_unique<CTypeInt<int32_t>>(tag);
    case INT64:
        return std::make_unique<CTypeInt<int64_t>>(tag);
    case UINT8:
        return std::make_unique<CTypeInt<uint8_t>>(tag);
    case UINT16:
        return std::make_unique<CTypeInt<uint16_t>>(tag);
    case UINT32:
        return std::make_unique<CTypeInt<uint32_t>>(tag);
    case UINT64:
        return std::make_unique<CTypeInt<uint64_t>>(tag);
    case FLOAT:
        return std::make_unique<CTypeFloat<float>>(tag);
    case DOUBLE:
        return std::make_unique<CTypeFloat<double>>(tag);
    case LONGDOUBLE:
        return std::make_unique<CTypeFloat<long double>>(tag);
    case COMPLEX_FLOAT:
        return std::make_unique<CTypeComplex<std::complex<float>>>(tag);
    case COMPLEX_DOUBLE:
        return std::make_unique<CTypeComplex<std::complex<double>>>(tag);
    case COMPLEX_LONGDOUBLE:
        return std::make_unique<CTypeComplex<std::complex<long double>>>(tag);
    case POINTER:
        return std::make_unique<CTypePointer>();
    case ARRAY:
        return std::make_unique<CTypeArray>(lean_ctor_get(obj, 0),
                                            lean_ctor_get(obj, 1));
    case STRUCT:
        return std::make_unique<CTypeStruct>(lean_ctor_get(obj, 0));
    case UNION:
        lean_internal_panic("UNION not supported");
    default:
        lean_internal_panic_unreachable();
    }
}

/** Get the number of elements. */
size_t CType::get_nelements() {
    if (m_ffi_type.elements) {
        size_t n = 0;
        while (m_ffi_type.elements[n])
            n++;
        return n;
    }
    return 0;
}

/** Get the array of struct offsets. */
std::vector<size_t> CType::get_offsets() {
    size_t n = get_nelements();
    std::vector<size_t> offsets;

    size_t offs[n];
    ffi_status status = ffi_get_struct_offsets(FFI_DEFAULT_ABI, &m_ffi_type, offs);

    if (status == FFI_BAD_TYPEDEF)
        return offsets;

    if (status != FFI_OK)
        lean_internal_panic("ffi_get_struct_offsets failed");

    for (size_t i = 0; i < n; i++)
        offsets.push_back(offs[i]);
    return offsets;
}

/** Get the size of a type. */
extern "C" lean_obj_res CType_size(b_lean_obj_arg type) {
    auto tp = CType::unbox(type);
    size_t size = tp->get_size();
    return lean_box(size);
}

/** Get the alignment of a type. */
extern "C" lean_obj_res CType_alignment(b_lean_obj_arg type) {
    auto tp = CType::unbox(type);
    size_t alignment = tp->get_alignment();
    return lean_box(alignment);
}

/** Get the offsets of a type. */
extern "C" lean_obj_res CType_offsets(b_lean_obj_arg type) {
    auto tp = CType::unbox(type);
    std::vector<size_t> offsets = tp->get_offsets();

    lean_object *array = lean_alloc_array(offsets.size(), offsets.size());
    for (size_t i = 0; i < offsets.size(); i++)
        lean_array_set_core(array, i, lean_box(offsets[i]));

    return array;
}
