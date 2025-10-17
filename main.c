// File: main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------- Lesson examples (minimal, self-contained) ----------

// L4: Functions — factorial (kept from your original code)
long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

// L1: Hello / compilation sanity
void ex_hello(void) {
    printf("Hello, C!\\n");
}

// L2: Variables, sizes, and format specifiers
void ex_types(void) {
    int i = 10; float f = 3.14f; double d = 2.71828; char c = 'A';
    printf("i=%d f=%.2f d=%.5f c=%c\\n", i, f, d, c);
    printf("sizeof(char)=%zu sizeof(int)=%zu sizeof(float)=%zu sizeof(double)=%zu\\n",
           sizeof(char), sizeof(int), sizeof(float), sizeof(double));
}

// L3: Control flow & loops
void ex_control(void) {
    int x = 7;
    if (x % 2 == 0) printf("%d is even\\n", x); else printf("%d is odd\\n", x);
    printf("for loop: ");
    for (int i = 0; i < 5; ++i) printf("%d ", i);
    printf("\\n");
}

// L4: Functions & pointers (swap)
static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
void ex_functions(void) {
    int a = 3, b = 9; swap(&a, &b); printf("swap -> a=%d b=%d\\n", a, b);
}

// L5: Arrays & pointers
void ex_arrays_pointers(void) {
    int arr[5] = {10,20,30,40,50};
    int *p = arr;
    printf("arr via indexing: ");
    for (int i=0;i<5;++i) printf("%d ", arr[i]);
    printf("\\narr via pointer:  ");
    for (int i=0;i<5;++i) printf("%d ", *(p+i));
    printf("\\n");
}

// L6: Strings & memory — simple strlen
size_t my_strlen(const char *s) { const char *t = s; while (*t) ++t; return (size_t)(t - s); }
void ex_strings(void) {
    char buf[32] = "Apple";
    strcat(buf, "Vision");
    printf("buf='%s' len=%zu\\n", buf, my_strlen(buf));
}

// L7: Dynamic memory — malloc/realloc/free
void ex_dynamic_memory(void) {
    int n = 5;
    int *a = (int*)malloc(n * sizeof(int));
    if (!a) { fprintf(stderr, "malloc failed\n"); return; }
    for (int i=0;i<n;++i) a[i] = i*i;
    printf("malloc: "); for (int i=0;i<n;++i) printf("%d ", a[i]); printf("\\n");
    n = 8;
    int *b = (int*)realloc(a, n * sizeof(int));
    if (!b) { fprintf(stderr, "realloc failed\n"); free(a); return; }
    for (int i=5;i<n;++i) b[i] = i*i;
    printf("realloc: "); for (int i=0;i<n;++i) printf("%d ", b[i]); printf("\\n");
    free(b);
}

// L8: Structs & layout
struct Student { int id; float gpa; char name[16]; };
void ex_structs(void) {
    struct Student s = {1, 3.9f, "Ujwal"};
    printf("Student{id=%d, gpa=%.2f, name=%s} sizeof(Student)=%zu\\n",
           s.id, s.gpa, s.name, sizeof(struct Student));
}

// Original factorial interactive example preserved
void ex_factorial_interactive(void) {
    int n;
    printf("Enter a non-negative integer (0-20): ");
    if (scanf("%d", &n) != 1) { fprintf(stderr, "Invalid input.\\n"); return; }
    if (n < 0 || n > 20) { fprintf(stderr, "Please enter a value between 0 and 20.\\n"); return; }
    printf("%d! = %lld\\n", n, factorial(n));
}

static void print_menu(void) {
    printf("\nC Examples Menu:\n");
    printf(" 1) Hello (Lesson 1)\n");
    printf(" 2) Types & sizes (Lesson 2)\n");
    printf(" 3) Control & loops (Lesson 3)\n");
    printf(" 4) Functions & swap (Lesson 4)\n");
    printf(" 5) Arrays & pointers (Lesson 5)\n");
    printf(" 6) Strings & strlen (Lesson 6)\n");
    printf(" 7) Dynamic memory (Lesson 7)\n");
    printf(" 8) Structs & layout (Lesson 8)\n");
    printf(" 9) Factorial (original)\n");
    printf(" 0) Exit\n");
    printf("Choose: ");
}

int main(void) {
    for (;;) {
        int choice;
        print_menu();
        if (scanf("%d", &choice) != 1) {
            // Clear bad input and exit
            fprintf(stderr, "\nInput error. Exiting.\n");
            return 1;
        }
        switch (choice) {
            case 1: ex_hello(); break;
            case 2: ex_types(); break;
            case 3: ex_control(); break;
            case 4: ex_functions(); break;
            case 5: ex_arrays_pointers(); break;
            case 6: ex_strings(); break;
            case 7: ex_dynamic_memory(); break;
            case 8: ex_structs(); break;
            case 9: ex_factorial_interactive(); break;
            case 0: printf("Bye!\\n"); return 0;
            default: printf("Unknown choice.\\n"); break;
        }
    }
}