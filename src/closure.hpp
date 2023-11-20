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

#include "callback.hpp"
#include "external_type.hpp"
#include "types.hpp"
#include <ffi.h>
#include <lean/lean.h>
#include <memory>
#include <vector>

/** Closure object for implementing callbacks. */
class Closure final : public ExternalType<Closure> {
  public:
    /** Create a closure from a callback function and argument spec. */
    Closure(b_lean_obj_arg rtype_obj, b_lean_obj_arg args_obj, lean_obj_arg cb_obj)
        : m_callback(new Callback(rtype_obj, args_obj, cb_obj)) {}

    /**
     * Delete the callback only when it is marked for deletion.
     *
     * Note that this leaks memory on purpose.
     */
    ~Closure() {
        if (m_delete)
            delete m_callback;
    }

    void del() { m_delete = true; }

    std::unique_ptr<Pointer> pointer() { return m_callback->pointer(); }

    const std::vector<lean_object *> children() { return {}; }

  private:
    bool m_delete;
    Callback *m_callback;
};
