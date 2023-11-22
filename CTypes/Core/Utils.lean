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
  Allocate a buffer of the given size and return a pointer.

  The buffer is initialized to zero.

  Buffers created this way should only be freed with the builtin `free`, not one
  loaded from a library. This avoids inconsistencies between different `malloc`
  implementations in libraries.
-/
@[extern "Utils_malloc"]
opaque malloc (size : @&Nat) : IO Pointer

/-- Free a buffer allocated with `malloc`. -/
@[extern "Utils_free"]
opaque free (pointer : @&Pointer) : IO Unit

end CTypes.Core
