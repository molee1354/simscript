# `Math`

This library includes functions that enable you to print error messages to `stderr`. All the functions in this page should be used after the standard library is declared using the `using` keyword.

This library includes common math functions. All the functions in this page should be used after the standard library is declared using the `using` keyword.

```javascript
using Math;
```

## `Math.sin(arg)`

A function that computes the sine function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the sine function computation for the radian value `arg`.

**Example**:

```javascript
var arg = 3.1415;
echo Math.sin(arg);

// Output
9.26536e-05
```

## `Math.cos(arg)`

A function that computes the cosine function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the cosine function computation for the radian value `arg`.

**Example**:

```javascript
var arg = 3.1415;
echo Math.cos(arg);

// Output
-1
```

## `Math.tan(arg)`

A function that computes the tangent function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the tangent function computation for the radian value `arg`.

**Example**:

```javascript
var arg = 3.1415;
echo Math.tan(arg);

// Output
-9.26536e-05
```

## `Math.asin(arg)`

A function that computes the arcsine function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the arcsine function computation as a radian value.

**Example**:

```javascript
var arg = 1;
echo Math.asin(arg);

// Output
1.5708
```

## `Math.acos(arg)`

A function that computes the arccosine function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the arccosine function computation as a radian value.

**Example**:

```javascript
var arg = 1;
echo Math.acos(arg);

// Output
0
```

## `Math.atan(arg)`

A function that computes the arctangent function for a given `arg` argument value in radians.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the arctangent function computation as a radian value.

**Example**:

```javascript
var arg = 1;
echo Math.acos(arg);

// Output
0
```

## `Math.floor(arg)`

A function that returns the largest integer value less than or equal to `arg`.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the largest integer value less than or equal to `arg`

**Example**:

```javascript
var arg = 1.5;
echo Math.floor(arg);

// Output
1
```

## `Math.ceil(arg)`

A function that returns the smallest integer value greater than or equal to `arg`.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the smallest integer value greater than or equal to `arg`

**Example**:

```javascript
var arg = 1.5;
echo Math.ceil(arg);

// Output
2
```

## `Math.ln(arg)`

A function that computes the natural logarithm of `arg`.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the natural logarithm of `arg`

**Example**:

```javascript
var arg = 10;
echo Math.ln(arg);

// Output
2.30259
```

## `Math.log(arg)`

A function that computes the base-10 logarithm of `arg`.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the base-10 logarithm of `arg`

**Example**:

```javascript
var arg = 10;
echo Math.log(arg);

// Output
1
```

## `Math.sqrt(arg)`

A function that computes the square root of `arg`.

- **arguments**: `arg` of type `Number`.
- **returns**: `Number` the square root of `arg`

**Example**:

```javascript
var arg = 9;
echo Math.log(arg);

// Output
3
```
