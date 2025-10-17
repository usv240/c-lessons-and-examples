# LESSON 3 — Control Flow & Loops

---

## Learning Goals

By the end of this lesson you will be able to:

- Write conditional code using `if`, `else if`, `else`
- Use `switch` statements for multiple choices
- Write loops using `for`, `while`, and `do-while`
- Control loops with `break`, `continue`, and `return`
- Understand how these translate to CPU branches under the hood  

---

## 1) `if`, `else if`, `else`

C executes blocks of code based on a **condition**.  
Any non-zero value is **true**, and zero is **false**.

### Example 1 — Basic Temperature Check
```c
#include <stdio.h>

int main(void) {
    int temp = 35;

    if (temp > 40)
        printf("It's too hot!\n");
    else if (temp >= 30)
        printf("It's warm.\n");
    else
        printf("It's cool.\n");

    return 0;
}
Output

```text
It's warm.
```

Only one branch runs. Inside the CPU, this becomes a comparison and a conditional jump (e.g., cmp, jle).

Example 2 — Even or Odd
```c
int num;
printf("Enter a number: ");
scanf("%d", &num);

if (num % 2 == 0)
    printf("Even\n");
else
    printf("Odd\n");
```
Key idea: Every if condition is an expression. `(num % 2 == 0)` evaluates to 1 (true) or 0 (false).

## 2) `switch` Statement
Use switch when you have many discrete cases.

Example — Day of the Week
```c
int day = 3;

switch (day) {
    case 1: printf("Monday\n"); break;
    case 2: printf("Tuesday\n"); break;
    case 3: printf("Wednesday\n"); break;
    default: printf("Invalid day\n"); break;
}
```
Output

```text
Wednesday
```

Important:
- `break;` prevents “fall-through” to the next case.
- Works only with integral types (int, char, enum).

Example — Mini Calculator
```c
int a = 8, b = 4, choice = 2;

switch (choice) {
    case 1: printf("Sum = %d\n", a + b); break;
    case 2: printf("Difference = %d\n", a - b); break;
    case 3: printf("Product = %d\n", a * b); break;
    case 4: printf("Quotient = %.2f\n", (float)a / b); break;
    default: printf("Invalid choice\n");
}
```
## 3) Loops — Repeating Until a Condition Stops It
C offers three main loop types.
All run until a condition becomes false (0).

### while Loop
Checks the condition before running the body.

```c
int i = 0;
while (i < 5) {
    printf("%d ", i);
    i++;
}
```
Output: `0 1 2 3 4`

If `i` starts ≥ 5, the loop never executes.

### do … while Loop
Runs the body at least once, then checks the condition.

```c
int i = 0;
do {
    printf("%d ", i);
    i++;
} while (i < 5);
```
Output: `0 1 2 3 4`
Even if `i = 10`, the loop would print 10 once.

### for Loop
Compact form of initialization + condition + update.

```c
for (int i = 0; i < 5; i++) {
    printf("%d ", i);
}
```
Same as:

```c
int i = 0;
while (i < 5) {
    printf("%d ", i);
    i++;
}
```
### Infinite Loops
```c
for (;;) { /* forever */ }
while (1) { /* forever */ }
```
Used in servers or embedded systems that run continuously.

## 4) Loop Control Keywords

| Keyword  | Meaning                                       |
|----------|-----------------------------------------------|
| break;   | Exit the current loop or switch immediately   |
| continue;| Skip current iteration → jump to next         |
| return;  | Exit the entire function                      |

Example — Skip and Break
```c
for (int i = 0; i < 10; i++) {
    if (i == 3) continue;  // skip 3
    if (i == 8) break;     // stop loop
    printf("%d ", i);
}
```
Output: `0 1 2 4 5 6 7`

## 5) Nested Loops
Loops inside loops = useful for grids, tables, matrices.

```c
for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 2; j++) {
        printf("(%d,%d) ", i, j);
    }
}
```
Output: `(1,1) (1,2) (2,1) (2,2) (3,1) (3,2)`

## 6) How It Works Under the Hood
When compiled, loops and conditionals become jump instructions at the assembly level.

| C Construct | Typical Assembly Equivalent                 |
|-------------|---------------------------------------------|
| if (x > 5)  | cmp then conditional jump (jle, jg, etc.)   |
| for / while | cmp + jmp back to loop start                |
| break       | unconditional jump (jmp) out of loop        |

Example snippet:

```asm
cmp $5, %eax   ; compare i with 5
jl loop_start  ; jump back if less
```
For performance engineers, branch prediction and pipeline efficiency are key — that’s why loop structure and condition ordering matter.

## Practice Tasks
Task 1 — Even or Odd
Prompt the user for an integer and print whether it’s even or odd.

```c
int n;
printf("Enter a number: ");
scanf("%d", &n);
if (n % 2 == 0)
    printf("Even\n");
else
    printf("Odd\n");
```
Task 2 — Sum of 1 to N
Compute 1 + 2 + ... + n using a for loop.

```c
int n, sum = 0;
printf("Enter n: ");
scanf("%d", &n);

for (int i = 1; i <= n; i++)
    sum += i;

printf("Sum = %d\n", sum);
```
Task 3 — Multiplication Table
```c
int n;
printf("Enter a number: ");
scanf("%d", &n);

for (int i = 1; i <= 10; i++)
    printf("%d × %d = %d\n", n, i, n * i);
```
Task 4 — Menu Calculator
Use a switch to choose operations.

```c
int choice, a, b;
printf("1.Add 2.Subtract 3.Multiply 4.Divide\n");
printf("Enter choice and two numbers: ");
scanf("%d %d %d", &choice, &a, &b);

switch (choice) {
    case 1: printf("Sum = %d\n", a + b); break;
    case 2: printf("Diff = %d\n", a - b); break;
    case 3: printf("Prod = %d\n", a * b); break;
    case 4: printf("Quotient = %.2f\n", (float)a / b); break;
    default: printf("Invalid choice\n");
}
```
## Debugging Tips
Always compile with:

```bash
gcc file.c -Wall -Wextra -pedantic -o output
```

Test edge cases:

- Loop boundaries (0, 1, max values)
- Switch default case
- Division by zero guard

Print intermediate values to understand flow:

```c
printf("Loop i = %d\n", i);
```
## Key Takeaways

| Concept                 | Summary                                  |
|-------------------------|------------------------------------------|
| if/else                 | Choose code based on conditions          |
| switch                  | Clean multi-branch for integers          |
| while / for / do-while  | Repeat work until condition fails        |
| break/continue/return   | Control execution inside loops           |
| Nested loops            | Combine loops for grids or tables        |
| CPU View                | All conditions → jumps and branches      |

## Next Lesson Preview — Lesson 4 : Functions & Scope
You’ll learn to modularize your programs by creating functions:

- Defining and calling your own functions
- Parameter passing (by value vs by reference)
- Return types, inline and static functions
- Call stack visualization

Stay curious and keep practicing loops — they’re the heartbeat of every program.