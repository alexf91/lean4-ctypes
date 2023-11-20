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
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  -- Callback functions have the signature (Array CValue) → IO CValue
  let add : Callback := fun args => do
    return .int (args[0]!.int! + args[1]!.int!)

  -- Create the closure.
  let closure ← Closure.mk .int #[.int, .int] add

  -- Call the function through the `Pointer.call` interface.
  let result ← closure.pointer.call .int #[.int 42, .int 11] #[]
  IO.println s!"42 + 11 = {result.int!}"

  -- Closures are not deleted by default, in case a C function stores a reference.
  -- They have to be marked for deletion by the user.
  -- Deleted closures are freed when the object is garbage collected. Otherwise they
  -- leak memory.
  closure.delete

  return 0
