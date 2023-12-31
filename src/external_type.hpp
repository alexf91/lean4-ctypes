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
#include <vector>

/**
 * External types are defined as opaque in Lean and are only accessed by other opaque
 * functions in Lean.
 *
 * The class provides boxing, unboxing and finalizer methods.
 * The template parameter is the class returned by unbox(), which is usually the class
 * itself. The derived class should probably be final.
 */
template <class T> class ExternalType {
  public:
    // Convert from C to Lean.
    lean_object *box() {
        if (m_class == nullptr)
            m_class = lean_register_external_class(finalize, foreach);
        return lean_alloc_external(m_class, this);
    }

    // Convert from Lean to C.
    static T *unbox(b_lean_obj_arg obj) {
        assert(lean_is_external(obj));
        return (T *)(lean_get_external_data(obj));
    }

    // Child objects of the object.
    // If foreach is called, then we iterate over this vector and apply the function to
    // every object.
    virtual const std::vector<lean_object *> children() = 0;

  private:
    // Deletes the object when it is garbage collected.
    static void finalize(void *p) { delete (T *)p; }
    static void foreach (void *obj, b_lean_obj_arg fn) {
        for (auto o : ((T *)obj)->children())
            lean_apply_1(fn, o);
    }

    // Registered class in Lean.
    inline static lean_external_class *m_class = nullptr;
};
