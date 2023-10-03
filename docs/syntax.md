# Syntax

## Comments

Comments in Simscript are very much like comments in C (or Java or Javascript).

```javascript
// This is a comment.

/*
 * This is a multi-line comment.
 * This line is ignored.
 * This line is also ignored.
 */
```

## Types

While Simscript does not require explicit type declarations, there are a few built-in types that variables and other bits of primitive data tend to fall into.

- `Boolean`: `true` or `false` values that are used often in conditionals and comparision operations.
- `Number`:  Any numeric data in Simscript is of type `Number`. Simscript does not differentiate between integers and floating-point decimals as of now.
- `String`: Any set of characters enclosed by double quotations `"` is considered a string.

## Variables

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

## Constants

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

## Blocks

Blocks in Simscript are specified with curly braces. Anything that comes within a set of curly braces is in its own scope, and scoping rules apply.

```javascript
{
    let foo = 3; 
    print "This is " + foo; // This is 3
}
let foo = 4;
print "This is " + foo; // This is 4
```

## Conditionals

Conditionals in Simscript are very intuitive. The common `if`, `else if` and `else` keywords are all supported.

```javascript
if (condition) {
    // code to execute for condition
} else if (condition) {
    // code to execute for condition
} else {
    // code to execute if conditions are not met
}
```

The common AND and OR operations are supported by the `and` and `or` keywords.

## Loops

Simscript supports both FOR and WHILE loops. The syntax for FOR loops is very similar to that of Java's and C's.

```javascript
// for
for (increment variable; end condition; incrementation) {
    // code to execute
}

// while
while (end condition) {
    // code to execute
}
```

Incrementing the increment variable should be done with the very primitive variable reassingment method:

```javascript
i = i + 1
```

And as of [version `v0.0.1`](../README.md), the `break` and `continue` keywords are yet to be implemented.


## Functions

Functions in Simscript are created using the `function` keyword. Functions may or may not have arguments, and they may or may not have return values.

```javascript
function func(foo, bar) {
    return foo + bar;
}
```

Simscript also supports closures, and along with that, functions in Simscript are considered *first class*, meaning that they can be passed in as arguments and returned from other functions.

```javascript
function outside() {
    var foo = 3;

    function inside() {
        print "This is " + foo;
    }

    return inside;
}

var closure = outside();
closure();
```

If the `foo` variable were to be declared using the `let` keyword, this code would not work as `foo` would be strictly scoped to the local scope that it was declared in. So when using closures, it is important to distinguish between the `var` and `let` keywords depending on its use case.

## Classes

Simscript supports basic object-oriented programming patterns. Class definitions can be created with the `class` keyword.

This is an example of a class in Simscript.

```javascript
class Animal {
    init(sound) {
        this.sound = sound;
    }
    speak() {
        print this.sound + "!";
    }
}

var dog = Animal("Bark");
dog.speak();
```

Running this program would result in:

```shell
Bark!
```

And as of [version `v0.0.1`](../README.md), there is not a direct way to create properties in a class. Instead, you can use the class constructor or other class methods like in the example above. The class constructor can be called by simply calling the class name as a function.

You may notice that class methods don't have a `function` keyword when they are being defined. This goes back to the fact that Simscript currently does not support direct property definitions within classes. This is because any name that is within the class definition is considered to be a method name. I realise that this limits the imaginative things that can be done in the language, so I may make efforts to fix this in the future.

All methods in a class are public by default.
