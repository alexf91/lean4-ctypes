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

#include "utils.hpp"
#include "lean/lean.h"
#include <cstdarg>
#include <cstdio>
#include <stdint.h>
#include <stdlib.h>

extern "C" {

/** Check if NDEBUG is defined. */
uint8_t debug_mode(lean_object *unused) {
#ifdef NDEBUG
    return 0;
#else
    return 1;
#endif
}

/* Print function type. */
typedef lean_object *(*lean_print_t)(lean_object *, lean_object *);

/** Regular fprintf(), but redirected through Lean. */
static inline void lean_vfprintf(lean_print_t fn, const char *fmt, va_list ap) {
    va_list apcopy;
    va_copy(apcopy, ap);

    // Determine the required buffer size.
    size_t needed = vsnprintf(nullptr, 0, fmt, ap) + 1;
    char *buffer = (char *)malloc(needed);
    vsprintf(buffer, fmt, apcopy);
    fn(lean_mk_string(buffer), nullptr);
    free(buffer);
}

/** Regular printf(), but redirected through Lean's stdout. */
void lean_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_print, fmt, ap);
}

/** Regular printf(), but redirected through Lean's stderr. */
void lean_eprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lean_vfprintf(lean_eprint, fmt, ap);
}
}
