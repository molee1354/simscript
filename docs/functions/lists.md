# List Methods

Simscript has a few built-in list methods. In this documentation, `head` indicates wherever index 0 is for a given list. Likewise, words like `end` and `tail` indicate wherever the last index for points for a given list.

## `[List].append(value)`

A method that adds a new element at the very end of the list.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.append(3);
echo foo;

// Output
[0, 1, 2, 3]
```

## `[List].prepend(value)`

A method that adds a new element at the very beginning of the list.

- **arguments**: `value` of any type.
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.append(5);
echo foo;

// Output
[5, 0, 1, 2]
```

## `[List].length()`

A method that returns the length of the list.

- **arguments**: `none`
- **returns**: `Number` The length of the list

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.length;

// Output
3
```

## `[List].contains(value)`

A method that checks to see if an element is contained in the list.

- **arguments**: `value` of any type.
- **returns**: `Bool` `true` if given value is found within the list.

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.contains(0);
echo foo.contains(3);

// Output
true
false
```

## `[List].find(value)`

A method that finds a given element in the list and returns the index where the first occurrence of it is found.

- **arguments**: `value` of any type to find within the list.
- **returns**: `Number` The index of the first occurrence of the given element. Returns `null` if value is not found.

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.find(0);
echo foo.find(3);

// Output
0
null
```

## `[List].delete(index)`

A method that finds deletes an element at the given index within the list.

- **arguments**: `index` of type `Number`
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.delete(1)
echo foo;

// Output
[0, 2]
```

## `[List].insert(index, value)`

A method that inserts an element at a given index to the list.

- **arguments**: `value` of any type, `index` of type `Number`. `index` cannot exceed the length of the list.
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.insert(3, "Khan");
echo foo;
foo.insert(0, -2);
echo foo;

// Output
[0, 1, 2, "Khan"]
[-2, 0, 1, 2, "Khan"]
```

## `[List].push(value)`

A method that adds an element to the head of the list. `push()` and `pop()` treat the list like a stack, where the stack top is at the head of the list.

- **arguments**: `value` of any type
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.push(0);
echo foo;

// Output
[0, 0, 1, 2]
```

## `[List].pop()`

A method that "pops" an element to the head of the list. Returns whatever that was popped.

- **arguments**: `none`
- **returns**: `value` of any type.

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.pop();
echo foo;

// Output
0
[1, 2]
```

## `[List].enqueue(value)`

A method that adds an element to the head of the list. `enqueue()` and `dequeue()` treat the list as a queue, where the 'first-in' element is at the tail of the list.

- **arguments**: `value` of any type
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.enqueue(-1);
echo foo;

// Output
[-1, 0, 1, 2]
```

## `[List].dequeue()`

A method that removes and returns the element at the end of the list.

- **arguments**: `none`
- **returns**: `value` of any type

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.dequeue();
echo foo;

// Output
2
[0, 1]
```

## `[List].extend(list)`

A method that extends a given list with another list.

- **arguments**: `list` of type list
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
echo foo.extend([3, 4]);
echo foo;

// Output
[0, 1, 2, 3, 4]
```

## `[List].reverse()`

A method that reverses the list.

- **arguments**: `none`
- **returns**: `none`

**Example**:

```javascript
var foo = [0, 1, 2];
foo.reverse();
echo foo;

// Output
[2, 1, 0]
```
