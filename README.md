# simscript `v0.0.5`

A ***sim***ple ***scr***ipting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Table of Contents

- [simscript `v0.0.5`](#simscript-v005)
  - [Table of Contents](#table-of-contents)
  - [Download](#download)
  - [Setup](#setup)
    - [Dependencies](#dependencies)
    - [Building](#building)
    - [Usage](#usage)
  - [Current Release](#current-release)
    - [Windows Binary](#windows-binary)
    - [Version Check](#version-check)
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

Here are some new features in version (`v0.0.4`). A full log of releases can be found [here](./docs/release.md).

**A new Windows executable has been compiled for Simscript in version `v0.0.4`**, along with some minor changes.

### Windows Binary

Starting from version `v0.0.4` a new Windows executable will be uploaded  in the `files` directory as `simscript.exe`. The Windows version always will be up-to-date on the `master` branch, and mostly on the `dev` branch as well.

### Version Check

Running the `simscript` command with the `--version` flag will now show the version number.

```shell
simscript --version


# output
simscript version vX.X.X
```

## Documentation

Documentation for Simscript can be found [here](./docs/syntax.md).
