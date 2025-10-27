# C lessons and examples

Learn modern C from first principles to systems-level topics with runnable code. This repo is a hands-on, human-friendly study path: short notes, clear mental models, and a single `main.c` you can run to try ideas quickly.

## What this repo contains

- `main.c` — a menu-driven examples harness you can compile and run.
- `notes/lesson01.md` … `notes/lesson17.md` — focused, GitHub-friendly lessons you can read in order or dip into as needed.
- `notes_` (and/or `notes/`) — consolidated notes if you prefer one long read.
- `hello.c` — a tiny program used in the debugging lesson to practice gdb/LLDB.
- [`notes/GDB commands.md`](./notes/GDB%20commands.md) — a concise GDB commands cheat sheet.

## Quick start (Windows, PowerShell)

From the workspace folder (path has spaces and an ampersand), run these commands in PowerShell:

```powershell
# 1) Go to the project folder
cd "C:\macOS project\C & Objective C"

# 2) Compile with MinGW GCC (adjust the gcc path if needed)
& "C:\MinGW\bin\gcc.exe" -std=c11 -Wall -Wextra -Wpedantic -O0 -g ".\main.c" -o ".\main.exe"

# 3) Run the examples harness
& ".\main.exe"
```

Tip: You can also use the VS Code build task named "C/C++: gcc.exe build active file".

## Read the lessons

- [Lesson 01: Understanding what C is & setting up](./notes/lesson01.md)
- [Lesson 02: Variables, data types & operators](./notes/lesson02.md)
- [Lesson 03: Control flow & loops](./notes/lesson03.md)
- [Lesson 04: Functions & scope](./notes/lesson04.md)
- [Lesson 05: Arrays & pointers](./notes/lesson05.md)
- [Lesson 06: Strings & memory](./notes/lesson06.md)
- [Lesson 07: Dynamic memory management](./notes/lesson07.md)
- [Lesson 08: Structures & memory layout](./notes/lesson08.md)
- [Lesson 09: File I/O & system calls](./notes/lesson09.md)
- [Lesson 10: Processes & threads](./notes/lesson10.md)
- [Lesson 11: Profiling & debugging tools](./notes/lesson11.md)
- [Lesson 12: OS internals & virtual memory](./notes/lesson12.md)
- [Lesson 13: Advanced concurrency & synchronization](./notes/lesson13.md)
- [Lesson 14: Performance optimization (cache, memory layout, SIMD)](./notes/lesson14.md)
- [Lesson 15: Apple Silicon Architecture Deep Dive](./notes/lesson15.md)
- [Lesson 16: Media Systems & HLS Internals](./notes/lesson16.md)
- [Lesson 17: Capstone Project — Mini Streaming Engine in C](./notes/lesson17.md)

## How to use the examples harness

- Compile and run `main.exe`.
- In the menu, pick a number to run an example (hello world, types, control flow, arrays/pointers, strings, dynamic memory, structs, etc.).
- Read the corresponding lesson to understand the concept, then tweak the code and rerun.

## Why this structure works

- Small, focused lessons with runnable examples build real intuition.
- A single program (`main.c`) keeps the feedback loop fast.
- Notes explain the "why", examples show the "how".

## Troubleshooting

- If PowerShell complains about `&`, make sure you wrap paths in quotes and use the call operator as shown above.
- If GCC isn’t found, install MinGW and update the path in the compile command.
- If you hit warnings, that’s good—compile with `-Wall -Wextra -Wpedantic` and fix them as you go.

## License

Personal learning materials. Use freely; attribution appreciated.
