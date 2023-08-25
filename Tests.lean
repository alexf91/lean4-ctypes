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

namespace Tests.FFI

  testcase mkSuccess := do
    discard <| Handle.mk "/usr/lib/libgmp.so" RTLD_NOW

  testcase mkFailure := do
    try
      discard <| Handle.mk "/does/not/exist.so" RTLD_NOW
      assertTrue false "dlopen() did not fail"
    catch e =>
      let msg := "/does/not/exist.so: cannot open shared object file: No such file or directory"
      assertTrue $ e.toString == msg

end Tests.FFI

#LTestMain
