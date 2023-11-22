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

/-- Library handle returned by `dlopen()`. -/
opaque Library.Nonempty : NonemptyType
def Library : Type := Library.Nonempty.type
instance : Nonempty Library := Library.Nonempty.property

namespace Library

  /--
    Flags for when the library should be resolved.
    See `man dlopen` for details.
  -/
  inductive ModeFlag where
    | RTLD_LAZY
    | RTLD_NOW

  /--
    Other options for `dlopen()`.
    See `man dlopen` for details.
  -/
  inductive OptionFlag where
    | RTLD_NOLOAD
    | RTLD_DEEPBIND
    | RTLD_GLOBAL
    | RTLD_LOCAL
    | RTLD_NODELETE

  /-- Thin wrapper around `dlopen()`. -/
  @[extern "Library_mk"]
  opaque mk (path : @&String) (mode : @&ModeFlag) (opts : @&Array OptionFlag) : IO Library

  /--
    Thin wrapper around `dlclose()`.

    This should be used carefully and is usually not necessary.
    Pointers might still depend on the library and get invalidated if it is unloaded.
    Note that this function can only be called once for each library and no new
    symbols can be created afterwards.
  -/
  @[extern "Library_close"]
  opaque close (library : @&Library) : IO Unit

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

/-- Get a symbol from the library. -/
instance : GetElem Library String (IO Pointer) (fun _ _ => True) := ⟨fun l s _ => l.symbol s⟩

end CTypes.Core
