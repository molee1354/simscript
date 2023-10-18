# simscript `v0.0.3`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript `v0.0.3`](#simscript-v003)
  - [Table of Contents](#table-of-contents)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Installation](#installation)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Loop keywords: `break` and `continue`](#loop-keywords-break-and-continue)
    - [Feature Testing](#feature-testing)
    - [Branches](#branches)
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

Here are some new features in version (`v0.0.3`). A full log of releases can be found [here](./docs/release.md).

Version `v0.0.3` comes with some impactful syntax updates, along with some changes to the compiler to support it.

### Loop keywords: `break` and `continue`

Trying to do anything in a programming language without loops is quite discouraging. Now, your friends won't make fun of you for not having `break` and `continue` keywords in your language! The syntax is very much like any other language: add either the `break` or `continue` keyword within the loop along with just an added semicolon `;` at the end. Specifics on the syntax can be found in the [docs](./docs/syntax.md).

### Feature Testing

While this is much more geared towards *developing* Simscript rather than using it, it should play an important role in communicating the current state of Simscript. Expanding on this feature should add much more to the language in the future.

### Branches

The program state at the `dev` branch will always be *backwards-compatible*, meaning that the newest Simscript binary compiled from the code in the `dev` branch will be able to (mostly) run the source code for previous versions. Assuming that there were no major syntax updates, the "rolling release" version of Simscript should be backwards-compatible, but you must bear in mind the potential breakages that will occur from new features.

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
