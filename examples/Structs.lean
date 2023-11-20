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
  -- Fibonacci iteration with a struct as state variable.
  let fib : Callback := fun args => do
    let state ← args[0]!.pointer!.read $ .struct #[.int, .int]

    -- Get the two values from the (int, int) struct.
    let a := state.struct![0]!.int!
    let b := state.struct![1]!.int!

    -- Update the values and return the result.
    args[0]!.pointer!.write (.struct #[.int b, .int (a + b)])
    return .int a

  -- The closure object for the C function.
  let closure ← Closure.mk .int #[.pointer] fib

  -- Initialize the state struct to (0, 1).
  let libc ← Library.mk "libc.so.6" .RTLD_NOW #[]
  let state ← (← libc["malloc"]).call .pointer #[.size_t (CType.struct #[.int, .int]).size] #[]
  state.pointer!.write $ .struct #[.int 0, .int 1]

  for _ in [0:8] do
    let result ← closure.pointer.call .int #[state] #[]
    IO.println $ repr result

  -- Cleanup
  closure.delete
  discard <| (← libc["free"]).call .void #[state] #[]

  return 0
