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

import CTypes
open CTypes

def main (_ : List String) : IO UInt32 := do
  let lib ← Library.mk "libc.so.6" #[.RTLD_NOW]
  let malloc ← Function.mk (← lib["malloc"]) .pointer #[.size_t]
  let free   ← Function.mk (← lib["free"])   .void    #[.pointer]
  let memset ← Function.mk (← lib["memset"]) .pointer #[.pointer, .int, .size_t]

  -- Create a struct with an integer and a fixed-length array.
  let type := CType.struct #[.int, .array .int 8]
  let p ← malloc.call #[.nat type.size]

  -- Set all values to 0xFF, resulting in -1 for all fields.
  discard <| memset.call #[p, .int 0xFF, .nat type.size]

  -- Read back the value.
  let value ← p.pointer!.read type
  IO.println s!"value: {repr value}"

  -- Free the allocated buffer.
  discard <| free.call #[p]

  return 0
