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

#include "ffi.h"
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>

/* Print function type. */
typedef lean_object *(*lean_print_t)(lean_object *, lean_object *);

/**
 * Regular fprintf(), but redirected through Lean.
 */
static inline void lean_vfprintf(lean_print_t fn, const char *fmt, va_list ap) {
    va_list apcopy;
    va_copy(apcopy, ap);

    // Determine the required buffer size.
    size_t needed = vsnprintf(NULL, 0, fmt, ap) + 1;
    char *buffer = malloc(needed);
    vsprintf(buffer, fmt, apcopy);
    fn(lean_mk_string(buffer), NULL);
    free(buffer);
}

/**
 * Regular printf(), but redirected through Lean's stdout.
 */
void lean_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_print, fmt, ap);
}

/**
 * Regular printf(), but redirected through Lean's stderr.
 */
void lean_eprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_eprint, fmt, ap);
}

/**
 * Classes defined in Lean.
 */
lean_external_class *Library_class = NULL;

/**
 * Finalize a Library.
 */
static void Library_finalize(void *p) {
    ffi_log("finalizing %p", p);
    Library *lib = (Library *)p;
    if (dlclose(lib->handle) != 0) {
        ffi_log("dlclose() failed: %s", dlerror());
    }
}

/**
 * Foreach for a Library handle.
 */
static void Library_foreach(void *mod, b_lean_obj_arg fn) {
}

/**
 * Convert a Library object from C to Lean.
 */
static inline lean_object *Library_box(Library *lib) {
    if (Library_class == NULL) {
        Library_class = lean_register_external_class(Library_finalize, Library_foreach);
    }
    return lean_alloc_external(Library_class, lib);
}

/**
 * Convert a Library object from Lean to C.
 */
static inline Library const *Library_unbox(b_lean_obj_arg lib) {
    return (Library *)(lean_get_external_data(lib));
}

/**
 * Create a new Library instance.
 */
lean_object *Library_mk(lean_object *path, uint32_t flags, lean_object *unused) {
    const char *p = lean_string_cstr(path);
    ffi_log("opening handle for %s with flags %u", p, flags);

    void *handle = dlopen(p, flags);
    if (handle == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(dlerror()));
        return lean_io_result_mk_error(err);
    }
    Library *lib = malloc(sizeof(Library));
    lib->handle = handle;
    ffi_log("%p", lib);
    return lean_io_result_mk_ok(Library_box(lib));
}
