# simscript `v0.0.5`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

Quote of the version: *"So it goes"* -Kurt Vonnegut.

## Table of Contents

- [simscript `v0.0.5`](#simscript-v005)
  - [Table of Contents](#table-of-contents)
  - [Download](#download)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Building](#building)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Non-global Variables for Simscript VM and Compiler](#non-global-variables-for-simscript-vm-and-compiler)
    - [Increment / Decrement Operations on Object Properties](#increment--decrement-operations-on-object-properties)
    - [New Tests](#new-tests)
  - [Documentation](#documentation)

## Download

Download the pre-compiled binaries from the [downloads page](./docs/downloads.md).

## Setup

Setting up simscript can be done in just a few steps.

### Dependencies

There aren't very many dependencies for building Simscript; however, it is important to mention that using `clang` will not properly compile the code.

```shell
make
gcc
```

Simscript uses `make` for building and `gcc` for code compilation.

### Building

Clone the git repostitory

```shell
git clone https://github.com/molee1354/simscript.git
```

`cd` into the new `simscript` directory

```shell
cd simscript
```

Build Simscript using either the `make` or the `make release` command, and the compiled binary should be found in both the current directory and the `bin/` directory.

```shell
make
```

If you really like Simscript and want to add it as a user binary, run the `make install` command.

```shell
make install
```

This will copy the simscript binary to the `/usr/local/bin/` directory. You may have to provide a password as the operation requires administrator privilege.

### Usage

You should be able to run Simscript from either of the compiled binaries. Call the binary with no arguments to enter the REPL, or pass in a file path as an argument to run a source file.

```shell
# Entering the REPL
./simscript

# running a source file
./simscript path/to/file.ss
```

Simscript can also be used to run files straight from the terminal if the path to the interpreter is known. Add a `#!` and the path to the Simscript binary in the first line of the file,

```javascript
#!/path/to/simscript

echo "Hello World!";
```

and call the file directly from the terminal.

```shell
path/to/script.ss
```

## Current Release

Here are some new features in version (`v0.0.5`). A full log of releases can be found [here](./docs/release.md).

This new version of Simscript is one of the biggest changes for the source code, but its manifestations in language syntax is minimal if at all. New features in version `v0.0.5` comprise of changes in the the Simscript virtual machine and the compiler, along with some important bug fixes.

### Non-global Variables for Simscript VM and Compiler

Prior to `v0.0.5`, each Simhub virtual machine and compiler had only one single representation at runtime, meaning that once the `simscript` binary was run, the VM and the compiler instance created was the only state that the code could ever be in. While this keeps the code simple, any performance improvements, particularly those involving the use of multiple threads, is very difficult or even impossible to implement. By changing the VM and compiler representations from global variables to passable parameters, new possibilities of performance optimizations now exist. Thorough testing was done to ensure that the current state of Simscript is backwards-compatible.

### Increment / Decrement Operations on Object Properties

Using increment/decrement operators (`++`, `--`), and operation-assignment operators (`+=`, `-=`, `*=`, `/=`) was unreliable with object properties, and in the case of the decrement operator `--`, even with normal variables. Changes in version `v0.0.5` fixed them.

### New Tests

Version `v0.0.5` come with some new tests and utility scripts to help with the development workflow.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
