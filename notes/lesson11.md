## LESSON 11 — Profiling & Debugging Tools

(How to inspect, trace, and optimize your running programs)

## Goals

By the end of this lesson, you’ll know how to:

- Debug C programs at runtime using gdb / lldb
- Trace system calls with strace or dtruss
- Detect memory leaks and invalid accesses with valgrind / AddressSanitizer
- Measure CPU, memory, and I/O performance using profilers (e.g., perf, Instruments)
- Interpret stack traces, core dumps, and profiling data

## 1) Two Sides of Observability

| Mode      | Purpose            | Example Tools                     |
|-----------|--------------------|-----------------------------------|
| Debugging | Fix wrong behavior | gdb, lldb, printf, core dump      |
| Profiling | Fix slow behavior  | perf, time, Instruments           |

## 2) Debugging Basics with gdb

Compile with symbols and start gdb:

```bash
gcc -g main.c -o main
gdb ./main
```

Inside GDB:

```text
(gdb) break main
(gdb) run
(gdb) next
(gdb) print x
(gdb) bt      # backtrace
(gdb) quit
```

Notes:

- `-g` adds symbol info (function names, lines).
- Use `bt` to see the call stack when a crash happens.
- Core dump files can be loaded later: `gdb ./main core`.

## 3) macOS equivalent — lldb

```bash
lldb ./main
(lldb) breakpoint set -n main
(lldb) run
(lldb) frame variable
(lldb) thread backtrace
```

Same ideas as GDB, but modernized for macOS / Xcode.

## 4) Tracing System Calls with strace

Use when your program interacts with files, network, or the kernel.

```bash
strace ./main
# or only trace file operations
strace -e trace=file ./main
```

Shows lines like:

```text
open("notes.txt", O_RDONLY) = 3
read(3, "Hello", 5)        = 5
write(1, "Hello", 5)       = 5
```

macOS alternative:

```bash
sudo dtruss ./main
```

## 5) Detecting Memory Issues with valgrind

```bash
gcc -g mem.c -o mem
valgrind --leak-check=full ./mem
```

Output example:

```text
HEAP SUMMARY:
   definitely lost: 40 bytes in 2 blocks
```

Fix by ensuring every `malloc()` has a matching `free()`.

Tip: use AddressSanitizer for faster results

```bash
gcc -fsanitize=address -g main.c -o main
./main
```

ASan crashes immediately on invalid memory access with a detailed report.

## 6) Profiling Performance with perf

Linux example:

```bash
perf stat ./main
```

Shows counts of CPU cycles, cache misses, page faults, etc.

For detailed hotspots:

```bash
perf record ./main
perf report
```

macOS: Instruments → Time Profiler.

## 7) Measuring Runtime Quickly

```bash
time ./main
```

Output:

```text
real 0m0.120s
user 0m0.050s
sys  0m0.020s
```

- user = CPU time in your code
- sys = time spent in system calls
- real = wall-clock time

## 8) Finding Hot Functions

Compile with `-pg` and use gprof:

```bash
gcc -pg main.c -o main
./main
gprof ./main gmon.out > report.txt
```

`report.txt` shows which functions took the most time.

## 9) Debugging Multithreaded Code

- Use gdb’s `info threads` and `thread apply all bt`
- Add log IDs or use `pthread_setname_np()` to tag threads
- Detect deadlocks by running under helgrind or ThreadSanitizer:

```bash
valgrind --tool=helgrind ./main
# or with Clang/LLVM
clang -fsanitize=thread -g main.c -lpthread -o main && ./main
```

## 10) Core Files and Crash Analysis

If a program crashes:

```bash
ulimit -c unlimited   # enable core dumps
./main
# Segmentation fault (core dumped)
gdb ./main core
(gdb) bt
```

You’ll see the exact function and line that crashed.

## 11) Apple Instruments (Performance Profiler)

In Xcode: Product → Profile → Instruments → choose:

- Time Profiler → CPU usage per function
- Allocations → memory tracking
- Leaks → detects leaks graphically

Used heavily by Apple Performance Engineering teams.

## 12) Summary Table

| Task         | Tool                | Key Command                              |
|--------------|---------------------|------------------------------------------|
| Run / debug  | gdb, lldb           | `gdb ./a.out`, `break main`              |
| Trace syscalls| strace / dtruss    | `strace -e trace=file ./a.out`           |
| Detect leaks | valgrind / asan     | `valgrind --leak-check=full`             |
| CPU profile  | perf / Instruments  | `perf stat ./a.out`                      |
| Timing       | time                | `time ./a.out`                           |
| Hotspots     | gprof               | `gcc -pg`, `gprof ./a.out`               |
| Thread bugs  | helgrind / tsan     | `valgrind --tool=helgrind`               |