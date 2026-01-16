## LESSON 18 — Compiler Internals & Optimization Strategies

(How compilers transform C into machine code and where performance gains hide)

### Simple Introduction

**What is a compiler?** A translator that turns your C code into machine instructions the CPU can run.

**Why learn about it?** Because choosing the right compiler flags and understanding what the compiler does can make your code 10–100x faster. It's not magic—it's just automated optimization.

**Key idea**: You write readable C code. The compiler's job is to translate it into fast machine code while keeping the meaning the same.

## Goals

By the end of this lesson, you'll understand:

- How the compiler translates C through multiple intermediate representations (IR)
- Compiler optimization levels (-O0, -O2, -O3) and what they actually do
- How to inspect generated assembly to spot performance issues
- Loop unrolling, inlining, and other peephole optimizations
- How compiler flags influence code generation on Apple Silicon

## 1) The Compiler Pipeline (Deep Dive)

You saw the basic pipeline in Lesson 1. Here's what happens inside the compiler itself:

```text
C Source Code
    ↓
Lexical Analysis (tokenize)
    ↓
Syntax Analysis (parse AST)
    ↓
Semantic Analysis (type checking)
    ↓
Intermediate Representation (IR generation)
    ↓
Optimization Passes (dead code elimination, loop unrolling, etc.)
    ↓
Code Generation (IR → Assembly)
    ↓
Assembly Code (.s file)
```

Each phase can be inspected or tuned.

## 2) Optimization Levels (Simple Explanation)

**Basic concept**: Higher optimization = slower to compile, faster to run.

GCC and Clang support standardized optimization flags:

| Level | Name             | What It Does                                    | Use Case                    |
|-------|------------------|------------------------------------------------|-----------------------------|
| -O0   | No optimization  | Fast compile, slow runtime; keeps all debug info| Development & debugging     |
| -O1   | Basic            | Small code size, modest speedup                 | Embedded/size-constrained   |
| -O2   | Recommended      | Aggressive optimizations, reasonable compile time| Production default          |
| -O3   | Aggressive       | Maximum speed; larger code; slower to compile  | Performance-critical code   |
| -Os   | Size optimize    | Prioritizes small binaries                      | Embedded systems            |
| -Oz   | Minimal size     | Ultra-small code (Clang only)                   | Severely size-constrained   |
| -Ofast| Fast (non-std)   | Relaxes strict C semantics for speed            | Research/benchmarks only    |

### Example: Same code, different optimizations

Compile the same function three ways:

```bash
gcc -O0 -S func.c
gcc -O2 -S func.c
gcc -O3 -S func.c
```

Then compare the `.s` files — you'll see -O3 produces shorter, faster loops.

## 3) Inspecting Generated Assembly

To see what the compiler actually produced:

```bash
gcc -O2 -S -masm=intel mycode.c -o mycode.s
# OR clang:
clang -O2 -S -masm=intel mycode.c -o mycode.s
```

Then open `mycode.s` and read the assembly. Look for:

- **Dead code**: Unused branches the compiler should have removed
- **Loop unrolling**: Repeated loop body (sign of good optimization)
- **Inlining**: Function code merged into caller (no call overhead)
- **Unnecessary moves**: Extra register-to-register operations

### Example: Function inlining

```c
static inline int add_one(int x) {
    return x + 1;
}

int main(void) {
    int result = add_one(5);
    return result;
}
```

With `-O2`, the compiler inlines `add_one`, so `main` is just:

```asm
mov $5, %eax
add $1, %eax
ret
```

Without `inline` hint or optimization, it would generate a `call` instruction (slower).

## 4) Common Compiler Optimizations

| Optimization        | What It Does                                 | Benefit           |
|---------------------|----------------------------------------------|-------------------|
| Dead code elim.     | Removes unused variables/branches            | Smaller code      |
| Common subexpr elim.| `a = x + y; b = x + y;` → compute once      | Fewer instructions|
| Loop unrolling      | Repeat loop body to reduce branch overhead   | Fewer jumps       |
| Function inlining   | Substitute function call with body           | No call overhead  |
| Constant folding    | `int x = 2 + 3;` becomes `int x = 5;`       | Fewer ops at runtime|
| Strength reduction  | `x * 4` → `x << 2` (shift is faster)         | Faster arithmetic |

## 5) Apple Silicon-Specific Compilation

On Apple Silicon, use these flags to leverage the hardware:

```bash
gcc -O3 -march=armv8.5-a -mtune=apple-m1 mycode.c -o mycode
# OR with clang:
clang -O3 -march=armv8.5-a -mtune=apple-m1 mycode.c -o mycode
```

| Flag                 | Effect                                        |
|----------------------|-----------------------------------------------|
| `-march=armv8.5-a`   | Target ARMv8.5 ISA (all Apple Silicon support)|
| `-mtune=apple-m1`    | Optimize for Apple M1 scheduling/cache       |
| `-march=armv9-a`     | Target newer Apple Silicon (M2+)             |
| `-fno-unroll-loops`  | Disable loop unrolling (if code size matters) |

## 6) Profiling at the Assembly Level

Use `objdump` or `otool` to inspect binaries:

```bash
# Linux/MinGW:
objdump -d mycode -M intel | less

# macOS:
otool -tv mycode | less
```

Look for:

- How many instructions per loop iteration
- Are P-cores vs E-cores being utilized well?
- Does the register allocation look efficient?

## 7) Inline Assembly (When You Need It)

For performance-critical inner loops, inline assembly lets you write the exact instructions you want:

```c
#include <stdio.h>

int fast_multiply(int a, int b) {
    int result;
    __asm__ __volatile__ (
        "mul %1, %2, %0"      // result = a * b
        : "=r" (result)       // output (r = register)
        : "r" (a), "r" (b)    // inputs
    );
    return result;
}

int main(void) {
    int x = fast_multiply(6, 7);
    printf("Result: %d\n", x);
    return 0;
}
```

**Warning**: Inline assembly is compiler-specific, platform-specific, and hard to maintain. Use only when the compiler won't optimize well enough and profiling proves it's the bottleneck.

## 8) Link-Time Optimization (LTO)

LTO allows the compiler to optimize across translation units (files):

```bash
gcc -O3 -flto file1.c file2.c -o program
```

The compiler defers optimization until link time, seeing the full program. This enables:
- Inlining across files
- Dead code elimination (even if called from other files)
- Better register allocation

**Trade-off**: Slower linking, larger binary, but faster runtime.

## 9) Compiler Warnings & Error Detection

Enable all warnings to catch potential performance issues early:

```bash
gcc -Wall -Wextra -Wpedantic -Wshadow -Wuninitialized mycode.c
```

Key warnings:

| Warning          | What It Catches                            |
|------------------|--------------------------------------------|
| `-Wshadow`       | Variable shadows outer scope (confusing)   |
| `-Wuninitialized`| Using uninitialized variables (UB)         |
| `-Wformat`       | Printf format mismatches (can cause crashes)|
| `-Wstrict-overflow` | Signed overflow (undefined behavior)    |

## 10) Benchmark: Optimization Impact

Write a simple loop and measure the impact of different optimization levels:

```c
#include <stdio.h>
#include <time.h>

int main(void) {
    long sum = 0;
    for (long i = 0; i < 1000000000; i++) {
        sum += i;
    }
    printf("Sum: %ld\n", sum);
    return 0;
}
```

Compile and time:

```bash
gcc -O0 -o bench0 bench.c && time ./bench0
gcc -O2 -o bench2 bench.c && time ./bench2
gcc -O3 -o bench3 bench.c && time ./bench3
```

You should see -O3 run **10–100x faster** than -O0 on this loop.

## 11) When Compilers Fail (and You Need Inline Assembly)

Compilers are very good but sometimes:

- Loop doesn't auto-vectorize (SIMD parallelization)
- Register allocation is suboptimal
- Cache behavior isn't what you expect

**Approach:**

1. Profile first (use perf/Instruments)
2. Check if it's actually a bottleneck (90% of time in this 1% of code?)
3. Inspect assembly (`objdump -d`)
4. Try compiler hints (pragmas, function attributes)
5. **Last resort**: Inline assembly

## 12) Summary: Compiler Internals to OS Performance Engineer

| Concept              | Why It Matters for Your Role                         |
|----------------------|------------------------------------------------------|
| Optimization levels  | Know which to use for production vs. debugging       |
| Assembly inspection  | Spot inefficiencies and verify optimizations worked |
| Apple Silicon flags  | Maximize hardware on Apple's current products        |
| LTO                  | Enable for shipping code to catch cross-file issues |
| Profiling-guided opt.| Use perf data to guide compiler decisions            |
| Inline assembly      | Emergency tool when compiler can't optimize enough  |

## Key Takeaways

- **-O2 is the standard** for production; use -O0 for development
- **Read the assembly** to understand what the compiler actually does
- **Compiler warnings are your friend** — fix them early
- **Benchmark before and after** optimizations; don't guess
- **Profile first** before resorting to manual optimization
- **Apple Silicon tuning** via `-march` and `-mtune` unlocks hardware potential

## Homework

1. Take a simple loop-heavy function
2. Compile with `-O0`, `-O2`, `-O3`
3. Compare the `.s` files (use `diff`)
4. Run each binary through `time` and note the speedup
5. Look for unrolled loops and inlined functions

Example starter:

```c
#include <stdio.h>

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(void) {
    int result = fibonacci(30);
    printf("Fib(30) = %d\n", result);
    return 0;
}
```

Compile 3 ways, time each, and examine the assembly. Compare sizes:

```bash
ls -lh fib0 fib2 fib3
```
