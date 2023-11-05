# CTypes for Lean 4

CTypes is a foreign function library for Lean 4, inspired by Python's [ctypes](https://docs.python.org/3/library/ctypes.html) module.
It provides C compatible data types, and allows calling functions in shared libraries. It can be used to wrap these libraries in pure Lean 4.

## Usage

Access to library functions is structured in two types:

* Library: Shared library opened with `dlopen()`
* Function: Function pointer annotated with data types

The `CType` inductive represents types available in C.
It contains definitions for signed and unsigned integers, floating point types, complex types, arrays, structs and pointers.

Values in Lean are passed and returned using the `LeanValue` inductive.

### Basic concepts

This example calls the function `pow()` in the library `libm.so.6`:
```Lean
import CTypes
open CTypes

def main (_ : List String) : IO UInt32 := do
  -- Open the library. See man page for dlopen() for flags.
  let lib ← Library.mk "libm.so.6" #[.RTLD_NOW]

  -- Lookup the symbol `pow`.
  -- As an alternative `libm["pow"]` can be used.
  let sym ← lib.symbol "pow"

  -- Annotate the function with `CType` types. `pow()` returns a double value
  -- and takes two double values as arguments.
  let pow ← Function.mk sym .double #[.double, .double]

  -- Call the function with `LeanValue` arguments.
  let result ← pow.call #[.float 1.4142, .float 2.0]

  -- The result has type `LeanValue.float`.
  IO.println s!"result: {result.float!}"

  return 0
```

### Pointers

While equivalents to basic C types exist in Lean, this is not the case for pointers.
The `Pointer` type is used to access raw memory from Lean.
They support pointer arithmetic and dereferencing.

Unless noted otherwise, the library will never allocate or free memory on its own.
This has to be done by the user with `malloc()`, `free()` and similar functions.

```Lean
import CTypes
open CTypes

def main (_ : List String) : IO UInt32 := do
  let lib ← Library.mk "libc.so.6" #[.RTLD_NOW]
  let malloc ← Function.mk (← lib["malloc"]) .pointer #[.size_t]
  let free   ← Function.mk (← lib["free"])   .void    #[.pointer]

  -- Allocate a `int16_t` buffer.
  let p ← malloc.call #[.nat CType.int16.size]
  -- Write the value.
  p.pointer!.write .int16 (.int 42)
  -- Read back the value.
  let value ← p.pointer!.read .int16

  IO.println s!"value: {repr value}"
  -- Free the allocated buffer.
  discard <| free.call #[p]

  return 0
```

### Structs and arrays

Structs are described as an array of types and instantiated as an array of values.
The data type description doesn't contain names of the fields.
This has to be handled by the user and the code that wraps the low-level function interface (e.g. conversion to structures).

(Fixed-length) arrays are described by their data type and their length.
Their Lean representation is immutable.
If a `LeanValue.array` is passed to a function, then a temporary copy is created and freed after the call.
Changes to the array are never observed in Lean.
If this is necessary or the callee might take ownership of the array, then a buffer has to be allocated with `malloc()`, like in the pointer example.
The main purpose of the `CType.array` and `LeanValue.array` types is as a member of a struct or for read-only buffer arguments.

```Lean
import CTypes
open CTypes

def main (_ : List String) : IO UInt32 := do
  let lib ← Library.mk "libc.so.6" #[.RTLD_NOW]
  let malloc ← Function.mk (← lib["malloc"]) .pointer #[.size_t]
  let free   ← Function.mk (← lib["free"])   .void    #[.pointer]
  let memset ← Function.mk (← lib["memset"]) .pointer #[.pointer, .int, .size_t]

  -- Create a struct with an integer and a fixed-length array.
  let type := CType.struct #[.int, .array .int 8]
  let p ← malloc.call #[.nat type.size]

  -- Set all values to 0xFF, resulting in -1 for all fields.
  discard <| memset.call #[p, .int 0xFF, .nat type.size]

  -- Read back the value.
  let value ← p.pointer!.read type
  IO.println s!"value: {repr value}"

  -- Free the allocated buffer.
  discard <| free.call #[p]

  return 0
```

The result is a nested `LeanValue`.

```
CTypes.LeanValue.struct
  #[CTypes.LeanValue.int -1,
    CTypes.LeanValue.array
      #[CTypes.LeanValue.int -1, CTypes.LeanValue.int -1, CTypes.LeanValue.int -1, CTypes.LeanValue.int -1,
        CTypes.LeanValue.int -1, CTypes.LeanValue.int -1, CTypes.LeanValue.int -1, CTypes.LeanValue.int -1]]
```
