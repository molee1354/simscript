# `IO`

This library deals with the day-to-day input-output operations. All the functions in this page should be used after the standard library is declared using the `using` keyword.

```javascript
using IO;
```

## `IO.print(value, ...)`

A function that prints the input values to `stdout`. Does not add a `\n` character at the end.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var name = "Khan";
IO.print("Hello", name);

// Output
Hello Khan
```

## `IO.println(value, ...)`

A function that prints the input values to `stdout`. Adds a `\n` character at the end.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var name = "Khan";
IO.println("Hello", name);

// Output
Hello Khan
```

## `IO.input(prompt)`

A function that prints the `prompt` to `stdout` and takes user input.

- **arguments**: `prompt` of `String` type.
- **returns**: `String` as `String`

**Example**:

```javascript
var name = IO.input("What is your name? : ");
IO.println("Hello", name);

// Output
What is your name? : Khan // user inputs name, "Khan" for instance.
Hello Khan
```
