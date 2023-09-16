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

  /-- Return void from a function. -/
  testcase callVoid requires (libgen : SharedLibrary) := do
    let lib ← libgen "void foo(void) {}"
    let foo ← Function.mk (← lib["foo"]) .void #[]
    assertEqual .unit (← foo.call #[])

  /-- Call a function with integer arguments and return value. -/
  testcase callInt requires (libgen : SharedLibrary) := do
    let lib ← libgen "int64_t add(int64_t a, int64_t b) {return a + b;}"
    let add ← Function.mk (← lib["add"]) .int64 #[.int64, .int64]
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

  /-- Call multiple functions in a library multiple times. -/
  testcase callMultiple requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "int64_t add(int64_t a, int64_t b) {return a + b;}\n" ++
                       "int64_t mul(int64_t a, int64_t b) {return a * b;}"
    let add ← Function.mk (← lib["add"]) .int64 #[.int64, .int64]
    let mul ← Function.mk (← lib["mul"]) .int64 #[.int64, .int64]
    assertEqual (.int 12) (← mul.call #[.int 2, .int 6])
    assertEqual (.int 10) (← add.call #[.int 3, .int 7])
    assertEqual (.int 12) (← add.call #[.int 4, .int 8])
    assertEqual (.int 45) (← mul.call #[.int 5, .int 9])

  /-- Call a function with overflowing arguments. -/
  testcase callOverflow_int8 requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "int8_t foo(int8_t a) {return a;}"
    let foo ← Function.mk (← lib["foo"]) .int8 #[.int8]
    assertEqual (.int (-128)) (← foo.call #[.int 128])
    assertEqual (.int (-128)) (← foo.call #[.int (-128)])
    assertEqual (.int 0) (← foo.call #[.int 256])

  /-- Call a function with a struct argument. -/
  testcase callStructArg requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "struct Foo { int8_t a; int16_t b; int32_t c; int64_t d; };\n" ++
                       "int64_t foo(struct Foo f) { return f.a + f.b + f.c + f.d; }"
    let foo ← Function.mk (← lib["foo"]) .int64 #[.struct #[.int8, .int16, .int32, .int64]]
    let v := LeanValue.struct #[.int 8, .int 16, .int 32, .int 64]
    assertEqual (.int 120) (← foo.call #[v])

  /-- Call a function with a struct return value. -/
  testcase callStructReturn requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "struct Foo { int8_t a; int16_t b; int32_t c; int64_t d; };\n" ++
                       "struct Foo foo(int8_t a, int16_t b, int32_t c, int64_t d) {\n" ++
                       "    struct Foo value = {a, b, c, d};\n" ++
                       "    return value;" ++
                       "}"
    let foo ← Function.mk (← lib["foo"]) (.struct #[.int8, .int16, .int32, .int64]) #[.int8, .int16, .int32, .int64]
    let values : Array LeanValue := #[.int 8, .int 16, .int 32, .int 64]
    let result ← foo.call values
    assertEqual (.struct values) result s!"value: {repr result}"

end Function

end Tests.Basic
