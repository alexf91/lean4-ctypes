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

/- Control logging and build output. -/
meta if get_config? debug |>.isSome then
  def debugFlags := #["-DDEBUG", "-ggdb"]
else
  def debugFlags := #["-DNDEBUG"]

package ctypes {
  precompileModules := true
  -- TODO: Don't hardcode libraries and library paths. This currently fixes some
  --       link errors with conflicting libraries provided by the Lean environment.
  moreLinkArgs := #[
    "-lffi",
    "-lstdc++",
    "-L/usr/lib",
    "/usr/lib/libc.so.6",
    "/usr/lib/libstdc++.so.6",
    "/usr/lib/libunwind.so.8"
  ] ++ debugFlags
}

require LTest from git "git@github.com:alexf91/LTest.git" @ "main"

def createTarget (pkg : Package) (cfile : FilePath) := do
  let oFile := pkg.buildDir / cfile.withExtension "o"
  let srcJob ← inputFile <| pkg.dir / cfile
  let flags := #[
    "-I", (← getLeanIncludeDir).toString,
    "-fPIC",
    "-Wall",
    "-std=c++20"
  ] ++ debugFlags
  buildO cfile.toString oFile srcJob flags "g++"

target ctype.o pkg : FilePath := createTarget pkg $ "src" / "ctype.cpp"
target function.o pkg : FilePath := createTarget pkg $ "src" / "function.cpp"
target leanvalue.o pkg : FilePath := createTarget pkg $ "src" / "leanvalue.cpp"
target library.o pkg : FilePath := createTarget pkg $ "src" / "library.cpp"
target memory.o pkg : FilePath := createTarget pkg $ "src" / "memory.cpp"
target symbol.o pkg : FilePath := createTarget pkg $ "src" / "symbol.cpp"
target utils.o pkg : FilePath := createTarget pkg $ "src" / "utils.cpp"

extern_lib libctypes pkg := do
  let name := nameToStaticLib "ctypes"
  let targets := #[
    (← fetch <| pkg.target ``ctype.o),
    (← fetch <| pkg.target ``function.o),
    (← fetch <| pkg.target ``leanvalue.o),
    (← fetch <| pkg.target ``library.o),
    (← fetch <| pkg.target ``memory.o),
    (← fetch <| pkg.target ``symbol.o),
    (← fetch <| pkg.target ``utils.o)
  ]
  buildStaticLib (pkg.nativeLibDir / name) targets

@[default_target]
lean_lib CTypes

-- Tests
lean_lib Tests
lean_exe tests {
  root := `Tests
}

-- Run tests with valgrind.
script valgrind (args : List String) do
  -- TODO: Support arguments
  if args.length != 0 then
    IO.eprintln "usage: lake run valgrind"
    return 1

  -- Build tests
  let p ← IO.Process.spawn {
    cmd := "lake",
    args := #["build", "-Kdebug", "tests"]
  }
  let result ← p.wait
  unless result == 0 do return result

  -- Run valgrind
  let p ← IO.Process.spawn {
    cmd := "valgrind",
    args := #["--leak-check=yes", "build/bin/tests"]
  }
  let result ← p.wait

  return result
