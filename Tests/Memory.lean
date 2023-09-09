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

namespace Tests.Memory
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
    let v ← m.read 0 .int8
    assertEqual v (.int (-1))

  testcase testReadInt_uint8 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint8
    assertEqual v (.int 255)

  testcase testReadInt_int16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int16
    assertEqual v (.int (-1))

  testcase testReadInt_uint16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint16
    assertEqual v (.int 65535)

  testcase testReadInt_int32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int32
    assertEqual v (.int (-1))

  testcase testReadInt_uint32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint32
    assertEqual v (.int 4294967295)

  testcase testReadInt_int64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int64
    assertEqual v (.int (-1))

  testcase testReadInt_uint64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint64
    assertEqual v (.int 18446744073709551615)

  /-- Read all floating point types. -/
  testcase testReadFloat_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40]
    let v ← m.read 0 .float
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  testcase testReadFloat_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40]
    let v ← m.read 0 .double
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  testcase testReadFloat_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x65, 0xb5, 0xee, 0x7f, 0x00, 0x00]
    let v ← m.read 0 .longdouble
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  /-- Read all complex types. -/
  testcase testReadComplex_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40, 0xdb, 0x0f, 0x49, 0xc0]
    let v ← m.read 0 .complex_float
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  testcase testReadComplex_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40,
                                         0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0xc0]
    let v ← m.read 0 .complex_double
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  testcase testReadComplex_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0xc0, 0x80, 0xf9, 0x9b, 0x7f, 0x00, 0x00]
    let v ← m.read 0 .complex_longdouble
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  /-- Dereference a pointer. -/
  testcase testDereferenceEmpty := do
    let m ← Memory.fromByteArray $ .mk (#[(0x00 : UInt8)] * 8)
    let nm ← m.dereference 0 0
    let na ← nm.toByteArray
    assertEqual na.data #[]

end Tests.Memory
