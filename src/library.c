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

#include "library.h"
#include "utils.h"
#include <dlfcn.h>
#include <lean/lean.h>
#include <string.h>

/** Unbox the Flag enum. */
static inline int Flag_unbox(b_lean_obj_arg flag) {
    assert(lean_is_scalar(flag));
    switch (lean_unbox(flag)) {
    case 0:
        return RTLD_LAZY;
    case 1:
        return RTLD_NOW;
    case 2:
        return RTLD_NOLOAD;
    case 3:
        return RTLD_DEEPBIND;
    case 4:
        return RTLD_GLOBAL;
    case 5:
        return RTLD_LOCAL;
    case 6:
        return RTLD_NODELETE;
    }
    assert(0);
    return -1;
}

/** Lean class */
lean_external_class *Library_class = NULL;

/** Finalize a Library. */
static void Library_finalize(void *p) {
    Library *lib = (Library *)p;
    utils_log("finalizing %s", lib->name);
    if (dlclose(lib->handle) != 0) {
        utils_log("dlclose() failed: %s", dlerror());
    }
    free(lib->name);
    free(lib);
}

/** Foreach for a Library handle. */
static void Library_foreach(void *mod, b_lean_obj_arg fn) {
    utils_log("NOT IMPLEMENTED");
}

/** Convert a Library object from C to Lean. */
static inline lean_object *Library_box(Library *lib) {
    if (Library_class == NULL) {
        Library_class = lean_register_external_class(Library_finalize, Library_foreach);
    }
    return lean_alloc_external(Library_class, lib);
}

/**
 * Create a new Library instance.
 *
 * All arguments passed to this function are borrowed.
 *
 * @param path Path of the library as a string.
 * @param flags Array with the flags to open the library.
 *
 * @return Library object or an exception.
 */
lean_object *Library_mk(b_lean_obj_arg path, b_lean_obj_arg flags,
                        lean_object *unused) {
    const char *p = lean_string_cstr(path);

    uint32_t openflags = 0;
    for (int i = 0; i < lean_array_size(flags); i++) {
        lean_object *o = lean_array_get_core(flags, i);
        openflags |= Flag_unbox(o);
    }
    utils_log("opening %s with flags 0x%08x", p, openflags);

    void *handle = dlopen(p, openflags);
    if (handle == NULL) {
        lean_object *err = lean_mk_io_user_error(lean_mk_string(dlerror()));
        return lean_io_result_mk_error(err);
    }
    Library *lib = malloc(sizeof(Library));
    lib->name = strdup(p);
    lib->handle = handle;
    return lean_io_result_mk_ok(Library_box(lib));
}
