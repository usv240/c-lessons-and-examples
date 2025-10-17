# C lessons and examples

Learn modern C from first principles to systems-level topics with runnable code. This repo is a hands-on, human-friendly study path: short notes, clear mental models, and a single `main.c` you can run to try ideas quickly.

## What this repo contains

- `main.c` — a menu-driven examples harness you can compile and run.
- `docs/lesson01.md` … `docs/lesson09.md` — focused, GitHub-friendly lessons you can read in order or dip into as needed.
- `notes_` (and/or `notes/`) — consolidated notes if you prefer one long read.

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

- [Lesson 01: Getting started with C](./docs/lesson01.md)
- [Lesson 02: Types, variables, and operators](./docs/lesson02.md)
- [Lesson 03: Control flow (if/else, loops, switch)](./docs/lesson03.md)
- [Lesson 04: Functions, scope, and the call stack](./docs/lesson04.md)
- [Lesson 05: Arrays and pointers](./docs/lesson05.md)
- [Lesson 06: Strings and memory](./docs/lesson06.md)
- [Lesson 07: Dynamic memory management](./docs/lesson07.md)
- [Lesson 08: Structures and memory layout](./docs/lesson08.md)
- [Lesson 09: Structures (recap) and prep for file I/O](./docs/lesson09.md)

If you like a single document, see [notes_](./notes_) or the [notes/](./notes/) directory.

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
