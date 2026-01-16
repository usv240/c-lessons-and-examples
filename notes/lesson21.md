## LESSON 21 — Building Custom Profiling & Measurement Tools

(Engineering your own performance observability into your systems)

### Simple Introduction

**What is instrumentation?** Adding little measurement points in your code to track what it's doing.

**Why build custom tools?** Sometimes you have a very specific question ("How many mallocs happen in this loop?"), and a general profiler gives you too much noise. Custom tools are fast, cheap, and focused.

**Key idea**: Use simple macros and counters to answer specific questions. Don't over-engineer.

**What you'll learn**: How to write macros that measure timing, count events, and export data without slowing down your code.

## Goals

By the end of this lesson, you'll understand:

- How to instrument code with lightweight profiling macros
- How to build a simple in-process performance sampler
- How to collect timing data with minimal overhead
- How to export metrics for analysis (JSON, CSV, protobuf)
- How to use custom tools to answer specific performance questions

## 1) Instrumentation Strategies (Pick One)

**Think of it like traffic monitoring**: You can watch every car (profiling), sample every 10th car (sampling), or count cars at a checkpoint (logging). Different tools for different questions.

There are three main approaches to measure performance:

| Strategy      | Overhead | Accuracy | Use Case                           |
|---------------|----------|----------|-------------------------------------|
| **Sampling**  | Minimal  | Approximate | Continuous monitoring, production  |
| **Logging**   | Low      | Detailed | Specific events, debug/trace       |
| **Profiling** | High     | Very detailed | Deep analysis, development        |

### Sampling

Periodically interrupt execution and record where the CPU is. Low overhead, non-intrusive.

```c
// Minimal sampling
void sample_periodic(void) {
    static int count = 0;
    if (++count % 1000 == 0) {
        // Take a sample of current call stack
        backtrace(buffer, max_depth);
    }
}
```

### Logging

Record specific events (function entry/exit, milestones) at the time they occur.

```c
#define LOG_EVENT(name) fprintf(timing_log, "%s at %lu ns\n", name, now_ns())
```

### Profiling

Full instrumentation; tracks every function call. High overhead but maximum detail.

```c
void func(void) __attribute__((instrumented));  // compiler wraps with timing code
```

## 2) Lightweight Instrumentation Macros

Build simple macros for timing without adding external libraries:

```c
#include <stdio.h>
#include <time.h>
#include <stdint.h>

// Get current nanoseconds (portable)
static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

// Measure a block of code
#define TIMER_START(name) \
    uint64_t name##_start = now_ns();

#define TIMER_END(name) \
    do { \
        uint64_t name##_end = now_ns(); \
        uint64_t name##_elapsed = name##_end - name##_start; \
        fprintf(stderr, "%s: %.3f ms\n", #name, name##_elapsed / 1e6); \
    } while (0)

// Measure function execution
#define MEASURE_FUNC_START() \
    uint64_t _func_start = now_ns()

#define MEASURE_FUNC_END(func_name) \
    do { \
        uint64_t _func_end = now_ns(); \
        printf("[PERF] %s: %.3f ms\n", func_name, (_func_end - _func_start) / 1e6); \
    } while (0)
```

### Example usage

```c
void process_data(int *arr, int len) {
    MEASURE_FUNC_START();
    
    TIMER_START(sort);
    qsort(arr, len, sizeof(int), cmp);
    TIMER_END(sort);
    
    TIMER_START(sum);
    int sum = 0;
    for (int i = 0; i < len; i++) sum += arr[i];
    TIMER_END(sum);
    
    MEASURE_FUNC_END(__func__);
}
```

Output:

```
[PERF] sort: 12.345 ms
[PERF] sum: 0.042 ms
[PERF] process_data: 12.389 ms
```

## 3) Building a Simple Performance Sampler

A sampler periodically captures the call stack and function names to build a profile.

```c
#include <stdio.h>
#include <time.h>
#include <execinfo.h>  // backtrace()
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 32
#define SAMPLE_FREQUENCY 1000  // samples per second

typedef struct {
    void *frames[MAX_FRAMES];
    int frame_count;
    uint64_t timestamp;
} sample_t;

#define MAX_SAMPLES 10000
static sample_t samples[MAX_SAMPLES];
static int sample_count = 0;

// Call this periodically (e.g., every 1ms)
void take_sample(void) {
    if (sample_count >= MAX_SAMPLES) return;
    
    sample_t *s = &samples[sample_count++];
    s->timestamp = now_ns();
    s->frame_count = backtrace(s->frames, MAX_FRAMES);
}

// Export samples as JSON
void export_samples_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "[\n");
    for (int i = 0; i < sample_count; i++) {
        sample_t *s = &samples[i];
        fprintf(f, "  {\n");
        fprintf(f, "    \"timestamp\": %lu,\n", s->timestamp);
        fprintf(f, "    \"frames\": [\n");
        
        char **symbols = backtrace_symbols(s->frames, s->frame_count);
        for (int j = 0; j < s->frame_count; j++) {
            fprintf(f, "      \"%s\"", symbols[j]);
            if (j < s->frame_count - 1) fprintf(f, ",");
            fprintf(f, "\n");
        }
        free(symbols);
        
        fprintf(f, "    ]\n");
        fprintf(f, "  }");
        if (i < sample_count - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "]\n");
    
    fclose(f);
}
```

### Using the sampler

```c
int main(void) {
    // Simulate sampling (in real code, use a timer or thread)
    for (int i = 0; i < 1000; i++) {
        process_data(arr, 10000);
        take_sample();
    }
    
    export_samples_json("samples.json");
    printf("Exported %d samples\n", sample_count);
    return 0;
}
```

Then convert `samples.json` to a flame graph:

```bash
# Install flamegraph tools
python3 samples_to_flame.py samples.json > flame.svg
# Open in browser
```

## 4) Counting-Based Instrumentation

Rather than timing, count occurrences of events:

```c
#include <stdatomic.h>

// Thread-safe counters
_Atomic long malloc_calls = 0;
_Atomic long free_calls = 0;
_Atomic long lock_acquisitions = 0;
_Atomic long lock_contentions = 0;

// Wrap malloc/free
void *malloc_tracked(size_t size) {
    atomic_fetch_add(&malloc_calls, 1);
    return malloc(size);
}

void free_tracked(void *ptr) {
    atomic_fetch_add(&free_calls, 1);
    free(ptr);
}

// Wrap lock/unlock
int mutex_lock_tracked(pthread_mutex_t *m) {
    atomic_fetch_add(&lock_acquisitions, 1);
    return pthread_mutex_lock(m);
}

// Export counters
void export_counters(void) {
    printf("Malloc calls: %ld\n", atomic_load(&malloc_calls));
    printf("Free calls: %ld\n", atomic_load(&free_calls));
    printf("Lock acquisitions: %ld\n", atomic_load(&lock_acquisitions));
    printf("Allocation balance: %ld\n",
           atomic_load(&malloc_calls) - atomic_load(&free_calls));
}
```

### Memory leak detector using counters

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    size_t allocated;      // bytes currently allocated
    size_t freed;          // bytes freed
    uint64_t alloc_count;  // number of allocations
    uint64_t free_count;   // number of frees
} alloc_stats_t;

static alloc_stats_t stats = {0};

void *tracked_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        stats.allocated += size;
        stats.alloc_count++;
    }
    return ptr;
}

void tracked_free(void *ptr) {
    if (ptr) {
        // Note: we don't know original size, but count frees
        stats.free_count++;
    }
    free(ptr);
}

void check_leaks(void) {
    printf("=== Memory Stats ===\n");
    printf("Allocations: %lu\n", stats.alloc_count);
    printf("Frees: %lu\n", stats.free_count);
    printf("Pending: %lu\n", stats.alloc_count - stats.free_count);
    printf("Bytes allocated: %zu\n", stats.allocated);
    
    if (stats.alloc_count != stats.free_count) {
        printf("⚠️ WARNING: Alloc/Free mismatch!\n");
    }
}
```

## 5) Latency Histogram

Collect latencies and build a histogram to understand distribution:

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HISTOGRAM_BINS 100
#define HISTOGRAM_MAX_US 10000  // 10ms max

typedef struct {
    uint64_t bins[HISTOGRAM_BINS];
    uint64_t underflow;
    uint64_t overflow;
    double min_us, max_us;
} histogram_t;

void histogram_add(histogram_t *h, double latency_us) {
    if (latency_us < 0) {
        h->underflow++;
        return;
    }
    if (latency_us > HISTOGRAM_MAX_US) {
        h->overflow++;
        return;
    }
    
    int bin = (int)(latency_us / (HISTOGRAM_MAX_US / HISTOGRAM_BINS));
    if (bin >= HISTOGRAM_BINS) bin = HISTOGRAM_BINS - 1;
    h->bins[bin]++;
    
    if (latency_us < h->min_us) h->min_us = latency_us;
    if (latency_us > h->max_us) h->max_us = latency_us;
}

void histogram_print(histogram_t *h) {
    printf("Latency Histogram (microseconds)\n");
    printf("Min: %.1f us, Max: %.1f us\n", h->min_us, h->max_us);
    printf("Underflow: %lu, Overflow: %lu\n\n", h->underflow, h->overflow);
    
    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        if (h->bins[i] == 0) continue;
        
        double lower = (double)i * HISTOGRAM_MAX_US / HISTOGRAM_BINS;
        double upper = (double)(i + 1) * HISTOGRAM_MAX_US / HISTOGRAM_BINS;
        
        printf("[%.1f - %.1f us]: ", lower, upper);
        for (int j = 0; j < h->bins[i] / 1000; j++) printf("█");
        printf(" %lu\n", h->bins[i]);
    }
}

// Example: measure frame latency
histogram_t frame_histogram = {0};

void render_frame(void) {
    uint64_t start = now_ns();
    // ... rendering work ...
    uint64_t end = now_ns();
    
    double latency_us = (end - start) / 1000.0;
    histogram_add(&frame_histogram, latency_us);
}

int main(void) {
    for (int i = 0; i < 1000; i++) {
        render_frame();
    }
    histogram_print(&frame_histogram);
    return 0;
}
```

## 6) JSON Export Format for Analysis

Structure your measurements as JSON for easy post-processing:

```c
#include <stdio.h>
#include <time.h>

typedef struct {
    const char *name;
    double duration_ms;
    uint64_t timestamp;
} measurement_t;

#define MAX_MEASUREMENTS 10000
static measurement_t measurements[MAX_MEASUREMENTS];
static int measurement_count = 0;

void record_measurement(const char *name, double duration_ms) {
    if (measurement_count >= MAX_MEASUREMENTS) return;
    
    measurements[measurement_count].name = name;
    measurements[measurement_count].duration_ms = duration_ms;
    measurements[measurement_count].timestamp = now_ns();
    measurement_count++;
}

void export_measurements_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "{\n");
    fprintf(f, "  \"measurements\": [\n");
    
    for (int i = 0; i < measurement_count; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"name\": \"%s\",\n", measurements[i].name);
        fprintf(f, "      \"duration_ms\": %.3f,\n", measurements[i].duration_ms);
        fprintf(f, "      \"timestamp\": %lu\n", measurements[i].timestamp);
        fprintf(f, "    }");
        if (i < measurement_count - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
}
```

Then analyze in Python:

```python
import json

with open('measurements.json') as f:
    data = json.load(f)

measurements = data['measurements']

# Group by name and compute stats
from collections import defaultdict
by_name = defaultdict(list)

for m in measurements:
    by_name[m['name']].append(m['duration_ms'])

for name, durations in by_name.items():
    print(f"{name}:")
    print(f"  Count: {len(durations)}")
    print(f"  Mean: {sum(durations)/len(durations):.3f} ms")
    print(f"  Min: {min(durations):.3f} ms")
    print(f"  Max: {max(durations):.3f} ms")
```

## 7) Answer-Specific Measurement Tools

Instead of general profilers, build tools to answer specific questions:

### Question: "How much time is spent in memory allocation?"

```c
typedef struct {
    uint64_t total_time_ns;
    uint64_t call_count;
} alloc_profiler_t;

static alloc_profiler_t malloc_prof = {0};
static alloc_profiler_t free_prof = {0};

void *malloc_profiled(size_t size) {
    uint64_t start = now_ns();
    void *ptr = malloc(size);
    uint64_t end = now_ns();
    
    malloc_prof.total_time_ns += (end - start);
    malloc_prof.call_count++;
    
    return ptr;
}

void print_alloc_stats(void) {
    double total_ms = malloc_prof.total_time_ns / 1e6;
    printf("Malloc: %.3f ms across %lu calls (avg: %.3f µs)\n",
           total_ms, malloc_prof.call_count,
           malloc_prof.total_time_ns / 1000.0 / malloc_prof.call_count);
}
```

### Question: "How much contention do we have on this lock?"

```c
typedef struct {
    pthread_mutex_t mutex;
    uint64_t acquisitions;
    uint64_t failed_tries;  // Would-block attempts
    double total_hold_time_ms;
} mutex_profiler_t;

int mutex_lock_profiled(mutex_profiler_t *m) {
    uint64_t start = now_ns();
    int result = pthread_mutex_lock(&m->mutex);
    uint64_t end = now_ns();
    
    if (result == 0) {
        m->acquisitions++;
    }
    
    return result;
}

void mutex_unlock_profiled(mutex_profiler_t *m) {
    pthread_mutex_unlock(&m->mutex);
}

void print_mutex_stats(mutex_profiler_t *m) {
    printf("Mutex stats:\n");
    printf("  Acquisitions: %lu\n", m->acquisitions);
    printf("  Contention: %.1f%%\n", 100.0 * m->failed_tries / m->acquisitions);
}
```

## 8) Putting It Together: A Minimal Profiling Library

```c
// profile.h
#pragma once

#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    const char *name;
    uint64_t total_ns;
    uint64_t call_count;
    uint64_t min_ns;
    uint64_t max_ns;
} profile_entry_t;

#define PROFILE_SLOTS 256
static profile_entry_t profile_table[PROFILE_SLOTS];
static int profile_count = 0;

typedef struct {
    uint64_t start_ns;
    profile_entry_t *entry;
} profile_scope_t;

void profile_start(profile_scope_t *scope, const char *name) {
    scope->start_ns = now_ns();
    
    // Find or create entry
    for (int i = 0; i < profile_count; i++) {
        if (strcmp(profile_table[i].name, name) == 0) {
            scope->entry = &profile_table[i];
            return;
        }
    }
    
    if (profile_count < PROFILE_SLOTS) {
        profile_table[profile_count].name = name;
        scope->entry = &profile_table[profile_count++];
    }
}

void profile_end(profile_scope_t *scope) {
    uint64_t end_ns = now_ns();
    uint64_t elapsed = end_ns - scope->start_ns;
    
    scope->entry->total_ns += elapsed;
    scope->entry->call_count++;
    if (elapsed < scope->entry->min_ns) scope->entry->min_ns = elapsed;
    if (elapsed > scope->entry->max_ns) scope->entry->max_ns = elapsed;
}

#define PROFILE(name) \
    profile_scope_t _ps; \
    profile_start(&_ps, name); \
    defer({ profile_end(&_ps); })

void profile_print(void) {
    printf("=== PROFILE RESULTS ===\n");
    for (int i = 0; i < profile_count; i++) {
        profile_entry_t *e = &profile_table[i];
        printf("%s: %.3f ms (%lu calls, avg: %.3f ms)\n",
               e->name, e->total_ns / 1e6, e->call_count,
               e->total_ns / 1e6 / e->call_count);
    }
}
```

## 9) Best Practices for Custom Tools

| Practice                 | Why                                         |
|--------------------------|---------------------------------------------|
| Measure overhead         | Ensure tool itself isn't the bottleneck    |
| Use atomic operations    | For thread-safe counters without locks     |
| Export structured data   | JSON/CSV for easy analysis post-run        |
| Answer specific questions| Don't over-generalize; build for your need |
| Test on real hardware    | Emulator/simulator doesn't represent prod  |
| Version your measurements| Track changes in tool definitions over time|
| Combine with real profilers| Custom tools complement, don't replace, perf |

## 10) When to Build vs. Buy

| Scenario                                | Build Custom | Use perf/Instruments |
|-----------------------------------------|--------------|----------------------|
| Measuring specific application events   | ✓            | ✗                    |
| Deep system-wide profiling              | ✗            | ✓                    |
| Production monitoring (low overhead)    | ✓            | ✗                    |
| Understanding memory allocation patterns| ✓            | ~                    |
| Kernel scheduler behavior               | ✗            | ✓                    |
| Custom test harness validation          | ✓            | ✗                    |

## Summary

| Concept              | What You Learned                                     |
|----------------------|------------------------------------------------------|
| Instrumentation      | Macros & wrappers for timing code                    |
| Sampling             | Periodic snapshots; low overhead                     |
| Counters             | Thread-safe event counting                           |
| Histograms           | Latency distribution analysis                        |
| JSON export          | Structured output for external analysis              |
| Custom tools         | Answer specific perf questions without bloat         |
| Overhead awareness   | Ensure measurement tool isn't the problem            |

## Key Takeaways

- **Start simple**: Macros and counters before building complex samplers
- **Measure overhead**: Does your tool slow down the code under test?
- **Export structured data**: JSON makes it easy to analyze elsewhere
- **Answer specific questions**: Don't measure everything; focus on unknowns
- **Combine approaches**: Use custom tools + perf/Instruments for complete picture
- **Version your code**: Keep baseline measurements synchronized with source

## Homework

1. Take a simple algorithm (e.g., a sorting routine or matrix multiply)
2. Instrument it with TIMER_START/TIMER_END macros
3. Add counters for function calls (e.g., comparisons in sort)
4. Run on different input sizes and export to JSON
5. Build a histogram of operation latencies
6. Analyze: Does latency scale linearly with size? Are there hot spots?

**Starter:**

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Instrumented quicksort
static long compare_count = 0;

int cmp(const void *a, const void *b) {
    compare_count++;
    return *(int*)a - *(int*)b;
}

int main(void) {
    int sizes[] = {100, 1000, 10000, 100000};
    
    for (size_t s = 0; s < 4; s++) {
        int *arr = malloc(sizes[s] * sizeof(int));
        for (int i = 0; i < sizes[s]; i++) {
            arr[i] = rand();
        }
        
        compare_count = 0;
        TIMER_START(quicksort);
        qsort(arr, sizes[s], sizeof(int), cmp);
        TIMER_END(quicksort);
        
        printf("Size: %d, Compares: %ld\n", sizes[s], compare_count);
        free(arr);
    }
    
    return 0;
}
```

Run this and chart Compares vs. Size. Expected: O(n log n) growth.
