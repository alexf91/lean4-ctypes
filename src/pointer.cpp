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

#include "pointer.hpp"
#include "lean/lean.h"
#include "types.hpp"
#include "utils.hpp"
#include <algorithm>
#include <complex>
#include <cstdint>
#include <stdexcept>

/** Call the pointer as a function. */
std::unique_ptr<CValue> Pointer::call(CType &rtype,
                                      std::vector<std::unique_ptr<CValue>> &args,
                                      std::vector<std::unique_ptr<CValue>> &vargs) {

    // Type buffer and vector for cleanup.
    std::vector<std::unique_ptr<CType>> types;
    ffi_type *argtypes[args.size() + vargs.size()];

    // Value buffer and vector for cleanup.
    std::vector<std::unique_ptr<uint8_t[]>> argbufs;
    void *argvals[args.size() + vargs.size()];

    for (size_t i = 0; i < args.size(); i++) {
        auto tp = args[i]->type();
        argtypes[i] = tp->ffitype();
        types.push_back(std::move(tp));

        auto buf = args[i]->to_buffer();
        argvals[i] = buf.get();
        argbufs.push_back(std::move(buf));
    }
    for (size_t i = 0; i < vargs.size(); i++) {
        auto tp = vargs[i]->type();
        argtypes[args.size() + i] = tp->ffitype();
        types.push_back(std::move(tp));

        auto buf = vargs[i]->to_buffer();
        argvals[args.size() + i] = buf.get();
        argbufs.push_back(std::move(buf));
    }

    // Prepare the CIF.
    ffi_cif cif;
    if (vargs.size() == 0) {
        ffi_status status =
            ffi_prep_cif(&cif, FFI_DEFAULT_ABI, args.size(), rtype.ffitype(), argtypes);
        if (status != FFI_OK)
            throw std::runtime_error("ffi_prep_cif() failed");
    } else {
        ffi_status status =
            ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, args.size(),
                             args.size() + vargs.size(), rtype.ffitype(), argtypes);
        if (status != FFI_OK)
            throw std::runtime_error("ffi_prep_cif_var() failed");
    }

    // Call the function.
    uint8_t rvalue[std::max(sizeof(ffi_arg), rtype.size())];
    ffi_call(&cif, (void (*)())m_pointer, rvalue, argvals);

    return CValue::from_buffer(rtype, rvalue);
}

/**
 * Dereference the pointer.
 */
extern "C" lean_obj_res Pointer_read(b_lean_obj_arg ptr, b_lean_obj_arg type,
                                     lean_object *unused) {
    auto p = Pointer::unbox(ptr);
    auto ct = CType::unbox(type);

    try {
        return lean_io_result_mk_ok(p->read(*ct)->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Write to the pointer address.
 */
extern "C" lean_obj_res Pointer_write(b_lean_obj_arg ptr, b_lean_obj_arg value,
                                      lean_object *unused) {
    auto p = Pointer::unbox(ptr);
    try {
        p->write(*CValue::unbox(value));
        return lean_io_result_mk_ok(lean_box(0));
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}

/**
 * Create a pointer from an address.
 */
extern "C" lean_obj_res Pointer_mk(size_t value) {
    return (new Pointer((uint8_t *)value))->box();
}

/**
 * Get the address of a pointer.
 */
extern "C" size_t Pointer_address(b_lean_obj_arg obj) {
    return (size_t)Pointer::unbox(obj)->pointer();
}

/**
 * Call a pointer with CValue arguments.
 */
extern "C" lean_obj_res Pointer_call(b_lean_obj_arg ptr_obj, b_lean_obj_arg rtype_obj,
                                     b_lean_obj_arg args_obj, b_lean_obj_arg vargs_obj,
                                     lean_object *unused) {

    auto ptr = Pointer::unbox(ptr_obj);
    auto rtype = CType::unbox(rtype_obj);

    // Regular arguments
    std::vector<std::unique_ptr<CValue>> args;
    for (size_t i = 0; i < lean_array_size(args_obj); i++) {
        lean_object *o = lean_array_get_core(args_obj, i);
        args.push_back(CValue::unbox(o));
    }

    // Variadic arguments
    std::vector<std::unique_ptr<CValue>> vargs;
    for (size_t i = 0; i < lean_array_size(vargs_obj); i++) {
        lean_object *o = lean_array_get_core(vargs_obj, i);
        vargs.push_back(CValue::unbox(o));
    }

    try {
        auto result = ptr->call(*rtype, args, vargs);
        return lean_io_result_mk_ok(result->box());
    } catch (const std::runtime_error &error) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(error.what()));
        return lean_io_result_mk_error(err);
    }
}
