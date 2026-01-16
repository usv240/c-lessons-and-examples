## LESSON 22 — Kernel Development Concepts

(Understanding the OS kernel: syscalls, drivers, scheduling, memory management, and synchronization)

### Simple Introduction

**What is the kernel?** Special privileged software that runs at the CPU's highest privilege level. It manages hardware, memory, processes, and threads.

**Why learn about it?** Because understanding the kernel helps you optimize: you'll know what's fast (direct memory access) vs. slow (syscalls).

**Key idea**: The kernel is a middleman between your code and hardware. Sometimes you have to ask permission (syscalls), which is slower than doing things directly. Smart coding works *with* the kernel, not against it.

**What you'll learn**: What syscalls are, how scheduling works, and why TLB misses matter—all at a level that makes sense without a PhD.

## Goals

By the end of this lesson, you'll understand:

- What the kernel is and why it's different from user-space code
- How syscalls bridge the user/kernel boundary
- How kernel drivers interface with hardware
- How the kernel manages processes, threads, and scheduling
- How virtual memory and page tables work from the kernel perspective
- Kernel synchronization primitives (spinlocks, semaphores, RCU)
- Common kernel subsystems and their performance implications
- How to profile and optimize kernel-space code

## 1) The Kernel vs. User-Space Boundary (Simple Explanation)

**Quick mental model**: The CPU has two modes: user mode (restricted, safe) and kernel mode (unrestricted, powerful).

### Privilege levels

All modern CPUs have multiple privilege levels:

| Level          | Access                        | Who Runs                    | Examples               |
|----------------|-------------------------------|----------------------------|------------------------|
| Ring 0 (Kernel)| All memory, all instructions  | OS kernel, device drivers   | Linux kernel, xnu      |
| Ring 1,2 (Unused in most OSs) | Intermediate        | Reserved                    | Rarely used             |
| Ring 3 (User)  | Limited memory, safe instructions | User applications  | Your C program          |

When your code tries something privileged (disk I/O, memory mapping, scheduling), the CPU raises an **exception**, and the kernel takes over.

### Context switch: user → kernel → user

```c
// User code
int result = open("/etc/passwd", O_RDONLY);  // syscall instruction
// CPU switches to kernel mode
// Kernel's open() handler runs
// Kernel returns result to user mode
printf("File descriptor: %d\n", result);
```

**Cost**: 100–1000s of nanoseconds per context switch (expensive!).

## 2) System Calls (Syscalls) — The Bridge

**Simple idea**: When your code needs something only the kernel can do (access a file, allocate memory at a specific address, etc.), you make a syscall. It's like raising your hand and asking the teacher.

**Cost**: Syscalls are slow (~100–1000 nanoseconds each) because the CPU has to switch privilege levels. That's not free.

A syscall is the controlled way user-space code requests kernel services.

### Common syscall categories

| Category       | Examples                          | Purpose                        |
|----------------|-----------------------------------|--------------------------------|
| File I/O       | open, read, write, close, ioctl   | Access files and devices       |
| Process/thread | fork, exec, exit, clone, wait     | Create/manage processes        |
| Memory         | mmap, mprotect, brk, sbrk         | Manage virtual memory          |
| Synchronization| futex, eventfd, semget, semop     | Inter-process synchronization  |
| Networking     | socket, bind, listen, accept      | Network communication          |
| Signals        | signal, sigaction, kill, sigprocmask | Interrupt handling          |
| Time           | clock_gettime, nanosleep, timer_* | Timing and delays              |

### How syscalls work (simplified)

```text
User Code
    ↓
syscall instruction (e.g., "syscall" on x86-64)
    ↓
CPU switches to kernel mode
    ↓
Kernel dispatcher routes to syscall handler
    ↓
Syscall handler (e.g., sys_open, sys_read)
    ↓
Kernel does the work (checks permissions, I/O, etc.)
    ↓
CPU switches back to user mode
    ↓
Return value in registers (EAX on x86)
    ↓
User code continues
```

### Example: strace to see syscalls

```bash
strace -c ./myprogram
```

Output shows every syscall and how much time was spent:

```
% time     seconds  usecs/call     calls    errors syscall
------	----------	-----------	---------	-----	----------------
 35.22	   0.145000	      29.000	     5000	  100	openat
 28.15	   0.116000	      23.200	     5000	      read
 20.33	   0.083700	      16.740	     5000	      write
  8.12	   0.033400	      13.360	     2500	      close
```

**Insight**: If openat is 35% of your time, file opening is a bottleneck. Consider caching, batching, or buffering.

## 3) Kernel Drivers & Hardware Interface

Kernel drivers are privileged code that directly access hardware.

### Architecture

```text
User Code (your app)
    ↓
System Call (e.g., read from /dev/video0)
    ↓
Kernel Subsystem (e.g., V4L2 — Video for Linux 2)
    ↓
Driver (e.g., usb-uvc.ko for USB video cameras)
    ↓
Hardware (USB controller, camera sensor)
```

### Three types of drivers

| Type        | Runs At | Access        | Examples                        |
|-------------|---------|---------------|---------------------------------|
| Kernel     | Ring 0  | Full hardware | USB, networking, storage       |
| Character  | Ring 0  | Sequential I/O | /dev/tty, /dev/random          |
| Block      | Ring 0  | Random I/O    | /dev/sda (disk), nvme devices  |

### Example: Writing a minimal kernel module (Linux)

```c
// hello.c — minimal kernel module

#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");

static int __init hello_init(void) {
    printk(KERN_INFO "Hello from kernel!\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye from kernel!\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

Build & load:

```bash
# Makefile
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# Load the module
sudo insmod hello.ko

# See the message
dmesg | tail

# Unload
sudo rmmod hello
```

**Note**: Kernel code is dangerous—a bug can crash the entire OS. Always test in VMs first.

## 4) Process & Thread Management in the Kernel

The kernel maintains a **process table** and **thread table** tracking all running processes.

### Process descriptor (simplified Linux task_struct)

```c
// Simplified version of kernel's process structure
struct task_struct {
    int pid;                    // Process ID
    char comm[16];              // Command name
    
    // Memory
    struct mm_struct *mm;       // Virtual memory info
    
    // Scheduling
    unsigned int policy;        // SCHED_FIFO, SCHED_RR, SCHED_NORMAL
    int prio;                   // Priority (0-139)
    unsigned long jiffies;      // Time slice remaining
    
    // Threads (children of this process)
    struct list_head thread_group;
    
    // File descriptors
    struct files_struct *files;
    
    // Signals
    struct signal_struct *signal;
    sigset_t blocked;           // Blocked signals
};
```

### Kernel scheduler

The kernel's job scheduler decides which thread runs on which CPU at any moment.

**Goals:**
- Fairness: Each thread gets CPU time
- Responsiveness: Interactive tasks don't stall
- Throughput: Maximize work done per second
- Energy efficiency: Use fewer cores when possible

**Algorithm** (Completely Fair Scheduler on Linux):

1. Maintain a **runqueue** per CPU (list of runnable threads)
2. Each thread has a **vruntime** (virtual runtime)
3. Always run the thread with lowest vruntime
4. After time slice, move to back of queue and lower priority

```c
// Simplified CFS scheduler logic
struct thread {
    unsigned long vruntime;  // virtual runtime
    int priority;
};

void schedule(void) {
    struct thread *next = pick_next_thread();  // Lowest vruntime
    context_switch(current, next);
}

void timer_interrupt(void) {
    current->vruntime += elapsed_time * (1024 / priority);
    if (current->vruntime > minimum_vruntime) {
        schedule();  // Preempt and reschedule
    }
}
```

## 5) Virtual Memory & Paging (Kernel Perspective)

The kernel manages virtual-to-physical address translation using **page tables**.

### Address translation (simplified)

```text
Virtual Address (your code sees this)
    ↓
Page Table Lookup (kernel maintains this)
    ↓
Physical Address (where data actually lives in RAM)
    ↓
RAM access
```

### Page table walk

On a 64-bit system, virtual addresses are translated through multiple levels:

```
Virtual Address: 0x7fff1234567a

Bit layout (ARM64):
┌────────────┬──────────┬──────────┬──────────┬──────────┬────────┐
│ Unused (16)│ L0 (9b)  │ L1 (9b)  │ L2 (9b)  │ L3 (9b)  │ Offset │
└────────────┴──────────┴──────────┴──────────┴──────────┴────────┘

Kernel does:
1. Read L0 entry from TTBR0 (page table base)
2. Read L1 entry from address in L0
3. Read L2 entry from address in L1
4. Read L3 entry (page) from address in L2
5. Add offset to get physical address
```

**Cost**: Without TLB cache, this is 4–5 memory accesses per lookup (very slow!). TLB caches recent translations (50+ entries).

### TLB misses

When the kernel can't find a translation in the TLB, it must walk the page table. This is expensive:

```bash
# Measure TLB misses
perf stat -e dTLB-loads,dTLB-load-misses ./myprogram
```

Example output:

```
dTLB-loads:      5,234,123
dTLB-load-misses:  123,456  (2.4% miss rate) ✓ Good
```

If miss rate > 5%, consider:
- Improving cache locality (Lesson 14)
- Using huge pages (2MB or 1GB instead of 4KB)
- Reducing working set size

## 6) Kernel Synchronization Primitives

The kernel provides different synchronization tools than user-space pthread primitives.

### Spinlocks (kernel-only)

For very short critical sections (microseconds), spinlocks are faster than mutexes:

```c
// Kernel spinlock (pseudocode)
spinlock_t mylock = SPINLOCK_INIT;

void critical_section(void) {
    spin_lock(&mylock);      // Spin until lock available
    // Critical section (must be short!)
    shared_data++;
    spin_unlock(&mylock);
}
```

**Why kernel-level only**: User-space can't disable interrupts, so spinning wastes CPU. Kernel can disable interrupts to prevent deadlock.

### Semaphores (count-based locking)

```c
// Kernel semaphore
struct semaphore {
    int count;              // Resource count
    wait_queue_head_t wait; // Threads waiting for resource
};

void down(semaphore *s) {
    if (--s->count < 0)
        sleep_on(&s->wait);  // Go to sleep, no spinning
}

void up(semaphore *s) {
    if (++s->count <= 0)
        wake_up(&s->wait);   // Wake sleeping thread
}
```

### RCU (Read-Copy-Update)

For data structures with many readers and few writers:

```c
// RCU example: reading a list
rcu_read_lock();
list_for_each_entry_rcu(node, &mylist, entry) {
    process(node->data);
}
rcu_read_unlock();  // No lock needed!

// Writer
new_node = allocate();
list_replace_rcu(&old_node, &new_node);
synchronize_rcu();  // Wait for all readers to finish
kfree(old_node);
```

**Benefit**: Readers never block; writer waits for readers.

## 7) Interrupt Handling & Bottom Halves

When hardware triggers an interrupt, the kernel must respond quickly.

### Interrupt handling flow

```text
Hardware Interrupt (e.g., network packet arrives)
    ↓
CPU calls interrupt handler (high priority, interrupts disabled)
    ↓
Handler does minimal work (read packet header)
    ↓
Schedule "bottom half" for deferred work
    ↓
Return from interrupt (interrupts re-enabled)
    ↓
Bottom half processes packet (lower priority, interrupts enabled)
```

**Rationale**: Hardware needs fast response; defer complex work to later.

### Softirqs and tasklets

```c
// Kernel bottom-half handling
static void my_tasklet_handler(unsigned long data) {
    // Deferred work (e.g., process packet payload)
    process_network_data((struct packet *)data);
}

DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 0);

// From interrupt context:
tasklet_schedule(&my_tasklet);  // Schedule deferred work
```

## 8) Memory Pressure & Reclaim

When memory is low, the kernel reclaims pages (evicts them to disk).

### Page reclaim algorithm

```
Allocate 100MB
    ↓
Check free pages
    ↓
Free pages < watermark?
    ↓
Yes: Run page reclaim
    ├─ Write dirty pages to disk (swap)
    ├─ Drop cache pages
    ├─ If still low: Trigger OOM killer
    └─ Oom killer: Kill process with largest memory
    ↓
Retry allocation
```

### Memory zones

The kernel divides RAM into zones for allocation purposes:

| Zone      | Memory Range | Purpose                    |
|-----------|--------------|----------------------------|
| ZONE_DMA  | 0–16 MB      | DMA-capable devices        |
| ZONE_DMA32| 0–4 GB       | 32-bit devices             |
| ZONE_NORMAL| 4 GB–high    | General purpose allocation |
| ZONE_HIGHMEM| Above high  | Only on 32-bit systems     |

**Implication**: Some devices (old USB, ISA) can only access ZONE_DMA. If allocating for them, use `GFP_DMA` flag.

## 9) Device Interrupts vs. Polling

Two ways hardware communicates with the kernel:

### Interrupt-driven (reactive)

```text
Kernel idle / running user code
    ↓
Hardware raises IRQ
    ↓
Kernel wakes up, handles event
    ↓
Return to user code
```

**Pros**: Low latency for bursty traffic, power-efficient
**Cons**: Overhead per event (context switch)

### Polling (proactive)

```text
Kernel loop:
    Check hardware status
    Process any pending events
    Repeat every few microseconds
```

**Pros**: No context switch overhead, predictable latency
**Cons**: Wastes CPU on idle checks, power-inefficient

**Trade-off**: High-frequency events (100K+ per second) prefer polling. Low-frequency prefer interrupts.

Modern kernels use **NAPI** (New API) to switch between modes dynamically.

## 10) Profiling Kernel Code

### perf (Linux)

Trace kernel functions:

```bash
# Record all kernel function calls
perf record -k 1 ./myprogram

# View kernel functions in report
perf report

# Trace specific syscalls
perf trace ./myprogram
```

### Ftrace (Linux function tracer)

```bash
# Enable tracing of sys_read
echo sys_read > /sys/kernel/debug/tracing/set_graph_function

# Start tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Run your program
./myprogram

# View trace
cat /sys/kernel/debug/tracing/trace | less
```

### kernelshark (GUI for ftrace)

Visual representation of kernel function execution timing.

### Instruments (macOS/iOS)

Use the "System Trace" template to see kernel scheduling, interrupts, and syscalls:

```bash
xcrun xctrace record --template 'System Trace' --output trace.trace ./myapp
```

## 11) Real-World Example: Network Packet Flow

Trace a packet from NIC to your application:

```text
Packet arrives at NIC
    ↓
Hardware raises IRQ
    ↓
Kernel interrupt handler (driver)
    ├─ Read packet from NIC
    ├─ Allocate sk_buff (kernel packet structure)
    └─ Schedule NAPI softirq
    ↓
Bottom half (NAPI poll)
    ├─ Demultiplex layer (IP, TCP, etc.)
    ├─ Checksum verification
    ├─ Route lookup
    └─ Queue to socket receive buffer
    ↓
Application (user-space recv())
    ├─ Check socket buffer
    ├─ Copy data to user buffer
    └─ Return to application
```

**Performance optimizations at each layer**:
- **Driver**: Increase RX ring size to buffer more packets before dropping
- **Network stack**: Enable GSO/GRO (coalesce packets to reduce overhead)
- **Application**: Use io_uring or DPDK to bypass kernel for ultra-low latency

## 12) Kernel Development Best Practices

| Practice                | Why                                        |
|-------------------------|-------------------------------------------|
| Write drivers in kernel | Direct hardware access, low latency        |
| Use kmalloc carefully   | Can't sleep; use GFP_ATOMIC if called from interrupt |
| Disable interrupts cautiously | Other interrupts get starved             |
| Test in VMs first       | Kernel bugs can corrupt disk / lose data  |
| Use lockdep             | Detects deadlocks at development time     |
| Measure syscall overhead| Profile to find expensive system calls    |
| Cache kernel data       | kernel working set should fit L3          |

## 13) Kernel vs. User-Space Trade-offs

| Consideration             | Kernel Space | User Space |
|---------------------------|--------------|------------|
| Performance (latency)     | Lower        | Higher     |
| Performance (throughput)  | Higher       | Moderate   |
| Complexity                | High         | Low        |
| Safety (stability)        | Dangerous    | Safe       |
| Debugging                 | Hard         | Easy       |
| Development time          | Slow         | Fast       |
| Hardware access           | Full         | Limited    |

**Strategy**: Start in user-space for correctness. Move hot paths to kernel only if profiling proves it's the bottleneck.

## Summary

| Concept                    | What You Learned                                     |
|---------------------------|------------------------------------------------------|
| Privilege levels           | Ring 0 (kernel) vs Ring 3 (user)                     |
| Syscalls                   | Controlled interface between user & kernel           |
| Drivers                    | Privileged code that accesses hardware directly      |
| Scheduling                 | How kernel decides which thread runs when            |
| Virtual memory             | Page tables, TLB, page reclaim                        |
| Synchronization (kernel)   | Spinlocks, semaphores, RCU                           |
| Interrupt handling         | IRQ handlers and deferred work (tasklets)            |
| Memory zones               | DMA, normal, highmem — allocation constraints        |
| Profiling kernel code      | perf, ftrace, Instruments                            |
| Packet flow example        | End-to-end kernel path for network data              |

## Key Takeaways

- **Syscalls are expensive**: ~100–1000s ns per call. Batch I/O when possible.
- **Kernel code is privileged but dangerous**: A bug crashes the whole OS. Start in user-space.
- **Scheduling affects performance**: High-priority tasks preempt low-priority ones.
- **Virtual memory has overhead**: TLB misses, page faults, address translation. Optimize working set.
- **Interrupts vs. polling**: Trade-off between latency and CPU efficiency. Modern kernels adapt dynamically.
- **Profile before optimizing**: Don't assume kernel is the bottleneck without evidence.

## Homework

### Part 1: Syscall profiling

1. Pick a simple I/O-bound program (e.g., read a large file in chunks)
2. Profile syscalls:
   ```bash
   strace -c ./myprogram
   ```
3. Identify the most expensive syscalls (by % time)
4. Suggest 3 optimizations:
   - Increase buffer size?
   - Batch reads?
   - Use mmap instead?
5. Implement one optimization and remeasure

### Part 2: Page fault analysis

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    // Allocate large array
    int *arr = malloc(100 * 1024 * 1024);  // 100 MB
    
    // Touch pages (trigger page faults)
    for (int i = 0; i < 100 * 1024 * 1024; i += 4096) {
        arr[i]++;
    }
    
    printf("Done\n");
    free(arr);
    return 0;
}
```

Measure page faults:

```bash
perf stat -e page-faults,major-faults,minor-faults ./page_fault
```

Count how many page faults occurred. Explain why.

### Part 3: Understanding CFS scheduler

Write a program with multiple threads of different priorities and measure scheduling fairness:

```c
#include <pthread.h>
#include <sched.h>
#include <stdio.h>

void *worker(void *arg) {
    long count = 0;
    for (int i = 0; i < 10000000; i++) {
        count++;
    }
    printf("Thread %ld: %ld iterations\n", (long)arg, count);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, (void *)1);
    pthread_create(&t2, NULL, worker, (void *)2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
```

Both threads should do ~same iterations (CFS fairness). Verify with `time ./scheduler`.
