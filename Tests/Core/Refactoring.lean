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

namespace Tests.Refactoring

  testcase testInt8 := do
    let type := CType.int8
    assertEqual type.size 1 s!"wrong size: {type.size}"
    assertEqual type.offsets #[]

  testcase testInt64 := do
    let type := CType.int64
    assertEqual type.size 8 s!"wrong size: {type.size}"
    assertEqual type.offsets #[]

  testcase testDouble := do
    let type := CType.double
    assertEqual type.size 8 s!"wrong size: {type.size}"
    assertEqual type.offsets #[]

  testcase testComplexDouble := do
    let type := CType.complex_double
    assertEqual type.size 16 s!"wrong size: {type.size}"
    assertEqual type.offsets #[] s!"offsets: {type.offsets}"

  testcase testStruct := do
    let type := CType.struct #[.uint8, .uint16, .uint32, .uint64, .float, .double]
    assertEqual type.size 32 s!"wrong size: {type.size}"
    assertEqual type.offsets #[0, 2, 4, 8, 16, 24] s!"offsets: {type.offsets}"

  testcase testNestedStruct := do
    let sa := CType.struct #[.uint32, .uint32]
    let sb := CType.struct #[.uint64, .uint64]
    let type := CType.struct #[sa, sb, sa, sb]
    assertEqual type.size 48 s!"wrong size: {type.size}"
    assertEqual type.offsets #[0, 8, 24, 32] s!"offsets: {type.offsets}"

  testcase testPointerMk := do
    let p := Pointer.mk 0
    assertEqual p.address 0 s!"wrong address: {p.address}"

  testcase testPointerMkMax := do
    let p := Pointer.mk 0xFFFFFFFFFFFFFFFF
    assertEqual s!"{p.address}" "18446744073709551615" s!"wrong address: {p.address}"

  testcase testPointerAdd := do
    let p := (Pointer.mk 128) + (32 : USize)
    assertEqual p.address 160 s!"wrong address: {p.address}"

  testcase testPointerSub := do
    let p := (Pointer.mk 128) - (32 : USize)
    assertEqual p.address 96 s!"wrong address: {p.address}"

  testcase testPointerReadInt requires (libgen : SharedLibrary) := do
    let lib ← libgen "int64_t pos = 42; int64_t neg = -42;"
    let testcases : List (CType × CValue × CValue) := [
      (.int8,   .int8   42, .int8   $       -42),
      (.int16,  .int16  42, .int16  $       -42),
      (.int32,  .int32  42, .int32  $       -42),
      (.int64,  .int64  42, .int64  $       -42),
      (.uint8,  .uint8  42, .uint8  $ 2^8  - 42),
      (.uint16, .uint16 42, .uint16 $ 2^16 - 42),
      (.uint32, .uint32 42, .uint32 $ 2^32 - 42),
      (.uint64, .uint64 42, .uint64 $ 2^64 - 42)
    ]

    for (ct, pv, nv) in testcases do
      let pos ← (← lib["pos"]).read ct
      let neg ← (← lib["neg"]).read ct
      assertEqual pos pv s!"wrong result for {repr ct}: pos = {repr pos}"
      assertEqual neg nv s!"wrong result for {repr ct}: neg = {repr neg}"

  testcase testPointerReadFloat requires (libgen : SharedLibrary) := do
    let lib ← libgen "float vf = 3.1415; double vd = 2.7182; long double vl = 1.4142;"
    let vf ← (← lib["vf"]).read .float
    let vd ← (← lib["vd"]).read .double
    let vl ← (← lib["vl"]).read .longdouble

    assertEqual vf.type .float
    assertTrue $ (vf.float! - 3.1415).abs < 0.1
    assertEqual vd.type .double
    assertTrue $ (vd.float! - 2.7182).abs < 0.1
    assertEqual vl.type .longdouble
    assertTrue $ (vl.float! - 1.4142).abs < 0.1

  testcase testPointerReadComplex requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "const complex float value = 3.1415 - 1.4142 * I;" ++
                       "complex float vf = value;" ++
                       "complex double vd = value;" ++
                       "complex long double vl = value;"
    let testcases : List (String × CType) := [
      ("vf", .complex_float),
      ("vd", .complex_double),
      ("vl", .complex_longdouble)
    ]
    for (n, ct) in testcases do
      let v ← (← lib[n]).read ct
      assertEqual v.type ct
      let (vr, vi) := v.complex!
      assertTrue $ (vr - 3.1415).abs < 0.1
      assertTrue $ (vi + 1.4142).abs < 0.1

  /-- Write integer values and read back the result. -/
  testcase testPointerWriteInt requires (libgen : SharedLibrary) := do
    let lib ← libgen "uint8_t value[32] = {0};"
    let p ← lib["value"]
    let testcases : List (CType × List CValue) := [
      (.int8,   [.int8   42, .int8   $       -42]),
      (.int16,  [.int16  42, .int16  $       -42]),
      (.int32,  [.int32  42, .int32  $       -42]),
      (.int64,  [.int64  42, .int64  $       -42]),
      (.uint8,  [.uint8  42, .uint8  $ 2^8  - 42]),
      (.uint16, [.uint16 42, .uint16 $ 2^16 - 42]),
      (.uint32, [.uint32 42, .uint32 $ 2^32 - 42]),
      (.uint64, [.uint64 42, .uint64 $ 2^64 - 42])
    ]

    -- Write values to memory and read back.
    for (ct, values) in testcases do
      for (i, v) in values.enum do
        p.write v
        let rb ← p.read ct
        assertEqual rb v s!"wrong result for {repr ct} (#{i}): {repr v} != {repr rb}"

  /-- Get a pointer and dereference it. --/
  testcase testPointerReadPointer requires (libgen : SharedLibrary) := do
    let lib ← libgen "uint64_t v = 42; uint64_t *p = &v;"
    let pp ← lib["p"]
    let pv ← pp.read .pointer
    let v ← pv.pointer!.read .uint64
    assertEqual v (.uint64 42)

  /-- Change the value of a pointer. -/
  testcase testPointerWritePointer requires (libgen : SharedLibrary) := do
    let lib ← libgen "uint64_t a = 41; uint64_t b = 42; uint64_t *p = &a;"
    let pa ← lib["a"]
    let pb ← lib["b"]
    let pp ← lib["p"]
    assertEqual (← pp.read .pointer).pointer! pa
    pp.write (.pointer pb)
    assertEqual (← pp.read .pointer).pointer! pb
    assertEqual (← (← pp.read .pointer).pointer!.read .uint64) (.uint64 42)

end Tests.Refactoring
