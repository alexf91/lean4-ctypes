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

/-- Basic types in C. -/
inductive BasicType where
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

namespace BasicType
  /-- Get the size of a basic type. -/
  @[extern "BasicType_sizeof_Nat"]
  opaque sizeof (type : @&BasicType) : Nat
end BasicType

end CTypes.FFI
