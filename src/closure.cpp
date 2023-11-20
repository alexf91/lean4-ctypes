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

#include "closure.hpp"
#include "lean/lean.h"
#include "pointer.hpp"
#include <stdexcept>

/** Create a closure. */
extern "C" lean_obj_res Closure_mk(b_lean_obj_arg rtype_obj, b_lean_obj_arg args_obj,
                                   lean_obj_arg cb_obj, lean_object *unused) {
    try {
        return lean_io_result_mk_ok((new Closure(rtype_obj, args_obj, cb_obj))->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/** Mark the closure for deletion. */
extern "C" lean_obj_res Closure_delete(b_lean_obj_arg closure_obj,
                                       lean_object *unused) {
    Closure::unbox(closure_obj)->del();
    return lean_io_result_mk_ok(lean_box(0));
}

/** Get the function pointer. */
extern "C" lean_obj_res Closure_pointer(b_lean_obj_arg closure_obj) {
    // Release the object before boxing. Once boxed, cleanup will be handled by Lean.
    return Closure::unbox(closure_obj)->pointer().release()->box();
}
