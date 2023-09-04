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

end Symbol


namespace Function

  /-- Create a new function. -/
  testcase mkPrimitive requires (s : SymSin) := do
    discard <| Function.mk s .double #[.double]

  /-- Call a function. -/
  testcase callSuccess requires (sin : FuncSin) := do
    let result ← sin.call #[.float (3.14159265359 / 2)]
    assertEqual result (.float 1.0) s!"wrong result: {repr result}"

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

  /-- Read all integer types. -/
  testcase testReadInt_int8 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual (-1) (← m.readInt 0 .int8)

  testcase testReadInt_uint8 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual 255 (← m.readInt 0 .uint8)

  testcase testReadInt_int16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual (-1) (← m.readInt 0 .int16)

  testcase testReadInt_uint16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual 65535 (← m.readInt 0 .uint16)

  testcase testReadInt_int32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual (-1) (← m.readInt 0 .int32)

  testcase testReadInt_uint32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual 4294967295 (← m.readInt 0 .uint32)

  testcase testReadInt_int64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual (-1) (← m.readInt 0 .int64)

  testcase testReadInt_uint64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    assertEqual 18446744073709551615 (← m.readInt 0 .uint64)

  /-- Read all floating point types. -/
  testcase testReadFloat_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40]
    let v ← m.readFloat 0 .float
    assertEqual v.toString "3.141593"

  testcase testReadFloat_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40]
    let v ← m.readFloat 0 .double
    assertEqual v.toString "3.141593"

  testcase testReadFloat_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x65, 0xb5, 0xee, 0x7f, 0x00, 0x00]
    let v ← m.readFloat 0 .longdouble
    assertEqual v.toString "3.141593"

  /-- Read all complex types. -/
  testcase testReadComplex_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40, 0xdb, 0x0f, 0x49, 0xc0]
    let v ← m.readComplex 0 .complex_float
    assertEqual v.1.toString "3.141593"
    assertEqual v.2.toString "-3.141593"

  testcase testReadComplex_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40,
                                         0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0xc0]
    let v ← m.readComplex 0 .complex_double
    assertEqual v.1.toString "3.141593"
    assertEqual v.2.toString "-3.141593"

  testcase testReadComplex_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0xc0, 0x80, 0xf9, 0x9b, 0x7f, 0x00, 0x00]
    let v ← m.readComplex 0 .complex_longdouble
    assertEqual v.1.toString "3.141593"
    assertEqual v.2.toString "-3.141593"

  /-- Dereference a pointer. -/
  testcase testDereferenceEmpty := do
    let m ← Memory.fromByteArray $ .mk (#[(0x00 : UInt8)] * 8)
    let nm ← m.dereference 0 0
    let na ← nm.toByteArray
    assertEqual na.data #[]

end Memory

namespace CType
  testcase testInt8 := do
    let type := CType.int8
    assertEqual type.size 1 "wrong size"
    assertEqual type.offsets #[]

  testcase testInt64 := do
    let type := CType.int64
    assertEqual type.size 8 "wrong size"
    assertEqual type.offsets #[]

  testcase testDouble := do
    let type := CType.double
    assertEqual type.size 8 "wrong size"
    assertEqual type.offsets #[]

  testcase testComplexDouble := do
    let type := CType.complex_double
    assertEqual type.size 16 "wrong size"
    assertEqual type.offsets #[] s!"offsets: {type.offsets}"

  testcase testStruct := do
    let type := CType.struct #[.uint8, .uint16, .uint32, .uint64, .float, .double]
    assertEqual type.size 32 "wrong size"
    assertEqual type.offsets #[0, 2, 4, 8, 16, 24] s!"offsets: {type.offsets}"

  testcase testNestedStruct := do
    let sa := CType.struct #[.uint32, .uint32]
    let sb := CType.struct #[.uint64, .uint64]
    let type := CType.struct #[sa, sb, sa, sb]
    assertEqual type.size 48 "wrong size"
    assertEqual type.offsets #[0, 8, 24, 32] s!"offsets: {type.offsets}"

  testcase testArrayEmpty := do
    let type := CType.array .uint32 0
    assertEqual type.size 0 "wrong size"
    assertEqual type.offsets #[] s!"offsets: {type.offsets}"

  testcase testArraySingle := do
    let type := CType.array .uint32 1
    assertEqual type.size 4 "wrong size"
    assertEqual type.offsets #[0] s!"offsets: {type.offsets}"

  testcase testArrayRegular := do
    let type := CType.array .uint32 8
    assertEqual type.size 32 "wrong size"
    assertEqual type.offsets #[0, 4, 8, 12, 16, 20, 24, 28] s!"offsets: {type.offsets}"

  testcase testArrayStruct := do
    let s := CType.struct #[.uint32, .uint32]
    let type := CType.array s 8
    assertEqual type.size 64 "wrong size"
    assertEqual type.offsets #[0, 8, 16, 24, 32, 40, 48, 56] s!"offsets: {type.offsets}"

end CType

end Tests.FFI

#LTestMain
