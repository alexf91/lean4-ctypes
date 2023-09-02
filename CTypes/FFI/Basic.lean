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

import CTypes.FFI.Types

set_option relaxedAutoImplicit false

namespace CTypes.FFI

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
end Library


/-- Symbol handle returned by `dlsym()`. -/
opaque Symbol.Nonempty : NonemptyType
def Symbol : Type := Symbol.Nonempty.type
instance : Nonempty Symbol := Symbol.Nonempty.property

namespace Symbol
  /-- Thin wrapper around `dlsym()`. -/
  @[extern "Symbol_mk"]
  opaque mk (library : @&Library) (symbol : @&String) : IO Symbol
end Symbol


/--
  A symbol with a return type and argument types.

  NOTE: Implementing this in C instead of a structure avoids constant unboxing
        of the argument and return types.
-/
opaque Function.Nonempty : NonemptyType
def Function : Type := Function.Nonempty.type
instance : Nonempty Function := Function.Nonempty.property

/-- Type for passing values to and from C functions. -/
inductive LeanType where
  | unit
  | int    (a : Int)
  | float  (a : Float)
deriving Repr, BEq, Inhabited

namespace LeanType
  /--
    Convert the buffer with the result of a function call to a `LeanType`.
    Doing most of it here instead of C makes the implementation easier.
    This works only for types without pointers.
  -/
  @[export LeanType_box]
  private def box (ct : CType) (result : UInt64) : LeanType := match ct with
  | .void => .unit
  | _     => panic s!"support for {repr ct} not implemented"

end LeanType


namespace Function
  /-- Create a new function instance from a symbol. -/
  @[extern "Function_mk"]
  opaque mk (s : @&Symbol) (returnType : @&CType) (argTypes : @&Array CType) : IO Function

  /-- Call a function with the given arguments. -/
  @[extern "Function_call"]
  opaque call (function : @&Function) (args : @&Array LeanType) : IO LeanType
end Function

end CTypes.FFI
