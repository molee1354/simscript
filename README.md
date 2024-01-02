# simscript `v0.0.8rc1`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript `v0.0.8rc1`](#simscript-v008rc1)
  - [Table of Contents](#table-of-contents)
  - [Download](#download)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Building](#building)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Lists](#lists)
    - [Standard Library](#standard-library)
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

Here are some new features in version (`v0.0.8rc1`). A full log of releases can be found [here](./docs/release.md).

Version `v0.0.7` is one of the biggest releases yet. And with how things are going, this should be something like the second-to-last patch update before all the syntax other language feature additions settle down. For a few patches after, most of the work will be setting up some of the Simscript standard libraries such that the language will have more of an identity as far as its use case goes.

### Lists

Simscript now has lists! Lists in Simscript are very similar to lists in some of the other dynamically typed languages, in that they are just memory sequences that can hold pretty much any piece of data. Along with lists comes a handful of list methods that do useful things like `append()`, `prepend()` and all the exciting stuff. More details on lists can be found in the [syntax](./docs/syntax.md) page and the [list methods](./docs/functions/lists.md) page.

### Standard Library

Using the `using` keyword, you can now call modules that are part of the standard library. At the moment there are only really two standard libraries set up, but there are plans on new and exciting standard libraries so don't get too disappointed at its rather underwhelming current state. You can look at some of the details at the [libraries](./docs/libraries/libs.md) page.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
