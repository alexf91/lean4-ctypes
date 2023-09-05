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

#include <algorithm>
#include <cstdint>

#include "ctype.hpp"
#include "lean/lean.h"
#include "leantype.hpp"
#include "symbol.hpp"
#include "utils.hpp"

/***************************************************************************************
 * Function functions
 **************************************************************************************/

/**
 * Initialize the Function object by converting the argument specification and
 * preparing the CIF.
 */
Function::Function(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
                   b_lean_obj_arg argtypes_object)
    : m_symbol(symbol), m_rtype_object(rtype_object),
      m_argtypes_object(argtypes_object) {

    // Unbox the return type and the arguments.
    m_rtype = CType::unbox(rtype_object);
    m_argtypes = new ffi_type *[get_nargs()];
    for (size_t i = 0; i < get_nargs(); i++)
        m_argtypes[i] = CType::unbox(lean_array_get_core(argtypes_object, i));

    // Create the call interface for the function.
    ffi_status stat =
        ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, get_nargs(), m_rtype, m_argtypes);
    if (stat != FFI_OK)
        throw "creating CIF failed";

    // Convert arguments into owned objects.
    lean_inc(symbol);
    lean_inc(rtype_object);
    lean_inc(argtypes_object);
}

/**
 * Free prepared structures and release referenced objects.
 */
Function::~Function() {
    lean_dec(m_symbol);
    lean_dec(m_rtype_object);
    lean_dec(m_argtypes_object);

    delete m_rtype;
    for (size_t i = 0; i < get_nargs(); i++)
        delete (CType *)m_argtypes[i];
    delete[] m_argtypes;
}

/**
 * Call the function with the given arguments.
 */
lean_obj_res Function::call(b_lean_obj_arg argvals_object) {
    size_t nargs = lean_array_size(argvals_object);
    if (nargs < get_nargs())
        throw "not enough arguments";
    else if (nargs > get_nargs())
        throw "variadic arguments not supported";

    // Construct the argument buffer.
    void *argvals[nargs];
    for (size_t i = 0; i < nargs; i++) {
        lean_object *arg = lean_array_get_core(argvals_object, i);
        LeanType *v = LeanType::unbox(arg);
        ffi_type *tp = m_argtypes[i];
        argvals[i] = v->to_buffer((CType *)tp);
        delete v;
    }

    // At least a buffer with the size of a register is required for the return buffer.
    size_t rsize = std::max((size_t)FFI_SIZEOF_ARG, m_rtype->size);
    void *rvalue = ::operator new(rsize);

    // Call the symbol handle.
    auto handle = (void (*)())Symbol::unbox(m_symbol)->get_handle();
    ffi_call(&m_cif, handle, rvalue, argvals);

    // TODO: Box the rvalue!!!

    // Cleanup the argument values and the return value.
    for (size_t i = 0; i < nargs; i++)
        delete argvals[i];
    delete rvalue;

    // TODO: Return the correct result.
    CType *ctype = CType::unbox(lean_box(CType::INT16));
    LeanTypeInt result((ssize_t)-1);
    lean_object *r = result.box(ctype);
    delete ctype;
    return r;
}

/** Convert a Function object from C to Lean. */
lean_object *Function::box() {
    if (Function::m_class == nullptr)
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

/** Create a new Function instance.  */
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

/** Call a function.  */
extern "C" lean_obj_res Function_call(b_lean_obj_arg function,
                                      b_lean_obj_arg argvals_obj, lean_object *unused) {

    try {
        Function *f = Function::unbox(function);
        lean_object *result = f->call(argvals_obj);
        return lean_io_result_mk_ok(result);
    } catch (const char *msg) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(msg));
        return lean_io_result_mk_error(err);
    }
}
