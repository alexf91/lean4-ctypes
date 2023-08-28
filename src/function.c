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

#include "function.h"
#include "library.h"
#include "symbol.h"
#include "utils.h"
#include <stdlib.h>

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
        utils_log("unboxing %s", primitive_types_names[index]);
        return primitive_types[index];
    case 24: // struct
        utils_log("struct: not supported");
        return NULL;
    case 25: // array
        utils_log("array: not supported");
        return NULL;
    }
    utils_log("unexpected type");
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
    utils_log("finalizing function for '%s'", Symbol_unbox(f->symbol)->name);
    lean_dec(f->symbol);
    free(f->arguments);
    free(f->cif);
    free(f);
}

/** Foreach for a Function handle. */
static void Function_foreach(void *mod, b_lean_obj_arg fn) {
    utils_log("NOT IMPLEMENTED");
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
    utils_log("creating function for '%s'", s->name);

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
        utils_log("creating CIF failed with error code %d", stat);
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
