# LESSON 6 — Strings & Memory  
*(Working with Text at the Byte Level)*  

---

## Goals
By the end of this lesson, you’ll be able to:  
- Understand how C stores strings in memory  
- Use and re-implement common string functions (`strlen`, `strcpy`, etc.)  
- Explain how null terminators (`'\0'`) work  
- Manage dynamic strings safely with `malloc` and `free`  
- Detect and avoid buffer overflows  

---

## 1) What Is a String in C?

C has **no built-in string type.**  
A “string” is simply a `char` array ending with a null terminator `'\0'`.

```c
char name[] = "Ujwal";
```

Memory Layout

```text
Index	0	1	2	3	4	5
Value	'U'	'j'	'w'	'a'	'l'	'\0'
```

'\0' marks the end — without it, functions like printf("%s") or strlen will keep reading random memory until they crash.

## 2) Declaring Strings
```c
char name1[] = "Ujwal";        // auto size → 6 bytes (incl '\0')
char name2[10] = "Apple";      // fixed size → rest unused
char *name3 = "Engineer";      // pointer to string literal (READ-ONLY)
```
Important
String literals ("Apple") live in read-only memory.
You cannot modify them:

```c
char *s = "Apple";
s[0] = 'B';   // Segmentation fault
```
Arrays are mutable:

```c
char s[] = "Apple";
s[0] = 'B';   // "Bpple"
```
## 3) String I/O
```c
#include <stdio.h>

int main(void) {
    char str[50];
    printf("Enter your name: ");
    scanf("%49s", str);        // reads until space, bounded
    printf("Hello, %s!\n", str);
    return 0;
}
```

Safer Input

```c
fgets(str, sizeof(str), stdin);
```
Reads up to n – 1 chars and adds \0.

Replaces unsafe old gets().

## 4) Common String Functions (<string.h>)

| Function              | Meaning                        |
|-----------------------|--------------------------------|
| `strlen(s)`           | Length (excluding `\0`)        |
| `strcpy(dest, src)`   | Copy string                    |
| `strncpy(dest, src,n)`| Copy up to n chars             |
| `strcat(dest, src)`   | Append src to dest             |
| `strcmp(a, b)`        | Compare two strings            |
| `strchr(s, c)`        | Find first char                |
| `strstr(s, sub)`      | Find substring                 |

Example
```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char s1[20] = "Apple";
    char s2[] = "Vision";
    strcat(s1, s2);                 // → "AppleVision"
    printf("%s %zu\n", s1, strlen(s1));
}
```

Output

```text
AppleVision 11
```
## 5) How strlen() Really Works
Let’s re-implement it manually.

```c
int my_strlen(const char *s) {
    int count = 0;
    while (*s != '\0') {   // stop at terminator
        count++;
        s++;               // move to next char
    }
    return count;
}
```
Uses pointer arithmetic.

Reads byte-by-byte until '\0'.

Does not count the terminator.

## 6) Dynamic Strings with malloc
Sometimes the string size is unknown ahead of time (e.g., user input).

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *str = malloc(20 * sizeof(char));
    if (!str) return 1;     // check for allocation failure

    strcpy(str, "Performance");
    printf("Word: %s\n", str);

    free(str);              // release heap memory
    return 0;
}
```

Notes

- malloc(n) → returns void* to n bytes of heap memory.
- Always check for NULL before use.
- Always free() when done to avoid leaks.

## 7) Buffer Overflow & Memory Safety
Copying too many chars into a small array overwrites adjacent memory.

```c
char s[5];
strcpy(s, "HelloWorld");   // Overflow!
```
Can crash or corrupt program data.

Safe Way
```c
strncpy(s, "HelloWorld", sizeof(s) - 1);
s[sizeof(s) - 1] = '\0';   // manual terminator
```

This discipline is critical in systems and performance engineering — memory corruption = crashes.

## 8) Pointer to String
```c
char *s = "Media";

printf("%c\n", *(s + 2));   // 'd'

for (const char *p = s; *p != '\0'; p++)
    printf("%c ", *p);
```

Output

```text
d
M e d i a
```

Here p walks through each character in memory, one byte at a time.

## Practice Exercises

1) Implement Your Own strcpy
```c
void my_strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));   // copy including '\0'
}
```

2) Count Vowels
```c
int count_vowels(const char *s) {
    int c = 0;
    for (; *s; s++)
        if (strchr("AEIOUaeiou", *s)) c++;
    return c;
}
```

3) Dynamic Concatenation
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char w1[50], w2[50];
    printf("Enter two words: ");
    scanf("%49s %49s", w1, w2);

    size_t len = strlen(w1) + strlen(w2) + 1;
    char *result = malloc(len);
    if (!result) return 1;

    strcpy(result, w1);
    strcat(result, w2);

    printf("Combined: %s\n", result);
    free(result);
}
```

4) Reverse String in Place
```c
void reverse(char *s) {
    char *start = s;
    char *end = s + strlen(s) - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}
```

Debug & Safety Tips

- Always initialize pointers → `char *p = NULL;`
- After `free(p);`, set `p = NULL;`
- Prefer `fgets()` over `scanf("%s")` for strings with spaces
- Avoid magic numbers → use `sizeof(array)` or `strlen()`

Run your program with:

```bash
valgrind ./a.out
```

to catch invalid reads/writes and leaks.

## Key Takeaways

| Concept              | Summary                                     |
|----------------------|---------------------------------------------|
| String               | Char array ending with `\0`                 |
| String Literal       | Read-only memory — don’t modify             |
| strlen, strcpy, strcat | Core library helpers in `<string.h>`       |
| Dynamic String       | Use `malloc` + `free` carefully             |
| Buffer Overflow      | Copy within bounds (`strncpy`)              |
| Pointer to String    | Iterate using `*p` and `p++`                |
| Safety Tools         | valgrind, compiler warnings                 |

## Next Lesson Preview — Lesson 7 : Dynamic Memory Management in Depth
We’ll dive deeper into the heap and see how C actually allocates and frees memory:

- malloc, calloc, realloc, free — internal differences
- Heap organization and fragmentation
- Memory alignment and block metadata
- Build your own mini allocator
- Tools → valgrind, asan, and Instruments for leak detection