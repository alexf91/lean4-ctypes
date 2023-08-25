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

set_option relaxedAutoImplicit false

namespace CTypes.FFI

/-- `println` function exported to C. -/
@[export lean_println]
def println (msg : String) : IO Unit := do
  IO.println msg

/-- `print` function exported to C. -/
@[export lean_print]
def print (msg : String) : IO Unit := do
  IO.print msg

/-- `eprintln` function exported to C. -/
@[export lean_eprintln]
def eprintln (msg : String) : IO Unit := do
  IO.eprintln msg

/-- `eprint` function exported to C. -/
@[export lean_eprint]
def eprint (msg : String) : IO Unit := do
  IO.eprint msg

/-- Library handle returned by `dlopen()`. -/
opaque Library.Nonempty : NonemptyType
def Library : Type := Library.Nonempty.type
instance : Nonempty Library := Library.Nonempty.property

/- TODO: Don't define this here, it's an implementation detail. -/
def RTLD_LAZY     : UInt32 := 0x00001
def RTLD_NOW      : UInt32 := 0x00002
def RTLD_NOLOAD   : UInt32 := 0x00004
def RTLD_DEEPBIND : UInt32 := 0x00008
def RTLD_GLOBAL   : UInt32 := 0x00100
def RTLD_LOCAL    : UInt32 := 0x00000
def RTLD_NODELETE : UInt32 := 0x01000

namespace Library
  /-- Slim wrapper around `dlopen()`. -/
  @[extern "Library_mk"]
  opaque mk (p : String) (flags : UInt32) : IO Library
end Library

end CTypes.FFI
