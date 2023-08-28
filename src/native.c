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
#include <stdlib.h>

/***************************************************************************************
 * Basic tools
 **************************************************************************************/

/** Check if NDEBUG is defined. */
uint8_t debug_mode(lean_object *unused) {
#ifdef NDEBUG
    return 0;
#else
    return 1;
#endif
}

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
    }
    assert(0);
    return -1;
}

/** Lean class */
lean_external_class *Library_class = NULL;

/** Finalize a Library. */
static void Library_finalize(void *p) {
    Library *lib = (Library *)p;
    native_log("finalizing %s", lib->name);
    if (dlclose(lib->handle) != 0) {
        native_log("dlclose() failed: %s", dlerror());
    }
    free(lib->name);
    free(lib);
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
static inline Library *Library_unbox(b_lean_obj_arg lib) {
    return (Library *)(lean_get_external_data(lib));
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
lean_object *Library_mk(b_lean_obj_arg path, b_lean_obj_arg flags,
                        lean_object *unused) {
    const char *p = lean_string_cstr(path);

    uint32_t openflags = 0;
    for (int i = 0; i < lean_array_size(flags); i++) {
        lean_object *o = lean_array_get_core(flags, i);
        assert(lean_is_scalar(o));
        openflags |= lean_unbox(o);
    }
    native_log("opening %s with flags 0x%08x", p, openflags);

    void *handle = dlopen(p, openflags);
    if (handle == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(dlerror()));
        return lean_io_result_mk_error(err);
    }
    Library *lib = malloc(sizeof(Library));
    lib->name = strdup(p);
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
    Symbol *s = (Symbol *)p;
    native_log("finalizing '%s' in %s", s->name, Library_unbox(s->library)->name);
    lean_dec(s->library);
    free(s->name);
    free(s);
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
static inline Symbol *Symbol_unbox(b_lean_obj_arg s) {
    return (Symbol *)(lean_get_external_data(s));
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
lean_object *Symbol_mk(b_lean_obj_arg lib, b_lean_obj_arg sym, lean_object *unused) {
    const char *name = lean_string_cstr(sym);
    const Library *l = Library_unbox(lib);

    native_log("opening '%s' in %s", name, l->name);

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

    // Make the library an owned object:
    lean_inc(lib);
    Symbol *s = malloc(sizeof(Symbol));
    s->handle = shandle;
    s->library = lib;
    s->name = strdup(name);
    return lean_io_result_mk_ok(Symbol_box(s));
}

/***************************************************************************************
 * CType enum
 **************************************************************************************/

/**
 * Primitive types defined in libffi. They are in the same order as the CType
 * enum. They are statically allocated.
 */
static ffi_type *primitive_types[] = {
    &ffi_type_void,          &ffi_type_uint8,          &ffi_type_sint8,
    &ffi_type_uint16,        &ffi_type_sint16,         &ffi_type_uint32,
    &ffi_type_sint32,        &ffi_type_uint64,         &ffi_type_sint64,
    &ffi_type_float,         &ffi_type_double,         &ffi_type_uchar,
    &ffi_type_schar,         &ffi_type_ushort,         &ffi_type_sshort,
    &ffi_type_uint,          &ffi_type_sint,           &ffi_type_ulong,
    &ffi_type_slong,         &ffi_type_longdouble,     &ffi_type_pointer,
    &ffi_type_complex_float, &ffi_type_complex_double, &ffi_type_complex_longdouble,
};

/** Names of primitive types for debugging. */
__attribute__((unused)) static const char *primitive_types_names[] = {
    "ffi_type_void",          "ffi_type_uint8",          "ffi_type_sint8",
    "ffi_type_uint16",        "ffi_type_sint16",         "ffi_type_uint32",
    "ffi_type_sint32",        "ffi_type_uint64",         "ffi_type_sint64",
    "ffi_type_float",         "ffi_type_double",         "ffi_type_uchar",
    "ffi_type_schar",         "ffi_type_ushort",         "ffi_type_sshort",
    "ffi_type_uint",          "ffi_type_sint",           "ffi_type_ulong",
    "ffi_type_slong",         "ffi_type_longdouble",     "ffi_type_pointer",
    "ffi_type_complex_float", "ffi_type_complex_double", "ffi_type_complex_longdouble",
};

/**
 * Unbox the CType enum to a ffi_type structure.
 *
 * @param tp Lean object contaning a CType type.
 */
ffi_type *CType_unbox(b_lean_obj_arg tp) {
    int index = lean_obj_tag(tp);
    switch (index) {
    case 0 ... 23:
        native_log("unboxing %s", primitive_types_names[index]);
        return primitive_types[index];
    case 24: // struct
        native_log("struct: not supported");
        return NULL;
    case 25: // array
        native_log("array: not supported");
        return NULL;
    }
    native_log("unexpected type");
    assert(0);
    return NULL;
}

/** Just test CType unboxing. */
lean_object *CType_test(b_lean_obj_arg tp, lean_object *unused) {
    ffi_type *ctype = CType_unbox(tp);
    if (ctype == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string("type not supported"));
        return lean_io_result_mk_error(err);
    }
    // ffi_type_free(ctype);
    return lean_io_result_mk_ok(lean_box(0));
}

/***************************************************************************************
 * Function functions
 **************************************************************************************/

/** Lean class */
lean_external_class *Function_class = NULL;

/** Finalize a Function. */
static void Function_finalize(void *p) {
    Function *f = (Function *)p;
    native_log("finalizing function for '%s'", Symbol_unbox(f->symbol)->name);
    lean_dec(f->symbol);
    free(f->arguments);
    free(f->cif);
    free(f);
}

/** Foreach for a Function handle. */
static void Function_foreach(void *mod, b_lean_obj_arg fn) {
    native_log("NOT IMPLEMENTED");
}

/** Convert a Function object from C to Lean. */
static inline lean_object *Function_box(Function *f) {
    if (Function_class == NULL) {
        Function_class =
            lean_register_external_class(Function_finalize, Function_foreach);
    }
    return lean_alloc_external(Function_class, f);
}

/** Convert a Function object from Lean to C. */
static inline Function *Function_unbox(b_lean_obj_arg f) {
    return (Function *)(lean_get_external_data(f));
}

/**
 * Create a new Function instance.
 *
 * On success, the symbol is owned and remains in memory at least until the function is
 * finalized. All other arguments are borrowed.
 *
 * @param symbol Lean object for the symbol used to create the function.
 * @param rtype Return type of the function as a CType.
 * @param args Array CTypes to specify the arguments.
 *
 * @return Function object or an exception.
 */
lean_object *Function_mk(b_lean_obj_arg symbol, b_lean_obj_arg rtype,
                         b_lean_obj_arg args, lean_object *unused) {

    Symbol *s = Symbol_unbox(symbol);
    native_log("creating function for '%s'", s->name);

    // Unbox the return type.
    ffi_type *return_type = CType_unbox(rtype);
    if (return_type == NULL) {
        lean_object *msg = lean_mk_string("return type not supported");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    // Unbox the arguments and copy them into a NULL terminated buffer.
    size_t nargs = lean_array_size(args);
    ffi_type **argtypes = malloc(sizeof(ffi_type *));
    for (int i = 0; i < nargs; i++) {
        argtypes[i] = CType_unbox(lean_array_get_core(args, i));
        if (argtypes[i] == NULL) {
            free(argtypes);
            lean_object *msg = lean_mk_string("argument type not supported");
            return lean_io_result_mk_error(lean_mk_io_user_error(msg));
        } else if (argtypes[i] == &ffi_type_void) {
            free(argtypes);
            lean_object *msg = lean_mk_string("argument type 'void' not supported");
            return lean_io_result_mk_error(lean_mk_io_user_error(msg));
        }
    }

    // Create the call interface for the function.
    ffi_cif *cif = calloc(1, sizeof(ffi_cif));

    ffi_status stat = ffi_prep_cif(cif, FFI_DEFAULT_ABI, nargs, return_type, argtypes);
    if (stat != FFI_OK) {
        free(argtypes);
        free(cif);
        native_log("creating CIF failed with error code %d", stat);
        lean_object *msg = lean_mk_string("creating CIF failed");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    // Combine everything into a function struct.
    Function *f = malloc(sizeof(Function));
    lean_inc(symbol);
    f->symbol = symbol;
    f->cif = cif;
    f->return_type = return_type;
    f->nargs = nargs;
    f->arguments = argtypes;

    return lean_io_result_mk_ok(Function_box(f));
}
