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

namespace Pointer
  /-- Create a Pointer from a byte array. -/
  @[extern "Pointer_fromByteArray"]
  opaque fromByteArray (buffer : @&ByteArray) : IO Pointer

  /-- Convert a Pointer back to a byte array. -/
  @[extern "Pointer_toByteArray"]
  opaque toByteArray (m : @&Pointer) : IO ByteArray

  /-- Create a Pointer from a type and a value. -/
  @[extern "Pointer_fromValue"]
  opaque fromValue (type : @&CType) (value : @&LeanValue) : IO Pointer

  /-- Allocate a new memory and initialize it to 0. -/
  def allocate (size : Nat) : IO Pointer := fromByteArray $ size.repeat (· ++ .mk #[(0 : UInt8)]) .empty

  /-- Get the size of the memory view. -/
  @[extern "Pointer_size"]
  opaque size (m : @&Pointer) : Nat

  /-- Check if the memory is allocated. -/
  @[extern "Pointer_allocated"]
  opaque allocated (m : @&Pointer) : Bool

  /-- Extract a slice from the memory view and return a new slice. -/
  @[extern "Pointer_extract"]
  opaque extract (m : @&Pointer) (b e : @&Nat) : IO Pointer

  /-- Dereference a pointer and create a new memory view with the given size. -/
  @[extern "Pointer_dereference"]
  opaque dereference (m : @&Pointer) (offset : @&Nat) (size : @&Nat) : IO Pointer

  /-- Read an arbitrary value from the memory view. -/
  @[extern "Pointer_read"]
  opaque read (m : @&Pointer) (offset : @&Nat) (type : @&CType) : IO LeanValue

end Pointer

end CTypes.FFI
