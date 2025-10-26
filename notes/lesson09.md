## LESSON 9 — File I/O & System Calls

(Bridging your C code and the OS file system)

## Goals

By the end of this lesson, you’ll be able to:

- Understand how C handles files (streams vs descriptors)
- Explain the difference between high-level I/O (fopen) and low-level I/O (open)
- Read and write both text and binary files
- Work with fseek, ftell, and buffering
- Serialize and deserialize structs
- Describe how system calls connect user code to the kernel

## 1) What Is File I/O?

File I/O = Input/Output operations with data stored in files on disk.

In C, you can:

- Read from a file
- Write to a file
- Append, seek, and close files

## 2) High-Level File I/O (stdio.h)

Core functions:

| Function               | Purpose                      |
|------------------------|------------------------------|
| fopen()                | Open a file                  |
| fclose()               | Close file                   |
| fprintf() / fscanf()   | Formatted write/read         |
| fgets() / fputs()      | String read/write            |
| fread() / fwrite()     | Binary read/write            |
| fseek(), ftell()       | Move and query file pointer  |

Example 1 — Write to file

```c
#include <stdio.h>

int main() {
    FILE *fp = fopen("output.txt", "w");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    fprintf(fp, "Hello, Apple Engineer!\n");
    fclose(fp);
    return 0;
}
```

Creates a file `output.txt` with text inside.

Example 2 — Read from file

```c
#include <stdio.h>

int main() {
    FILE *fp = fopen("output.txt", "r");
    if (!fp) return 1;

    char buffer[100];
    while (fgets(buffer, sizeof(buffer), fp))
        printf("%s", buffer);

    fclose(fp);
    return 0;
}
```

Example 3 — Append mode

```c
FILE *fp = fopen("log.txt", "a");
fprintf(fp, "New log entry\n");
fclose(fp);
```

Opens file and writes at the end.

## 3) File Modes

| Mode | Meaning                         |
|------|---------------------------------|
| "r"  | Read                            |
| "w"  | Write (truncate existing file)  |
| "a"  | Append                          |
| "r+" | Read + Write                    |
| "w+" | Write + Read (overwrite)        |
| "a+" | Append + Read                   |

## 4) fseek(), ftell(), rewind()

These control where in the file you read/write.

```c
fseek(fp, 0, SEEK_END);  // move to end
long pos = ftell(fp);    // current position
rewind(fp);              // back to start
```

Use this to measure file size or skip bytes.

## 5) Reading/Writing Binary Data

Example — Save structs to a file

```c
#include <stdio.h>
#include <stdlib.h>

struct Record {
    int id;
    float score;
};

int main() {
    struct Record r1 = {1, 9.8};

    FILE *fp = fopen("data.bin", "wb");
    fwrite(&r1, sizeof(r1), 1, fp);
    fclose(fp);

    struct Record r2;
    fp = fopen("data.bin", "rb");
    fread(&r2, sizeof(r2), 1, fp);
    fclose(fp);

    printf("ID: %d, Score: %.1f\n", r2.id, r2.score);
}
```

Output:

```text
ID: 1, Score: 9.8
```

This is binary serialization — how structured data is stored efficiently.

## 6) Low-Level I/O (System Calls)

For performance-critical tasks, you use POSIX system calls instead of stdio buffering.

```c
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("raw.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return 1;

    char msg[] = "Low-level I/O\n";
    write(fd, msg, strlen(msg));
    close(fd);
}
```

Key functions:

| Function | Purpose            |
|----------|--------------------|
| open()   | Open file descriptor|
| read()   | Read bytes         |
| write()  | Write bytes        |
| lseek()  | Move pointer       |
| close()  | Close descriptor   |

File descriptors are small integers (0 = stdin, 1 = stdout, 2 = stderr).

## 7) Buffered vs Unbuffered I/O

| Type       | Example          | Behavior                                      |
|------------|------------------|-----------------------------------------------|
| Buffered   | fprintf, fread   | Data stored in memory before write (efficient) |
| Unbuffered | write()          | Direct kernel call, slower for small writes   |

Buffered I/O is faster for small data; unbuffered gives more control.

## 8) Error Handling

Always check for errors:

```c
if (fp == NULL) {
    perror("fopen failed");
    exit(1);
}
```

Use errno for detailed error codes (from `<errno.h>`).

## Practice Exercises

1) Text File Copy

- Open `source.txt`, copy all lines into `dest.txt`.

2) Binary Database

- Store multiple `struct Student` records in a binary file.
- Implement “add”, “read all”, and “search by ID”.

3) Log Writer

- Continuously append logs with timestamps to a file using `fopen("a")`.

4) File Size Finder

- Use `fseek(fp, 0, SEEK_END)` and `ftell(fp)` to print file size in bytes.

---

## HOW A FILE SYSTEM WORKS INTERNALLY

### 1) Big Picture

A file system (FS) is the part of the operating system that:

- Organizes data on disks (SSD/HDD)
- Tracks which blocks belong to which files
- Stores metadata (names, sizes, permissions)
- Translates file operations (open, read, write) into actual disk reads/writes

Think of it as the database of the disk — it maps filenames to data blocks and keeps everything consistent.

### 2) The Journey from Your Code to the Disk

Let’s start with a simple program:

```c
FILE *fp = fopen("notes.txt", "w");
fprintf(fp, "Hello World!");
fclose(fp);
```

### 3) Step-by-step flow (human + technical view)

| Step | What happens (simple)                       | What happens (technical) |
|------|---------------------------------------------|---------------------------|
| 1    | You call `fopen()`, `fprintf()`, etc.       | C standard library (stdio) buffers data and eventually calls system calls like `open()`, `write()` |
| 2    | Your code asks the OS to open/write         | The kernel receives a request via a system call |
| 3    | OS chooses the file system                  | VFS layer provides one interface; looks up "notes.txt" and finds/creates its inode |
| 4    | File system finds where the file lives      | Inode stores metadata: size, perms, timestamps, and pointers to data blocks (e.g., inode #42 → blocks 100–102) |
| 5    | Data is split into blocks                   | Each block (usually 4 KB) holds part of your file; inode tracks which blocks belong to it |
| 6    | OS keeps data in RAM for speed              | Reads/writes go through the page cache; write-back to disk happens asynchronously |
| 7    | OS asks hardware to read/write              | FS driver converts logical blocks → physical sectors; device driver talks to SSD/HDD |
| 8    | Bytes are read/written on the device        | HDD: magnetic sectors via moving heads; SSD: electronic flash cells |
| 9    | Crash protection                            | Journal logs pending writes; on reboot, unfinished ones are replayed for consistency |
| 10   | Close file                                  | `fclose()` flushes buffers; caches updated; file descriptors released |

### 4) Key components inside the file system

| Component        | Role                                                                 |
|------------------|----------------------------------------------------------------------|
| Superblock       | Global info about the file system (type, size, block size)           |
| Inode table      | Metadata for every file (perms, timestamps, data block pointers)     |
| Directory entries| Maps filenames → inode numbers                                       |
| Data blocks      | Store the actual file contents                                       |
| Journal          | Log area to recover from crashes safely                              |
| Page cache       | RAM area used for faster access                                      |

### 5) Virtual File System (VFS)

The VFS is a bridge between your program and specific file systems. You always use the same system calls (`open`, `read`, `write`) even if the underlying FS is APFS, ext4, NTFS, or FAT32. The VFS routes requests to the correct FS driver.

### 6) File descriptors and file tables

When you open a file, the OS gives your process a small integer called a file descriptor — like a ticket for the opened file.

Example:

```text
0 = stdin
1 = stdout
2 = stderr
3 = your file (notes.txt)
```

Each process has its own file descriptor table, but multiple processes can share the same underlying file (via shared inodes).

### 7) SSD vs HDD (performance view)

| Feature            | HDD                        | SSD                              |
|--------------------|----------------------------|-----------------------------------|
| Data access        | Mechanical, ~10 ms         | Electronic, <0.1 ms               |
| Ideal for          | Sequential reads/writes    | Random I/O                        |
| Needs optimization | Head movement              | Wear leveling, TRIM               |
| Example FS         | ext4                       | APFS (optimized for SSD)          |

APFS is SSD-optimized with:

- Copy-on-write (COW) — never overwrites data directly
- Snapshots and cloning for fast duplication
- Space sharing across volumes

### 8) Full simplified flow diagram

```text
Your C Code (fopen, fwrite)
    ↓
C Library (stdio)
    ↓
System Call (open, write)
    ↓
Kernel (VFS Layer)
    ↓
File System Driver (APFS, ext4, etc.)
    ↓
Inode Table + Directory Lookup
    ↓
Page Cache (RAM)
    ↓
Block I/O + Device Driver
    ↓
Physical Storage (SSD / HDD)
```

### 9) Summary

| Concept  | Simple explanation      | Technical function                           |
|----------|--------------------------|----------------------------------------------|
| File     | Named data on disk       | Inode + data blocks                          |
| Inode    | File’s ID card           | Stores metadata + block addresses            |
| Directory| Folder listing           | Maps names → inode numbers                   |
| Block    | Small piece of data      | Usually 4 KB chunk on disk                   |
| VFS      | Translator               | Connects apps to all file systems            |
| Journal  | Safety log               | Prevents corruption on crash                 |
| Cache    | Speed booster (RAM)      | Buffers reads/writes before disk access      |

### 10) In short

- Files are broken into blocks stored on disk.
- Each file has an inode tracking its blocks and metadata.
- The OS uses caches to make I/O faster.
- Journaling improves safety during crashes.
- The VFS makes all file systems look the same to programs.

## How file data reaches a char variable in C

### Big picture

When you write code like:

```c
FILE *fp = fopen("data.txt", "r");
char c = fgetc(fp);
```

you’re reading a byte stored on disk and moving it into your program’s memory, inside a C variable.

### Step-by-step flow: from file to variable

| Step | Simple view                                  | Technical view |
|------|----------------------------------------------|----------------|
| 1    | Open file with `fopen("data.txt", "r")`    | `fopen()` calls `open()`; kernel locates inode, opens it, returns a descriptor |
| 2    | Call `fgetc(fp)` to read one char             | stdio checks its buffer; if empty, it issues a `read()` system call |
| 3    | OS loads data into RAM                        | Kernel reads file’s data blocks; copies bytes into page cache; returns them to stdio |
| 4    | stdio buffering                               | `FILE*` stream has its own buffer (e.g., ~4 KB); `fgetc()` returns one byte from it |
| 5    | Variable assignment                           | The byte (e.g., 'A' or '\n') is copied into your `char` variable on the stack |

### Memory path visualization

```text
Disk (bytes on SSD/HDD)
   ↓
Kernel File System (VFS + inode + data blocks)
   ↓
Page Cache (RAM)
   ↓
stdio Buffer (user-space memory)
   ↓
char c (on your stack)
```

By the time you get `char c = fgetc(fp);`, the data has already traveled through multiple caching layers — which makes file I/O much faster than direct disk access.

### Example walkthrough

File: `data.txt`

```text
HELLO
```

Code:

```c
#include <stdio.h>

int main() {
    FILE *fp = fopen("data.txt", "r");
    char c;

    while ((c = fgetc(fp)) != EOF)
    printf("Char: %c\n", c);

    fclose(fp);
    return 0;
}
```

Execution flow:

- `fopen()` → kernel opens the file, returns a `FILE*`.
- `fgetc()` → library fetches ~4 KB into buffer from disk (via page cache).
- Each call returns one byte from that buffer into `c`.
- You print `c`, then move to the next character.

### Why buffers exist

Without buffers, reading a 1 MB file one char at a time would require 1 million disk reads — painfully slow.

With buffering:

- OS fetches ~4 KB blocks into page cache
- stdio fetches those blocks into its user-space buffer
- Your loop consumes from memory — fast

So when you write `fgetc(fp)`, you’re usually reading from memory, not the disk.

### Writing works the same way

```c
FILE *fp = fopen("log.txt", "w");
char c = 'A';
fputc(c, fp);
fclose(fp);
```

Flow:

- 'A' is written into stdio’s output buffer in RAM.
- When the buffer is full (or when you `fclose()`), the library flushes it.
- Kernel writes it to the page cache; later, the OS writes those bytes to disk blocks.

### Summary

| Concept        | What happens                                              |
|----------------|-----------------------------------------------------------|
| `fgetc()`      | Reads one byte at a time from stdio buffer                |
| stdio buffer   | A (~4 KB) memory block managed by `FILE*`                 |
| Page cache     | OS-level RAM that stores recently used file data          |
| Disk           | Stores the actual bytes permanently                       |
| `char` variable| Lives on your stack; holds the byte you just read         |
| Performance    | Reads from memory (cache), not from disk, most of the time|

In short:

- The `char` variable ultimately gets one byte from the file.
- That byte usually comes from RAM, not directly from the disk.
- Buffers exist at both user level (stdio) and kernel level (page cache).
- This layered design gives speed, safety, and consistency.