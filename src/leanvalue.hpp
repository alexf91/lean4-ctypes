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

#include "ctype.hpp"
#include "utils.hpp"
#include <complex>
#include <cstdint>
#include <lean/lean.h>
#include <memory>

class CType;

extern "C" {
/** Create a LeanValue.unit object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkUnit(b_lean_obj_arg obj);
/** Create a LeanValue.int object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkInt(b_lean_obj_arg obj);
/** Create a LeanValue.float object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkFloat(double obj);
/** Create a LeanValue.complex object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkComplex(double a, double b);
/** Create a LeanValue.struct object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkStruct(b_lean_obj_arg values);
/** Check if a CType is compatible with a LeanValue. */
LEAN_EXPORT_WEAK uint8_t Types_compatible(b_lean_obj_arg ct, b_lean_obj_arg lv);
}

/**
 * A type in Lean.
 */
class LeanValue {
  public:
    /** The object tags of the inductive type defined in Lean.  */
    enum ObjectTag {
        UNIT,
        INT,
        FLOAT,
        COMPLEX,
        STRUCT,
        LENGTH,
    };

    LeanValue(ObjectTag tag) : m_tag(tag) {}
    virtual ~LeanValue() {}

    /**
     * Box the type to a Lean object.
     * To treat values correctly, we require the corresponding C type.
     */
    virtual lean_obj_res box(const CType &ct) = 0;

    /** Convert from Lean to this class. */
    static std::unique_ptr<LeanValue> unbox(b_lean_obj_arg obj);

    /**
     * Convert the type to a buffer for calling the function.
     * The value is converted to the given CType first.
     */
    virtual std::unique_ptr<uint8_t[]> to_buffer(const CType &ct) = 0;

    /** Convert from a buffer and a CType back to a LeanValue object.  */
    static std::unique_ptr<LeanValue> from_buffer(const CType &ct,
                                                  const uint8_t *buffer);

    /** Get the object tag. */
    ObjectTag get_tag() const { return m_tag; }

  private:
    ObjectTag m_tag;
};

// TODO: Use Template here?

/** LeanValue specialization for Unit types. */
class LeanValueUnit : public LeanValue {
  public:
    LeanValueUnit();
    ~LeanValueUnit() {}

    lean_obj_res box(const CType &ct);

    std::unique_ptr<uint8_t[]> to_buffer(const CType &ct);
};

/** LeanValue specialization for Integer types. */
class LeanValueInt : public LeanValue {
  public:
    /** Constructor for integer values. */
    LeanValueInt(size_t value);
    LeanValueInt(ssize_t value);

    /** Constructor for LeanValue.int objects. */
    LeanValueInt(b_lean_obj_arg obj);

    ~LeanValueInt() {}

    lean_obj_res box(const CType &ct);

    std::unique_ptr<uint8_t[]> to_buffer(const CType &ct);

  private:
    // The representation of the value as a 64 bit value.
    uint64_t m_value;
};

/**
 * LeanValue specialization for Float types.
 * We use double as the internal representation, since this is what Lean uses.
 */
class LeanValueFloat : public LeanValue {
  public:
    /** Constructor for floating point values. */
    LeanValueFloat(double value);

    /** Constructor for LeanValue.float objects. */
    LeanValueFloat(b_lean_obj_arg obj);

    ~LeanValueFloat() {}

    lean_obj_res box(const CType &ct);

    std::unique_ptr<uint8_t[]> to_buffer(const CType &ct);

  private:
    double m_value;
};

/**
 * LeanValue specialization for complex types.
 * We use complex<double> as the internal representation, since this is what Lean uses
 * for floats.
 */
class LeanValueComplex : public LeanValue {
  public:
    /** Constructor for complex floating point values. */
    LeanValueComplex(std::complex<double> value);

    /** Constructor for LeanValue.complex objects. */
    LeanValueComplex(b_lean_obj_arg obj);

    ~LeanValueComplex() {}

    lean_obj_res box(const CType &ct);

    std::unique_ptr<uint8_t[]> to_buffer(const CType &ct);

  private:
    std::complex<double> m_value;
};

/**
 * LeanValue specialization for struct types.
 */
class LeanValueStruct : public LeanValue {
  public:
    /** Constructor for struct values. */
    LeanValueStruct(std::vector<LeanValue *> values);

    /** Constructor for LeanValue.struct objects. */
    LeanValueStruct(b_lean_obj_arg obj);

    ~LeanValueStruct();

    lean_obj_res box(const CType &ct);

    std::unique_ptr<uint8_t[]> to_buffer(const CType &ct);

  private:
    std::vector<LeanValue *> m_values;
};
