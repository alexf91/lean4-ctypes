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

#include "external_type.hpp"
#include <lean/lean.h>

class Library final : public ExternalType<Library> {
  public:
    Library(b_lean_obj_arg path, b_lean_obj_arg flags);
    ~Library();

    /** Get the name of the library. */
    const char *get_path(void) { return m_path; }

    /** Get the handle of the library. */
    void *get_handle(void) { return m_handle; }

  private:
    // Path of the library for debugging.
    char *m_path;
    // Handle returned by dlopen().
    void *m_handle;
};
