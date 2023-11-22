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
import Tests.Core.Fixtures
open LTest
open CTypes.Core

namespace Tests.Utils

  /-- Allocate a buffer and write and read it. -/
  testcase testMalloc := do
    let type : CType := .struct #[.int, .int]
    let pointer ← malloc type.size
    try
      let u : CValue := .struct #[.int 100, .int 200]
      pointer.write u
      let v ← pointer.read type
      assertEqual u v s!"read wrong value: {repr v}"
    finally
      free pointer

end Tests.Utils
