# LESSON 2 — Variables, Data Types & Operators

---

## Learning Goals
By the end of this lesson, you’ll be able to:

- Understand what a variable really is in memory  
- Know all primitive data types and their typical sizes  
- Use the right format specifiers in `printf` / `scanf`  
- Apply arithmetic, relational, logical, and bitwise operators  
- Practice with short hands-on exercises  

---

## What Is a Variable in C?

A variable is just a named spot in memory that stores a value.  
When you write:

```c
int age = 25;
```

The compiler:

- Reserves 4 bytes (on most 64-bit systems) in RAM
- Stores the binary form of 25 in those bytes
- Associates that memory address with the label `age`

Essentially:

- `age` → name
- `25` → value
- memory location → where it actually lives

Example 1 — Print Value and Address

```c
#include <stdio.h>

int main(void) {
    int age = 25;
    printf("Value: %d, Address: %p\n", age, (void*)&age);
    return 0;
}
```

Output (example):

```text
Value: 25, Address: 0x7ffee12f8b4c
```

`&age` gives the address of the variable in memory. Later you’ll learn how pointers use this address directly.

Example 2 — Changing Values

```c
int score = 80;
score = score + 10;
printf("%d\n", score);   // 90
```

C copies values into memory, so each variable stores its own copy.

## Data Types and Memory Sizes
Different data types store different amounts of information.

| Type           | Typical Size (bytes) | Range                             | Format Specifier |
|----------------|-----------------------|-----------------------------------|------------------|
| char           | 1                     | −128 → 127                        | `%c` or `%d`     |
| unsigned char  | 1                     | 0 → 255                           | `%u`             |
| short          | 2                     | −32 768 → 32 767                  | `%hd`            |
| int            | 4                     | −2 147 483 648 → 2 147 483 647    | `%d`             |
| unsigned int   | 4                     | 0 → 4 294 967 295                 | `%u`             |
| long           | 8 (on 64-bit)         | larger range                      | `%ld`            |
| float          | 4                     | ≈ 6 decimal digits precision      | `%f`             |
| double         | 8                     | ≈ 15 decimal digits precision     | `%lf`            |
| _Bool          | 1                     | 0 or 1                            | `%d`             |

Check on your machine:

```c
printf("sizeof(int) = %zu bytes\n", sizeof(int));
```

Use `%zu` because `sizeof` returns an unsigned `size_t`.

Deep Concept: “Type = How CPU Interprets Memory”
The same bits can mean different things depending on the type.

| Stored Bits | Interpreted As | Result     |
|-------------|----------------|------------|
| 0100 0001   | char           | 'A'        |
| 0100 0001   | int            | 65 decimal |

So types tell the compiler how to read the raw memory.

Example 3 — Type Meaning

```c
int x = 65;
char c = x;  // same bits
printf("%d %c\n", x, c); // 65 A
```

Same data, different interpretation.

## Format Specifiers Cheat Sheet

| Purpose   | Specifier | Example                       |
|-----------|-----------|-------------------------------|
| Integer   | `%d`/`%i` | `printf("%d", 10);`           |
| Float     | `%f`      | `printf("%.2f", 3.14);`       |
| Double    | `%lf`     | `printf("%lf", 2.718);`       |
| Character | `%c`      | `printf("%c", 'A');`          |
| String    | `%s`      | `printf("%s", "C");`         |
| Address   | `%p`      | `printf("%p", (void*)&x);`    |
| Unsigned  | `%u`      | `printf("%u", 42u);`          |
| Long int  | `%ld`     | `printf("%ld", 1000000L);`    |

Example 4

```c
int x = 10;
float y = 3.14f;
char z = 'A';
printf("x=%d, y=%.2f, z=%c\n", x, y, z);
```

Output

```text
x=10, y=3.14, z=A
```

## Operators in C
C supports a rich set of operators. Let’s go category by category.

### Arithmetic Operators
`+ - * / %`

```c
int a = 10, b = 3;
printf("%d %d %d %d %d\n", a+b, a-b, a*b, a/b, a%b);
```

Output → `13 7 30 3 1`

If both operands are `int`, `/` does integer division (fractions truncated). Use float for true division:

```c
printf("%.2f\n", 10.0 / 3); // 3.33
```

### Relational Operators
`== != > < >= <=`

Return 1 (true) or 0 (false).

```c
int x = 5, y = 10;
printf("%d %d\n", x == y, x < y); // 0 1
```

### Logical Operators
`&&` (AND), `||` (OR), `!` (NOT)

```c
int x = 3, y = 5;
if (x > 0 && y > 0)
    printf("Both positive\n");
```

Truth table (for `&&`):

| x | y | x && y |
|---|---|--------|
| 0 | 0 | 0      |
| 1 | 0 | 0      |
| 0 | 1 | 0      |
| 1 | 1 | 1      |

### Assignment Operators
`=, +=, -=, *=, /=, %=`

```c
int n = 5;
n += 2;  // n = n + 2 → 7
n *= 3;  // n = n * 3 → 21
```

### Bitwise Operators
Work directly on the binary bits of integers — vital for optimization and hardware code.

- `&` (AND) — Example: `5 & 3` → `1` (`0101 & 0011 = 0001`)
- `|` (OR) — Example: `5 | 3` → `7` (`0101 | 0011 = 0111`)
- `^` (XOR) — Example: `5 ^ 3` → `6` (`0110`)
- `~` (NOT) — Example: `~5` → depends on `int` size (bitwise inverse)
- `<<` (Left shift) — Example: `5 << 1` → `10` (`1010`)
- `>>` (Right shift) — Example: `5 >> 1` → `2` (`0010`)

Shifts multiply/divide by 2 each step for unsigned integers.

### Increment & Decrement
`++` and `--`

| Form | Description                          |
|------|--------------------------------------|
| ++x  | Pre-increment — increase, then use   |
| x++  | Post-increment — use, then increase  |

```c
int x = 5;
printf("%d %d\n", ++x, x++); // 6 6
printf("%d\n", x);           // 7
```

## Mini Exercises

Exercise 1 — Type & Size Explorer
Write a program that prints the size of each type using `sizeof()`.

```c
#include <stdio.h>

int main(void) {
    printf("Type\tSize(bytes)\n");
    printf("char\t%zu\n", sizeof(char));
    printf("int\t%zu\n", sizeof(int));
    printf("float\t%zu\n", sizeof(float));
    printf("double\t%zu\n", sizeof(double));
    return 0;
}
```

Try it on your machine and compare outputs — they differ by OS/CPU.

Exercise 2 — Simple Calculator

```c
#include <stdio.h>

int main(void) {
    int a, b;
    printf("Enter two integers: ");
    scanf("%d %d", &a, &b);

    printf("Sum = %d\n", a + b);
    printf("Diff = %d\n", a - b);
    printf("Prod = %d\n", a * b);
    printf("Quotient = %.2f\n", (float)a / b);
    printf("Remainder = %d\n", a % b);
    return 0;
}
```

Experiment:

- Swap to `float a, b` and change specifiers to `%f`
- Observe difference in division results

Exercise 3 — Bit Playground

```c
#include <stdio.h>

int main(void) {
    unsigned int x = 5, y = 3;
    printf("x & y = %u\n", x & y);
    printf("x | y = %u\n", x | y);
    printf("x ^ y = %u\n", x ^ y);
    printf("~x = %u\n", ~x);
    return 0;
}
```

Predict each in binary before running!

## Debugging Tip
Always compile with maximum warnings:

```bash
gcc file.c -Wall -Wextra -pedantic -o output
```

This warns you about:

- Type mismatches (int ↔ float)
- Unused variables
- Precision loss or missing format specifiers

A clean build (no warnings) is your first step to correct and performant C code.

## Key Takeaways

| Concept            | Summary                                         |
|--------------------|-------------------------------------------------|
| Variable           | Named storage in memory                         |
| Data types         | Define size + interpretation of bits            |
| `sizeof()`         | Finds byte size of any type                     |
| Format specifiers  | Tell printf/scanf how to read/write             |
| Operators          | Arithmetic, relational, logical, bitwise, assignment |
| Compile flags      | `-Wall -Wextra -pedantic` → see all warnings    |

## What’s Next — Lesson 3 Preview
Next lesson covers:

- Control Flow: `if`, `else`, `switch`, loops
- How C decides what runs and when
- Writing small interactive programs with conditions