# `Error`

This library includes functions that enable you to print error messages to `stderr`. All the functions in this page should be used after the standard library is declared using the `using` keyword.

```javascript
using Error;
```

## `Error.print(value, ...)`

A function that prints the input values to `stderr`. Does not add a `\n` character at the end.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var name = "Khan";
Error.print("Hello", name);

// Output
Hello Khan
```

## `Error.println(value, ...)`

A function that prints the input values to `stderr`. Adds a `\n` character at the end.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var name = "Khan";
Error.println("Hello", name);

// Output
Hello Khan
```
