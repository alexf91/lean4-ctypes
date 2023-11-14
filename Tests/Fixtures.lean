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
open CTypes
open System

/--
  Fixture for generating a temporary directory.
  The directory is deleted during cleanup.
-/
fixture TemporaryDirectory FilePath FilePath where
  setup := do
    let dir ← IO.Process.run {cmd := "mktemp", args := #["-d", "/tmp/CTypes.XXXXXXXXXX"]}
    let dir := FilePath.mk dir.trim
    set dir
    return dir
  teardown := do
    IO.FS.removeDirAll (← get)


/--
  Generate a temporary library from the given code and open it.
  The path must exist and be a directory.
-/
private def generateLibrary (path : FilePath) (code : String) : IO Library := do
  assert! ← path.pathExists
  assert! ← path.isDir

  let defaultHeaders := [
    "#include <stdio.h>",
    "#include <stdint.h>",
    "#include <stdlib.h>",
    "#include <complex.h>",
    "#include <assert.h>"
  ].foldr (· ++ "\n" ++ ·) ""

  let cfile := path / "library.c"  |>.toString
  let ofile := path / "library.o"  |>.toString
  let lib   := path / "library.so" |>.toString

  -- Write to file
  IO.FS.withFile cfile IO.FS.Mode.write fun h =>
    h.putStr $ defaultHeaders ++ code

  -- Compile the object file
  discard <| IO.Process.run {
    cmd := "gcc",
    args := #["-o", ofile, "-c", "-Wall", "-Werror", "-O0", "-ggdb", "-fPIC",  cfile]
  }

  -- Link the shared library
  discard <| IO.Process.run {
    cmd := "gcc",
    args := #["-shared", "-o", lib, ofile]
  }
  Library.mk lib #[.RTLD_NOW]

/--
  Fixture for generating a temporary shared library.
  The setup function returns a function that takes C code and compiles it
  to a library. It then returns the path of the library.
-/
fixture SharedLibrary FilePath (String → IO Library) requires (td : TemporaryDirectory) where
  setup := do return generateLibrary td


/--
  Fixture for access to `libc`.
 -/
fixture LibC Unit Library where
  setup := Library.mk "/usr/lib/libc.so.6" #[.RTLD_NOW]
