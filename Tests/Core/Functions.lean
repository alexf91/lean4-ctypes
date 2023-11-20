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

import LTest
import CTypes
import Tests.Core.Fixtures
open LTest
open CTypes.Core

namespace Tests.Functions

  testcase testCallRegular requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "uint32_t foo(void) { return 42; }"
    let foo ← lib["foo"]
    let value ← foo.call .uint32 #[] #[]
    assertEqual value (.uint32 42) s!"wrong result: {repr value}"

  testcase testCallVariadic requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "int sum(int a, ...) {" ++
                       "    va_list ap; int sum = a; int n;" ++
                       "    va_start(ap, a);" ++
                       "    while ((n = va_arg(ap, int)) != 0) sum += n;" ++
                       "    va_end(ap);" ++
                       "    return sum;}"
    let sum ← lib["sum"]
    let value ← sum.call .int #[.int 8] #[.int 16, .int 32, .int 0]
    assertEqual value (.int 56)

  /-- Use a wrong variadic argument. -/
  testcase testCallVariadicError requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "int foo(int a, ...) {return 0;}"
    let foo ← lib["foo"]
    try
      discard <| foo.call .int #[.int 8] #[.int 16, .short 32, .int 0]
    catch e =>
      assertEqual e.toString "ffi_prep_cif_var() failed"
      return
    assertTrue false "foo.call did not fail"

  /-- Create a closure and call it as a pointer. -/
  testcase testCallClosure := do
    let callback : Callback := fun args => do
      let value := args.foldl (fun a e => a + e.int!) 0
      return .int value

    let closure ← Closure.mk .int #[.int, .int] callback
    try
      let args := #[.int 42, .int 11]
      let result ← closure.pointer.call .int args #[]
      assertEqual result (.int 53) s!"wrong return value: {repr result}"
    finally
      closure.delete

  -- TODO: Fix whatever causes this error.
  -- /--
  --   This has caused a segmentation fault before.
  --   The segmentation fault is not caused in this testcase, but in one that comes after
  --   this one.
  -- -/
  -- testcase testCallClosureSegfault := do
  --   let callArgs : IO.Ref (Array CValue) ← IO.mkRef #[]
  --   let callback : Callback := fun args => do
  --     callArgs.set args
  --     return .void

  --   let closure ← Closure.mk .void #[.int, .int] callback
  --   try
  --     let args : Array CValue := #[.int 42, .int 11]
  --     discard <| closure.pointer.call .void args #[]
  --   finally
  --     closure.delete

end Tests.Functions
