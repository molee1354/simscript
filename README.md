# simscript v0.0.1

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript v0.0.1](#simscript-v001)
  - [Table of Contents](#table-of-contents)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Installation](#installation)
  - [Current Release](#current-release)
    - [Added built-in functions](#added-built-in-functions)
    - [Constant Variables](#constant-variables)
    - [Strictly Local Variables](#strictly-local-variables)
    - [`+` Operator Overload](#-operator-overload)
  - [Documentation](#documentation)

## Setup

Setting up simscript can be done in just a few steps.

### Dependencies

There aren't very many dependencies for building Simscript; however, it is important to mention that using `clang` will not properly compile the code.

```shell
make
gcc
```

Simscript uses `make` for building and `gcc` for code compilation.

### Installation

Clone the git repostitory to your local machine

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

You should be able to run Simscript from either of the compiled binaries. Call the binary with no arguments to enter the REPL, or pass in a file path as an argument to run a source file.

```shell
# Entering the REPL
./simscript

# running a source file
./simscript file.ss
```

## Current Release

Here are some new features in version (`v0.0.1`).

### Added built-in functions

New built-in functions are added. Their specific usage can be found [here](./docs/functions.md)

- `sleep()`
- `exit()`
- `system()`
- `puts()`

### Constant Variables

The `const` keyword that allows for the creation of constant variables that cannot be reassigned. Their usage can be found in the [documentation](./docs/syntax.md).

### Strictly Local Variables

The `let` keyword creates a variable that cannot be accessed in any other scope other than the one it is declared in.

### `+` Operator Overload

The `+` operator can now concatenate `Number` and `String` types into a single string.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).