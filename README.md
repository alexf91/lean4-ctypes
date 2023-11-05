# CTypes for Lean 4

CTypes is a foreign function library for Lean 4, inspired by Python's [ctypes](https://docs.python.org/3/library/ctypes.html) module.
It provides C compatible data types, and allows calling functions in shared libraries. It can be used to wrap these libraries in pure Lean 4.

## Usage

Access to library functions is structured in three types:

* Library: Shared library opened with `dlopen()`
* Symbol: Symbol in a library, opened with `dlsym()`
* Function: Symbol annotated with data types

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

  -- Create the symbol for the function `pow`.
  -- As an alternative `libm["pow"]` can be used.
  let sym ← Symbol.mk lib "pow"

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
