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

import CTypes.FFI.Types

set_option relaxedAutoImplicit false

namespace CTypes.FFI

/-- Access to raw memory in C. -/
opaque Memory.Nonempty : NonemptyType
def Memory : Type := Memory.Nonempty.type
instance : Nonempty Memory := Memory.Nonempty.property

namespace Memory
  /-- Create a Memory from a byte array. -/
  @[extern "Memory_fromByteArray"]
  opaque fromByteArray (buffer : @&ByteArray) : IO Memory

  /-- Convert a Memory back to a byte array. -/
  @[extern "Memory_toByteArray"]
  opaque toByteArray (m : @&Memory) : IO ByteArray

  /-- Allocate a new memory and initialize it to 0. -/
  @[extern "Memory_allocate"]
  opaque allocate (size : @&USize) : IO Memory

  /-- Get the size of the memory view. -/
  @[extern "Memory_size"]
  opaque size (m : @&Memory) : Nat

  /-- Check if the memory is allocated. -/
  @[extern "Memory_allocated"]
  opaque allocated (m : @&Memory) : Bool

  /-- Extract a slice from the memory view and return a new slice. -/
  @[extern "Memory_extract"]
  opaque extract (m : Memory) (b e : @&Nat) : IO Memory

  /-- Read an integer type from the memory view. -/
  @[extern "Memory_readInt"]
  opaque readInt (m : @&Memory) (offset : @&Nat) (type : @&CType) : IO Int

  /-- Read a floating point type from the memory view. -/
  @[extern "Memory_readFloat"]
  opaque readFloat (m : @&Memory) (offset : @&Nat) (type : @&CType) : IO Float

  /-- Internal implementation for reading complex numbers. -/
  @[extern "Memory_readComplex"]
  private opaque _readComplex (m : @&Memory) (offset : @&Nat) (type : @&CType) : IO FloatArray
  /-- Read a complex floating point type from the memory view. -/
  def readComplex (m : @&Memory) (offset : @&Nat) (type : @&CType) : IO (Float × Float) := do
    let a ← _readComplex m offset type
    return (a.get! 0, a.get! 1)

  /-- Dereference a pointer and create a new memory view with the given size. -/
  @[extern "Memory_dereference"]
  opaque dereference (m : @&Memory) (offset : @&Nat) (size : @&Nat) : IO Memory

end Memory

end CTypes.FFI
