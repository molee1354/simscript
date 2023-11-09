# Built-in Functions

Simscript comes with a few built-in functions. They usually are direct implementations of C functions into the Simscript virtual machine, so their behavior can be modified directly in the source code.

## `clock()`

A function that returns the number of clock ticks that passed since the start of the program.

- **arguments**: `none`
- **returns**: `Number`
  - The number of clock ticks elapsed since the start of the program.

## `sleep(sec)`

A function to delay the execution of the current program for a certain number of seconds.

- **arguments**:
  - `sec` of type `Number`.
    - The number of seconds to delay the execution of the current program.
- **returns**: `none`

## `exit(code)`

A function to exit the current program with a given exit code.

- **arguments**:
  - `code` of type `Number`.
    - The exit code with which the program will exit.
- **returns**: `none`

## `system(cmd)`

A function to run a shell command. Spawns a child process that is separate from the current process running `simscript`.

- **arguments**:
  - `cmd` of type `String`.
  - The shell command to run in the shell.
- **returns**: `none`

## `puts(str)`

A function to print out a string to the console. Does not support string formatting.

- **arguments**:
  - `str` of type `String`.
  - The string to print out to the console.
- **returns**: `none`
