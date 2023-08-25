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
lean_external_class *Handle_class = NULL;

/**
 * Finalize a Handle.
 */
static void Handle_finalize(void *p) {
    ffi_log("finalizing %p", p);
    Handle *h = (Handle *)p;
    if (dlclose(h->handle) != 0) {
        ffi_log("dlclose() failed: %s", dlerror());
    }
}

/**
 * Foreach for a Handle.
 */
static void Handle_foreach(void *mod, b_lean_obj_arg fn) {
}

/**
 * Convert a Handle object from C to Lean.
 */
static inline lean_object *Handle_box(Handle *h) {
    if (Handle_class == NULL) {
        Handle_class = lean_register_external_class(Handle_finalize, Handle_foreach);
    }
    return lean_alloc_external(Handle_class, h);
}

/**
 * Convert a Handle object from Lean to C.
 */
static inline Handle const *Handle_unbox(b_lean_obj_arg h) {
    return (Handle *)(lean_get_external_data(h));
}

/**
 * Create a new Handle instance.
 */
lean_object *Handle_mk(lean_object *path, uint32_t flags, lean_object *unused) {
    const char *p = lean_string_cstr(path);
    ffi_log("opening handle for %s with flags %u", p, flags);

    void *handle = dlopen(p, flags);
    if (handle == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(dlerror()));
        return lean_io_result_mk_error(err);
    }
    Handle *h = malloc(sizeof(Handle));
    h->handle = handle;
    ffi_log("%p", h);
    return lean_io_result_mk_ok(Handle_box(h));
}
