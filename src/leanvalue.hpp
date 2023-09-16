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

#include "utils.hpp"
#include <complex>
#include <cstdint>
#include <lean/lean.h>
#include <memory>
#include <vector>

extern "C" {
/** Create a LeanValue.unit object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkUnit(b_lean_obj_arg obj);
/** Create a LeanValue.int object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkInt(b_lean_obj_arg obj);
/** Create a LeanValue.nat object. */
LEAN_EXPORT_WEAK lean_obj_res LeanValue_mkNat(b_lean_obj_arg obj);
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
        NAT,
        FLOAT,
        COMPLEX,
        STRUCT,
        LENGTH,
    };

    LeanValue(ObjectTag tag) : m_tag(tag) {}
    virtual ~LeanValue() {}

    /** Box the type to a Lean object.  */
    virtual lean_obj_res box() = 0;

    /** Convert from Lean to this class. */
    static std::unique_ptr<LeanValue> unbox(b_lean_obj_arg obj);

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

    lean_obj_res box();
};

/** LeanValue specialization for signed integer types. */
class LeanValueInt : public LeanValue {
  public:
    /** Constructor for signed integer values. */
    LeanValueInt(int64_t value);

    /** Constructor for LeanValue.int objects. */
    LeanValueInt(b_lean_obj_arg obj);

    ~LeanValueInt() {}

    lean_obj_res box();

    // TODO: bad practice
    int64_t get_value() const { return m_value; }

  private:
    int64_t m_value; // Maximum size of values in C.
};

/** LeanValue specialization for unsigned integer types. */
class LeanValueNat : public LeanValue {
  public:
    /** Constructor for signed integer values. */
    LeanValueNat(uint64_t value);

    /** Constructor for LeanValue.int objects. */
    LeanValueNat(b_lean_obj_arg obj);

    ~LeanValueNat() {}

    lean_obj_res box();

    // TODO: bad practice
    uint64_t get_value() const { return m_value; }

  private:
    uint64_t m_value; // Maximum size of values in C.
};

/**
 * LeanValue specialization for Float types.
 * We use double as the internal representation, since this is what Lean uses.
 */
class LeanValueFloat : public LeanValue {
  public:
    /** Constructor for floating point values. */
    LeanValueFloat(long double value);

    /** Constructor for LeanValue.float objects. */
    LeanValueFloat(b_lean_obj_arg obj);

    ~LeanValueFloat() {}

    lean_obj_res box();

    // TODO: bad practice
    double get_value() const { return m_value; }

  private:
    double m_value; // Lean doesn't use higher precision.
};

/**
 * LeanValue specialization for complex types.
 * We use complex<double> as the internal representation, since this is what Lean uses
 * for floats.
 */
class LeanValueComplex : public LeanValue {
  public:
    /** Constructor for complex floating point values. */
    LeanValueComplex(std::complex<long double> value);

    /** Constructor for LeanValue.complex objects. */
    LeanValueComplex(b_lean_obj_arg obj);

    ~LeanValueComplex() {}

    lean_obj_res box();

    // TODO: bad practice
    std::complex<double> get_value() const { return m_value; }

  private:
    std::complex<double> m_value; // Lean doesn't use higher precision.
};

/**
 * LeanValue specialization for struct types.
 */
class LeanValueStruct : public LeanValue {
  public:
    /** Constructor for struct values. */
    LeanValueStruct(std::vector<std::unique_ptr<LeanValue>> values);

    /** Constructor for LeanValue.struct objects. */
    LeanValueStruct(b_lean_obj_arg obj);

    ~LeanValueStruct();

    lean_obj_res box();

    // TODO: Bad practice
    const std::vector<const LeanValue *> get_values() const {
        std::vector<const LeanValue *> values;
        for (size_t i = 0; i < m_values.size(); i++)
            values.push_back(m_values[i].get());
        return values;
    }

  private:
    std::vector<std::unique_ptr<LeanValue>> m_values;
};
