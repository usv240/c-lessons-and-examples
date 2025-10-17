# LESSON 4 — Functions & Scope

---

## Learning Goals
By the end of this lesson, you’ll be able to:

- Declare, define, and call your own functions  
- Understand how arguments are passed (by value vs by reference)  
- Explain scope, lifetime, and storage classes  
- Visualize the **call stack** during execution  
- Organize multi-file C projects  

---

## 1) What Is a Function?

A **function** is a named, reusable block of code that performs one specific task.

```c
#include <stdio.h>

int add(int a, int b) {       // Function definition
    return a + b;
}

int main(void) {              // main() is also a function
    int sum = add(5, 7);      // Function call
    printf("Sum = %d\n", sum);
    return 0;
}
Output

```text
Sum = 12
```

Functions help you modularize large programs into smaller, logical pieces.

## 2) Function Structure

| Part             | Meaning                          | Example                   |
|------------------|----------------------------------|---------------------------|
| Return Type      | Type of value returned to caller | int, float, void          |
| Name             | Function identifier              | add, printDetails         |
| Parameters       | Input variables from caller      | (int a, int b)            |
| Body             | Actual code inside `{ }`         | `{ return a + b; }`       |
| Return Statement | Sends result back to caller      | `return sum;`             |

Example — Void Function
```c
void greet(void) {
    printf("Hello!\n");
}

int main(void) {
    greet();   // no return, no parameters
    return 0;
}
```
## 3) Declaration vs Definition vs Call

| Stage                     | Meaning                                  | Example                                  |
|---------------------------|------------------------------------------|------------------------------------------|
| Declaration / Prototype   | Tells the compiler the function signature| `int add(int, int);`                      |
| Definition                | Contains actual code                     | `int add(int x, int y){ return x + y; }` |
| Call                      | Executes the function                    | `sum = add(3, 4);`                        |

Declarations usually go into header files (`.h`).
Definitions go into source files (`.c`).

## 4) Call Stack — What Really Happens
When one function calls another, C creates a stack frame in memory.

Example — main() calls add()
```c
int add(int a, int b) {
    return a + b;
}

int main(void) {
    int result = add(5, 7);
    printf("%d\n", result);
}
```

Under the hood:

CPU pushes a = 5, b = 7 onto the stack.

Jumps to add() function.

Creates a new stack frame (local vars live here).

Executes code, returns 12.

Frame is popped and control returns to main().

Simplified diagram:

```
|------------| ← top of stack (main locals)
| a = 5      |
| b = 7      |
| return →12 |
|------------|
```
This understanding helps you debug crashes (like stack overflow in recursion).

## 5) Pass-by-Value vs Pass-by-Reference
Pass-by-Value (default)
The function receives a copy of the variable.
Changes inside the function do not affect the original.

```c
void change(int x) { x = 10; }

int main(void) {
    int a = 5;
    change(a);
    printf("%d\n", a);   // 5 — unchanged
}
```
Each function gets its own copy on the stack.

Pass-by-Reference (using Pointers)
You pass the address of the variable, so the callee can modify it directly.

```c
void change(int *ptr) { *ptr = 10; }

int main(void) {
    int a = 5;
    change(&a);
    printf("%d\n", a);   // 10 — changed
}
```
Conceptually:

- Pass-by-value → copy of data
- Pass-by-reference → copy of address

This idea powers most system-level APIs and performance-sensitive C libraries.

## 6) Scope and Lifetime

| Variable Type | Where Visible             | Lifetime                     | Example                     |
|---------------|---------------------------|------------------------------|-----------------------------|
| Local         | Inside its function       | Created/destroyed each call  | `int x;` inside `main()`    |
| Global        | Everywhere after decl.    | Entire program run           | `int count;` outside funcs  |
| Static Local  | Only inside its function  | Preserved between calls      | `static int x;`             |

Example — Static Local Variable
```c
#include <stdio.h>

void counter(void) {
    static int count = 0; // keeps its value
    count++;
    printf("Count: %d\n", count);
}

int main(void) {
    counter(); // 1
    counter(); // 2
    counter(); // 3
}
```
Each time counter() runs, the same count variable is reused.

## 7) Storage Classes

| Keyword  | Meaning                                  | Where Used        | Notes                             |
|----------|-------------------------------------------|-------------------|-----------------------------------|
| auto     | Default for locals                        | Inside functions  | Usually omitted                   |
| register | Suggest store in CPU register             | Locals            | Compiler may ignore               |
| static   | Preserve value / limit visibility         | Locals or globals | Common in helper functions        |
| extern   | Exists in another translation unit        | Globals           | Used in multi-file projects       |

Example — Using `extern`

file1.c

```c
int global = 42;
```

file2.c

```c
#include <stdio.h>
extern int global;  // tells compiler: defined elsewhere

int main(void) {
    printf("%d\n", global);
    return 0;
}
```

Compile together:

```bash
gcc file1.c file2.c -o app
```
## 8) Inline & Static Functions

| Keyword | Purpose                                        | Example                                     | Notes                          |
|---------|------------------------------------------------|---------------------------------------------|--------------------------------|
| inline  | Suggest embedding code to avoid call overhead  | `inline int square(int x){return x*x;}`     | For small math helpers         |
| static  | Limit function visibility to current file      | `static void helper(void){/*...*/}`         | Prevents name conflicts        |

You can combine them:

```c
static inline int min(int a, int b) { return a < b ? a : b; }
```

— used in performance-critical header utilities.

## Mini Exercises
1) Swap Two Numbers — Value vs Reference
```c
void swap_value(int a, int b) {
    int temp = a;
    a = b;
    b = temp;
    printf("Inside swap_value: a=%d b=%d\n", a, b);
}

void swap_ref(int *pa, int *pb) {
    int temp = *pa;
    *pa = *pb;
    *pb = temp;
}

int main(void) {
    int x = 5, y = 10;
    swap_value(x, y);
    printf("After swap_value: x=%d y=%d\n", x, y);

    swap_ref(&x, &y);
    printf("After swap_ref: x=%d y=%d\n", x, y);
}
```
2) Recursive Factorial
```c
int fact(int n) {
    if (n == 0) return 1;
    return n * fact(n - 1);
}

int main(void) {
    int n = 5;
    printf("Factorial(%d) = %d\n", n, fact(n));
}
```
Output: `Factorial(5) = 120`

Each recursive call creates a new stack frame → pop when returning.

3) Global Counter
```c
#include <stdio.h>

int counter = 0;  // global variable

void inc(void) { counter++; }
void dec(void) { counter--; }

int main(void) {
    inc(); inc(); dec();
    printf("Final Counter = %d\n", counter);
}
```
## Organizing Multi-File Projects
Typical layout:
```
project/
 ├── main.c
 ├── math_utils.c
 ├── math_utils.h
```

`math_utils.h`

```c
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

int add(int, int);
int square(int);

#endif
```

`math_utils.c`

```c
#include "math_utils.h"
int add(int a, int b){ return a + b; }
int square(int x){ return x * x; }
```

`main.c`

```c
#include <stdio.h>
#include "math_utils.h"

int main(void){
    printf("%d\n", add(2, 3));
    printf("%d\n", square(4));
    return 0;
}
```

Compile together:

```bash
gcc main.c math_utils.c -o app
```
## Debug & Best Practices
Always compile with warnings:

```bash
gcc -Wall -Wextra -pedantic file.c -o file
```
Keep functions small and focused (1 purpose = 1 function)

Use meaningful names (calculateTotal, not calc)

Comment what a function does, not how

Avoid too many globals — prefer parameters

## Key Takeaways

| Concept                        | Summary                                              |
|--------------------------------|------------------------------------------------------|
| Function                       | Reusable unit of code with optional inputs/outputs   |
| Declaration vs Definition vs Call | Tells compiler, implements logic, executes it       |
| Call Stack                     | Each call creates a temporary frame on stack         |
| Pass-by-Value                  | Copy of data passed (default)                        |
| Pass-by-Reference              | Address passed using pointers                        |
| Scope & Lifetime               | Local, Global, Static variables                      |
| Storage Classes                | auto, register, static, extern                       |
| Inline & Static Functions      | Optimize and limit visibility in large projects      |

## Next Lesson Preview — Lesson 5 : Arrays & Pointers
Coming up next you’ll learn about:

- Declaring and initializing arrays
- Iterating and indexing them in loops
- Pointers and how they relate to arrays
- const, void *, and pointer safety
- Memory diagrams showing how arrays live in RAM