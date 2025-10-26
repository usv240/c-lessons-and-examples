## LESSON 10 — Processes & Threads

(Concurrent execution in user space and how the OS schedules it)

## Goals

By the end, you’ll be able to:

- Understand what a process is (your program in execution)
- Explain how threads share memory inside a process
- Create processes (`fork`, `exec`)
- Create threads (`pthread_create`)
- Describe scheduling, context switching, and synchronization

## 1) What Is a Process?

Simple:

A process is a running program with its own memory space.

Technical:

When you run `./a.out`, the OS:

- Loads your compiled program into memory (code, stack, heap)
- Creates a Process Control Block (PCB) — metadata about the process
- Gives it:
  - A unique PID (process ID)
  - Its own address space (isolated memory)
  - CPU time via the scheduler

Process memory layout:

```text
+-------------------+  ← High address
| Stack (local vars)|
|-------------------|
| Heap (malloc data)|
|-------------------|
| Data (globals)    |
|-------------------|
| Text (code)       |
+-------------------+  ← Low address
```

Each process has its own copy of this layout.

## 2) fork() — Creating a New Process

In UNIX-like systems, new processes are created by copying an existing one using `fork()`.

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Start\n");
    pid_t pid = fork();

    if (pid == 0)
        printf("Child process\n");
    else
        printf("Parent process\n");

    return 0;
}
```

Output (order may vary):

```text
Start
Parent process
Child process
```

`fork()` creates a child process (clone of parent). Both processes continue execution after `fork()`. The child gets a new PID and its own copy of memory.

## 3) exec() — Replacing a Process

`exec()` replaces the current process’s code with a new program.

```c
#include <unistd.h>
int main() {
    char *args[] = {"ls", "-l", NULL};
    execvp("ls", args);
    return 0;  // Only runs if exec fails
}
```

The current process is replaced by `ls -l`.

So:

- `fork()` = make a new process
- `exec()` = replace current process with another

Used together by shells (e.g., bash) to run commands.

## 4) wait() — Synchronizing Processes

The parent process can wait for its child to finish using `wait()`.

```c
#include <sys/wait.h>
#include <unistd.h>

int main() {
    if (fork() == 0)
        execlp("ls", "ls", NULL);
    else
        wait(NULL);  // Parent waits for child
}
```

This prevents the parent from exiting before its child completes.

## 5) Threads — Lighter Than Processes

A thread is a smaller execution unit within a process.

| Aspect     | Process                        | Thread                         |
|------------|--------------------------------|--------------------------------|
| Memory     | Independent                    | Shared with other threads      |
| Overhead   | Heavy to create                | Lightweight to create          |
| Isolation  | Full isolation                 | Shared heap and globals        |
| Creation   | `fork()`                       | `pthread_create()`             |

Multiple threads inside one process can run simultaneously on different CPU cores and share data easily.

## 6) pthread — POSIX Threads Example

```c
#include <stdio.h>
#include <pthread.h>

void* task(void *arg) {
    printf("Hello from thread!\n");
    return NULL;
}

int main() {
    pthread_t t;
    pthread_create(&t, NULL, task, NULL);
    pthread_join(t, NULL);
    printf("Thread finished.\n");
}
```

Output:

```text
Hello from thread!
Thread finished.
```

`pthread_create()` starts a new thread. `pthread_join()` waits for that thread to finish.

All threads share the same:

- Code
- Heap (dynamic memory)
- Global variables

But each has its own:

- Stack (function calls, local variables)

## 7) Synchronization — Avoid Race Conditions

When multiple threads modify shared data, problems can occur.

Example (race condition):

```c
int counter = 0;

void* add(void *arg) {
    for (int i = 0; i < 1000; i++)
        counter++;
}
```

If two threads run this together, the result might not be 2000 due to race conditions.

Use a mutex (mutual exclusion):

```c
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* add(void *arg) {
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
}
```

This ensures only one thread updates `counter` at a time.

## 8) Scheduling & Context Switching

The CPU scheduler rapidly switches between threads and processes, giving each a small slice of time (e.g., 5 ms). It saves registers and the program counter, then restores others — that’s a context switch.

Efficient scheduling ensures:

- Smooth multitasking
- Balanced CPU utilization
- Fair execution across processes

## 9) Processes vs Threads Summary

| Feature        | Process                     | Thread                           |
|----------------|-----------------------------|----------------------------------|
| Memory         | Independent                 | Shared                           |
| Communication  | Slow (IPC)                  | Fast (shared memory)             |
| Creation       | `fork()`                     | `pthread_create()`               |
| Switching      | Heavy                       | Lightweight                      |
| Crash effect   | Only that process crashes   | May crash the entire process     |

## 10) Real-World Analogy

Imagine your process as a house. Each thread is a person inside that house. They share the same kitchen (heap), but each has their own bedroom (stack). If one person messes up the kitchen, everyone’s affected — that’s why synchronization matters.

## Summary

| Concept   | Description                                      |
|-----------|--------------------------------------------------|
| Process   | A program in execution (isolated memory)         |
| Thread    | A smaller execution unit within a process        |
| `fork()`   | Creates a child process                          |
| `exec()`   | Replaces current process with another program    |
| `pthread`  | POSIX thread library                             |
| `mutex`    | Prevents concurrent data modification            |
| Scheduler | Decides which process/thread runs next           |

