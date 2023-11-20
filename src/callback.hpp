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

#pragma once

#include "types.hpp"
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

/**
 * Callback function.
 *
 * This is mainly used to encapsulate types required while a libffi closure
 * is alive.
 * A Callback might stay alive even after the associated Closure object is
 * garbage collected.
 */
class Callback {
  public:
    Callback(b_lean_obj_arg rtype_obj, b_lean_obj_arg args_obj, lean_obj_arg cb_obj);

    ~Callback();

    std::unique_ptr<Pointer> pointer() {
        return std::make_unique<Pointer>((uint8_t *)m_function);
    }

  private:
    /** Callback wrapper. */
    static void binding(ffi_cif *cif, void *ret, void *args[], void *data);

  private:
    lean_object *m_cb_obj;
    ffi_cif m_cif;
    ffi_closure *m_closure;
    void *m_function;
    std::unique_ptr<CType> m_rtype;
    std::vector<std::unique_ptr<CType>> m_argtypes;
    ffi_type **m_ffi_argtypes;
};
