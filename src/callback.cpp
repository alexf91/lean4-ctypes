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

#include "callback.hpp"
#include "lean/lean.h"
#include "pointer.hpp"
#include <ffi.h>
#include <iostream>
#include <stdexcept>

/** Create the callback option. */
Callback::Callback(b_lean_obj_arg rtype_obj, b_lean_obj_arg args_obj,
                   lean_obj_arg cb_obj)
    : m_cb_obj(cb_obj), m_rtype(CType::unbox(rtype_obj)) {

    size_t nargs = lean_array_size(args_obj);
    m_ffi_argtypes = new ffi_type *[nargs];
    for (size_t i = 0; i < nargs; i++) {
        m_argtypes.push_back(CType::unbox(lean_array_get_core(args_obj, i)));
        m_ffi_argtypes[i] = m_argtypes[i]->ffitype();
    }
    m_closure = (ffi_closure *)ffi_closure_alloc(sizeof(ffi_closure), &m_function);
    if (m_closure == nullptr)
        lean_internal_panic("ffi_closure_alloc() failed");

    ffi_status status = ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, nargs, m_rtype->ffitype(),
                                     m_ffi_argtypes);
    if (status != FFI_OK)
        throw std::runtime_error("ffi_prep_cif() failed");

    status = ffi_prep_closure_loc(m_closure, &m_cif, binding, this, m_function);
    if (status != FFI_OK)
        throw std::runtime_error("ffi_prep_closure_loc() failed");
}

/**
 * Delete the callback object.
 */
Callback::~Callback() {
    lean_dec(m_cb_obj);
    delete[] m_ffi_argtypes;
    ffi_closure_free(m_closure);
}

/** Callback wrapper. */
void Callback::binding(ffi_cif *cif, void *ret, void *args[], void *data) {
    Callback *this_ = static_cast<Callback *>(data);
    assert(cif == &this_->m_cif);

    size_t nargs = this_->m_argtypes.size();
    std::vector<std::unique_ptr<CValue>> args_vec;
    lean_object *args_obj = lean_alloc_array(nargs, nargs);
    for (size_t i = 0; i < nargs; i++) {
        args_vec.push_back(
            CValue::from_buffer(*this_->m_argtypes[i], (uint8_t *)args[i]));
        lean_array_set_core(args_obj, i, args_vec[i]->box());
    }

    lean_object *result = lean_apply_2(this_->m_cb_obj, args_obj, lean_io_mk_world());
    // TODO: How do we handle exceptions?
    assert(lean_io_result_is_ok(result));

    auto buffer = CValue::unbox(lean_io_result_get_value(result))->to_buffer();
    memcpy(ret, buffer.get(), this_->m_rtype->size());
}
