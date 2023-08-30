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

package ctypes {
  precompileModules := true
  moreLinkArgs := #["-lffi"]
}

require LTest from git "git@github.com:alexf91/LTest.git" @ "main"

/- Control logging output. -/
meta if get_config? debug |>.isSome then
  def debugFlags := #["-DDEBUG", "-ggdb"]
else
  def debugFlags := #["-DNDEBUG"]

def createTarget (pkg : Package) (cfile : FilePath) := do
  let oFile := pkg.buildDir / cfile.withExtension "o"
  let srcJob ← inputFile <| pkg.dir / cfile
  let flags := #[
    "-I", (← getLeanIncludeDir).toString,
    "-fPIC",
    "-Wall"
  ] ++ debugFlags
  buildO cfile.toString oFile srcJob flags "cc"

target utils.o pkg : FilePath := createTarget pkg $ "src" / "utils.c"
target library.o pkg : FilePath := createTarget pkg $ "src" / "library.c"
target symbol.o pkg : FilePath := createTarget pkg $ "src" / "symbol.c"
target function.o pkg : FilePath := createTarget pkg $ "src" / "function.c"

extern_lib libctypes pkg := do
  let name := nameToStaticLib "ctypes"
  let targets := #[
    (← fetch <| pkg.target ``utils.o),
    (← fetch <| pkg.target ``library.o),
    (← fetch <| pkg.target ``symbol.o),
    (← fetch <| pkg.target ``function.o)
  ]
  buildStaticLib (pkg.nativeLibDir / name) targets

@[default_target]
lean_lib CTypes

-- Tests
lean_exe tests {
  root := `Tests
}
