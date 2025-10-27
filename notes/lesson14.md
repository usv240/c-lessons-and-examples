## LESSON 14 — Performance Optimization: CPU Cache, Memory Layout & SIMD

## Goals

By the end, you’ll know how to:

- Read how a CPU actually executes your code
- Use the cache hierarchy effectively
- Write cache-friendly C code
- Understand alignment, prefetching, and branch prediction
- Apply SIMD (vectorization) to accelerate loops

## 1) CPU Memory Hierarchy

The CPU is much faster than RAM. To keep it fed, hardware uses layers of cache:

```text
CPU Core
  ↓
L1 Cache  (~32 KB, ~1 cycle)
L2 Cache  (~256 KB, ~5–10 cycles)
L3 Cache  (~tens of MB, ~20–50 cycles)
Main Memory (RAM)  (~100 ns)
Disk / SSD (μs–ms)
```

Rule of thumb: the closer to the CPU, the smaller and faster.

## 2) Locality of Reference

Your code runs fast if it keeps using nearby data.

| Type              | Idea                   | Example                      |
|-------------------|------------------------|------------------------------|
| Temporal Locality | Reuse the same data soon | Accessing `arr[i]` again soon |
| Spatial Locality  | Access nearby data     | Looping `arr[i]`, `arr[i+1]` |

Good (sequential access):

```c
for (int i = 0; i < n; i++) sum += arr[i];
```

Bad (cache-unfriendly, random access):

```c
for (int i = 0; i < n; i++) sum += arr[index[i]];
```

## 3) Memory Alignment & Struct Padding

CPUs read memory in aligned chunks (4, 8, 16 bytes). Misaligned access costs extra cycles.

Example:

```c
struct A {
    char  c;   // 1 byte
    int   x;   // 4 bytes
}; // size = 8 bytes (not 5) due to padding
```

Use `#pragma pack(1)` only when you must shrink binary data — misaligned structs slow the CPU.

## 4) Cache-Friendly Design

- Process arrays sequentially
- Group frequently used fields together
- Avoid random pointer chasing
- Use structure of arrays (SoA) instead of array of structs (AoS) if you read fields independently

## 5) Branch Prediction

CPUs guess which way `if` statements will go. A mispredict costs ~10–20 cycles.

Better:

```c
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

if (likely(flag)) fast_path();
else slow_path();
```

`likely()` is a compiler hint macro available in GCC/Clang.

## 6) Measuring Performance

```bash
time ./a.out
perf stat ./a.out          # Linux
sudo powermetrics --samplers tasks --show-process-name | Select-String a.out  # macOS (PowerShell filtering)
```

Metrics:

- cycles, instructions, cache-misses
- IPC = instructions per cycle (higher = better)

## 7) SIMD (Vectorization)

Single Instruction Multiple Data — one instruction processes many elements.

Conceptually:

```c
for (int i = 0; i < 4; i++)
    c[i] = a[i] + b[i];
```

Compiled to one SIMD add (example):

```text
ADDPS xmm0, xmm1   // adds 4 floats at once
```

Using intrinsics:

```c
#include <immintrin.h>

__m128 a = _mm_loadu_ps(A);
__m128 b = _mm_loadu_ps(B);
__m128 c = _mm_add_ps(a, b);
_mm_storeu_ps(C, c);
```

Apple Silicon supports NEON SIMD (ARM vector engine).

## 8) Loop Unrolling & Prefetching

- Unrolling: do more work per iteration to reduce branch overhead
- Prefetching: tell CPU to load data before you need it

```c
__builtin_prefetch(&arr[i + 64], 0, 1);
```

## 9) Compiler Optimizations

Use:

```bash
gcc -O3 -march=native main.c -o main
```

Flags:

- `-O2`/`-O3` → enable loop vectorization
- `-funroll-loops` → loop unrolling
- `-ffast-math` → approximate faster math ops

Inspect assembly:

```bash
objdump -d main | less
```

## 10) Detecting Hot Spots

```bash
perf record ./main
perf report   # see which function consumes most cycles
```

Optimize only the top 5–10% hot paths.

## 11) Apple Silicon Highlights

| Feature           | Notes                                      |
|-------------------|--------------------------------------------|
| Unified Memory    | CPU + GPU share same physical RAM          |
| Big-LITTLE Cores  | Performance vs Efficiency core scheduling  |
| NEON SIMD         | 128-bit vector ops                          |
| AMX Matrix Cores  | For machine learning / media workloads     |

Optimizing for cache locality + vectorization yields large speedups on M‑series chips.

## 12) Checklist for Performance Code

- Access data sequentially
- Align data structures
- Minimize branches inside loops
- Use SIMD where possible
- Profile before optimizing
- Measure cache misses and IPC

## Summary

| Concept           | Purpose                                   |
|-------------------|-------------------------------------------|
| Cache Hierarchy   | Fast access to recent data                |
| Locality          | Use nearby data for speed                 |
| Alignment         | Avoid extra CPU loads                     |
| SIMD              | Process multiple values per instruction   |
| Prefetch & Unroll | Reduce stalls                             |
| Profiling         | Find true bottlenecks                     |