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

#include "symbol.hpp"
#include "library.hpp"
#include "utils.hpp"
#include <cstring>
#include <dlfcn.h>
#include <lean/lean.h>
#include <stdlib.h>

/**
 * Initialize the Symbol object by opening the symbol with dlsym().
 *
 * Raises an exception with an error message on error.
 */
Symbol::Symbol(b_lean_obj_arg lib, b_lean_obj_arg sym) {
    const char *name = lean_string_cstr(sym);
    Library *l = Library::unbox(lib);
    utils_log("opening '%s' in %s", name, l->get_name());

    // Clear dlerror() to distinguish between errors and NULL.
    dlerror();
    m_handle = dlsym(l->get_handle(), name);
    if (m_handle == nullptr) {
        char *msg = dlerror();
        if (msg != nullptr)
            throw msg;
    }
    // Make the library an owned object.
    lean_inc(lib);
    m_library = lib;
    m_name = strdup(name);
}

/**
 * Close the library handle and free the path.
 */
Symbol::~Symbol() {
    lean_dec(m_library);
    free(m_name);
}

/**
 * Create a new Symbol instance.
 *
 * On success, the library is owned and remains in memory at least until the
 * symbol is finalized. All other arguments are borrowed.
 *
 * @param lib Library object in which the symbol is opened.
 * @param sym Name of the symbol as a Lean string.
 *
 * @return Symbol object or an exception.
 */
extern "C" lean_obj_res Symbol_mk(b_lean_obj_arg lib, b_lean_obj_arg sym,
                                  lean_object *unused) {

    try {
        Symbol *s = new Symbol(lib, sym);
        return lean_io_result_mk_ok(s->box());
    } catch (const char *msg) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(msg));
        return lean_io_result_mk_error(err);
    }
}
