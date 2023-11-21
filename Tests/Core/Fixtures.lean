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
open CTypes.Core
open System

/-- Create a temporary directory. -/
private def tempDir : IO FilePath := do
    let dir ← IO.Process.run {cmd := "mktemp", args := #["-d", "/tmp/CTypes.XXXXXXXXXX"]}
    return FilePath.mk dir.trim

/-- Execute a function with a temporary directory. -/
private def withTempDir (fn : FilePath → IO α) : IO α := do
  let dir ← tempDir
  try
    fn dir
  finally
    IO.FS.removeDirAll dir

/--
  Fixture for generating a temporary directory.
  The directory is deleted during cleanup.
-/
fixture TemporaryDirectory FilePath FilePath where
  setup := do
    let dir ← tempDir
    set dir
    return dir
  teardown := do
    IO.FS.removeDirAll (← get)

/-- Default headers to include for compiler detection and libraries itself. --/
private def defaultHeaders := [
  "#include <stdio.h>",
  "#include <stdint.h>",
  "#include <stdlib.h>",
  "#include <complex.h>",
  "#include <assert.h>",
  "#include <stdarg.h>"
]

-- Store the compiler so we don't check for every testcase.
private initialize CC : IO.Ref (Option String) ← IO.mkRef none

/-- Look for a working C compiler. -/
private def findCompiler (options : List String) : IO String := do
  if (← CC.get).isNone then
    for cmd in options do
      let result ← withTempDir fun dir => do
        let cfile := dir / "test.c"  |>.toString
        -- Create the dummy C file.
        IO.FS.withFile cfile IO.FS.Mode.write fun h =>
          let headers := defaultHeaders.foldr (· ++ "\n" ++ ·) ""
          h.putStr $ headers ++ "int main(int argc, char **argv) {return 0;}"

        let ofile := dir / "test.o"  |>.toString
        IO.Process.output {
          cmd := cmd
          args := #["-c", "-o", ofile, cfile]
          cwd := dir
          -- TODO: Remove only the prefix added by Lake. We could get it by calling
          --       `lake env` with `LD_LIBRARY_PATH` set to `none`.
          env := #[("LD_LIBRARY_PATH", none)]
        }
      if result.exitCode == 0 then
        CC.set cmd
        break

  if (← CC.get).isNone then
    throw $ IO.userError "no suitable compiler found"
  return (← CC.get).get!

/--
  Generate a temporary library from the given code and open it.
  The path must exist and be a directory.
-/
private def generateLibrary (path : FilePath) (code : String) : IO Library := do
  assert! ← path.pathExists
  assert! ← path.isDir

  let headers := defaultHeaders.foldr (· ++ "\n" ++ ·) ""

  let cfile := path / "library.c"  |>.toString
  let ofile := path / "library.o"  |>.toString
  let lib   := path / "library.so" |>.toString

  -- Write to file
  IO.FS.withFile cfile IO.FS.Mode.write fun h =>
    h.putStr $ headers ++ code

  -- Check the absolute path in case the compiler shipped with Lean is used.
  -- TODO: Prefer `LEAN_CC` or maybe `CTYPES_CC` or `LTEST_CC`.
  let compiler ← findCompiler ["gcc", "clang", "/usr/bin/clang",
    "clang-11", "clang-12", "clang-13", "clang-14", "clang-15", "clang-16"]

  -- Compile the object file
  let out ← IO.Process.output {
    cmd := compiler,
    args := #["-o", ofile, "-c", "-Wall", "-Werror", "-O0", "-ggdb", "-fPIC",  cfile]
    env := #[("LD_LIBRARY_PATH", none)]
  }
  if out.exitCode != 0 then
    IO.eprint out.stderr
    throw $ IO.userError "compilation failed"

  -- Link the shared library
  let out ← IO.Process.output {
    cmd := compiler,
    args := #["-shared", "-o", lib, ofile]
    env := #[("LD_LIBRARY_PATH", none)]
  }
  if out.exitCode != 0 then
    IO.eprint out.stderr
    throw $ IO.userError "linking failed"

  Library.mk lib .RTLD_NOW #[]

/--
  Fixture for generating a temporary shared library.
  The setup function returns a function that takes C code and compiles it
  to a library. It then returns the path of the library.

  TODO: Close library in teardown.
-/
fixture SharedLibrary FilePath (String → IO Library) requires (td : TemporaryDirectory) where
  setup := do return generateLibrary td


/--
  Fixture for access to `libc`.
 -/
fixture LibC Unit Library where
  setup := Library.mk "/usr/lib/libc.so.6" .RTLD_NOW #[]
