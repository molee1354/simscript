# Releases History

## Current Release `v0.0.8rc1`

Version `v0.0.8` is a comparatively smaller release that aims to fill the little gaps in the language. With significant improvements to strings, a newly added math standard library, and more helpful error messages, this patch should set Simscript up in a good spot for future, more additive improvements.

### Strings

Strings now support escape characters. I am surprised that the implementation of a feature that's so fundamental has been ignored for so long, but now that it's added, it's opened up avenues for the addition of new and more exciting features in the future!

### Math Standard Library

A math library has been added to the Simscript Standard Library. More details can be found [here](./libraries/Math.md). Happy mathing!

### Prettier Error Messages

Error messages now are more consistent and visually appealing. Runtime errors have color highlights to the functions that hold the error, and the overall formatting has been fixed so that any error message will appear to be more consistent throughout.

## Version `v0.0.7`

Version `v0.0.7` is one of the biggest releases yet. And with how things are going, this should be something like the second-to-last patch update before all the syntax other language feature additions settle down. For a few patches after, most of the work will be setting up some of the Simscript standard libraries such that the language will have more of an identity as far as its use case goes.

### Lists

Simscript now has lists! Lists in Simscript are very similar to lists in some of the other dynamically typed languages, in that they are just memory sequences that can hold pretty much any piece of data. Along with lists comes a handful of list methods that do useful things like `append()`, `prepend()` and all the exciting stuff. More details on lists can be found in the [syntax](./syntax.md) page and the [list methods](./functions/lists.md) page.

### Standard Library

Using the `using` keyword, you can now call modules that are part of the standard library. At the moment there are only really two standard libraries set up, but there are plans on new and exciting standard libraries so don't get too disappointed at its rather underwhelming current state. You can look at some of the details at the [libraries](./libraries/libs.md) page.

## Version `v0.0.6`

Simscript `v0.0.6` comes with some new and exciting changes, along with significant syntax updates. **This update will be one of the few updates where source code from previous versions won't be compatible**. So bear that in mind.

### Importing Modules with the `module` Keyword

Simscript can now import external modules. This is a really exciting new addition as this allows for the development of larger-scale projects. You can check the updated syntax in the [syntax](./syntax) page.

### Updated Syntax with the `local` Keyword

Setting variable behavior with keywords prior to version `v0.0.6` was a bit unintuitive. Keywords used to set variable behavior did not fully reflect its functionality (looking at you, `let`). To fix this, a new `local` keyword was added and the `const` keyword is now a 'first-level' keyword when declaring variables, meaning that it now should come right before the variable name. Details in it usage can be found on the [syntax](./syntax.md) page.

## Version `v0.0.5`

This new version of Simscript is one of the biggest changes for the source code, but its manifestations in language syntax is minimal if at all. New features in version `v0.0.5` comprise of changes in the the Simscript virtual machine and the compiler, along with some important bug fixes.

### Non-global Variables for Simscript VM and Compiler

Prior to `v0.0.5`, each Simhub virtual machine and compiler had only one single representation at runtime, meaning that once the `simscript` binary was run, the VM and the compiler instance created was the only state that the code could ever be in. While this keeps the code simple, any performance improvements, particularly those involving the use of multiple threads, is very difficult or even impossible to implement. By changing the VM and compiler representations from global variables to passable parameters, new possibilities of performance optimizations now exist. Thorough testing was done to ensure that the current state of Simscript is backwards-compatible.

### Increment / Decrement Operations on Object Properties

Using increment/decrement operators (`++`, `--`), and operation-assignment operators (`+=`, `-=`, `*=`, `/=`) was unreliable with object properties, and in the case of the decrement operator `--`, even with normal variables. Changes in version `v0.0.5` fixed them.

### New Tests

Version `v0.0.5` come with some new tests and utility scripts to help with the development workflow.

## Version `v0.0.4`

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

## Version `v0.0.3`

Version `v0.0.3` comes with some impactful syntax updates, along with some changes to the compiler to support it.

### Loop Keywords: `break` and `continue`

Trying to do anything in a programming language without loops is quite discouraging. Now, your friends won't make fun of you for not having `break` and `continue` keywords in your language! The syntax is very much like any other language: add either the `break` or `continue` keyword within the loop along with just an added semicolon `;` at the end. Specifics on the syntax can be found in the [docs](syntax.md).

### Feature Testing

While this is much more geared towards *developing* Simscript rather than using it, it should play an important role in communicating the current state of Simscript. Expanding on this feature should add much more to the language in the future.

### Branches

The program state at the `dev` branch will always be *backwards-compatible*, meaning that the newest Simscript binary compiled from the code in the `dev` branch will be able to (mostly) run the source code for previous versions. Assuming that there were no major syntax updates, the "rolling release" version of Simscript should be backwards-compatible, but you must bear in mind the potential breakages that will occur from new features.

## Version `v0.0.2`

Version `v0.0.2` is rather a smaller update focused on better loop usability and further optimizations.

### Added New Operators

Simscript now has support for *operation and assignment* operators (`+=`, `-=`, `*=`, `/=`), and increment/decrement operators (`++`, `--`). Have fun writing cleaner for-loops.

### Optimizations

Version `v0.0.2` comes with better [memory optimizations](https://craftinginterpreters.com/optimization.html#nan-boxing) under the hood. While smaller scripts won't be affected as much, the optimizations do come through on a larger scale.

## Version `v0.0.1`

Here are some new features in version (`v0.0.1`).

### Added Built-in Functions

New built-in functions are added. Their specific usage can be found [here](./functions/builtin.md)

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
