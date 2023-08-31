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

#include "utils.h"
#include <ffi.h>
#include <lean/lean.h>

/**
 * Function handle created by Function_mk().
 */
typedef struct {
    lean_object *symbol;
    // The original objects passed as arguments to Function_mk().
    lean_object *rtype_obj;
    lean_object *argtypes_obj;
    // The prepared call interface.
    ffi_cif *cif;
    // The CIF requires the unboxed types to stay in memory.
    ffi_type *rtype;
    size_t nargs;
    ffi_type **argtypes;
} Function;

/** Boxing of LeanType objects. */
LEAN_EXPORT_WEAK lean_obj_res LeanType_box(b_lean_obj_arg o, uint64_t addr);
// #define LeanType_box_unit() lean_box(0)
// LEAN_EXPORT_WEAK lean_obj_res LeanType_box_int(b_lean_obj_arg o);
// LEAN_EXPORT_WEAK lean_obj_res LeanType_box_float(b_lean_obj_arg o);
