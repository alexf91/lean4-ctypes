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
#include "ctype.hpp"
#include "pointer.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdlib>
#include <lean/lean.h>
#include <stdexcept>

/***************************************************************************************
 * Function functions
 **************************************************************************************/

/**
 * Initialize the Function object by converting the argument specification and
 * preparing the CIF.
 */
Function::Function(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
                   b_lean_obj_arg argtypes_object)
    : m_pointer(symbol) {

    // Unbox the return type. We don't allow arrays as arguments.
    m_rtype = CType::unbox(rtype_object);
    m_ffi_rtype = m_rtype->get_ffi_type();
    // if (m_rtype->get_tag() == CType::ARRAY)
    //     throw std::runtime_error("invalid return type: array not allowed");

    // Unbox argument types. We don't allow void or arrays.
    size_t nargs = lean_array_size(argtypes_object);
    m_ffi_argtypes = new ffi_type *[nargs];
    for (size_t i = 0; i < nargs; i++) {
        auto ct = CType::unbox(lean_array_get_core(argtypes_object, i));

        // if (ct->get_tag() == CType::VOID) {
        //     delete[] m_ffi_argtypes;
        //     throw std::runtime_error("invalid argument type: void not allowed");
        // } else if (ct->get_tag() == CType::ARRAY) {
        //     delete[] m_ffi_argtypes;
        //     throw std::runtime_error("invalid argument type: array not allowed");
        // }

        m_ffi_argtypes[i] = ct->get_ffi_type();
        m_argtypes.push_back(std::move(ct));
    }

    // Create the call interface for the function.
    ffi_status stat =
        ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, nargs, m_ffi_rtype, m_ffi_argtypes);
    if (stat != FFI_OK) {
        delete[] m_ffi_argtypes;
        throw std::runtime_error("creating CIF failed");
    }

    // Convert arguments into owned objects.
    lean_inc(m_pointer);
}

/**
 * Free prepared structures and release referenced objects.
 */
Function::~Function() {
    delete[] m_ffi_argtypes;
    lean_dec(m_pointer);
}

/**
 * Call the function with the given arguments.
 */
lean_obj_res Function::call(b_lean_obj_arg argvals_object) {
    size_t nargs = lean_array_size(argvals_object);
    if (nargs < get_nargs())
        throw std::runtime_error("not enough arguments");
    else if (nargs > get_nargs())
        throw std::runtime_error("variadic arguments not supported");

    // Construct the argument buffer. Using a vector automates cleanup after an
    // exception.
    std::vector<std::unique_ptr<uint8_t[]>> argbufs;
    void *argvals[nargs];
    for (size_t i = 0; i < nargs; i++) {
        lean_object *arg = lean_array_get_core(argvals_object, i);
        auto v = CValue::unbox(arg);
        argbufs.push_back(m_argtypes[i]->buffer(*v));
        argvals[i] = argbufs[i].get();
    }

    // At least a buffer with the size of a register is required for the return
    // buffer.
    size_t rsize = std::max((size_t)FFI_SIZEOF_ARG, m_rtype->get_size());
    uint8_t rvalue[rsize];

    // Call the symbol handle.
    auto handle = (void (*)())Pointer::unbox(m_pointer)->get_pointer();
    ffi_call(&m_cif, handle, rvalue, argvals);

    return m_rtype->instance(rvalue)->box();
}

/** Create a new Function instance.  */
extern "C" lean_obj_res Function_mk(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
                                    b_lean_obj_arg argtypes_object,
                                    lean_object *unused) {

    try {
        Function *f = new Function(symbol, rtype_object, argtypes_object);
        return lean_io_result_mk_ok(f->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
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
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}
