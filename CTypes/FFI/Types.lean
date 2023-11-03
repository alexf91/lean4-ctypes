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

namespace CTypes.FFI

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
  | pointer (type : CType)
  | array (type : CType) (size : Nat)
  | struct (elements : Array CType)
  | union (elements : Array CType)
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


/-- Access to raw memory in C. -/
opaque Memory.Nonempty : NonemptyType
def Memory : Type := Memory.Nonempty.type
instance : Nonempty Memory := Memory.Nonempty.property

-- TODO: Show address in type representation.
instance : Repr Memory := ⟨fun _ _ => "Memory<TODO>"⟩

-- TODO: Compare if possible.
instance : BEq Memory := ⟨fun _ _ => false⟩


/-- Values in Lean. -/
inductive LeanValue where
  | unit
  | int (a : Int)
  | nat (a : Nat)
  | float (a : Float)
  | complex (a b : Float)
  | array (values : Array LeanValue)
  | struct (values : Array LeanValue)
  | pointer (memory : Memory)
deriving Repr, BEq


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
  private def mkPointer (memory : @&Memory) : LeanValue := .pointer memory

end LeanValue


end CTypes.FFI
