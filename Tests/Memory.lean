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
    let ba ← nm.toByteArray
    assertFalse nm.allocated s!"allocated: {nm.allocated}"
    assertEqual ba.data na.data s!"data: {ba}"

  /-- Create a memory from a value and read it back. -/
  testcase testFromValue := do
    let m ← Memory.fromValue .int64 (.int 42)
    assertEqual m.size 8 s!"invalid size: {m.size}"
    assertEqual (← m.read 0 .int64) (.int 42)

  /-- Create a memory from a struct and read it back. -/
  testcase testFromStruct := do
    let tp := CType.struct #[.int8, .int8, .int8]
    let m ← Memory.fromValue tp (.struct #[.int 42, .int 43, .int 44])
    assertEqual (← m.read 0 tp) (.struct #[.int 42, .int 43, .int 44])

  /-- Create a memory from a struct with an array. -/
  testcase testFromStruct_array := do
    let tp := CType.struct #[.int8, .array .uint8 4]
    let m ← Memory.fromValue tp (.struct #[.int 42, .array #[.nat 0, .nat 1, .nat 2, .nat 3]])

  /-- Read all integer types. -/
  testcase testRead_int8 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int8
    assertEqual v (.int (-1))

  testcase testRead_uint8 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint8
    assertEqual v (.nat 255)

  testcase testRead_int16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int16
    assertEqual v (.int (-1))

  testcase testRead_uint16 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint16
    assertEqual v (.nat 65535)

  testcase testRead_int32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int32
    assertEqual v (.int (-1))

  testcase testRead_uint32 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint32
    assertEqual v (.nat 4294967295)

  testcase testRead_int64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .int64
    assertEqual v (.int (-1))

  testcase testRead_uint64 := do
    let m ← Memory.fromByteArray $ .mk (#[(0xFF : UInt8)] * 8)
    let v ← m.read 0 .uint64
    assertEqual v (.nat 18446744073709551615)

  /-- Read all floating point types. -/
  testcase testRead_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40]
    let v ← m.read 0 .float
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  testcase testRead_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40]
    let v ← m.read 0 .double
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  testcase testRead_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x65, 0xb5, 0xee, 0x7f, 0x00, 0x00]
    let v ← m.read 0 .longdouble
    assertEqual s!"{repr v}" s!"{repr (LeanValue.float 3.141593)}"

  /-- Read all complex types. -/
  testcase testRead_complex_float := do
    let m ← Memory.fromByteArray $ .mk #[0xdb, 0x0f, 0x49, 0x40, 0xdb, 0x0f, 0x49, 0xc0]
    let v ← m.read 0 .complex_float
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  testcase testRead_complex_double := do
    let m ← Memory.fromByteArray $ .mk #[0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0x40,
                                         0x18, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x09, 0xc0]
    let v ← m.read 0 .complex_double
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  testcase testRead_complex_longdouble := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0xc0, 0x68, 0x21, 0xa2, 0xda, 0x0f, 0xc9,
                                         0x00, 0xc0, 0x80, 0xf9, 0x9b, 0x7f, 0x00, 0x00]
    let v ← m.read 0 .complex_longdouble
    assertEqual s!"{repr v}" s!"{repr (LeanValue.complex 3.141593 (-3.141593))}"

  testcase testRead_struct := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                         0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f]
    let tp := CType.struct #[.uint8, .uint16, .uint32, .uint64]
    -- Prerequisites for the type
    assertEqual tp.offsets #[0, 2, 4, 8] "wrong assumption about alignment"
    assertEqual tp.size 16 "wrong assumption about size"
    let v ← m.read 0 tp
    -- Assume little endian order
    assertEqual v (.struct #[.nat 0, .nat 770, .nat 117835012, .nat 1084818905618843912])

  /-- Read a struct with an array. -/
  testcase testRead_structArray := do
    let m ← Memory.fromByteArray $ .mk #[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07]
    let tp := CType.struct #[.uint32, .array .uint8 4]
    let v ← m.read 0 tp
    assertEqual v (.struct #[.nat 50462976, .array #[.nat 4, .nat 5, .nat 6, .nat 7]])

  /-- Dereference a pointer. -/
  testcase testDereferenceEmpty := do
    let m ← Memory.fromByteArray $ .mk (#[(0x00 : UInt8)] * 8)
    let nm ← m.dereference 0 0
    let na ← nm.toByteArray
    assertEqual na.data #[]

end Tests.Memory
