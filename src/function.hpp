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

#include "utils.hpp"
#include <ffi.h>
#include <lean/lean.h>

class Function {
  public:
    Function(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
             b_lean_obj_arg argtypes_object);
    ~Function();

    /** Call the function with the given arguments. */
    lean_obj_res call(b_lean_obj_arg argvals_object);

    /** Convert a Function object from C to Lean. */
    lean_object *box(void);

    /** Unbox a Function from a Lean object. */
    static Function *unbox(b_lean_obj_arg obj);

    /** Get the number of arguments. */
    size_t get_nargs() { return lean_array_size(m_argtypes_object); }

  private:
    static void finalize(void *p);
    static void foreach (void *mod, b_lean_obj_arg fn);

  private:
    // Symbol assocated with the function.
    lean_object *m_symbol;
    // Return type as a Lean object.
    lean_object *m_rtype_object;
    // Argument types as a Lean object.
    lean_object *m_argtypes_object;
    // CIF to call the function.
    ffi_cif m_cif;
    // Return type used in the CIF.
    ffi_type *m_rtype;
    // Argument types used in the CIF.
    ffi_type **m_argtypes;
    // Registered class in Lean.
    inline static lean_external_class *m_class = nullptr;
};

/** Boxing of LeanType objects. */
extern "C" LEAN_EXPORT_WEAK lean_obj_res LeanType_box(b_lean_obj_arg o, uint64_t addr);