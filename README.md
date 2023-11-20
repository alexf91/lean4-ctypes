# CTypes for Lean 4

CTypes is a foreign function library for Lean 4, inspired by Python's [ctypes](https://docs.python.org/3/library/ctypes.html) module.
It provides C compatible data types, and allows calling functions in shared libraries. It can be used to wrap these libraries in pure Lean 4.

## Usage

Access to library functions is structured done with the `Library` type.
It is a wrapper around `dlopen()` for loading and `dlsym()` for looking up a symbol.

The `CType` inductive represents types available in C.
It contains definitions for signed and unsigned integers, floating point types, complex types, arrays, structs and pointers.

Values are represented with the `CValue` type.
It directly matches the `CType` type.

The low-level implementation is in the `CTypes.Core` namespace and implements basic types and function calls.
It is a wrapper around `libffi`.

### Basic concepts

This example calls the function `pow()` in the library `libm.so.6`:
```Lean
import CTypes
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  -- Open the library. See man page for dlopen() for flags.
  let lib ← Library.mk "libm.so.6" .RTLD_NOW #[]

  -- Lookup the symbol `pow`.
  -- As an alternative `libm["pow"]` can be used.
  let pow ← lib.symbol "pow"

  -- Call the function with two regular `CValue` objects and no variadic arguments.
  let result ← pow.call .double #[.double 1.4142, .double 2.0] #[]

  -- The result has type `CType.double`.
  IO.println s!"result: {repr result}"

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
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  let lib ← Library.mk "libc.so.6" .RTLD_NOW #[]
  let malloc ← lib["malloc"]
  let free   ← lib["free"]

  -- Allocate a `int16_t` buffer.
  let p ← malloc.call .pointer #[.size_t CType.int16.size] #[]
  -- Write the value.
  p.pointer!.write (.int16 42)
  -- Read back the value.
  let value ← p.pointer!.read .int16

  IO.println s!"value: {repr value}"
  -- Free the allocated buffer.
  discard <| free.call .void #[p] #[]

  return 0
```

### Callbacks

Lean functions can be called from C by creating a `Closure` object.
Pointers to the closure can be passed to C functions or called like regular function pointers.

```Lean
import CTypes
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  -- Callback functions have the signature (Array CValue) → IO CValue
  let add : Callback := fun args => do
    return .int (args[0]!.int! + args[1]!.int!)

  -- Create the closure.
  let closure ← Closure.mk .int #[.int, .int] add

  -- Call the function through the `Pointer.call` interface.
  let result ← closure.pointer.call .int #[.int 42, .int 11] #[]
  IO.println s!"42 + 11 = {result.int!}"

  -- Closures are not deleted by default, in case a C function stores a reference.
  -- They have to be marked for deletion by the user.
  -- Deleted closures are freed when the object is garbage collected. Otherwise they
  -- leak memory.
  closure.delete

  return 0
```

### Structs and arrays

Structs are described as an array of types and instantiated as an array of values.
There is no direct support for arrays in the `CTypes.Core` library.
If arrays should be passed to functions, then the buffer has to be allocated with `malloc()`.
If an array is a member of a struct, then the array can created with a struct with one value for each array element.

Arrays created this way can not be passed to functions.

Here is an example that uses callbacks and a pointer to a struct to calculate the Fibonacci sequence:

```Lean
import CTypes
open CTypes.Core

def main (_ : List String) : IO UInt32 := do
  -- Fibonacci iteration with a struct as state variable.
  let fib : Callback := fun args => do
    let state ← args[0]!.pointer!.read $ .struct #[.int, .int]

    -- Get the two values from the (int, int) struct.
    let a := state.struct![0]!.int!
    let b := state.struct![1]!.int!

    -- Update the values and return the result.
    args[0]!.pointer!.write (.struct #[.int b, .int (a + b)])
    return .int a

  -- The closure object for the C function.
  let closure ← Closure.mk .int #[.pointer] fib

  -- Initialize the state struct to (0, 1).
  let libc ← Library.mk "libc.so.6" .RTLD_NOW #[]
  let state ← (← libc["malloc"]).call .pointer #[.size_t (CType.struct #[.int, .int]).size] #[]
  state.pointer!.write $ .struct #[.int 0, .int 1]

  for _ in [0:8] do
    let result ← closure.pointer.call .int #[state] #[]
    IO.println $ repr result

  -- Cleanup
  closure.delete
  discard <| (← libc["free"]).call .void #[state] #[]

  return 0
```
