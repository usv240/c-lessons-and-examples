# LESSON 1 — Understanding What C Is & Setting Up

## Goal of This Lesson
By the end, you will:
- Understand what makes **C** special  
- Know how **C code runs** on your computer  
- Be able to **compile and run** your first C program  
- Understand the **anatomy** of a simple C program  

---

## What Is C?
C is a **low-level, compiled, procedural** programming language.  
It’s the foundation for:
- Operating systems (Linux, parts of macOS kernel)
- Embedded systems and device drivers
- Performance-critical applications (e.g., Apple’s Core Media)

When you write C, you’re talking **directly to CPU + memory** — no runtime VM, no garbage collector.

**Why C is special**

| Aspect     | Description                         | Example                 |
|-----------|-------------------------------------|-------------------------|
| Low-level | Direct memory access via pointers   | `int *p = &x;`          |
| Compiled  | Produces machine instructions       | Runs natively on CPU    |
| Procedural| Functions & control flow            | `int add(int a, int b)` |
| Portable  | Recompile to run on many OS/CPUs    | Linux, macOS, Windows   |

---

## The Compilation Pipeline
When you run:

```bash
gcc file.c -o file
```

Four major steps happen under the hood:

| Step | Tool   | Description                        | Output                       |
|------|--------|------------------------------------|------------------------------|
| 1    | cpp    | Expands #include, #define, #ifdef  | file.i                       |
| 2    | gcc -S | Translates C to assembly           | file.s                       |
| 3    | as     | Assembles to machine code          | file.o                       |
| 4    | ld     | Links your code with libraries     | Executable (a.out or custom) |

Try it, step by step:

```bash
gcc -E hello.c > hello.i        # 1) Preprocessed C
gcc -S hello.c                  # 2) Assembly (.s)
gcc -c hello.c                  # 3) Object file (.o)
gcc hello.o -o hello            # 4) Link to executable
objdump -d hello.o | less       # Inspect machine code
```

Your First C Program

```c
#include <stdio.h>  // Standard I/O header

int main(void) {
    printf("Hello, World!\n");
    return 0;       // 0 = success
}
```

Compile & Run

```bash
gcc hello.c -o hello
./hello
```

Output

```
Hello, World!
```

Anatomy of a C Program

| Component            | Purpose                                                                  |
|----------------------|--------------------------------------------------------------------------|
| `#include <stdio.h>` | Preprocessor directive — imports declarations for printf, scanf, etc.    |
| `int main(void)`     | Program entry point; must return int                                     |
| `{ ... }`            | Function body (statements)                                               |
| `printf()`           | Prints to console (linked from C stdlib at link time)                    |
| `return 0;`          | Exit status for OS (0 success, non-zero error)                           |

Concept Spotlight — What Happens in Memory
When you run the program:

- OS loads the executable into RAM
- CPU starts executing at `main`
- Your process gets:
  - Code/Text segment → compiled instructions
  - Data segment → globals/statics
  - Stack → function calls, local variables
  - Heap → dynamic memory (malloc/free)
- Program ends → OS receives the exit code (0 or non-zero)

Tiny example

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int a = 5;                  // stack
    int *p = malloc(sizeof *p); // heap
    if (!p) return 1;           // allocation failed
    *p = 10;
    printf("%d %d\n", a, *p);
    free(p);                    // always free
    return 0;
}
```

Mini Exercise #1 — Variables & Printing

```c
#include <stdio.h>

int main(void) {
    int x = 10, y = 20, z = 30;
    int sum = x + y + z;
    float avg = sum / 3.0f;

    printf("Sum = %d\n", sum);
    printf("Average = %.2f\n", avg);
    return 0;
}
```

Try:

- Change `int` → `float` and observe output/format
- Add `char grade = 'A';` and print with `%c`

Common format specifiers

| Type              | Specifier | Example                    |
|-------------------|-----------|----------------------------|
| int               | %d        | `printf("%d", 5);`         |
| float/double      | %f        | `printf("%f", 3.14);`      |
| char              | %c        | `printf("%c", 'A');`       |
| C string (char *) | %s        | `printf("%s", "Hello");`  |

Mini Exercise #2 — Watch Compilation in Action

```bash
gcc -E hello.c > step1.i
gcc -S hello.c
gcc -c hello.c
objdump -d hello.o | less
```

Notice: `printf` isn’t in your `.o`; it’s resolved at link time.

Good Habits Early On
Compile with warnings:

```bash
gcc -Wall -Wextra -pedantic file.c -o file
```

Read man pages locally: `man 3 printf`, `man 3 malloc`

Poke at sizes:

```c
printf("sizeof(int) = %zu\n", sizeof(int));
printf("sizeof(double) = %zu\n", sizeof(double));
```

Bonus Clarifications You’ll See Often

Return codes from `main`

- `return 0;` → success
- `return 1;` (non-zero) → error (shell sees it via `$?` on Linux/macOS)

Valid `main` signatures (per the C standard)

```c
int main(void);
int main(int argc, char *argv[]);
```

Avoid `void main()` — not standard, undefined behavior on many systems.

String literals vs char arrays

```c
const char *s = "Apple"; // read-only memory; do not modify s[i]
char t[] = "Apple";      // writable array on stack; safe to modify t[i]
```

Dynamic strings with malloc/calloc

```c
char *buf = malloc(64);          // uninitialized
char *zbuf = calloc(64, 1);      // zero-initialized ('\0') — great for strings
strcpy(zbuf, "Hello");
free(zbuf);
```

Common Errors to Intentionally Trigger & Learn

Missing semicolon

```c
int x = 5
```

Compiler says: error: expected ';' after expression

Wrong format specifier

```c
float f = 1.5f;
printf("%d\n", f);  // mismatch
```

Compiler warning: format specifies int but argument has type float

Misspelled `main`

```c
int mian(void) { return 0; } // typo
```

Linker error: undefined reference to 'main'

Returning 1 instead of 0

```c
return 1; // program ran, but signals error to shell/scripts
```

Check exit code:

```bash
echo $?
# 1
```

Key Takeaways

| Concept             | What You Learned                                      |
|---------------------|--------------------------------------------------------|
| Compilation stages  | Preprocess → Compile → Assemble → Link                 |
| Program structure   | `#include`, `main`, `{}`, `return`                     |
| Memory model        | Code, Data, Stack, Heap                                |
| Exit status         | 0 success, non-zero error                              |
| Printing types      | `%d`, `%f`, `%c`, `%s`                                 |
| Best practice       | Always compile with warnings (-Wall -Wextra -pedantic) |

Homework Practice
Write a program that:

- Declares three integers
- Computes their sum and average
- Prints both

Then:

- Change a variable to `float`
- Add a `char` and a `char[]` string and print them
- Recompile with `-Wall -Wextra -pedantic` and fix any warnings

Starter

```c
#include <stdio.h>

int main(void) {
    int a = 4, b = 7, c = 9;
    int sum = a + b + c;
    float avg = sum / 3.0f;

    char grade = 'A';
    char name[] = "Ujwal";

    printf("Sum = %d, Avg = %.2f\n", sum, avg);
    printf("Grade = %c, Name = %s\n", grade, name);
    return 0;
}
```

Phase 1 Checklist
You can now:

- Write & run a C program
- Explain the compilation pipeline
- Describe `main`, `return`, and `#include`
- Visualize memory (stack vs heap)
- Notice & fix basic compile errors/warnings

Next Up: Lesson 2 — Variables, Data Types & Operators