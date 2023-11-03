# simscript `v0.0.7rc1`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript `v0.0.7rc1`](#simscript-v007rc1)
  - [Table of Contents](#table-of-contents)
  - [Download](#download)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Building](#building)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Importing Modules with the `module` Keyword](#importing-modules-with-the-module-keyword)
    - [Updated Syntax with the `local` Keyword](#updated-syntax-with-the-local-keyword)
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

Here are some new features in version (`v0.0.7rc1`). A full log of releases can be found [here](./docs/release.md).

Simscript `v0.0.6` comes with some new and exciting changes, along with significant syntax updates. **This update will be one of the few updates where source code from previous versions won't be compatible**. So bear that in mind.

### Importing Modules with the `module` Keyword

Simscript can now import external modules. This is a really exciting new addition as this allows for the development of larger-scale projects. You can check the updated syntax in the [syntax](./syntax) page.

### Updated Syntax with the `local` Keyword

Setting variable behavior with keywords prior to version `v0.0.6` was a bit unintuitive. Keywords used to set variable behavior did not fully reflect its functionality (looking at you, `let`). To fix this, a new `local` keyword was added and the `const` keyword is now a 'first-level' keyword when declaring variables, meaning that it now should come right before the variable name. Details in it usage can be found on the [syntax](./syntax.md) page.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
