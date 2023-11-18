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

#include "library.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <lean/lean.h>
#include <stdexcept>

/** Unbox the Flag enum. */
static inline int Flag_unbox(b_lean_obj_arg flag) {
    assert(lean_is_scalar(flag));
    switch (lean_unbox(flag)) {
    case 0:
        return RTLD_LAZY;
    case 1:
        return RTLD_NOW;
    case 2:
        return RTLD_NOLOAD;
    case 3:
        return RTLD_DEEPBIND;
    case 4:
        return RTLD_GLOBAL;
    case 5:
        return RTLD_LOCAL;
    case 6:
        return RTLD_NODELETE;
    }
    lean_internal_panic_unreachable();
}

/**
 * Initialize the library object by opening the shared library with dlopen().
 *
 * Raises an exception with an error message on error.
 */
Library::Library(b_lean_obj_arg path, b_lean_obj_arg flags) : m_closed(false) {
    const char *p = lean_string_cstr(path);
    uint64_t openflags = 0;
    for (size_t i = 0; i < lean_array_size(flags); i++) {
        lean_object *o = lean_array_get_core(flags, i);
        openflags |= Flag_unbox(o);
    }
    void *handle = dlopen(p, openflags);
    if (handle == NULL)
        throw std::runtime_error(std::string(dlerror()));

    m_path = strdup(p);
    m_handle = handle;
}

/**
 * Free the path.
 *
 * We don't close the handle here, because another symbol might still
 * depend on it even if the library is already out of scope.
 * Reference counting has to be handled by the user.
 */
Library::~Library() { free(m_path); }

/** Lookup a symbol in the library. */
Pointer *Library::symbol(const char *name) {
    if (m_closed)
        throw std::runtime_error("library already closed");

    // Clear dlerror() to distinguish between errors and NULL.
    dlerror();
    void *p = dlsym(m_handle, name);
    if (p == nullptr) {
        char *msg = dlerror();
        if (msg != nullptr)
            throw std::runtime_error(std::string(msg));
    }
    return new Pointer((uint8_t *)p);
}

/** Close the library. */
void Library::close() {
    if (m_closed)
        throw std::runtime_error("library already closed");

    m_closed = true;
    int result = dlclose(m_handle);
    if (result != 0) {
        char *msg = dlerror();
        throw std::runtime_error(std::string(msg));
    }
}

/**
 * Create a new Library instance.
 *
 * All arguments passed to this function are borrowed.
 *
 * @param path Path of the library as a string.
 * @param flags Array with the flags to open the library.
 *
 * @return Library object or an exception.
 */
extern "C" lean_obj_res Library_mk(b_lean_obj_arg path, b_lean_obj_arg flags,
                                   lean_object *unused) {
    try {
        Library *lib = new Library(path, flags);
        return lean_io_result_mk_ok(lib->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Get the path of the library.
 */
extern "C" lean_obj_res Library_path(b_lean_obj_arg obj) {
    return lean_mk_string(Library::unbox(obj)->path());
}

/**
 * Lookup a symbol in the library.
 *
 * On success, the library is owned and remains in memory at least until the
 * pointer is finalized. All other arguments are borrowed.
 *
 * @param lib Library object in which the symbol is opened.
 * @param name Name of the symbol as a Lean string.
 *
 * @return Pointer to the symbol or an exception.
 */
extern "C" lean_obj_res Library_symbol(b_lean_obj_arg lib, b_lean_obj_arg name,
                                       lean_object *unused) {

    try {
        Pointer *p = Library::unbox(lib)->symbol(lean_string_cstr(name));
        return lean_io_result_mk_ok(p->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/** Close the library. */
extern "C" lean_obj_res Library_close(b_lean_obj_arg lib, lean_object *unused) {
    try {
        Library::unbox(lib)->close();
        return lean_io_result_mk_ok(lean_box(0));
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}
