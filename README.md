# simscript v0.0.1

A simple scripting language based on the [*Crafting Interpreters*](https://craftinginterpreters.com/) book.

## Syntax

### Comments

Comments in Simscript are very much like comments in C (or Java or Javascript).

```javascript
// This is a comment.

/*
 * This is a multi-line comment.
 * This line is ignored.
 * This line is also ignored.
 */
```

### Types

--- TYPES in Simscript ---

- bool, string, number

### Variables

Variables in Simscript are dynamically typed. So we needn't worry about the cerebral overhead that comes from type checking when writing code.

Here we call a few different types of variables in Simscript:

```javascript
var foo = 3;
let bar = "Khan";
```

Both the `var` keyword and the `let` keyword allow us to declare variables. The `var` keyword creates a varaible that can by called across scopes, which is particularly useful for closures. The `let` keyword creates a variable that is strictly local. Variables created using the `let` keyword may not be called outside its local scope. The differences in usage become evident when using closures.

*Declaring* a variable and *defining* a variable are two different things in Simscript. You can declare a variable and go on to define it later in your code by assigning a value.

```javascript
var foo;
let bar;

foo = 3;
bar = "Khan";
```

### Constants

Constants (or constant variables) in Simscript are created by adding the `const` keyword in front of a variable declaration. Constants, as the name suggests, cannot be reassigned. They must also be have simultaneous declaration and definition. The same scoping rules apply as with variables.

```javascript
const var foo = 3;
const let bar = "Khan";

// This is not allowed
foo = 4;

// This is also not allowed
const var baz;
baz = 4;
```

### Conditionals

--- CONDITIONALS in Simscript ---

### Loops

--- LOOPS in Simscript ---

### Functions

--- FUNCTIONS in Simscript ---

- Built-in functions

### Classes

--- CLASSES in Simscript ---

- Inheritance
