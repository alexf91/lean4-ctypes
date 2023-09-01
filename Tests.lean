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
open LTest
open CTypes.FFI

namespace Tests.FFI

/-- Get the call trace from logging messages. -/
def callTrace (streams : Streams) : List String :=
  let split := fun (s : String) => s.splitOn ":" |>.get! 1 |>.splitOn " " |>.get! 0
  String.fromUTF8Unchecked streams.stderr |>.splitOn "\n" |>.dropLast |>.map split

/-- Fixture for `libm`. -/
fixture LibMath Unit Library where
  setup := Library.mk "/usr/lib/libm.so.6" #[.RTLD_NOW]

/-- Fixture for the symbol `sin()`. -/
fixture SymSin Unit Symbol requires (m : LibMath) where
  setup := Symbol.mk m "sin"

/-- Fixture for the function `sin()`. -/
fixture FuncSin Unit Function requires (s : SymSin) where
  setup := Function.mk s .double #[.double]


namespace Library

  /-- Successfully open a library. -/
  testcase mkSuccess := do
    discard <| Library.mk "/usr/lib/libgmp.so" #[.RTLD_NOW]

  /-- Fail to open a library. -/
  testcase mkFailure := do
    try
      discard <| Library.mk "/does/not/exist.so" #[.RTLD_NOW]
      assertTrue false "Library.mk did not fail"
    catch e =>
      let msg := "/does/not/exist.so: cannot open shared object file: No such file or directory"
      assertTrue (e.toString == msg) s!"invalid error message: {e}"

end Library

namespace Symbol

  /-- Successfully get a symbol. -/
  testcase mkSuccess requires (h : LibMath) := do
    discard <| Symbol.mk h "sin"

  /-- Fail to get a symbol. -/
  testcase mkFailure requires (h : LibMath) := do
    try
      discard <| Symbol.mk h "doesnotexist"
      assertTrue false "Symbol.mk did not fail"
    catch e =>
      let msg := "/usr/lib/libm.so.6: undefined symbol: doesnotexist"
      assertTrue (e.toString == msg) s!"invalid error message: {e}"

  /-
    These tests require compilation with debug output to check if everything
    is finalized as expected.
  -/
  namespace DebugOutput
    testcase mkSuccess := do
      /- Only run in debug mode. -/
      if !debugMode () then
        return

      let (r, streams) ← captureResult do
        let h ← Library.mk "/usr/lib/libm.so.6" #[.RTLD_NOW]
        discard <| Symbol.mk h "sin"
      assertTrue r.isOk

      let expected := ["Library_mk", "Symbol_mk", "Symbol_finalize", "Library_finalize"]
      let trace := callTrace streams
      assertEqual trace expected $ toString trace

    testcase mkFailure := do
      /- Only run in debug mode. -/
      if !debugMode () then
        return

      let (r, streams) ← captureResult do
        let h ← Library.mk "/usr/lib/libm.so.6" #[.RTLD_NOW]
        discard <| Symbol.mk h "doesnotexist"
      assertTrue !r.isOk

      let expected := ["Library_mk", "Symbol_mk", "Library_finalize"]
      let trace := callTrace streams
      assertEqual trace expected $ toString trace

  end DebugOutput

end Symbol


namespace Function

  /-- Create a new function. -/
  testcase mkPrimitive requires (s : SymSin) := do
    discard <| Function.mk s .double #[.double]

  /-- Call a function. -/
  testcase callSuccess requires (sin : FuncSin) := do
    let result ← sin.call #[.float (3.14159265359 / 2)]
    assertEqual result (.float 1.0) s!"wrong result: {repr result}"

  /-
    These tests require compilation with debug output to check if everything
    is finalized as expected.
  -/
  namespace DebugOutput
    testcase mkSuccess := do
      /- Only run in debug mode. -/
      if !debugMode () then
        return

      let (r, streams) ← captureResult do
        let h ← Library.mk "/usr/lib/libm.so.6" #[.RTLD_NOW]
        let s ← Symbol.mk h "ldexp"
        discard <| Function.mk s .double #[.double, .sint]
      assertTrue r.isOk

      let expected := [
        "Library_mk", "Symbol_mk", "Function_mk",
        "CType_unbox", "CType_unbox", "CType_unbox",
        "Function_finalize", "Symbol_finalize", "Library_finalize"
      ]
      let trace := callTrace streams
      assertEqual trace expected $ toString trace

  end DebugOutput

end Function

namespace Memory
  testcase testConversion := do
    let a := ByteArray.mk #[0, 1, 2, 3, 4, 5, 6, 7]
    let m ← Memory.fromByteArray a
    let b ← m.toByteArray
    assertEqual a.data b.data s!"{b.data}"

  testcase testAllocate := do
    let m ← Memory.allocate 32
    let a ← m.toByteArray
    assertEqual a.data (#[(0 : UInt8)] * 32)

  testcase testExtract := do
    let a := ByteArray.mk #[0, 1, 2, 3, 4, 5, 6, 7]
    let m ← Memory.fromByteArray a
    let na := a.extract 1 7
    let nm ← m.extract 1 7
    assertFalse nm.allocated
    assertEqual (← nm.toByteArray).data na.data

end Memory

end Tests.FFI

#LTestMain
