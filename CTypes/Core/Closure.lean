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

/--
  Closure object to implement callbacks from C.
-/
opaque Closure.Nonempty : NonemptyType
def Closure : Type := Closure.Nonempty.type
instance : Nonempty Closure := Closure.Nonempty.property

def Callback := Array CValue → IO CValue

namespace Closure
  /--
    Create a Closure from a signature and a callback function.

    For lifetime management, this is an `IO` function. C functions might store a
    reference to the closure, so we only free it if it is marked for deletion by
    the user.

    Note that this leaks memory if the object is garbage collected before it is
    marked for deletion.
  -/
  @[extern "Closure_mk"]
  opaque mk (rtype : @&CType) (args : @&Array CType) (callback : Callback) : IO Closure

  /--
    Mark the closure for deletion.

    The closure is deleted when the object is garbage collected.
  -/
  @[extern "Closure_delete"]
  opaque delete (c : @&Closure) : IO Unit

  /-- Get a pointer to the C function. -/
  @[extern "Closure_pointer"]
  opaque pointer (c : @&Closure) : Pointer

end Closure

end CTypes.Core
