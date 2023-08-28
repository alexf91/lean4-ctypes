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

#include <ffi.h>
#include <lean/lean.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/** We need this for functions exported from Lean. */
#define LEAN_EXPORT_WEAK __attribute__((weak)) LEAN_EXPORT

/**
 * Prototypes for exported functions.
 *
 * It is not clear what the second argument is used for, but it probably has
 * something to do with the IO monad.
 */
LEAN_EXPORT_WEAK lean_object *lean_print(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_object *lean_println(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_object *lean_eprintln(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_object *lean_eprint(lean_object *str, lean_object *unknown);

/**
 * Regular printf(), but redirected through Lean's stdout.
 */
void lean_printf(const char *fmt, ...);

/**
 * Regular printf(), but redirected through Lean's stderr.
 */
void lean_eprintf(const char *fmt, ...);

/**
 * Print a log message.
 */
#ifndef NDEBUG
#define native_log(...)                                                                \
    do {                                                                               \
        lean_eprintf("[FFI] %s:%s - ", __FILE__, __func__);                            \
        lean_eprintf(__VA_ARGS__);                                                     \
        lean_eprintf("\n");                                                            \
    } while (0)
#else
#define native_log(...)
#endif /* NDEBUG */

/**
 * Library handle returned by dlopen().
 */
typedef struct {
    char *name; // Only for debugging
    void *handle;
} Library;

/**
 * Symbol handle returned by dlsym().
 */
typedef struct {
    char *name; // Only for debugging
    void *handle;
    lean_object *library;
} Symbol;

/**
 * Function handle created by Function_mk().
 */
typedef struct {
    lean_object *symbol;
    ffi_cif *cif;
    ffi_type *return_type;
    size_t nargs;         // Number of arguments
    ffi_type **arguments; // NULL terminated array.
} Function;
