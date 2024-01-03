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
    - [Strings](#strings)
    - [Math Standard Library](#math-standard-library)
    - [Prettier Error Messages](#prettier-error-messages)
  - [Documentation](#documentation)

## Download

Download the pre-compiled binaries from the [downloads page](./docs/downloads.md).

## Setup

Setting up simscript can be done in just a few steps.

### Dependencies

There aren't very many dependencies for building Simscript: just a simple C compiler will suffice (usually `gcc` or `clang`).
| Dependency | Usage |
| --- | --- |
| `gcc/clang` | C compiler for building Simscript from source |
| `make` | Build automation tool for compiling Simscript |

### Building

Clone the git repository

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

Version `v0.0.8` is a comparatively smaller release that aims to fill the little gaps in the language. With significant improvements to strings, a newly added math standard library, and more helpful error messages, this patch should set Simscript up in a good spot for future, more additive improvements.

### Strings

Strings now support escape characters. I am surprised that the implementation of a feature that's so fundamental has been ignored for so long, but now that it's added, it's opened up avenues for the addition of new and more exciting features in the future!

### Math Standard Library

A math library has been added to the Simscript Standard Library. More details can be found [here](./libraries/Math.md). Happy mathing!

### Prettier Error Messages

Error messages now are more consistent and visually appealing. Runtime errors have color highlights to the functions that hold the error, and the overall formatting has been fixed so that any error message will appear to be more consistent throughout.

## Documentation

Simscript documentation can be found [here](./docs/syntax.md).
