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

/** Regular fprintf(), but redirected through Lean. */
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

/** Regular printf(), but redirected through Lean's stdout. */
void lean_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_print, fmt, ap);
}

/** Regular printf(), but redirected through Lean's stderr. */
void lean_eprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_eprint, fmt, ap);
}

/***************************************************************************************
 * Library functions
 **************************************************************************************/

/** Lean class */
lean_external_class *Library_class = NULL;

/** Finalize a Library. */
static void Library_finalize(void *p) {
    Library *lib = (Library *)p;
    ffi_log("finalizing handle for %s at %p", lib->path, lib);
    if (dlclose(lib->handle) != 0) {
        ffi_log("dlclose() failed: %s", dlerror());
    }
}

/** Foreach for a Library handle. */
static void Library_foreach(void *mod, b_lean_obj_arg fn) {
    ffi_log("NOT IMPLEMENTED");
}

/** Convert a Library object from C to Lean. */
static inline lean_object *Library_box(Library *lib) {
    if (Library_class == NULL) {
        Library_class = lean_register_external_class(Library_finalize, Library_foreach);
    }
    return lean_alloc_external(Library_class, lib);
}

/** Convert a Library object from Lean to C. */
static inline Library const *Library_unbox(b_lean_obj_arg lib) {
    return (Library *)(lean_get_external_data(lib));
}

/** Create a new Library instance. */
lean_object *Library_mk(b_lean_obj_arg path, uint32_t flags, lean_object *unused) {
    const char *p = lean_string_cstr(path);
    ffi_log("opening handle for %s with flags 0x%08x", p, flags);

    void *handle = dlopen(p, flags);
    if (handle == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(dlerror()));
        return lean_io_result_mk_error(err);
    }
    Library *lib = malloc(sizeof(Library));
    lib->path = p;
    lib->handle = handle;
    return lean_io_result_mk_ok(Library_box(lib));
}

/***************************************************************************************
 * Symbol functions
 **************************************************************************************/

/** Lean class */
lean_external_class *Symbol_class = NULL;

/** Finalize a Symbol. */
static void Symbol_finalize(void *p) {
    ffi_log("finalizing %p", p);
}

/** Foreach for a Symbol handle. */
static void Symbol_foreach(void *mod, b_lean_obj_arg fn) {
    ffi_log("NOT IMPLEMENTED");
}

/** Convert a Symbol object from C to Lean. */
static inline lean_object *Symbol_box(Symbol *sym) {
    if (Symbol_class == NULL) {
        Symbol_class = lean_register_external_class(Symbol_finalize, Symbol_foreach);
    }
    return lean_alloc_external(Symbol_class, sym);
}

/** Convert a Symbol object from Lean to C. */
static inline Symbol const *Symbol_unbox(b_lean_obj_arg s) {
    return (Symbol *)(lean_get_external_data(s));
}

/** Create a new Symbol instance. */
lean_object *Symbol_mk(lean_object *lib, b_lean_obj_arg sym, lean_object *unused) {
    const char *name = lean_string_cstr(sym);
    const Library *l = Library_unbox(lib);

    ffi_log("opening %s in %s", name, l->path);

    // Clear dlerror() to distinguish between errors and NULL.
    dlerror();
    void *shandle = dlsym(l->handle, name);
    if (shandle == NULL) {
        char *msg = dlerror();
        if (msg != NULL) {
            // Symbol is not NULL.
            lean_object *err = lean_mk_io_user_error(lean_mk_string(msg));
            return lean_io_result_mk_error(err);
        }
    }

    Symbol *s = malloc(sizeof(Symbol));
    s->handle = shandle;
    return lean_io_result_mk_ok(Symbol_box(s));
}
