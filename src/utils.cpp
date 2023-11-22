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

#include "utils.hpp"
#include "pointer.hpp"
#include <cstdlib>
#include <lean/lean.h>

/** Allocate a buffer. */
extern "C" lean_obj_res Utils_malloc(b_lean_obj_arg size_obj) {
    size_t size = lean_unbox(size_obj);
    void *buffer = calloc(size, sizeof(char));

    if (buffer == nullptr)
        lean_internal_panic_out_of_memory();

    return lean_io_result_mk_ok((new Pointer((uint8_t *)buffer))->box());
}

/** Free a buffer. */
extern "C" lean_obj_res Utils_free(b_lean_obj_arg pointer_obj) {
    free(Pointer::unbox(pointer_obj)->pointer());
    return lean_io_result_mk_ok(lean_box(0));
}
