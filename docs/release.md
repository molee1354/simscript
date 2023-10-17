
# Releases History

## Current Release `v0.0.2`

Version `v0.0.2` is rather a smaller update focused on better loop usability and further optimizations.

### Added New Operators

Simscript now has support for *operation and assignment* operators (`+=`, `-=`, `*=`, `/=`), and increment/decrement operators (`++`, `--`). Have fun writing cleaner for-loops.

### Optimizations

Version `v0.0.2` comes with better [memory optimizations](https://craftinginterpreters.com/optimization.html#nan-boxing) under the hood. While smaller scripts won't be affected as much, the optimizations do come through on a larger scale.

## Version `v0.0.1`

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