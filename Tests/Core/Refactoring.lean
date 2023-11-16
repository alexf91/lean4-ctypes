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

  testcase testReadVoid := do
    let value ← Pointer.null.read .void
    assertEqual value .void

end Tests.Refactoring
