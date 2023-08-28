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

/**
 * Library handle returned by dlopen().
 */
typedef struct {
    char *name; // Only for debugging
    void *handle;
} Library;

/** Convert a Library object from Lean to C. */
static inline Library *Library_unbox(b_lean_obj_arg lib) {
    return (Library *)(lean_get_external_data(lib));
}
