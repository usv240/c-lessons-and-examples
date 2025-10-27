## LESSON 15 — Apple Silicon Architecture Deep Dive

(Unified Memory, Core Design, and Performance Behavior)

## Goals

By the end, you’ll understand:

- How Apple Silicon’s SoC is organized (CPU + GPU + Neural Engine + Media Engines)
- What Unified Memory Architecture (UMA) means and why it matters
- How performance vs efficiency cores work together
- How macOS schedules threads across clusters
- Where optimization opportunities exist for C/C++/media workloads

## 1) The System on a Chip (SoC)

Apple Silicon = everything integrated onto one die:

```text
+-------------------------------------------------------+
| CPU (Perf + Eff Cores) |  GPU  | Neural Engine | Media |
+-------------------------------------------------------+
| Shared L2/L3 Cache + Unified Memory Fabric            |
+-------------------------------------------------------+
```

CPU, GPU, and accelerators share the same memory and cache fabric — no separate VRAM copy.

## 2) Unified Memory Architecture (UMA)

Old Intel design: `CPU RAM ↔ copy ↔ GPU VRAM`

Apple UMA: All components access the same physical memory.

Advantages:

- Zero-copy data sharing between CPU/GPU
- Lower latency → faster frame processing
- Lower power usage (battery + thermals)

When you `malloc()` or `mmap()` memory, it’s part of the same UMA pool. Frameworks (Core Media, Metal, Accelerate) optimize for it automatically.

## 3) Core Types — Performance vs Efficiency

| Type                | Purpose                           | Example Cluster               | Typical Use                    |
|---------------------|-----------------------------------|-------------------------------|--------------------------------|
| Performance (P-core)| High speed, out-of-order execution| “Firestorm” / “Avalanche”     | Compiling, streaming, rendering|
| Efficiency (E-core) | Low power, simpler in-order cores | “Icestorm” / “Blizzard”       | Background tasks, I/O, decoding|

macOS dynamically moves threads between clusters via the scheduler. Developers can set hints (`QOS_CLASS_USER_INTERACTIVE`, `QOS_CLASS_BACKGROUND`) to influence placement.

## 4) Memory Subsystem and Cache

Each cluster has:

```text
L1 cache (per core) → L2 shared (cluster) → SLC (System Level Cache)
```

The SLC is shared by GPU + CPU → huge boost for streaming pipelines.

| Cache Level   | Typical Size | Latency (cycles) | Shared By       |
|---------------|--------------|------------------|-----------------|
| L1 Instr/Data | 64 KB        | ~3               | Per core        |
| L2            | ~12 MB       | ~10              | Cluster         |
| SLC (L3)      | 32 MB+       | ~20              | All components  |

## 5) Scheduling Behavior (macOS / Darwin)

The XNU kernel uses a hybrid Mach + BSD scheduler:

- Each thread has a priority and QoS class
- Higher QoS tends to run on performance cores
- Background threads migrate to efficiency cores
- Thermal/battery conditions affect core allocation

Example (Apple’s GCD):

```objective-c
#include <dispatch/dispatch.h>

dispatch_queue_t q = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
dispatch_async(q, ^{ heavy_compute(); });
```

Runs on P-cores. Using `QOS_CLASS_BACKGROUND` biases toward E-cores automatically.

## 6) Vector and Matrix Engines

| Engine        | Description                        | Use                           |
|---------------|------------------------------------|-------------------------------|
| NEON          | 128-bit SIMD unit on each core     | Vectorized math, media ops    |
| AMX (Matrix)  | Dedicated matrix accelerator       | ML and video filtering        |
| Media Engines | HW H.264/H.265/ProRes decode/encode| Offloads Core Media           |
| Neural Engine | 16-core array for AI tasks         | Core ML, Vision, Speech       |

These enable parallel computation without burning CPU cycles. Core Media and AVFoundation use them automatically.

## 7) Thermal & Power Management

- DVFS (Dynamic Voltage and Frequency Scaling): adjusts clock speed per core
- Cluster throttling when TDP limit hit
- Profile thermal impact via Instruments → Energy Log

Rules of thumb:

- Keep cores busy, not hot
- Avoid polling loops
- Use system sleep APIs and dispatch timers

## 8) Profiling on Apple Silicon

| Goal                | Tool                                 | Usage                                        |
|---------------------|--------------------------------------|----------------------------------------------|
| CPU usage per core  | powermetrics                          | `sudo powermetrics --samplers tasks`         |
| Energy impact       | Instruments → Energy Log              | Visual graph                                 |
| Hardware counters   | Instruments → Time Profiler → Counters| Cache misses, IPC                            |
| GPU load            | Xcode Metal System Trace              | GPU busy %, bandwidth                         |

## 9) Optimization Checklist

- Keep data in cache (SLC-friendly access)
- Batch work to reduce context switches
- Use GCD for core-aware thread dispatch
- Avoid copying buffers between CPU/GPU (UMA shared)
- Profile thermal and IPC regularly

## 10) Why This Matters for Streaming Performance Engineers

Every millisecond saved in Core Media’s decode/render loop depends on:

- Minimizing cache misses and memory copies
- Scheduling video/audio threads on optimal cores
- Using hardware accelerators instead of CPU loops

## Summary

| Concept     | Description                                 |
|-------------|---------------------------------------------|
| UMA         | Shared memory between CPU, GPU, accelerators|
| P vs E Cores| Performance vs power efficiency roles        |
| Cache Fabric| Multi-level shared caches (SLC)              |
| NEON / AMX  | Hardware vector and matrix engines           |
| Scheduler   | QoS-aware thread placement                   |
| Thermal Mgmt| Frequency scaling and energy optimization    |