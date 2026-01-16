## LESSON 19 — Analyzing System Traces & Performance Logs

(How to read and interpret real-world system traces to find bottlenecks)

### Simple Introduction

**What is a system trace?** A recording of everything your program did: which functions ran, how long they took, what threads used, etc.

**Why traces matter**: You can't optimize what you don't measure. A trace shows you exactly where time is being wasted.

**Key insight**: Don't guess. Always profile first, then optimize based on what you find.

**What you'll learn**: How to capture traces, read them, and spot performance problems without needing a PhD in systems.

## Goals

By the end of this lesson, you'll understand:

- How to capture system traces (Activity Monitor, Instruments, perf, kdebug)
- How to read flame graphs and call stacks from traces
- How to spot CPU, memory, I/O, and thermal bottlenecks in logs
- How to correlate events across components (e.g., disk I/O → CPU spike)
- How to report findings in a way that enables cross-team debugging

## 1) System Tracing Landscape (Simple Overview)

**Bottom line**: Each OS has different profiling tools, but they all answer the same question: "Where is my program spending time?"

Different OS's expose performance data in different ways:

| OS      | Primary Tool     | Subtools/Alternatives         | Output Format           |
|---------|------------------|-------------------------------|-------------------------|
| macOS   | Instruments      | Activity Monitor, Console.app  | .trace files, flamegraph|
| iOS     | Xcode Instruments| Console.app                   | .trace, Sysdiagnose     |
| Linux   | perf             | ftrace, strace, eBPF          | perf.data, text logs    |
| Windows | Windows Perf.    | ETW, xperf                    | .etl files              |

As an OS Performance Engineer, you'll primarily work with:
- **macOS/iOS**: Instruments (System Trace, Time Profiler, Core Data tools)
- **Linux**: perf, ftrace, eBPF
- **Cross-platform**: Custom analysis scripts

## 2) macOS Instruments (Time Profiler)

### Capture a trace

```bash
# Option 1: From Xcode
# - Open Xcode → Product → Profile
# - Select "Time Profiler"
# - Run your app/binary
# - Record for 30 seconds
# - Stop and save

# Option 2: Command line
xcrun xctrace record --template 'System Trace' --output trace.trace ./myprogram
```

### What you see

- **Call stack samples**: Where was the CPU at regular intervals (typically every 1ms)?
- **Threads**: Which threads were active; were they on P-core or E-core?
- **Duration**: How long did the trace run?
- **Idle time**: Periods where threads weren't running (blocked on I/O, locks, etc.)

### Reading a Time Profiler trace

1. **Hot functions**: Sorted by % of total time
   - If `malloc` is 15%, you have memory allocation contention
   - If `pthread_cond_wait` is 30%, threads are waiting (bottleneck upstream)

2. **Call stacks**: Click a function to see the path that led to it
   - Reveals unexpected call chains
   - Shows how deep the stack is (deep stacks = more overhead)

3. **System Trace view**: Threads over time
   - See which cores threads ran on
   - Spot thread migrations (expensive on Apple Silicon)
   - Notice if threads idle (red → blocked on I/O or lock)

## 3) Linux perf (Performance Events)

### Capture a trace

```bash
# Record CPU samples for 10 seconds
perf record -F 99 -p $(pidof myprogram) -- sleep 10

# Or with all events:
perf record -a -F 99 -- ./myprogram

# Generate a flamegraph
perf record -F 99 -g ./myprogram
perf script | ./stackcollapse-perf.pl | ./flamegraph.pl > out.svg
```

### What you see

```bash
perf report
```

Shows:
- **Overhead %**: Percent of samples in each function
- **Shared object**: Which binary/library
- **Symbol**: Function name
- **Children %**: Time spent in this function + callees
- **Self %**: Time spent only in this function

### Example output

```
    12.34%  myprogram  libc.so.6     [.] malloc
     8.92%  myprogram  myprogram     [.] process_frame
     7.15%  myprogram  libpthread.so [.] pthread_mutex_lock
```

**Interpretation**: Your program spends 12% of time in malloc, suggesting memory allocation is hot.

## 4) Flame Graphs (Universal Tool)

A flame graph shows call stacks as a visual hierarchy:

```
- [root]
  - main (100%)
    - process_loop (80%)
      - decode_frame (45%)
        - decompress (30%)
        - color_convert (15%)
      - render_frame (35%)
        - copy_buffer (20%)
        - update_display (15%)
    - wait_for_network (20%)
      - recv_socket (18%)
      - tcp_ack (2%)
```

**Width = time spent; height = call depth**

### How to read it

- **Wide blocks** = hot functions (spend a lot of time)
- **Tall stacks** = deep call chains (more overhead)
- **Flat regions** = single function dominating
- **Jagged edges** = many different code paths (less predictable)

### Generate from perf

```bash
perf record -F 99 -g ./myapp
perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
# Open flame.svg in a browser
```

## 5) Spotting Common Bottlenecks in Traces

### CPU bottleneck

**Signs**:
- All cores/threads maxed out
- High CPU % in top-level functions
- No idle time

**What to do**:
- Parallelize more work
- Optimize hot functions (see Lesson 14)
- Check for cache misses (perf stat)

### Memory bottleneck

**Signs**:
- `malloc` / `free` in top 10% of samples
- Cache misses (perf stat)
- High page faults (Activity Monitor → Memory tab)

**What to do**:
- Reduce allocations (pool memory, arena allocators)
- Improve cache locality (Lesson 14)
- Check for memory leaks (valgrind, leaks)

### I/O bottleneck

**Signs**:
- Threads blocked on `read()`, `write()`, `recv()`
- Long idle times in trace
- Disk utilization 100%

**What to do**:
- Overlap I/O with computation (async I/O, threads)
- Increase buffer sizes
- Use memory-mapped files (mmap)

### Lock contention (Threading bottleneck)

**Signs**:
- Threads waiting on `pthread_mutex_lock`, `pthread_cond_wait`
- Many context switches
- Lock holders preempted (on P-cores, running on E-core)

**What to do**:
- Reduce lock hold time
- Use lock-free data structures
- Consider GCD or thread pools (Lesson 13)

### Thermal throttling

**Signs**:
- CPU frequency drops mid-trace
- P-cores clock down to E-core speeds
- Sudden performance cliff

**What to do**:
- Reduce peak power (parallelize, not sequential heavy compute)
- Improve cache behavior (fewer misses = less power)
- Profile on sustained workloads (not just startup)

## 6) Correlating Events Across Components

Real bottlenecks involve multiple layers. Example scenario:

**Trace shows**: Network thread blocked 30% of the time
↓
**Find**: `recv()` call waiting for data
↓
**Question**: Why is data delayed?
↓
**Check server logs**: Client losing packets (network congestion)
↓
**Action**: Reduce bitrate, implement adaptive quality

### Tools for cross-component analysis

| Layer          | Tool                           | What It Shows              |
|----------------|--------------------------------|----------------------------|
| CPU/Thread     | perf, Instruments              | Where time is spent        |
| Memory         | valgrind, perf, Activity Mon.  | Allocation & cache misses  |
| Disk I/O       | iostat, fs_usage (macOS)       | Read/write latency, throughput |
| Network        | tcpdump, Instruments           | Packets, RTT, packet loss  |
| Power          | powermetrics, Instruments      | CPU/GPU watts, thermal     |

## 7) Example: Analyzing a Real Streaming Pipeline Trace

Suppose you capture a trace of Lesson 17's mini-engine and see:

**Raw data:**
- Network thread: 5% CPU
- Decoder thread: 45% CPU
- Renderer thread: 20% CPU
- Main/Scheduler: 30% (lock overhead)

**Initial interpretation**: Decoder is hot.

**Deeper analysis**:
1. Inspect decoder's hot functions: 35% in `memcpy` (inefficient buffer copy)
2. Check lock contention: Main thread holds lock for 2ms, blocks network thread
3. Review call stacks: Decoder copies frame 3 times unnecessarily

**Recommendations**:
1. Use zero-copy techniques (shared pointers)
2. Reduce lock hold time (split locks into reader/writer)
3. Parallelize more (decode while rendering previous frame)

**Result**: 60% reduction in decoder time, network thread no longer blocked.

## 8) Generating Performance Reports

When you find an issue, document it professionally:

### Report template

```
Title: Buffer Contention in Media Decoder

Summary:
- Decoder thread 45% CPU, spending 35% in memcpy
- Network thread blocked 20% waiting for decoder locks

Root Cause:
- Buffer copied 3 times during decode (CPU→GPU→Render)
- Decoder holds lock for 2ms, preventing network from refilling

Evidence:
- Flame graph: memcpy at top of decode_frame call stack
- Instruments: Main thread blocked on pthread_mutex, 47 context switches/sec

Proposed Fix:
1. Use GPU zero-copy (Unified Memory, MTLHeap)
2. Reader-writer locks (multiple readers, single writer)
3. Double-buffering to decouple stages

Estimated Impact:
- 30% reduction in decoder CPU
- 50% reduction in context switches
- 15% improvement in overall throughput
```

## 9) Tools Comparison & When to Use Each

| Tool        | Best For                         | Learning Curve | Output Style |
|-------------|----------------------------------|-----------------|--------------|
| Instruments | macOS/iOS debugging, visual      | Moderate        | GUI, traces  |
| perf        | Linux CPU/cache analysis        | Steep           | CLI, reports |
| strace      | System call tracing             | Easy            | Text log     |
| Flame graph | Visual call stack analysis      | Easy            | SVG graphic  |
| valgrind    | Memory & threading issues      | Moderate        | CLI output   |
| Xcode debug | Stepping & inspection           | Easy            | GUI          |

## 10) Performance Analysis Workflow

1. **Identify the problem**: "Video is choppy" or "Memory usage climbs over time"
2. **Capture a trace**: Instruments, perf, or custom script
3. **Find the hot functions**: Sort by % CPU or time spent
4. **Inspect call stacks**: Navigate to understand context
5. **Correlate events**: Is it CPU, memory, I/O, locks, or thermal?
6. **Form hypothesis**: "Decoder allocates too much" or "Renderer blocks on GPU"
7. **Propose fix**: Code change, configuration, or architecture shift
8. **Measure impact**: Re-run trace, compare before/after
9. **Report findings**: Document for cross-team collaboration

## Summary

| Concept                  | What You Learned                                     |
|--------------------------|------------------------------------------------------|
| Trace capture            | Instruments, perf, strace for different OSs         |
| Flame graphs             | Visual representation of call stacks & time spent    |
| Bottleneck identification| CPU, memory, I/O, locks, thermal — symptoms & tools |
| Cross-team correlation   | Linking trace findings to system/network layers     |
| Performance reports      | Professional documentation of issues & fixes        |
| Analysis workflow        | Systematic approach to finding & fixing perf bugs   |

## Key Takeaways

- **Always trace first**: Don't guess where time is spent
- **Read call stacks**: Context matters (same function, different paths = different costs)
- **Correlate layers**: CPU spike often caused by I/O or lock contention upstream
- **Quantify impact**: "Reduces malloc calls from 1M to 100K" is actionable
- **Focus on sustained workloads**: Startup/shutdown aren't representative

## Homework

1. Pick a simple program (e.g., Lesson 17's streaming engine)
2. Capture a trace:
   - **macOS**: `xcrun xctrace record --template 'System Trace' --output trace.trace ./stream`
   - **Linux**: `perf record -F 99 -g ./stream`
3. Generate a flame graph or open the trace in Instruments
4. Identify the hottest function (most time spent)
5. Inspect its call stack and suggest an optimization
6. Write a one-paragraph report of your findings

Starter analysis script:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITERS 1000000

int main(void) {
    char *buf1 = malloc(4096);
    char *buf2 = malloc(4096);
    
    for (int i = 0; i < ITERS; i++) {
        memcpy(buf1, buf2, 4096);  // Hot function to trace
    }
    
    printf("Done\n");
    free(buf1);
    free(buf2);
    return 0;
}
```

Trace it and see `memcpy` dominate. Then optimize (vectorize, reduce copies, etc.).
