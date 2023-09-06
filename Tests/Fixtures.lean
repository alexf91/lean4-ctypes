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

import LTest
import CTypes
open LTest
open CTypes.FFI


/-- Fixture for `libm`. -/
fixture LibMath Unit Library where
  setup := Library.mk "/usr/lib/libm.so.6" #[.RTLD_NOW]

/-- Fixture for the symbol `sin()`. -/
fixture SymSin Unit Symbol requires (m : LibMath) where
  setup := Symbol.mk m "sin"

/-- Fixture for the function `sin()`. -/
fixture FuncSin Unit Function requires (s : SymSin) where
  setup := Function.mk s .double #[.double]
