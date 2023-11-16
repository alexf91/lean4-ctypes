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

#include <lean/lean.h>
#include <stdexcept>

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
