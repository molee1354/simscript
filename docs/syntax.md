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

### `Boolean`

`true` or `false` values that are used often in conditionals and comparison operations.

### `Number`

Any numeric data in Simscript is of type `Number`. Simscript does not differentiate between integers and floating-point decimals.

### `String`

Any set of characters enclosed by single `'` or double `"` quotation marks is considered a string.

Strings support escape characters, so

```javascript
var foo = "Hello\nKhan!";
echo foo;
```

would result in

```shell
Hello
Khan!
```

Raw strings in Simscript are defined by prepending a single `r` in front of the quotation marks that enclose a string value.

```javascript
var foo = r"Hello\nKhan!";
echo foo;
```

would result in

```shell
Hello\nKhan!
```

## Variables

Variables in Simscript are dynamically typed. So we needn't worry about the cerebral overhead that comes from type checking when writing code.

Here we define some variables in Simscript:

```javascript
var foo = 3;
local var bar = "Khan";
```

The `var` keyword allows us to declare variables. An optional `local` keyword can be added before the `var` keyword to control the scope of the variable. Local variables cannot be called outside its local scope, meaning that nested (or enclosing) scopes will not have access to the variable.

*Declaring* a variable and *defining* a variable are two different things in Simscript. You can declare a variable and go on to define it later in your code by assigning a value.

```javascript
var foo;
local var bar;

foo = 3;
bar = "Khan";
```

## Constants

Constants (or constant variables) in Simscript are created by adding the `const` keyword in front of a variable name. Constants, as the name suggests, cannot be reassigned. They must also be have simultaneous declaration and definition. The same scoping rules apply as with variables.

```javascript
const foo = 3;
local const bar = "Khan";

// This is not allowed
foo = 4;

// This is also not allowed
const baz;
baz = 4;
```

Adding the `local` in front of a constant declaration scopes the constant to the local scope, meaning that it won't be accessed in any other scope (either nested or enclosing).

## Lists

Lists in Simscript are continuous blocks sections of data pieces that can hold pretty much anything. In other words, if you are familiar with lists in Python, Simscript lists will not be a surprise.

Simply declare them using square brackets and you can subscript, `append()`, `prepend()`, `push()`, `pop()`, and so on. List can also function as stacks and queues, depending on what list method you call.

Creating a list is as simple as:

```javascript
var foo = [0,1,2,3];
echo foo; // [0, 1, 2, 3]

echo foo[1]; // 1
```

You can call list methods with the dot `.` operator:

```javascript
foo.append(4);
echo foo; // [0, 1, 2, 3, 4]
var bar = foo.pop();

echo foo; // [1, 2, 3, 4]
echo bar; // 0
```

With stack and queue data structure methods, the element on index 0 is considered to be the "head" and the last element is considered to be the "tail". So `push()` will add an element to index 0, and `dequeue()` will remove and return the element at the very last index.

## Blocks

Blocks in Simscript are specified with curly braces. Anything that comes within a set of curly braces is in its own scope, and scoping rules apply.

```javascript
{
    local var foo = 3; 
    echo "This is " + foo; // This is 3
}
local var foo = 4;
echo "This is " + foo; // This is 4
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
/* For-loop syntax
 * for (increment variable; end condition; incrementation) {...}
 */
for (var i=0; i<5; i++) { /* do something */ }

/* While-loop syntax
 * while (end condition) {...}
 */
var i = 0;
while (i<5) { /* do something */ }

```

And as of [version `v0.0.3`](release.md), the `break` and `continue` keywords have been implemented. Their syntax is very much like what you would expect.

```javascript
for (var i=0; i<10; i++) {
    if (i%2 == 0) {
        continue;
    }
    // do something
}

var n = 0;
while (true) {
    if (n > 9) {
        break;
    }
    // do something
    n++;
}

```

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
    // using the 'var' keyword for variable access across scopes
    var foo = 3;

    function inside() {
        echo "This is " + foo;
    }

    // functions may be returned
    return inside;
}

var closure = outside();
closure();
```

If the `foo` variable were to be declared as a local variable, this code would not work as `foo` would be strictly scoped to the local scope that it was declared in. So when using closures, it is important to distinguish between the `var` and `local var` keywords depending on its use case.

## Classes

Simscript supports basic object-oriented programming patterns. Class definitions can be created with the `class` keyword.

This is an example of using a class in Simscript.

```javascript
class Animal {
    init(sound) {
        this.sound = sound;
    }
    speak() {
        echo this.sound + "!";
    }
}

var dog = Animal("Bark");
dog.speak();
```

Running this program would result in

```shell
Bark!
```

And as of [version `v0.0.5`](release.md), there is not a direct way to create properties in a class. Instead, you can use the class constructor or other class methods like in the example above. The `this` keyword can be used (very much like in Java) to refer the current class that is being defined.

Objects can be created by calling a class constructor, which is simply the class name as a function with the appropriate arguments.

You may notice that class methods don't have a `function` keyword when they are being defined. This goes back to the fact that Simscript currently does not support direct property definitions within classes. This is because any name that is within the class definition is considered to be a method name. I realise that this limits the imaginative things that can be done in the language, so I may make efforts to fix this in the future.

All methods in a class are public by default.

### Inheritance

Classes in Simscript support inheritance. Using the `extends` keyword, a child class can be created. Within a child class, the `super` keyword can be used to refer to the parent class and gain access to some of the methods there.

Here is quick example of how inheritance works in Simscript

```javascript
class Animal {
    init(type) {
        this.type = type;
    }
    speak(sound) {
        echo sound + "!";
    }
}

class Dog extends Animal {
    init(name) {
        this.name = name;
        super.init("Dog");
    }
    bark() {
        echo "My name is " + this.name;
        this.speak("Bark");
    }
}

var pet = Dog("Khan");
pet.bark();
```

The code above would output:

```shell
My name is Khan
Bark!
```

## Modules

Simscript supports import modules. Variables, functions, and classes defined in an external module can be imported to the current module with the `module` keyword.

Say we have a file `module.ss` where some functions, variables, and classes are defined.

```javascript
// module.ss
class Animal {
    speak(sound) {
        echo sound + "!";
    }
}

function greet(name) {
    echo "Hello, " + name;
}

var name = "Khan";
greet(name);
```

The module keyword can be used to import `module.ss` into `file.ss`. The `module` keyword runs the code inside whatever module file is being referenced.

```javascript
// file.ss
module "module.ss";
```

Running this code would result in:

```javascript
Hello, Khan
```

The output is from the `greet()` function call in `module.ss`.

After the `module` keyword, a relative path to the module should be provided as a string so that Simscript will be able to parse the file.

In the example above, whatever code was in the `"module.ss"` will be run. However, you won't be able to access the symbols as the import alias hasn't been defined yet.

To access the symbols from within `file.ss`, an import alias has to be created.

```javascript
// file.ss
module myModule = "module.ss";

echo myModule.name; 
myModule.greet("Marley");
myModule.Animal()
```

Running this code would result in:

```javascript
Hello, Khan    // from "module.ss"
Khan           // from "file.ss"
Hello, Marley  // from "file.ss"
Bark!          // from "file.ss"
```

## The Standard Library

You can call the Simscript standard library with the `using` keyword. Unlike modules; however, you can't assign arbitrary variable names.

To call the `IO` standard library you can do the following:

```javascript
using IO;

IO.println("Hello Khan!");
```

Which would result in:

```shell
Hello Khan!
```

More information about using standard libraries and the specific details for each library can be found in the [libraries](./libraries/libs.md) page.
