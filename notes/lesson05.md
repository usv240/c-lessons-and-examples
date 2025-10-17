# LESSON 5 — Arrays & Pointers  
*(a.k.a. The Real Power of C)*  

---

## Goals
By the end of this lesson, you’ll be able to:

- Understand how arrays are laid out in memory (contiguous blocks)  
- Know what a pointer really is and how it works  
- Do pointer arithmetic safely  
- Use `const` and `void *` pointers correctly  
- See how arrays and pointers are connected inside the compiler  
- Practice through diagrams + hands-on code  

---

## 1) What Is an Array?

An **array** is a **block of contiguous memory** that holds elements of the same type.

```c
int numbers[5] = {10, 20, 30, 40, 50};
```

In Memory (assuming 4-byte ints)

```text
Address → 0x100 | 10 |
           0x104 | 20 |
           0x108 | 30 |
           0x10C | 40 |
           0x110 | 50 |
```

Each element is stored right next to the previous one.
The compiler calculates addresses like this:

```text
address_of(numbers[i]) = base_address + (i * sizeof(int))
```

Access

```c
numbers[0];   // 10
numbers[4];   // 50
```

Arrays give you random access: you can reach any element in O(1) time.

## 2) Array Access and Indexing

```c
#include <stdio.h>
int main(void) {
    int arr[3] = {5, 10, 15};
    for (int i = 0; i < 3; i++) {
        printf("%d ", arr[i]);
    }
    return 0;
}
```

Output

```text
5 10 15
```

C does not perform bounds checking.
If you write arr[5], you’ll read/write memory that doesn’t belong to you → undefined behavior.
That’s why understanding memory safety is vital in systems programming.

## 3) What Is a Pointer?
A pointer is a variable that stores the address of another variable.

```c
int x = 42;
int *p = &x;

printf("x = %d\n", x);
printf("Address of x = %p\n", &x);
printf("Pointer p holds = %p\n", p);
printf("Value at p = %d\n", *p);
```

Example Output

```text
x = 42
Address of x = 0x7ffee3a21b4c
Pointer p holds = 0x7ffee3a21b4c
Value at p = 42
```

| Symbol | Meaning                                      |
|--------|----------------------------------------------|
| &x     | address of x                                 |
| *p     | value stored at that address (dereference)   |

Think of a pointer as a “note with a house address written on it”.

## 4) Arrays and Pointers — They’re Connected
In most expressions, arrays automatically turn into pointers to their first element.

```c
int a[3] = {1, 2, 3};
int *p = a;              // same as &a[0]
printf("%d %d\n", *p, *(p+1));   // 1 2
```
At compile time, a[i] is actually defined as:

```c
*(a + i)
```
That’s why both indexing and pointer arithmetic work identically.

## 5) Pointer Arithmetic
When you increment a pointer, it moves to the next element, not just +1 byte.

```c
int a[3] = {10, 20, 30};
int *p = a;

printf("%d\n", *p);   // 10
p++;
printf("%d\n", *p);   // 20
```
If sizeof(int) = 4, then p++ actually adds 4 to the address.

Example Visualization

```text
p = 0x100
*p = 10
p++ → 0x104
*p = 20
```
## 6) Pointers & Arrays Together
```c
#include <stdio.h>

int main(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    int *ptr = arr;

    for (int i = 0; i < 5; i++) {
        printf("arr[%d] = %d | *(ptr+%d) = %d\n",
               i, arr[i], i, *(ptr+i));
    }
    return 0;
}
```

Output

```text
arr[0]=10 | *(ptr+0)=10
arr[1]=20 | *(ptr+1)=20
...
```

Confirms that arrays and pointers are two views of the same memory.

## 7) const and void * Pointers

const pointer — protect the value

```c
int x = 5;
const int *p = &x;
*p = 10;   // ❌ error – cannot modify through *p
```
You can still change p itself (p = &y;), but not what it points to.

void * pointer — generic pointer type
Used for “untyped” memory, especially with malloc.

```c
void *ptr = malloc(10 * sizeof(int));
if (ptr != NULL) {
    int *arr = (int*)ptr;
    arr[0] = 100;
    printf("%d\n", arr[0]);
    free(ptr);
}
```
Since void * has no type, you must cast it back before use.

## 8) Pointer to Pointer
A pointer can also store the address of another pointer.

```c
int x = 5;
int *p = &x;
int **pp = &p;

printf("%d\n", **pp);  // 5
```
Useful for:

- Returning multiple results from a function
- Managing arrays of strings (char **argv in main)

Analogy: x → value, p → address of x, pp → address of p.

## 9) Common Pointer Pitfalls

| Mistake                     | What Happens                         |
|----------------------------|--------------------------------------|
| Using uninitialized pointer| Random garbage address → crash       |
| Forgetting free()          | Memory leak                           |
| Double free()              | Undefined behavior                    |
| Dereferencing NULL         | Segmentation fault                    |
| Out-of-bounds pointer math | Memory corruption                     |

Best habits

```c
int *p = NULL;
if (p) printf("%d", *p);   // safe check
```
Run with:

```bash
valgrind ./a.out
```
to detect invalid accesses and leaks.

## Practice Exercises

1) Reverse Array Using Pointers
```c
void reverse(int *arr, int size) {
    int *start = arr;
    int *end = arr + size - 1;
    while (start < end) {
        int tmp = *start;
        *start = *end;
        *end = tmp;
        start++; end--;
    }
}
```
Test in main() → verify elements are reversed in place.

2) Average of Array (Using Pointers Only)
```c
float average(int *arr, int n) {
    int sum = 0;
    for (int *p = arr; p < arr + n; p++)
        sum += *p;
    return (float)sum / n;
}
```
3) Pointer Playground — Addresses + Values
```c
int arr[4] = {2, 4, 6, 8};
for (int i = 0; i < 4; i++) {
    printf("&arr[%d] = %p  Value = %d  PointerExpr = %p\n",
           i, &arr[i], arr[i], arr + i);
}
```
Observe how each address increases by sizeof(int).

4) Swap Function (Pointer Version)
```c
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(void) {
    int x = 5, y = 10;
    swap(&x, &y);
    printf("x=%d y=%d\n", x, y);  // x=10 y=5
}
```

Debug tips

- Always initialize pointers (= NULL)
- Use printf("%p") to trace addresses
- Prefer sizeof(*ptr) instead of hard-coded types inside malloc

```c
int *p = malloc(5 * sizeof *p);
```

- Free only once and set pointer to NULL after free()

## Key Takeaways

| Concept                   | Summary                                      |
|---------------------------|----------------------------------------------|
| Array                     | Contiguous memory for same type elements     |
| Pointer                   | Variable that stores an address              |
| * / &                     | Dereference / Address-of                     |
| Pointer Arithmetic        | Moves by element size, not bytes             |
| Array–Pointer Relationship| `a[i] == *(a+i)`                              |
| const / void *            | Protect data / generic memory                |
| Pointer to Pointer        | Multi-level address reference                |
| Common Pitfalls           | NULL, leaks, double free, out-of-bounds      |
| Tools                     | valgrind for memory safety                   |

## Next Lesson Preview — Lesson 6 : Strings & Dynamic Memory
You’ll learn how C stores and handles text:

- Character arrays and null terminators
- strlen, strcpy, strcmp internals
- Memory layout of string literals
- Buffer overflow and secure string handling
- malloc and free for dynamic strings