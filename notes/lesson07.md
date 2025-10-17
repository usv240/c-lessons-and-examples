# LESSON 7 — Dynamic Memory Management  
*(Heap, malloc, fragmentation, and profiling)*  

---

## Goals
By the end of this lesson, you’ll be able to:

- Understand how the heap differs from the stack  
- Use `malloc`, `calloc`, `realloc`, and `free` correctly  
- Explain fragmentation and alignment  
- Detect memory leaks and dangling pointers  
- Profile your program with Valgrind or AddressSanitizer  
- Write a simple custom memory tracker  

---

## 1) Stack vs Heap — Two Main Memory Regions

| Property | **Stack** | **Heap** |
|-----------|-----------|----------|
| Allocation | Automatic (at compile-time) | Manual (at runtime) |
| Lifetime | Until function returns | Until you call `free()` |
| Managed by | Compiler | Programmer |
| Example | `int x = 10;` | `int *p = malloc(sizeof(int));` |

### Memory Layout (Conceptual)
+-------------------+ ← High memory addresses
| Command-line args |
| Environment vars |
| Heap (grows up) | ← malloc(), calloc()
| ... free space ...|
| Stack (grows down)| ← function calls
+-------------------+ ← Low memory

```text
```

Think of the stack as a short-term “scratch pad”,  
and the heap as long-term “dynamic storage”.

---

## 2) malloc(), calloc(), realloc(), free()

### malloc — Allocate Uninitialized Memory
```c
int *ptr = malloc(5 * sizeof(int));
Allocates 5 × sizeof(int) bytes.

Returns void* → cast optional in C ((int*) for readability).

Memory contains garbage values until you write to it.

### calloc — Allocate and Zero-Initialize
```c
int *ptr = calloc(5, sizeof(int));
Allocates 5 × sizeof(int) bytes,

Initializes all bytes to 0.
Great for predictable results (especially before loops).

### realloc — Resize Existing Block
```c
ptr = realloc(ptr, 10 * sizeof(int));
Expands or shrinks the existing allocation.

May move the block elsewhere (copies old data).

Returns new pointer → always reassign it safely:

```c
int *temp = realloc(ptr, newSize);
if (temp) ptr = temp;
```

### free — Release Memory
```c
free(ptr);
ptr = NULL;   // prevent dangling pointer
```
Gives the block back to the heap allocator.

Always pair every malloc / calloc / realloc with a matching free().

## 3) Behind the Scenes — How Heap Allocation Works
When you call malloc(size) →
1️⃣ Your program asks the OS for memory via brk() or mmap().
2️⃣ The C library manages free blocks internally.
3️⃣ You get a pointer to the usable data region.

Each block usually stores hidden metadata:

```text
[ size | flags | data ... ]
          ↑
          pointer returned to you
```
Writing beyond the allocated size corrupts this metadata → crash.

## 4) Example — Dynamic Array
```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n;
    printf("Enter number of elements: ");
    scanf("%d", &n);

    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < n; i++) arr[i] = i * i;
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);

    free(arr);
    return 0;
}
```
Flow

User enters n

Program allocates space dynamically

Uses and then frees it

## 5) Common Mistakes and Their Consequences

| Mistake             | Effect                        |
|---------------------|-------------------------------|
| Forgetting free()   | Memory leak                   |
| Freeing twice       | Undefined behavior            |
| Using after free()  | Dangling pointer → crash      |
| Allocating too little | Buffer overflow             |
| Ignoring malloc failure | Segmentation fault        |

Always initialize pointers to NULL and check before free().

## 6) Detecting Memory Issues

Using Valgrind (Linux/macOS)

```bash
gcc program.c -g -o program
valgrind --leak-check=full ./program
```
Example output:

```text
HEAP SUMMARY:
    definitely lost: 16 bytes in 1 blocks
    indirectly lost: 0 bytes
```
→ “definitely lost” means a leak (not freed).

Using AddressSanitizer (ASan)

```bash
gcc -fsanitize=address -g program.c -o program
./program
```
Detects:

Out-of-bounds access

Use-after-free

Leaks (with summary)

Great for macOS and CI pipelines.

## 7) Fragmentation and Alignment
Over time, repeated malloc/free calls leave tiny gaps → fragmentation.

| Address | Block     | Size |
|---------|-----------|------|
| 0x1000  | allocated | 16 B |
| 0x1010  | allocated | 32 B |
| 0x1030  | free      | —    |
| 0x1040  | allocated | 24 B |

Allocators also align blocks to word boundaries (usually 8 bytes on 64-bit systems) for performance.

Efficient alignment reduces CPU load on memory access — important for high-speed video/audio buffers.

## 8) Mini Project — Custom Memory Tracker
```c
#include <stdio.h>
#include <stdlib.h>

static size_t total_alloc = 0;

void* track_malloc(size_t size) {
    total_alloc += size;
    return malloc(size);
}

void track_free(void *ptr, size_t size) {
    total_alloc -= size;
    free(ptr);
}

int main(void) {
    int *arr = track_malloc(5 * sizeof(int));
    track_free(arr, 5 * sizeof(int));
    printf("Total allocated: %zu bytes\n", total_alloc);
}
```
✅ Helps visualize how much heap you use — a mini foundation for writing your own allocator.

## Practice Tasks

Task 1 — Dynamic Matrix
Allocate a 2D array using pointers to pointers:

```c
int **matrix = malloc(rows * sizeof(int*));
for (int i = 0; i < rows; i++)
    matrix[i] = malloc(cols * sizeof(int));
```
Fill it, print it, then free() each row and finally matrix.

Task 2 — String Expander with realloc
Start with a small buffer, grow as the user types:

```c
char *buf = malloc(10);
int len = 0, ch;
while ((ch = getchar()) != '\n') {
    buf[len++] = ch;
    if (len % 10 == 0)
        buf = realloc(buf, len + 10);
}
buf[len] = '\0';
printf("%s\n", buf);
free(buf);
```
Task 3 — Memory Leak Experiment
Remove free() on purpose, then run:

```bash
valgrind --leak-check=full ./program
```
Observe leak summary and fix it by adding free().

Bonus Example — Safe Reallocation Pattern
```c
int *data = malloc(5 * sizeof(int));
int *temp = realloc(data, 10 * sizeof(int));
if (temp) data = temp;
else {
    free(data);
    fprintf(stderr, "Realloc failed\n");
}
```
Always use a temporary pointer to avoid losing the original block if reallocation fails.

## Debug and Profiling Tips
Compile with warnings → gcc -Wall -Wextra -pedantic

Run with ASan or Valgrind often

Set freed pointers to NULL

Use sizeof(*ptr) instead of type name inside malloc() calls:

```c
int *arr = malloc(n * sizeof *arr);
```

Add logging around alloc/free to trace memory usage in large projects.

## Key Takeaways

| Concept         | Summary                                          |
|-----------------|--------------------------------------------------|
| Stack vs Heap   | Automatic vs manual lifetime                     |
| malloc / calloc | Allocate uninitialized / zeroed memory           |
| realloc         | Resize existing block                            |
| free            | Release heap memory                              |
| Fragmentation   | Small holes between blocks → inefficiency        |
| Alignment       | Addresses rounded to word boundaries             |
| Common Bugs     | Leaks, double free, dangling pointers            |
| Tools           | Valgrind and ASan for debugging and profiling    |
| Safe Pattern    | Always check return values and nullify pointers  |

## Next Lesson Preview — Lesson 8 : Structures & Memory Layout
Coming up:

- Defining and nesting structs
- Memory alignment and padding
- Using sizeof() and layout analysis
- Pointers to structs and dynamic allocation
- Mini Project → Student Database using Structures