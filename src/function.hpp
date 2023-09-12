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

#include "ctype.hpp"
#include "external_type.hpp"
#include "utils.hpp"
#include <cstdint>
#include <ffi.h>
#include <lean/lean.h>

class Function final : public ExternalType<Function> {
  public:
    Function(b_lean_obj_arg symbol, b_lean_obj_arg rtype_object,
             b_lean_obj_arg argtypes_object);
    ~Function();

    /**
     * Call the function with the given arguments.
     * The result is a boxed LeanValue object. Note that the result is not yet
     * in the IO monad.
     */
    lean_obj_res call(b_lean_obj_arg argvals_object);

    /** Get the number of arguments. */
    size_t get_nargs() { return m_argtypes.size(); }

  private:
    // Symbol assocated with the function.
    lean_object *m_symbol;

    // Unboxed CType for the return type.
    std::unique_ptr<CType> m_rtype;
    // Unboxed CTypes for the argument types.
    std::vector<std::unique_ptr<CType>> m_argtypes;

    // CIF to call the function.
    ffi_cif m_cif;
    // Return type used in the CIF.
    ffi_type *m_ffi_rtype;
    // Argument types used in the CIF.
    ffi_type **m_ffi_argtypes;
};
