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

#include "function.hpp"
#include "library.hpp"
#include "symbol.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>

/** Forward declaration. */
ffi_type *CType_unbox(b_lean_obj_arg tp);

/** Unbox an array type. */
ffi_type *CType_unbox_array(b_lean_obj_arg tp) {
    size_t length = lean_unbox(lean_ctor_get(tp, 1));
    lean_object *atype_obj = lean_ctor_get(tp, 0);

    // utils_log("unboxing array of type %s and length %d",
    //           primitive_types_names[lean_unbox(atype_obj)], length);

    // Describe an array by setting the elements field length times.
    // TODO: We currently unpack it every time to make freeing easier.
    ffi_type *result = (ffi_type *)calloc(1, sizeof(ffi_type));
    result->type = FFI_TYPE_STRUCT;
    result->elements = (ffi_type **)calloc(length + 1, sizeof(ffi_type *));
    for (size_t i = 0; i < length; i++)
        result->elements[i] = CType_unbox(atype_obj);

    return result;
}

/** Unbox a struct type. */
ffi_type *CType_unbox_struct(b_lean_obj_arg tp) {
    lean_object *elements = lean_ctor_get(tp, 0);
    size_t length = lean_array_size(elements);
    utils_log("unboxing struct with %d elements", length);

    ffi_type *result = (ffi_type *)calloc(1, sizeof(ffi_type));
    result->type = FFI_TYPE_STRUCT;
    result->elements = (ffi_type **)calloc(length + 1, sizeof(ffi_type *));
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
    // int index = lean_obj_tag(tp);
    // switch (index) {
    // case CTYPE_PRIMITIVE_FIRST ... CTYPE_PRIMITIVE_LAST:
    //     utils_log("unboxing %s", primitive_types_names[index]);
    //     return primitive_types[index];
    // case CTYPE_STRUCT: // struct
    //     return CType_unbox_struct(tp);
    // case CTYPE_ARRAY: // array
    //     return CType_unbox_array(tp);
    // }
    // utils_log("unexpected type");
    // assert(0);
    return NULL;
}

/******************************************************************************
 * Conversion matrix from LeanType and ffi_type pair to a void pointer.
 *****************************************************************************/
void *LeanType_unbox_int(ffi_type *ffi_tp, lean_object *lean_tp) {
    size_t value = lean_unbox(lean_ctor_get(lean_tp, 0));
    utils_log("value: %d", value);
    return NULL;
}

void *LeanType_unbox_float(ffi_type *ffi_tp, b_lean_obj_arg lean_tp) {
    double value = lean_unbox_float(lean_tp);
    utils_log("value: %f", value);

    if (ffi_tp == &ffi_type_float) {
        float *v = (float *)malloc(sizeof(float));
        *v = value;
        return v;
    } else if (ffi_tp == &ffi_type_double) {
        double *v = (double *)malloc(sizeof(double));
        *v = value;
        return v;
    } else if (ffi_tp == &ffi_type_longdouble) {
        long double *v = (long double *)malloc(sizeof(long double));
        *v = value;
        return v;
    }

    return NULL;
}

void *LeanType_unbox(ffi_type *ffi_tp, lean_object *lean_tp) {
    switch (lean_obj_tag(lean_tp)) {
    case 0: // unit
        utils_log("unit: not allowed as argument");
        assert(0);
        break;
    case 1: // int
        return LeanType_unbox_int(ffi_tp, lean_tp);
    case 2: // float
        return LeanType_unbox_float(ffi_tp, lean_tp);
    }

    utils_log("argument type not recognized");
    return NULL;
}

/***************************************************************************************
 * Function functions
 **************************************************************************************/

/**
 * Initialize the Function object by converting the argument specification and
 * preparing the CIF.
 */
Function::Function(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
                   b_lean_obj_arg argtypes_object) {
    Symbol *s = Symbol::unbox(symbol);
    utils_log("creating function for '%s'", s->get_name());

    // Unbox the return type.
    m_rtype = CType_unbox(rtype_object);
    if (m_rtype == NULL)
        throw "return type not supported";

    // Unbox the arguments and copy them into buffer.
    size_t nargs = lean_array_size(argtypes_object);
    m_argtypes = (ffi_type **)malloc(nargs * sizeof(ffi_type *));
    for (size_t i = 0; i < nargs; i++) {
        m_argtypes[i] = CType_unbox(lean_array_get_core(argtypes_object, i));
        if (m_argtypes[i] == NULL) {
            free(m_argtypes);
            throw "argument type not supported";
        }
    }

    // Create the call interface for the function.
    ffi_status stat = ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, nargs, m_rtype, m_argtypes);
    if (stat != FFI_OK) {
        free(m_argtypes);
        utils_log("creating CIF failed with error code %d", stat);
        throw "creating CIF failed";
    }

    // Convert arguments into owned objects.
    lean_inc(symbol);
    m_symbol = symbol;

    lean_inc(rtype_object);
    m_rtype_object = rtype_object;

    lean_inc(argtypes_object);
    argtypes_object = argtypes_object;
}

/**
 * Free prepared structures and release referenced objects.
 */
Function::~Function() {
    utils_log("finalizing function for '%s'", Symbol::unbox(m_symbol)->get_name());

    for (size_t i = 0; i < get_nargs(); i++)
        ffi_type_free(m_argtypes[i]);

    lean_dec(m_symbol);
    lean_dec(m_rtype_object);
    lean_dec(m_argtypes_object);
    free(m_argtypes);
}

/**
 * Call the function with the given arguments.
 */
lean_obj_res Function::call(b_lean_obj_arg argvals_object) {
    size_t nargs = lean_array_size(argvals_object);
    if (nargs < get_nargs()) {
        lean_object *msg = lean_mk_string("not enough arguments");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    } else if (nargs > get_nargs()) {
        lean_object *msg = lean_mk_string("variadic arguments not supported");
        return lean_io_result_mk_error(lean_mk_io_user_error(msg));
    }

    void *argvals[nargs];
    for (size_t i = 0; i < nargs; i++) {
        lean_object *arg = lean_array_get_core(argvals_object, i);
        argvals[i] = LeanType_unbox(m_argtypes[i], arg);
    }

    // TODO: Cleanup
    void *rvalue = malloc(std::min((size_t)FFI_SIZEOF_ARG, m_rtype->size));
    auto handle = (void (*)())Symbol::unbox(m_symbol)->get_handle();
    ffi_call(&m_cif, handle, rvalue, argvals);

    lean_object *result = LeanType_box(m_rtype_object, (uint64_t)rvalue);
    free(rvalue);

    for (size_t i = 0; i < nargs; i++)
        free(argvals[i]);

    return lean_io_result_mk_ok(result);
}

/** Convert a Function object from C to Lean. */
lean_object *Function::box() {
    if (Function::m_class == NULL)
        Function::m_class = lean_register_external_class(finalize, foreach);
    return lean_alloc_external(m_class, this);
}

/**
 * Unwrap a Function object from a Lean object.
 */
Function *Function::unbox(b_lean_obj_arg obj) {
    return (Function *)(lean_get_external_data(obj));
}

/** Finalize a Function. */
void Function::finalize(void *p) {
    Function *f = (Function *)p;
    delete f;
}

/** Foreach for a Function type. */
void Function::foreach (void *mod, b_lean_obj_arg fn) { utils_log("NOT IMPLEMENTED"); }

/**
 * Create a new Function instance.
 *
 * On success, the symbol is owned and remains in memory at least until the function is
 * finalized. All other arguments are borrowed.
 *
 * @param symbol Lean object for the symbol used to create the function.
 * @param rtype_obj Return type of the function as a CType.
 * @param args Array CTypes to specify the arguments.
 *
 * @return Function object or an exception.
 */
extern "C" lean_obj_res Function_mk(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
                                    b_lean_obj_arg argtypes_object,
                                    lean_object *unused) {

    try {
        Function *f = new Function(symbol, rtype_object, argtypes_object);
        return lean_io_result_mk_ok(f->box());
    } catch (const char *msg) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(msg));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Call a function.
 *
 * @param function Function object that should be called.
 * @param argvals Array of argument values.
 *
 * @return Result of the call wrapped in a LeanType object.
 */
extern "C" lean_obj_res Function_call(b_lean_obj_arg function,
                                      b_lean_obj_arg argvals_obj, lean_object *unused) {

    Function *f = Function::unbox(function);
    lean_object *result = f->call(argvals_obj);
    return lean_io_result_mk_ok(result);
}
