--
-- Copyright 2023 Alexander Fasching
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
--

import Lake
open Lake DSL

package Example {
  precompileModules := true

  -- TODO: Remove this once we can inherit these flags from library
  --       dependencies.
  moreLinkArgs := #[
    "-lffi",
    "-ldl"
  ]
}

require ctypes from ".."


@[default_target]
lean_exe Math
@[default_target]
lean_exe Pointers
@[default_target]
lean_exe Callbacks
@[default_target]
lean_exe Structs
