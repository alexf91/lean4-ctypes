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

#include <lean/lean.h>

class Symbol {
  public:
    Symbol(b_lean_obj_arg lib, b_lean_obj_arg sym);
    ~Symbol();

    /** Convert a Symbol object from C to Lean. */
    lean_object *box(void);

    /** Unbox a Symbol from a Lean object. */
    static Symbol *unbox(b_lean_obj_arg obj);

    /** Get the name of the symbol. */
    const char *get_name(void) { return m_name; }

    /** Get the handle of the symbol. */
    void *get_handle(void) { return m_handle; }

    /** Get the library object of the symbol. */
    lean_object *get_library(void) { return m_library; }

  private:
    static void finalize(void *p);
    static void foreach (void *mod, b_lean_obj_arg fn);

  private:
    // Name of the symbol for debugging.
    char *m_name;
    // Handle returned by dlsym().
    void *m_handle;
    // Library object for reference counting.
    lean_object *m_library;
    // Registered class in Lean.
    inline static lean_external_class *m_class = nullptr;
};
