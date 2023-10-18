# simscript `v0.0.2`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript `v0.0.2`](#simscript-v002)
  - [Table of Contents](#table-of-contents)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Installation](#installation)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Added New Operators](#added-new-operators)
    - [Optimizations](#optimizations)
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
./simscript file.ss
```

Simscript can also be used to run files straight from the terminal if the path to the interpreter is known. Add a `#!` and the path to the Simscript binary in the first line of the file,

```javascript
#!/path/to/simscript

echo "Hello World!";
```

and call the file directly from the terminal.

```shell
./filename
```

## Current Release

Here are some new features in version (`v0.0.2`). A full log of releases can be found [here](./docs/release.md).
Version `v0.0.2` is rather a smaller update focused on better loop usability and further optimizations.

### Added New Operators

Simscript now has support for *operation and assignment* operators (`+=`, `-=`, `*=`, `/=`), and increment/decrement operators (`++`, `--`). Have fun writing cleaner for-loops.

### Optimizations

Version `v0.0.2` comes with better [memory optimizations](https://craftinginterpreters.com/optimization.html#nan-boxing) under the hood. While smaller scripts won't be affected as much, the optimizations do come through on a larger scale.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
