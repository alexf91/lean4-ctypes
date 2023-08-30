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

static bool is_primitive(ffi_type *tp) {
    for (size_t i = 0; i < sizeof(primitive_types) / sizeof(primitive_types[0]); i++)
        if (tp == primitive_types[i])
            return true;
    return false;
}

static void ffi_type_free(ffi_type *tp) {
    if (is_primitive(tp))
        return;

    // For arrays and struct types.
    // TODO: Let's hope there is nothing static in there that we don't have in the
    //       primitive_types array.
    for (size_t i = 0; tp->elements && tp->elements[i]; i++)
        ffi_type_free(tp->elements[i]);
    free(tp->elements);
    free(tp);
}

/** Forward declaration. */
ffi_type *CType_unbox(b_lean_obj_arg tp);

/** Unbox an array type. */
ffi_type *CType_unbox_array(b_lean_obj_arg tp) {
    size_t length = lean_unbox(lean_ctor_get(tp, 1));
    lean_object *atype_obj = lean_ctor_get(tp, 0);

    utils_log("unboxing array of type %s and length %d",
              primitive_types_names[lean_unbox(atype_obj)], length);

    // Describe an array by setting the elements field length times.
    // TODO: We currently unpack it every time to make freeing easier.
    ffi_type *result = calloc(1, sizeof(ffi_type));
    result->elements = calloc(length + 1, sizeof(ffi_type *));
    for (size_t i = 0; i < length; i++)
        result->elements[i] = CType_unbox(atype_obj);

    return result;
}

/** Unbox a struct type. */
ffi_type *CType_unbox_struct(b_lean_obj_arg tp) {
    lean_object *elements = lean_ctor_get(tp, 0);
    size_t length = lean_array_size(elements);
    utils_log("unboxing struct with %d elements", length);

    ffi_type *result = calloc(1, sizeof(ffi_type));
    result->elements = calloc(length + 1, sizeof(ffi_type *));
    for (size_t i = 0; i < length; i++)
        result->elements[i] = CType_unbox(lean_array_get_core(elements, i));

    return result;
}

/**
 * Unbox the CType enum to a ffi_type structure.
 *
 * @param tp Lean object contaning a CType type.
 */
ffi_type *CType_unbox(b_lean_obj_arg tp) {
    int index = lean_obj_tag(tp);
    switch (index) {
    case 0 ... 23: // primitive types
        utils_log("unboxing %s", primitive_types_names[index]);
        return primitive_types[index];
    case 24: // struct
        utils_log("struct: not supported");
        return CType_unbox_struct(tp);
    case 25: // array
        return CType_unbox_array(tp);
    }
    utils_log("unexpected type");
    assert(0);
    return NULL;
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

    for (size_t i = 0; i < f->nargs; i++)
        ffi_type_free(f->arguments[i]);

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
lean_obj_res Function_mk(b_lean_obj_arg symbol, b_lean_obj_arg rtype,
                         b_lean_obj_arg args, lean_object *unused) {

    Symbol *s = Symbol_unbox(symbol);
    utils_log("creating function for '%s'", s->name);

    // Unbox the return type.
    ffi_type *return_type = CType_unbox(rtype);
    if (return_type == NULL) {
        lean_object *msg = lean_mk_string("return type not supported");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    // Unbox the arguments and copy them into buffer.
    size_t nargs = lean_array_size(args);
    ffi_type **argtypes = malloc(nargs * sizeof(ffi_type *));
    for (int i = 0; i < nargs; i++) {
        argtypes[i] = CType_unbox(lean_array_get_core(args, i));
        if (argtypes[i] == NULL) {
            free(argtypes);
            lean_object *msg = lean_mk_string("argument type not supported");
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

/**
 * Call a function.
 *
 * @param function Function object that should be called.
 * @param argvals Array of argument values.
 *
 * @return Result of the call wrapped in a LeanType object.
 */
lean_obj_res Function_call(b_lean_obj_arg function, b_lean_obj_arg argvals,
                           lean_object *unused) {

    Function *f = Function_unbox(function);
    lean_object *msg = lean_mk_string("NOT IMPLEMENTED");
    return lean_io_result_mk_error(lean_mk_io_user_error(msg));
}
