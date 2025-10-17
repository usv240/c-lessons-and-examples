# LESSON 8 — Structures & Memory Layout  
*(Organizing data, understanding alignment, and efficiency)*  

---

## Goals
By the end of this lesson, you’ll be able to:

- Define and use `struct` types  
- Understand how data is laid out in memory  
- Explain padding, alignment, and `sizeof()`  
- Work with pointers and nested structs  
- Dynamically allocate and free structs on the heap  
- Optimize struct layout for performance  

---

## 1) What Is a Structure?

A **structure** groups multiple variables (of possibly different types) into a single logical unit.

```c
#include <stdio.h>

struct Student {
    int id;
    float gpa;
    char name[30];
};

int main(void) {
    struct Student s1 = {1, 3.9, "Ujwal"};
    printf("ID: %d, GPA: %.1f, Name: %s\n", s1.id, s1.gpa, s1.name);
}
```

Output

```text
ID: 1, GPA: 3.9, Name: Ujwal
```

Think of a struct as a custom data type—like building your own “mini-object” in plain C.

## 2) Accessing Structure Members
Use:

- . (dot) → for direct variables
- -> (arrow) → for struct pointers

```c
struct Student s = {2, 4.0, "Arpita"};
struct Student *ptr = &s;

printf("%s\n", s.name);     // dot operator
printf("%s\n", ptr->name);  // arrow operator (same result)
```

Shortcut:
ptr->x is equivalent to (*ptr).x.

## 3) Memory Layout Inside Structs
Example:

```c
struct Example {
    char a;
    int b;
    char c;
};
```

Possible Layout (64-bit CPU)

```text
Address    Data  Bytes
0x1000     a     1
0x1001     pad   3
0x1004     b     4
0x1008     c     1
0x1009     pad   3
```

Total size = 12 bytes (only 6 are actual data).
Padding is added so that int b begins at a 4-byte boundary → aligned memory access.

## 4) sizeof() and Alignment
```c
#include <stdio.h>
struct A { char c; int i; };
struct B { int i; char c; };

int main(void) {
    printf("sizeof(struct A) = %zu\n", sizeof(struct A));
    printf("sizeof(struct B) = %zu\n", sizeof(struct B));
}
```

Example Output (on 64-bit)

```text
sizeof(struct A) = 8
sizeof(struct B) = 8
```
Even though order changed, alignment keeps both at 8 bytes.
The compiler ensures each member starts at an address multiple of its natural size (int → 4 bytes).

## 5) Reducing Padding (Memory Optimization)
Reorder members from largest → smallest to minimize wasted space.

```c
struct Optimized {
    int id;
    char code;
}; // compact – no extra gaps
```

Optional → Tight Packing

```c
#pragma pack(1)
struct Tight {
    char a;
    int b;
    char c;
};
#pragma pack()
```
⚠️ Using #pragma pack(1) removes padding but may hurt performance on some CPUs (unaligned access = extra cycles).

## 6) Nested Structures
You can include one struct inside another:

```c
struct Address {
    char city[20];
    int zip;
};

struct Student {
    int id;
    float gpa;
    struct Address addr;
};

int main(void) {
    struct Student s = {1, 3.8, {"Charlotte", 28223}};
    printf("%s %d\n", s.addr.city, s.addr.zip);
}
```
✅ Real-world modeling → students have addresses, devices have specs, etc.

## 7) Pointer to Struct (Dynamic Allocation)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Device {
    int id;
    char name[20];
};

int main(void) {
    struct Device *d = malloc(sizeof(struct Device));
    d->id = 101;
    strcpy(d->name, "iPhone");
    printf("%d %s\n", d->id, d->name);
    free(d);
}
```

Output

```text
101 iPhone
```
💡 Allocated on the heap, accessed via pointer (->).

## 8) Arrays of Structures
```c
struct Student group[3] = {
    {1, 3.8, "Ujwal"},
    {2, 3.7, "Arpita"},
    {3, 3.9, "Alex"}
};

for (int i = 0; i < 3; i++)
    printf("%d %s\n", group[i].id, group[i].name);
```
Each element is a complete struct.
Common pattern for tables, databases, and buffers.

## 9) Struct Pointers in Functions
Pass struct pointers instead of full structs to avoid copies.

```c
void printStudent(const struct Student *s) {
    printf("Name: %s, GPA: %.2f\n", s->name, s->gpa);
}
```

```c
int main(void) {
    struct Student a = {10, 3.95, "Jules"};
    printStudent(&a);
}
```
Efficient for large structs (passes 8-byte pointer vs copying entire block).

## 10) Struct Alignment and CPU Cache
Well-aligned = faster CPU access.
Misaligned = extra cycles or hardware traps (especially on ARM / Apple Silicon).

Example:

```c
struct Bad {
    char a;
    int b;
};

struct Good {
    int b;
    char a;
};
```
✅ struct Good is smaller and cache-friendly (less padding).
Apple engineers routinely reorder members to match cache lines.

## 11) Dynamic Struct Arrays (Practical Use)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Employee {
    int id;
    char name[30];
};

int main(void) {
    int n;
    printf("How many employees? ");
    scanf("%d", &n);

    struct Employee *emp = malloc(n * sizeof(struct Employee));

    for (int i = 0; i < n; i++) {
        printf("Enter ID and Name: ");
        scanf("%d %s", &emp[i].id, emp[i].name);
    }

    printf("\nEmployee List:\n");
    for (int i = 0; i < n; i++)
        printf("%d %s\n", emp[i].id, emp[i].name);

    free(emp);
}
```
✅ Pattern used in real apps → dynamic databases, object lists, sensor records, etc.

## 12) Visualizing Struct Layout with sizeof()
```c
#include <stdio.h>

struct Mixed {
    char c;
    int i;
    double d;
};

int main(void) {
    printf("char: %zu, int: %zu, double: %zu\n",
           sizeof(char), sizeof(int), sizeof(double));
    printf("struct Mixed: %zu\n", sizeof(struct Mixed));
}
```

Output (typical 64-bit)

```text
char: 1, int: 4, double: 8
struct Mixed: 16
```
Padding aligns int and double to their boundaries.

## Practice Exercises
1) Student Database
Store 3 students in an array of structs and print the topper (highest GPA).

2) Dynamic Employee Allocator
Ask for n, allocate n employees dynamically, input their details, print and free.

3) Size and Padding Visualizer
Create a struct with char, int, and double, print sizeof() and sketch memory layout.

## Debug and Optimization Tips
Compile with -Wall -Wextra -pedantic.

Use sizeof(struct) instead of hard-coded numbers when allocating.

Reorder members to reduce padding (L2 cache efficiency matters).

Avoid tight packing unless working with binary protocols.

Prefer passing struct pointers in functions for performance.

## Key Takeaways

| Concept            | Summary                                       |
|--------------------|-----------------------------------------------|
| struct             | Custom type that groups related variables      |
| Member Access      | . for values, -> for pointers                  |
| Padding and Alignment | Added bytes for CPU-friendly addresses     |
| sizeof()           | Includes padding                               |
| Optimization       | Reorder members or pack carefully              |
| Nested Structs     | Useful for hierarchical data                   |
| Dynamic Allocation | `malloc(sizeof(struct X))` + `free()`          |
| Cache Awareness    | Aligned structs load faster                    |
| Safe Pattern       | Pass struct pointers to functions              |

## Next Lesson Preview — Lesson 9 : File I/O and System Calls
In the next lesson, your programs start talking to the Operating System:

- fopen, fread, fprintf, fclose
- Low-level file descriptors (open, read, write)
- Binary files and struct serialization
- Buffering and I/O efficiency