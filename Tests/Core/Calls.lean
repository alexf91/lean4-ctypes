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

namespace Tests.Calls

  testcase testCall requires (libgen : SharedLibrary) := do
    let lib ← libgen $ "uint32_t foo(void) { return 42; }"
    let foo ← lib["foo"]
    let value ← foo.call .uint32 #[] #[]
    assertEqual value (.uint32 42) s!"wrong result: {repr value}"

end Tests.Calls
