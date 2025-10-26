## LESSON 12 — Operating System Internals & Virtual Memory

(How memory, processes, and the kernel actually work under the hood)

## Goals

By the end of this lesson, you’ll be able to:

- Explain what virtual memory is and why it exists
- Describe how the OS and hardware cooperate to isolate processes
- Define page tables, the MMU, and page faults
- Explain CPU scheduling and context switching
- Describe what happens when memory “runs out” (swapping)

## 1) Physical vs Virtual Memory

Simple view:

- The CPU doesn’t access real memory directly.
- Each process sees its own private address space, as if it owns the whole RAM.

Technical view:

- Physical memory (RAM) = real hardware
- Virtual memory = what each process thinks it has
- The Memory Management Unit (MMU) translates virtual → physical addresses
- The kernel’s virtual memory system manages this illusion.

Example mapping:

```text
virtual:  0x7ffc1234
      ↓  (via page table)
physical frame: 0x001234
```

Each mapping represents a page (usually 4 KB of memory).

## 2) Why Virtual Memory Exists

| Benefit    | Explanation                                  |
|------------|----------------------------------------------|
| Isolation  | One process can’t touch another’s memory.     |
| Protection | Bugs or attacks can’t corrupt the kernel.     |
| Convenience| Each program sees continuous memory.          |
| Efficiency | OS can move data in/out of RAM dynamically.   |

## 3) Paging and Page Tables

- The OS divides memory into fixed-size pages (commonly 4 KB).
- Each process has a page table — a mapping of virtual pages to physical frames.

| Virtual Page | Physical Frame    | Flags   |
|--------------|-------------------|---------|
| 0x0001       | 0x2001            | R/W     |
| 0x0002       | 0x3010            | R/O     |
| 0x0003       | None (on disk)    | Swapped |

- If a page isn’t in memory, it’s stored temporarily on disk (swap).
- When accessed, a page fault occurs and the OS loads it back into RAM.

## 4) The Role of the MMU (Memory Management Unit)

The MMU (hardware inside the CPU):

- Uses the page table to translate addresses
- Enforces permissions (e.g., user vs kernel mode)
- Raises exceptions for invalid memory (segfaults)

That’s why a bad pointer causes a segmentation fault — the MMU blocks access to unmapped pages.

```c
int *p = NULL;
*p = 5;  // segmentation fault
```

## 5) Kernel vs User Space

```text
+----------------------+
| Kernel Space         | ← full privileges
| (device drivers, FS) |
+----------------------+
| User Space           | ← restricted (your code)
| (your process)       |
+----------------------+
```

The OS kernel runs in privileged mode and directly manages:

- Memory mapping
- File systems
- Process creation and scheduling
- Hardware drivers

Your C code runs in user mode and must make system calls to request these services.

## 6) Context Switching

When the CPU switches from one process to another, it:

- Saves current CPU registers, stack pointer, and program counter
- Loads the next process’s context (its registers, page table)
- Updates the MMU to use the new process’s address space

This happens thousands of times per second — called time slicing.

## 7) Page Faults

When a process accesses memory not currently in RAM:

1. The MMU triggers a page fault interrupt.
2. The OS checks:
   - Valid but swapped out? → Load it from disk.
   - Invalid (bad pointer)? → Kill the process (segfault).

Page faults enable flexible virtual memory — but too many cause thrashing (slow performance).

## 8) Swapping and the Page Cache

When RAM fills up:

- Least recently used pages are swapped to disk (swap space).
- Recently accessed files are cached in the page cache (RAM) for faster reads.

This is why file I/O often looks “instant” — it’s served from cached pages, not disk.

## 9) CPU Scheduling Overview

The scheduler decides which process/thread runs next.

| Policy            | Description                                        |
|-------------------|----------------------------------------------------|
| Round Robin       | Each process gets equal CPU time slices.           |
| Priority-based    | Important processes run first.                     |
| Multilevel Queues | Interactive tasks prioritized over batch jobs.     |

Each time slice is ~1–10 ms. Between slices, the CPU performs a context switch.

## 10) Example Flow: Reading a File in Virtual Memory Context

1. User program calls `fread()`
2. System call → kernel (VFS)
3. Kernel reads file blocks → page cache
4. Page cache pages mapped into process address space
5. MMU translates addresses as code reads bytes

Your process never sees “real” RAM; it sees virtual pages mapped by the OS.

## 11) Address Space Layout Example

```text
+------------------+  High address
| Stack (grows ↓)  |
|------------------|
| Heap (malloc)    |
|------------------|
| Data (globals)   |
|------------------|
| Code/Text (funcs)|
+------------------+  Low address
```

Each process has its own copy of this structure. The MMU ensures these virtual addresses point to different physical frames for different processes.

## 12) Tools to Visualize Memory

| Tool          | Purpose                        | Example/Notes                          |
|---------------|--------------------------------|----------------------------------------|
| `pmap <pid>`  | Show memory map of process     | Which regions are mapped where         |
| `vm_stat`     | Show paging stats on macOS     | Pageins, pageouts                      |
| `top` / `htop`| Show memory use live           | Resident + Virtual memory              |
| `valgrind`    | Detect invalid accesses        | Reports leaks, invalid frees           |

## 13) Real-World Relevance (Apple Context)

Apple’s Darwin kernel (in macOS, iOS, visionOS):

- Uses Mach VM (virtual memory system)
- Supports copy-on-write (COW) memory (used by `fork`, snapshots, APFS)
- Optimized for unified memory (CPU + GPU share the same physical memory on Apple Silicon)

Understanding this directly helps with debugging and optimizing CoreMedia, HLS, and multithreaded Apple frameworks.

## Summary

| Concept        | Description                                   |
|----------------|-----------------------------------------------|
| Virtual Memory | Illusion of continuous memory for each process|
| Page           | 4 KB unit of virtual memory                   |
| Page Table     | Mapping from virtual → physical               |
| MMU            | Hardware that enforces mappings               |
| Page Fault     | Triggered when accessing unmapped memory      |
| Swapping       | Moving inactive pages to disk                 |
| Scheduler      | Decides which process runs next               |
| Context Switch | Swap CPU state between processes              |


