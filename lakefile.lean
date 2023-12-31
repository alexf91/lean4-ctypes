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

/-
  Control logging and build output.
  Note that changing this option requires building with `lake -R`.
-/
meta if get_config? debug |>.isSome then
  def debugFlags := #["-DDEBUG", "-ggdb"]
else
  def debugFlags := #["-DNDEBUG"]

/- Flags for running with `gprof`. -/
meta if get_config? gprof |>.isSome then
  def profileFlags := #["-ggdb", "-pg"]
else
  def profileFlags : Array String := #[]

package ctypes {
  precompileModules := true
  moreLinkArgs := #[
    "-lffi",
    "-ldl"
  ] ++ debugFlags ++ profileFlags
}

require LTest from git "https://github.com/alexf91/LTest.git" @ "main"

/-- Compute a trace for files included by a target file. -/
def extraDepTrace (cfile : FilePath) : BuildM BuildTrace := do
  -- Get the list of local file dependencies.
  -- Compilation is still possible, even if g++ does not exist.
  let out ← IO.Process.output {cmd := "g++", args := #["-MM", cfile.toString]}
  if out.exitCode != 0 then
    IO.eprint out.stderr
    return .nil

  let deps := out.stdout.replace " \\\n" "" |>.dropWhile (· != ' ')
    |>.trim.splitOn.map System.FilePath.mk

  -- Create hashes and MTimes.
  let hashes ← deps.mapM fun p => computeFileHash p
  let mtimes ← deps.mapM fun p => getMTime p

  -- Combine traces.
  let traces := (List.zip hashes mtimes).map fun (h, m) => BuildTrace.mk h m
  return traces.foldr .mix .nil


/-- Create a target from a C++ file. -/
def createTarget (pkg : Package) (cfile : FilePath) := do
  let oFile := pkg.buildDir / cfile.withExtension "o"
  let srcJob ← inputFile <| pkg.dir / cfile
  let weakArgs := #[
    "-I", (← getLeanIncludeDir).toString
  ]
  let traceArgs := #[
    "-fPIC",
    "-Wall",
    "-std=c++20"
  ] ++ debugFlags ++ profileFlags
  let cFile := pkg.dir / cfile
  let cxx := (← IO.getEnv "LEAN_CC").getD "clang++"
  buildO cFile.toString oFile srcJob weakArgs traceArgs cxx (extraDepTrace cFile)

target callback.o pkg : FilePath := createTarget pkg $ "src" / "callback.cpp"
target closure.o pkg : FilePath := createTarget pkg $ "src" / "closure.cpp"
target library.o pkg : FilePath := createTarget pkg $ "src" / "library.cpp"
target pointer.o pkg : FilePath := createTarget pkg $ "src" / "pointer.cpp"
target types.o pkg : FilePath := createTarget pkg $ "src" / "types.cpp"
target utils.o pkg : FilePath := createTarget pkg $ "src" / "utils.cpp"

target types_ctype.o pkg : FilePath := createTarget pkg $ "src" / "types" / "ctype.cpp"
target types_cvalue.o pkg : FilePath := createTarget pkg $ "src" / "types" / "cvalue.cpp"
target types_common.o pkg : FilePath := createTarget pkg $ "src" / "types" / "common.cpp"

extern_lib libctypes pkg := do
  let name := nameToStaticLib "ctypes"
  let targets := #[
    (← fetch <| pkg.target ``callback.o),
    (← fetch <| pkg.target ``closure.o),
    (← fetch <| pkg.target ``library.o),
    (← fetch <| pkg.target ``pointer.o),
    (← fetch <| pkg.target ``types.o),
    (← fetch <| pkg.target ``utils.o),
    (← fetch <| pkg.target ``types_ctype.o),
    (← fetch <| pkg.target ``types_cvalue.o),
    (← fetch <| pkg.target ``types_common.o)
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
    args := #["build", "-R", "-Kdebug", "tests"]
  }
  let result ← p.wait
  unless result == 0 do return result

  -- Get the library path we need for valgrind. This should be the same as in the
  -- Lake environment when tests are run with `lake exe`.
  let libs ← Lake.getEnvSharedLibPath

  -- Run valgrind
  let p ← IO.Process.spawn {
    cmd := "valgrind"
    args := #[
      "--leak-check=yes",
      "--track-origins=yes",
      ".lake/build/bin/tests",
      "-v"
    ]
    env := #[(sharedLibPathEnvVar, libs.toString)]
  }
  p.wait

-- Run cppcheck
script cppcheck (args : List String) do
  if args.length != 0 then
    IO.eprintln "usage: lake run cppcheck"
    return 1

  let p ← IO.Process.spawn {
    cmd := "cppcheck"
    args := #["src"]
  }
  p.wait
