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
  testcase mkSuccess requires (libgen : SharedLibrary) := do
    let lib ← libgen "int foo(void) { return 0; }"
    discard <| Symbol.mk lib "foo"

  /-- Fail to get a symbol. -/
  testcase mkFailure requires (libgen : SharedLibrary) := do
    let lib ← libgen "int foo(void) { return 0; }"
    try
      discard <| Symbol.mk lib "doesnotexist"
      assertTrue false "Symbol.mk did not fail"
    catch e =>
      let msg := "undefined symbol: doesnotexist"
      assertTrue (e.toString.endsWith msg) s!"invalid error message: {e}"

end Symbol


namespace Function

  /-- Create a new function. -/
  testcase mkPrimitive requires (libgen : SharedLibrary) := do
    let lib ← libgen "uint32_t foo(uint32_t a, uint32_t b) { return 0; }"
    let s ← lib["foo"]
    discard <| Function.mk s .uint32 #[.uint32, .uint32]

  /-- Call a function with integer arguments and return value. -/
  testcase callInt requires (libgen : SharedLibrary) := do
    let lib ← libgen "int8_t add(int8_t a, int8_t b) {return a + b;}"
    let add ← Function.mk (← lib["add"]) .int8 #[.int8, .int8]
    let r ← add.call #[.int 41, .int 1]
    assertEqual r (.int 42) s!"result: {repr r}"

  /-- Call a function with float arguments and return value. -/
  testcase callFloat requires (libgen : SharedLibrary) := do
    let lib ← libgen "double add(double a, double b) {return a + b;}"
    let add ← Function.mk (← lib["add"]) .double #[.double, .double]
    let r ← add.call #[.float 41.0, .float 1.0]
    assertEqual r (.float 42.0) s!"result: {repr r}"

  testcase callComplex requires (libgen : SharedLibrary) := do
    let lib ← libgen "complex double add(complex double a, complex double b) {return a + b;}"
    let add ← Function.mk (← lib["add"]) .complex_double #[.complex_double, .complex_double]
    let r ← add.call #[.complex 41.0 1.0, .complex 1.0 41.0]
    assertEqual r (.complex 42.0 42.0) s!"result: {repr r}"

end Function

end Tests.Basic
