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
    if (tag > LAST_PRIMITIVE)
        throw std::invalid_argument("tag is not for a primitive type");
    const ffi_type *tp = type_map[tag];

    // TODO: Bad practice to do it this way. This should be done in a copy constructor
    // or something similar. CType should probably not make ffi_type a public superclass
    // at all.
    size = tp->size;
    alignment = tp->alignment;
    type = tp->type;
    elements = tp->elements;
}

// Constructor for array types.
CType::CType(CType *tp, size_t length) {
    m_tag = ARRAY;
    size = 0;
    alignment = 0;
    type = FFI_TYPE_STRUCT;

    elements = new ffi_type *[length + 1]();
    for (size_t i = 0; i < length; i++)
        elements[i] = tp;

    // Initialize size and alignment fields.
    ffi_get_struct_offsets(FFI_DEFAULT_ABI, this, nullptr);
}

// Constructor for struct types.
CType::CType(std::vector<CType *> e) {
    m_tag = STRUCT;
    size = 0;
    alignment = 0;
    type = FFI_TYPE_STRUCT;

    elements = new ffi_type *[e.size() + 1]();
    for (size_t i = 0; i < e.size(); i++)
        elements[i] = e[i];

    // Initialize size and alignment fields.
    ffi_get_struct_offsets(FFI_DEFAULT_ABI, this, nullptr);
}

CType::~CType() {
    switch (m_tag) {
    case ARRAY:
        if (elements) {
            if (elements[0])
                delete (CType *)elements[0];
            delete[] elements;
        }
        break;
    case STRUCT:
        if (elements) {
            for (size_t i = 0; elements[i]; i++)
                delete (CType *)elements[i];
            delete[] elements;
        }
        break;
    case UNION:
        lean_internal_panic("union not implemented");
    default:
        assert(elements == nullptr || is_complex());
    }
}

/** Unbox the type into a CType class. */
CType *CType::unbox(b_lean_obj_arg obj) {
    ObjectTag tag = (ObjectTag)lean_obj_tag(obj);
    if (tag <= LAST_PRIMITIVE) {
        return new CType(tag);
    } else if (tag == ARRAY) {
        size_t length = lean_unbox(lean_ctor_get(obj, 1));

        // If the length is 0 we would lose the reference to tp, so we don't even unbox
        // it. Technically this is illegal anyway, but we support it for consistency.
        if (length == 0) {
            return new CType(nullptr, length);
        } else {
            CType *type = CType::unbox(lean_ctor_get(obj, 0));
            return new CType(type, length);
        }
    } else if (tag == STRUCT) {
        std::vector<CType *> elements;
        lean_object *array = lean_ctor_get(obj, 0);
        assert(lean_is_array(array));

        for (size_t i = 0; i < lean_array_size(array); i++) {
            CType *tp = CType::unbox(lean_array_get_core(array, i));
            elements.push_back(tp);
        }
        return new CType(elements);
    } else if (tag == UNION) {
        lean_internal_panic("union not implemented");
    }
    lean_internal_panic_unreachable();
}

/** Get the number of elements. */
size_t CType::get_nelements() {
    if (elements) {
        size_t n = 0;
        while (elements[n])
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
    ffi_status status = ffi_get_struct_offsets(FFI_DEFAULT_ABI, this, offs);

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
    CType *tp = CType::unbox(type);
    size_t size = tp->get_size();
    delete tp;
    return lean_box(size);
}

/** Get the alignment of a type. */
extern "C" lean_obj_res CType_alignment(b_lean_obj_arg type) {
    CType *tp = CType::unbox(type);
    size_t alignment = tp->get_alignment();
    delete tp;
    return lean_box(alignment);
}

/** Get the offsets of a type. */
extern "C" lean_obj_res CType_offsets(b_lean_obj_arg type) {
    CType *tp = CType::unbox(type);
    std::vector<size_t> offsets = tp->get_offsets();
    delete tp;

    lean_object *array = lean_alloc_array(offsets.size(), offsets.size());
    for (size_t i = 0; i < offsets.size(); i++)
        lean_array_set_core(array, i, lean_box(offsets[i]));

    return array;
}
