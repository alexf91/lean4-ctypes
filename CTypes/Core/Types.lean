--
-- Copyright 2023 Alexander Fasching
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
-- http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--

set_option relaxedAutoImplicit false

namespace CTypes.Core

/-- Wrapper around a raw `void` pointer in C. -/
opaque Pointer.Nonempty : NonemptyType
def Pointer : Type := Pointer.Nonempty.type
instance : Nonempty Pointer := Pointer.Nonempty.property


namespace Pointer
  /-- Create a pointer from an integer value. -/
  @[extern "Pointer_mk"]
  opaque mk (p : @&USize) : Pointer

  /-- NULL pointer. -/
  def null : Pointer := Pointer.mk 0

  /-- Get the address as an integer. -/
  @[extern "Pointer_address"]
  opaque address (p : @&Pointer) : USize

end Pointer

instance : Inhabited Pointer := ⟨Pointer.mk 0⟩
instance : Repr Pointer := ⟨fun p _ => s!"CTypes.Pointer<{p.address}>"⟩
instance : BEq Pointer := ⟨fun a b => a.address == b.address⟩

@[default_instance high]
instance : HAdd Pointer USize Pointer := ⟨fun p n => .mk (p.address + n)⟩
@[default_instance high]
instance : HSub Pointer USize Pointer := ⟨fun p n => .mk (p.address - n)⟩

/--
  Types in C and `libffi`.

  These types correspond directly to a subset of types available in `libffi`.
  Notably, a type for arrays is missing, as these types are emulated with structs.

  They also correspond directly to the `CValue` type.
-/
inductive CType where
  | void
  | int8
  | int16
  | int32
  | int64
  | uint8
  | uint16
  | uint32
  | uint64
  | float
  | double
  | longdouble
  | complex_float
  | complex_double
  | complex_longdouble
  | pointer
  | struct (elements : Array CType)
deriving Inhabited, Repr

namespace CType
  /-- Get the size of a basic type. -/
  @[extern "CType_size"]
  opaque size (type : @&CType) : Nat

  /-- Get struct offsets. -/
  @[extern "CType_offsets"]
  opaque offsets (type : @&CType) : Array Nat

  /-- Get alignment. -/
  @[extern "CType_alignment"]
  opaque alignment (type : @&CType) : Nat
end CType


/--
  The `CType` type, but with an associated value to make it a `CValue`.
-/
inductive CValue where
  | void
  | int8               (a   : Int)
  | int16              (a   : Int)
  | int32              (a   : Int)
  | int64              (a   : Int)
  | uint8              (a   : Nat)
  | uint16             (a   : Nat)
  | uint32             (a   : Nat)
  | uint64             (a   : Nat)
  | float              (a   : Float)
  | double             (a   : Float)
  | longdouble         (a   : Float)
  | complex_float      (a b : Float)
  | complex_double     (a b : Float)
  | complex_longdouble (a b : Float)
  | pointer            (a   : Pointer)
  | struct             (a   : Array CValue)
deriving Inhabited, Repr, BEq


namespace CValue

  /-- Get the type of a `CValue`. -/
  @[export CValue_type]
  private partial def CValue.type : CValue → CType
    | .void .. => .void
    | .int8 .. => .int8
    | .int16 .. => .int16
    | .int32 .. => .int32
    | .int64 .. => .int64
    | .uint8 .. => .uint8
    | .uint16 .. => .uint16
    | .uint32 .. => .uint32
    | .uint64 .. => .uint64
    | .float .. => .float
    | .double .. => .double
    | .longdouble .. => .longdouble
    | .complex_float .. => .complex_float
    | .complex_double .. => .complex_double
    | .complex_longdouble .. => .complex_longdouble
    | .pointer .. => .pointer
    | .struct e => .struct (e.map type)

  /- Constructors for values. Avoids dealing with ctor magic in C++. -/
  @[export CValue_mk_void]
  private def mk_void := CValue.void
  @[export CValue_mk_int8]
  private def mk_int8 := CValue.int8
  @[export CValue_mk_int16]
  private def mk_int16 := CValue.int16
  @[export CValue_mk_int32]
  private def mk_int32 := CValue.int32
  @[export CValue_mk_int64]
  private def mk_int64 := CValue.int64
  @[export CValue_mk_uint8]
  private def mk_uint8 := CValue.uint8
  @[export CValue_mk_uint16]
  private def mk_uint16 := CValue.uint16
  @[export CValue_mk_uint32]
  private def mk_uint32 := CValue.uint32
  @[export CValue_mk_uint64]
  private def mk_uint64 := CValue.uint64
  @[export CValue_mk_float]
  private def mk_float := CValue.float
  @[export CValue_mk_double]
  private def mk_double := CValue.double
  @[export CValue_mk_longdouble]
  private def mk_longdouble := CValue.longdouble
  @[export CValue_mk_complex_float]
  private def mk_complex_float := CValue.complex_float
  @[export CValue_mk_complex_double]
  private def mk_complex_double := CValue.complex_double
  @[export CValue_mk_complex_longdouble]
  private def mk_complex_longdouble := CValue.complex_longdouble
  @[export CValue_mk_pointer]
  private def mk_pointer := CValue.pointer
  @[export CValue_mk_struct]
  private def mk_struct := CValue.struct

  /- Functions to extract a value from a `CValue`. -/
  def int? : CValue → Option Int
  | .int8  a => a
  | .int16 a => a
  | .int32 a => a
  | .int64 a => a
  | _        => none

  def nat? : CValue → Option Nat
  | .uint8  a => a
  | .uint16 a => a
  | .uint32 a => a
  | .uint64 a => a
  | _         => none

  def float? : CValue → Option Float
  | .float      a => a
  | .double     a => a
  | .longdouble a => a
  | _             => none

  def complex? : CValue → Option (Float × Float)
  | .complex_float      a b => (a, b)
  | .complex_double     a b => (a, b)
  | .complex_longdouble a b => (a, b)
  | _                       => none

  def pointer? : CValue → Option Pointer
  | .pointer p => p
  | _          => none

  def struct? : CValue → Option (Array CValue)
  | .struct e => e
  | _         => none

  @[export CValue_get_int]
  def int! a := (int? a).get!

  @[export CValue_get_nat]
  def nat a := (nat? a).get!

  @[export CValue_get_float]
  def float! a := (float? a).get!

  @[export CValue_get_complex]
  def complex! a := (complex? a).get!

  @[export CValue_get_pointer]
  def pointer! a := (pointer? a).get!

  @[export CValue_get_struct]
  def struct! a := (struct? a).get!

end CValue

namespace Pointer

  /--
    Dereference the pointer and cast it to the given `CType`.
    This then creates a `LeanValue` value.
  -/
  @[extern "Pointer_read"]
  opaque read (p : @&Pointer) (type : @&CType) : IO CValue

  /--
    Assign a value to the address the pointer points to.
    The given `LeanValue` value is cast to the given `CType` first and
    then copied to the memory location.
  -/
  @[extern "Pointer_write"]
  opaque write (p : @&Pointer) (value : @&CValue) : IO Unit

  /--
    Call a pointer as a function.

    Regular and variadic arguments are split to differ between variadic and regular
    functions. This is necessary because some platforms have a different calling
    convention for these functions.

    Calling a function with an empty `vargs` parameter calls `ffi_prep_cif()`, but
    with a nonempty `vargs` it calls `ffi_prep_cif_var()` to prepare the CIF.
  -/
  @[extern "Pointer_call"]
  opaque call (p : @&Pointer) (rtype : @&CType) (args : @&Array CValue) (vargs : @&Array CValue) : IO CValue

end Pointer

end CTypes.Core
