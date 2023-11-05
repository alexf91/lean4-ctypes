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

namespace CTypes

/-- Types in C. -/
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
  | array (type : CType) (size : Nat)
  | struct (elements : Array CType)
  | union (elements : Array CType)

  -- Aliased types depending on size.
  | char
  | short
  | int
  | long
  | longlong
  | ssize_t
  | uchar
  | ushort
  | uint
  | ulong
  | ulonglong
  | size_t
  | time_t

deriving Repr, BEq

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


/-- Wrapper around a raw `void` pointer in C. -/
opaque Pointer.Nonempty : NonemptyType
def Pointer : Type := Pointer.Nonempty.type
instance : Nonempty Pointer := Pointer.Nonempty.property


/-- Values in Lean. -/
inductive LeanValue where
  | unit
  | int (a : Int)
  | nat (a : Nat)
  | float (a : Float)
  | complex (a b : Float)
  | array (values : Array LeanValue)
  | struct (values : Array LeanValue)
  | pointer (ptr : Pointer)


namespace LeanValue

  /-- Create a `LeanValue.unit` object. -/
  @[export LeanValue_mkUnit]
  private def mkUnit (_ : @&Unit) : LeanValue := .unit

  /-- Create a `LeanValue.int` object. -/
  @[export LeanValue_mkInt]
  private def mkInt (a : @&Int) : LeanValue := .int a

  /-- Create a `LeanValue.nat` object. -/
  @[export LeanValue_mkNat]
  private def mkNat (a : @&Nat) : LeanValue := .nat a

  /-- Create a `LeanValue.float` object. -/
  @[export LeanValue_mkFloat]
  private def mkFloat (a : @&Float) : LeanValue := .float a

  /-- Create a `LeanValue.complex` object. -/
  @[export LeanValue_mkComplex]
  private def mkComplex (a b : @&Float) : LeanValue := .complex a b

  /-- Create a `LeanValue.array` object. -/
  @[export LeanValue_mkArray]
  private def mkArray (values : @&Array LeanValue) : LeanValue := .array values

  /-- Create a `LeanValue.struct` object. -/
  @[export LeanValue_mkStruct]
  private def mkStruct (values : @&Array LeanValue) : LeanValue := .struct values

  /-- Create a `LeanValue.pointer` object. -/
  @[export LeanValue_mkPointer]
  private def mkPointer (ptr : @&Pointer) : LeanValue := .pointer ptr

end LeanValue


namespace Pointer
  /-- Create a pointer from an integer value. -/
  @[extern "Pointer_mk"]
  opaque mk (p : @&USize) : Pointer

  /-- NULL pointer. -/
  def null : Pointer := Pointer.mk 0

  /-- Get the address as an integer. -/
  @[extern "Pointer_address"]
  opaque address (p : @&Pointer) : USize

  /--
    Dereference the pointer and cast it to the given `CType`.
    This then creates a `LeanValue` value.
  -/
  @[extern "Pointer_read"]
  opaque read (p : @&Pointer) (type : @&CType) : IO LeanValue

  /--
    Assign a value to the address the pointer points to.
    The given `LeanValue` value is cast to the given `CType` first and
    then copied to the memory location.
  -/
  @[extern "Pointer_write"]
  opaque write (p : @&Pointer) (type : @&CType) (value : @&LeanValue) : IO Unit

end Pointer

instance : Inhabited Pointer := ⟨Pointer.mk 0⟩
instance : Repr Pointer := ⟨fun p _ => s!"CTypes.Pointer<{p.address}>"⟩
instance : BEq Pointer := ⟨fun a b => a.address == b.address⟩

@[default_instance high]
instance : HAdd Pointer USize Pointer := ⟨fun p n => .mk (p.address + n)⟩
@[default_instance high]
instance : HSub Pointer USize Pointer := ⟨fun p n => .mk (p.address - n)⟩

deriving instance Repr for LeanValue
deriving instance BEq for LeanValue


namespace LeanValue
  /- Getter functions for all value types. -/
  def int? : LeanValue → Option Int
  | .int a => some a
  | _      => none

  def nat? : LeanValue → Option Nat
  | .nat a => some a
  | _      => none

  def float? : LeanValue → Option Float
  | .float a => some a
  | _        => none

  def complex? : LeanValue → Option (Float × Float)
  | .complex a b => some (a, b)
  | _            => none

  def array? : LeanValue → Option (Array LeanValue)
  | .array a => some a
  | _        => none

  def struct? : LeanValue → Option (Array LeanValue)
  | .struct a => some a
  | _         => none

  def pointer? : LeanValue → Option Pointer
  | .pointer a => some a
  | _          => none

  def int!     := fun a => (int? a).get!
  def nat!     := fun a => (nat? a).get!
  def float!   := fun a => (float? a).get!
  def complex! := fun a => (complex? a).get!
  def array!   := fun a => (array? a).get!
  def struct!  := fun a => (struct? a).get!
  def pointer! := fun a => (pointer? a).get!

end LeanValue


end CTypes
