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

#include "pointer.hpp"
#include "lean/lean.h"
#include "types.hpp"
#include "utils.hpp"
#include <algorithm>
#include <complex>
#include <cstdint>
#include <stdexcept>

/**
 * Dereference the pointer.
 */
extern "C" lean_obj_res Pointer_read(b_lean_obj_arg ptr, b_lean_obj_arg type,
                                     lean_object *unused) {
    auto p = Pointer::unbox(ptr);
    auto ct = CType::unbox(type);

    try {
        return lean_io_result_mk_ok(p->read(ct)->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Write to the pointer address.
 */
extern "C" lean_obj_res Pointer_write(b_lean_obj_arg ptr, b_lean_obj_arg value,
                                      lean_object *unused) {
    auto p = Pointer::unbox(ptr);
    try {
        p->write(*CValue::unbox(value));
        return lean_io_result_mk_ok(lean_box(0));
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Create a pointer from an address.
 */
extern "C" lean_obj_res Pointer_mk(size_t value) {
    return (new Pointer((uint8_t *)value))->box();
}

/**
 * Get the address of a pointer.
 */
extern "C" size_t Pointer_address(b_lean_obj_arg obj) {
    return (size_t)Pointer::unbox(obj)->get_pointer();
}

/**
 * Call a pointer with CValue arguments.
 */
extern "C" lean_obj_res Pointer_call(b_lean_obj_arg ptr_obj, b_lean_obj_arg rt_obj,
                                     b_lean_obj_arg args_obj, b_lean_obj_arg vargs_obj,
                                     lean_object *unused) {

    lean_object *err = lean_mk_io_user_error(lean_mk_string("not implemented"));
    return lean_io_result_mk_error(err);
}
