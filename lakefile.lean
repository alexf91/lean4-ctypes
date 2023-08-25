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

import Lake
open Lake DSL

package ctypes

require LTest from git "git@github.com:alexf91/LTest.git" @ "main"

/- Control logging output. -/
meta if get_config? debug |>.isSome then
  def debugFlags := #["-DDEBUG"]
else
  def debugFlags := #["-DNDEBUG"]

def createTarget (pkg : Package) (cfile : FilePath) := do
  let oFile := pkg.buildDir / cfile.withExtension "o"
  let srcJob ← inputFile <| pkg.dir / cfile
  let flags := #["-I", (← getLeanIncludeDir).toString, "-fPIC"] ++ debugFlags
  buildO cfile.toString oFile srcJob flags "cc"

target ffi.o pkg : FilePath := createTarget pkg $ "src" / "ffi.c"

extern_lib libctypes pkg := do
  let name := nameToStaticLib "ctypes"
  let targets := #[← fetch <| pkg.target ``ffi.o]
  buildStaticLib (pkg.nativeLibDir / name) targets

@[default_target]
lean_lib CTypes {
  precompileModules := true
}

-- Tests
lean_exe tests {
  root := `Tests
}
