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

#include <complex>
#include <cstdint>
#include <lean/lean.h>
#include <memory>

#include "ctype.hpp"
#include "utils.hpp"

class CType;

extern "C" {
/** Create a LeanType.unit object. */
LEAN_EXPORT_WEAK lean_obj_res LeanType_mkUnit(b_lean_obj_arg obj);
/** Create a LeanType.int object. */
LEAN_EXPORT_WEAK lean_obj_res LeanType_mkInt(b_lean_obj_arg obj);
/** Create a LeanType.float object. */
LEAN_EXPORT_WEAK lean_obj_res LeanType_mkFloat(double obj);
}

/**
 * A type in Lean.
 */
class LeanType {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        UNIT,
        INT,
        FLOAT,
        LENGTH,
    };

    LeanType(ObjectTag tag) { m_tag = tag; }
    virtual ~LeanType() {}

    /** Get a string representation of the type. */
    const char *to_string() { lean_internal_panic("not implemented"); }

    /**
     * Box the type to a Lean object.
     * To treat values correctly, we require the corresponding C type.
     */
    virtual lean_obj_res box(CType *ct) = 0;
    lean_obj_res box(std::shared_ptr<CType> ct) { return box(ct.get()); }

    /** Convert from Lean to this class. */
    static LeanType *unbox(b_lean_obj_arg obj);

    /**
     * Convert the type to a buffer for calling the function.
     * The value is converted to the given CType first.
     */
    virtual void *to_buffer(CType *ct) = 0;
    void *to_buffer(std::shared_ptr<CType> ct) { return to_buffer(ct.get()); }

    /** Get the object tag. */
    ObjectTag get_tag() { return m_tag; }

  private:
    ObjectTag m_tag;
};

// TODO: Use Template here?

/** LeanType specialization for Unit types. */
class LeanTypeUnit : public LeanType {
  public:
    LeanTypeUnit();
    ~LeanTypeUnit() {}

    lean_obj_res box(CType *ct);

    void *to_buffer(CType *ct);
};

/** LeanType specialization for Integer types. */
class LeanTypeInt : public LeanType {
  public:
    /** Constructor for integer values. */
    LeanTypeInt(size_t value);
    LeanTypeInt(ssize_t value);

    /** Constructor for integer objects. */
    LeanTypeInt(b_lean_obj_arg obj);

    ~LeanTypeInt() {}

    lean_obj_res box(CType *ct);

    void *to_buffer(CType *ct);

  private:
    // The representation of the value as a 64 bit value.
    uint64_t m_value;
};

/**
 * LeanType specialization for Float types.
 * We use double as the internal representation, since this is what Lean uses.
 * TODO: This might cause a loss of precision.
 */
class LeanTypeFloat : public LeanType {
  public:
    /** Constructor for floating point values. */
    LeanTypeFloat(double value);

    /** Constructor for floating point objects. */
    LeanTypeFloat(b_lean_obj_arg obj);

    ~LeanTypeFloat() {}

    lean_obj_res box(CType *ct);

    void *to_buffer(CType *ct);

  private:
    double m_value;
};
