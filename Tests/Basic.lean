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
import Tests.Fixtures
open LTest
open CTypes.FFI

namespace Tests.Basic

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

end Symbol


namespace Function

  /-- Create a new function. -/
  testcase mkPrimitive requires (s : SymSin) := do
    discard <| Function.mk s .double #[.double]

  /-- Call a function. -/
  testcase callSuccess requires (sin : FuncSin) := do
    let result ← sin.call #[.float (3.14159265359 / 2)]
    assertEqual result (.float 1.0) s!"wrong result: {repr result}"

  /-- Call a custom function. -/
  testcase callCustom requires (libgen : SharedLibrary) := do
    let lib ← libgen "int8_t foo(int8_t a, int8_t b) {return a + b;}"
    let foo ← Function.mk (← lib["foo"]) .int8 #[.int8, .int8]
    let r ← foo.call #[.int 41, .int 1]
    assertEqual r (.int 42) s!"result: {repr r}"

end Function

end Tests.Basic
