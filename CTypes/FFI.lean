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

/-- Flags for opening a shared library. -/
inductive Flag where
  | RTLD_LAZY
  | RTLD_NOW
  | RTLD_NOLOAD
  | RTLD_DEEPBIND
  | RTLD_GLOBAL
  | RTLD_LOCAL
  | RTLD_NODELETE

/-- Library handle returned by `dlopen()`. -/
opaque Library.Nonempty : NonemptyType
def Library : Type := Library.Nonempty.type
instance : Nonempty Library := Library.Nonempty.property

namespace Library
  /-- Slim wrapper around `dlopen()`. -/
  @[extern "Library_mk"]
  opaque mk (path : String) (flags : Array Flag) : IO Library
end Library


/-- Symbol handle returned by `dlsym()`. -/
opaque Symbol.Nonempty : NonemptyType
def Symbol : Type := Symbol.Nonempty.type
instance : Nonempty Symbol := Symbol.Nonempty.property

namespace Symbol
  /-- Slim wrapper around `dlsym()`. -/
  @[extern "Symbol_mk"]
  opaque mk (library : Library) (symbol : String) : IO Symbol
end Symbol

end CTypes.FFI
