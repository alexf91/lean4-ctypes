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

#include <complex>
#include <cstdint>
#include <lean/lean.h>

#include "ctype.hpp"
#include "utils.hpp"

/**
 * A type in Lean.
 */
class LeanType {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag { UNIT, INT, FLOAT, LENGTH };

    LeanType(ObjectTag tag);
    ~LeanType();

    /** Get a string representation of the type. */
    const char *to_string() { return "not implemented"; }

    /** Box the type to a Lean object. */
    lean_obj_res unbox();

    /** Convert from Lean to this class. */
    static LeanType *unbox(b_lean_obj_arg obj);

    /** Convert the type to a buffer for calling the function. */
    void *to_buffer();

    /** Get the object tag. */
    ObjectTag get_tag() { return m_tag; }

  private:
    ObjectTag m_tag;
};
