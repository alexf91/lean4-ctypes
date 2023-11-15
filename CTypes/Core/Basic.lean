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

import CTypes.Core.Types

set_option relaxedAutoImplicit false

namespace CTypes.Core

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

  /--
    Get a pointer to a symbol in the library.
    Raises an `IO.Error` exception if the lookup fails.
  -/
  @[extern "Library_symbol"]
  opaque symbol (library : @&Library) (name : @&String) : IO Pointer
end Library

instance : Repr     Library := ⟨fun lib _ => s!"CTypes.Library<{lib.path}>"⟩
instance : ToString Library := ⟨fun lib   => s!"{repr lib}"⟩

/-- Get a symbol from the library. -/
instance : GetElem Library String (IO Pointer) (fun _ _ => True) := ⟨fun l s _ => l.symbol s⟩


/-- A symbol with a return type and argument types. -/
opaque Function.Nonempty : NonemptyType
def Function : Type := Function.Nonempty.type
instance : Nonempty Function := Function.Nonempty.property

namespace Function
  /-- Create a new function instance from a symbol. -/
  @[extern "Function_mk"]
  opaque mk (s : @&Pointer) (returnType : @&CType) (argTypes : @&Array CType) : IO Function

  /-- Call a function with the given arguments. -/
  @[extern "Function_call"]
  opaque call (function : @&Function) (args : @&Array LeanValue) : IO LeanValue
end Function


end CTypes.Core
