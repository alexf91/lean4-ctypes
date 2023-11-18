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

import CTypes
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  -- Open the library. See man page for dlopen() for flags.
  let lib ← Library.mk "libm.so.6" .RTLD_NOW #[]

  -- Lookup the symbol `pow`.
  -- As an alternative `libm["pow"]` can be used.
  let pow ← lib.symbol "pow"

  -- Call the function with two regular `CValue` objects and no variadic arguments.
  let result ← pow.call .double #[.double 1.4142, .double 2.0] #[]

  -- The result has type `CType.double`.
  IO.println s!"result: {repr result}"

  return 0
