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
#include <stdarg.h>
#include <stdio.h>

/** We need this for functions exported from Lean. */
#define LEAN_EXPORT_WEAK __attribute__((weak)) LEAN_EXPORT

/**
 * Prototypes for exported functions.
 *
 * It is not clear what the second argument is used for, but it probably has
 * something to do with the IO monad.
 */
LEAN_EXPORT_WEAK lean_obj_res lean_print(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_obj_res lean_println(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_obj_res lean_eprintln(lean_object *str, lean_object *unknown);
LEAN_EXPORT_WEAK lean_obj_res lean_eprint(lean_object *str, lean_object *unknown);

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
#define utils_log(...)                                                                 \
    do {                                                                               \
        lean_eprintf("[FFI] %s:%s - ", __FILE__, __func__);                            \
        lean_eprintf(__VA_ARGS__);                                                     \
        lean_eprintf("\n");                                                            \
    } while (0)
#else
/* clang-format off */
static inline void utils_log(...) { }
/* clang-format on */
#endif /* NDEBUG */

/** Minimum of two values. */
#define min(a, b)                                                                      \
    ({                                                                                 \
        __typeof__(a) _a = (a);                                                        \
        __typeof__(b) _b = (b);                                                        \
        _a < _b ? _a : _b;                                                             \
    })

/** Maximum of two values. */
#define max(a, b)                                                                      \
    ({                                                                                 \
        __typeof__(a) _a = (a);                                                        \
        __typeof__(b) _b = (b);                                                        \
        _a > _b ? _a : _b;                                                             \
    })
