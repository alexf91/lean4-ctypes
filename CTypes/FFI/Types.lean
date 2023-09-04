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
  | uint8
  | int16
  | uint16
  | int32
  | uint32
  | int64
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

end CTypes.FFI
