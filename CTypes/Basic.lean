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

import CTypes.Types

set_option relaxedAutoImplicit false

namespace CTypes

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
  /-- Thin wrapper around `dlopen()`. -/
  @[extern "Library_mk"]
  opaque mk (path : @&String) (flags : @&Array Flag) : IO Library

  /-- Get the path the library for which the library was created. -/
  @[extern "Library_path"]
  opaque path (library : @&Library) : String
end Library

instance : Repr     Library := ⟨fun lib _ => s!"CTypes.Library<{lib.path}>"⟩
instance : ToString Library := ⟨fun lib   => s!"{repr lib}"⟩


/-- Symbol handle returned by `dlsym()`. -/
opaque Symbol.Nonempty : NonemptyType
def Symbol : Type := Symbol.Nonempty.type
instance : Nonempty Symbol := Symbol.Nonempty.property

namespace Symbol
  /-- Thin wrapper around `dlsym()`. -/
  @[extern "Symbol_mk"]
  opaque mk (library : @&Library) (symbol : @&String) : IO Symbol

  /-- Get the name of the symbol. -/
  @[extern "Symbol_name"]
  opaque name (symbol : @&Symbol) : String

  /-- Get the library of the symbol. -/
  @[extern "Symbol_library"]
  opaque library (symbol : @&Symbol) : Library

end Symbol

instance : Repr     Symbol := ⟨fun s _ => s!"CTypes.Symbol<{s.library.path}:{s.name}>"⟩
instance : ToString Symbol := ⟨fun s   => s!"{repr s}"⟩

/-- Get a symbol from the library. -/
def Library.get (lib : Library) (sym : String) : IO Symbol := Symbol.mk lib sym
instance : GetElem Library String (IO Symbol) (fun _ _ => True) := ⟨fun l s _ => l.get s⟩


/-- A symbol with a return type and argument types. -/
opaque Function.Nonempty : NonemptyType
def Function : Type := Function.Nonempty.type
instance : Nonempty Function := Function.Nonempty.property

namespace Function
  /-- Create a new function instance from a symbol. -/
  @[extern "Function_mk"]
  opaque mk (s : @&Symbol) (returnType : @&CType) (argTypes : @&Array CType) : IO Function

  /-- Call a function with the given arguments. -/
  @[extern "Function_call"]
  opaque call (function : @&Function) (args : @&Array LeanValue) : IO LeanValue
end Function


end CTypes
