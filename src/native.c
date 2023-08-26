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

#include "native.h"
#include <assert.h>
#include <dlfcn.h>
#include <ffi.h>
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

/** Unbox the Flag enum. */
int Flag_unbox(uint8_t flag) {
    switch (flag) {
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
    default:
        assert(0);
    }
}

/** Lean class */
lean_external_class *Library_class = NULL;

/** Finalize a Library. */
static void Library_finalize(void *p) {
    Library *lib = (Library *)p;
    native_log("finalizing handle for %s at %p", lib->path, lib);
    if (dlclose(lib->handle) != 0) {
        native_log("dlclose() failed: %s", dlerror());
    }
}

/** Foreach for a Library handle. */
static void Library_foreach(void *mod, b_lean_obj_arg fn) {
    native_log("NOT IMPLEMENTED");
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
lean_object *Library_mk(b_lean_obj_arg path, b_lean_obj_arg flags,
                        lean_object *unused) {
    const char *p = lean_string_cstr(path);

    uint32_t openflags = 0;
    for (int i = 0; i < lean_array_size(flags); i++) {
        lean_object *o = lean_array_get_core(flags, i);
        assert(lean_is_scalar(o));
        openflags |= lean_unbox(o);
    }
    native_log("opening handle for %s with flags 0x%08x", p, openflags);

    void *handle = dlopen(p, openflags);
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
    native_log("finalizing %p", p);
}

/** Foreach for a Symbol handle. */
static void Symbol_foreach(void *mod, b_lean_obj_arg fn) {
    native_log("NOT IMPLEMENTED");
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
lean_object *Symbol_mk(b_lean_obj_arg lib, b_lean_obj_arg sym, lean_object *unused) {
    const char *name = lean_string_cstr(sym);
    const Library *l = Library_unbox(lib);

    native_log("opening %s in %s", name, l->path);

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

/***************************************************************************************
 * FFI interface
 **************************************************************************************/

/** Unbox the CType enum to a ffi_type structure. */
ffi_type *CType_unbox(b_lean_obj_arg tp) {
    switch (lean_obj_tag(tp)) {
    case 0:
        native_log("unboxing ffi_type_void");
        return &ffi_type_void;
    case 1:
        native_log("unboxing ffi_type_uint8");
        return &ffi_type_uint8;
    case 2:
        native_log("unboxing ffi_type_sint8");
        return &ffi_type_sint8;
    case 3:
        native_log("unboxing ffi_type_uint16");
        return &ffi_type_uint16;
    case 4:
        native_log("unboxing ffi_type_sint16");
        return &ffi_type_sint16;
    case 5:
        native_log("unboxing ffi_type_uint32");
        return &ffi_type_uint32;
    case 6:
        native_log("unboxing ffi_type_sint32");
        return &ffi_type_sint32;
    case 7:
        native_log("unboxing ffi_type_uint64");
        return &ffi_type_uint64;
    case 8:
        native_log("unboxing ffi_type_sint64");
        return &ffi_type_sint64;
    case 9:
        native_log("unboxing ffi_type_float");
        return &ffi_type_float;
    case 10:
        native_log("unboxing ffi_type_double");
        return &ffi_type_double;
    case 11:
        native_log("unboxing ffi_type_uchar");
        return &ffi_type_uchar;
    case 12:
        native_log("unboxing ffi_type_schar");
        return &ffi_type_schar;
    case 13:
        native_log("unboxing ffi_type_ushort");
        return &ffi_type_ushort;
    case 14:
        native_log("unboxing ffi_type_sshort");
        return &ffi_type_sshort;
    case 15:
        native_log("unboxing ffi_type_uint");
        return &ffi_type_uint;
    case 16:
        native_log("unboxing ffi_type_sint");
        return &ffi_type_sint;
    case 17:
        native_log("unboxing ffi_type_ulong");
        return &ffi_type_ulong;
    case 18:
        native_log("unboxing ffi_type_slong");
        return &ffi_type_slong;
    case 19:
        native_log("unboxing ffi_type_longdouble");
        return &ffi_type_longdouble;
    case 20:
        native_log("unboxing ffi_type_pointer");
        return &ffi_type_pointer;
    case 21:
        native_log("unboxing ffi_type_complex_float");
        return &ffi_type_complex_float;
    case 22:
        native_log("unboxing ffi_type_complex_double");
        return &ffi_type_complex_double;
    case 23:
        native_log("unboxing ffi_type_complex_longdouble");
        return &ffi_type_complex_longdouble;
    case 24: // struct
        native_log("struct: not supported");
        return NULL;
    case 25: // array
        native_log("array: not supported");
        return NULL;
    default:
        native_log("unexpected type");
        assert(0);
    }
}

/** Just test CType unboxing. */
lean_object *CType_test(b_lean_obj_arg tp, lean_object *unused) {
    ffi_type *ffitype = CType_unbox(tp);
    if (ffitype == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string("type not supported"));
        return lean_io_result_mk_error(err);
    }
    return lean_io_result_mk_ok(lean_box(0));
}
