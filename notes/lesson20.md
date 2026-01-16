## LESSON 20 — Performance Metrics, Dashboards & Trending

(How to track, measure, and visualize performance across versions and teams)

### Simple Introduction

**What is a performance metric?** A number you measure repeatedly: frame latency, memory usage, CPU%, etc.

**Why track metrics?** So you know if your code is getting faster or slower as you make changes. Without metrics, you're just guessing.

**Key insight**: Not all metrics matter. Measure things that affect real users (latency, memory, crashes)—not obscure things like "cache evictions."

**What you'll learn**: How to pick metrics, detect when code gets slower, and communicate performance to your team.

## Goals

By the end of this lesson, you'll understand:

- How to define meaningful performance metrics (not just FPS or throughput)
- How to collect baseline data and track regression/improvement over time
- How to build simple performance dashboards
- How to communicate performance trends to teams
- How to set performance budgets and alert on violations

## 1) What Makes a Good Performance Metric? (Simple Rules)

**Quick answer**: A metric should measure something real that users care about.

Not all metrics are useful. A good metric is:

| Property          | Example (Good)                    | Example (Bad)                 |
|-------------------|-----------------------------------|-------------------------------|
| **Measurable**    | "p99 frame latency: 16.5ms"       | "App feels fast"              |
| **Actionable**    | "malloc calls increased 12%"      | "Performance down 5%"         |
| **Relevant**      | "p95 load time: 800ms"            | "CPU usage: 47%"              |
| **Normalized**    | "ms per MB processed"             | "Total time (varies w/ input)" |
| **Traceable**     | Linked to code commit              | Anonymous data point          |

## 2) Common Performance Metrics by Layer

### User-Facing Metrics

| Metric                | Measured How                | Acceptable Target          | Red Flag      |
|-----------------------|------------------------------|----------------------------|---------------|
| Frame latency (p99)   | Time from input→rendered     | ≤16ms (60 fps)             | >33ms (30 fps)|
| Load time (p95)       | Time to interactive          | <1s                        | >3s           |
| Memory (peak)         | Max resident set size        | <100MB (typical app)       | >500MB        |
| Battery drain (mAh/h) | Measured during workload     | <50 mAh/h (streaming)      | >200 mAh/h    |
| Startup time          | First frame to screen        | <200ms                     | >1s           |

### System Metrics

| Metric                | Measured How                | Acceptable Target          | Red Flag      |
|-----------------------|------------------------------|----------------------------|---------------|
| CPU utilization       | % of cores in use            | Varies (50–80%)            | >95% sustained|
| Cache miss rate       | perf stat                   | <10% L3 miss rate          | >30%          |
| Thermal (°C)          | Sensors                      | <65°C sustained            | >85°C         |
| Context switches/sec  | perf stat                   | <1000/sec                  | >10000/sec    |
| Page faults/sec       | vmstat, perf stat           | <100/sec                   | >1000/sec     |

### Networking/Media Metrics

| Metric                | Measured How                | Acceptable Target          | Red Flag      |
|-----------------------|------------------------------|----------------------------|---------------|
| Bitrate adaptation    | Track ABR switches           | <2 switches/min (stable)   | >5/min        |
| Rebuffer events       | Count stalls                 | 0 per hour                 | >5 per hour   |
| Frame drops           | Decoder/renderer             | 0 per minute               | >10 per hour  |
| Keyframe distance     | IDR frame interval           | 2–5s (typical)             | >10s or <1s   |

## 3) Baseline & Regression Detection

### Setting a baseline

Baseline = a known-good state against which you measure change.

**Steps:**

1. Choose a reference commit (typically main branch, stable release)
2. Run your workload 10 times, record each metric
3. Calculate mean and standard deviation
4. Store in version control (e.g., `perf-baseline.json`)

**Example baseline file:**

```json
{
  "commit": "abc123def456",
  "date": "2026-01-15",
  "metrics": {
    "frame_latency_p99_ms": {
      "mean": 14.5,
      "stddev": 1.2,
      "samples": 10
    },
    "memory_peak_mb": {
      "mean": 87.3,
      "stddev": 2.1,
      "samples": 10
    },
    "malloc_calls": {
      "mean": 1243000,
      "stddev": 15000,
      "samples": 10
    }
  }
}
```

### Detecting regressions

After code changes, re-measure. If a metric exceeds baseline + 2 std dev, it's likely a regression.

```c
// C program to check regression
#include <stdio.h>
#include <math.h>

int main(void) {
    double baseline_mean = 14.5;
    double baseline_stddev = 1.2;
    double new_measurement = 17.8;  // Recent run
    
    double z_score = (new_measurement - baseline_mean) / baseline_stddev;
    // z_score = (17.8 - 14.5) / 1.2 = 2.75
    
    if (z_score > 2.0) {
        printf("REGRESSION DETECTED: %.1f sigma above baseline\n", z_score);
        printf("Baseline: %.1f ± %.1f ms\n", baseline_mean, baseline_stddev);
        printf("Measured: %.1f ms\n", new_measurement);
    }
    return 0;
}
```

**Rule of thumb**: If z_score > 2.0, investigate; if > 3.0, halt the commit.

## 4) Building a Simple Performance Dashboard

A dashboard collects metrics over time and visualizes trends.

### Minimal text-based dashboard

```bash
#!/bin/bash
# perf_monitor.sh — capture metrics every hour

OUTDIR="perf_logs"
mkdir -p "$OUTDIR"

while true; do
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    
    # CPU usage
    CPU=$(top -bn1 | grep "CPU" | awk '{print $2}')
    
    # Memory
    MEM=$(top -bn1 | grep "Mem" | awk '{print $3}')
    
    # Frame latency (example: grep app logs)
    LATENCY=$(tail -1 app.log | awk '{print $NF}')
    
    # Write to CSV
    echo "$TIMESTAMP,$CPU,$MEM,$LATENCY" >> "$OUTDIR/metrics.csv"
    
    sleep 3600  # every hour
done
```

Then visualize with a simple plot:

```bash
# Install gnuplot, then:
gnuplot << EOF
set datafile separator ","
plot 'perf_logs/metrics.csv' using 1:2 with lines title 'CPU %'
EOF
```

### JSON-based dashboard (more structured)

```c
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

void log_metrics(const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) return;
    
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    
    // Example: mock metrics (in reality, measure real values)
    double cpu_percent = 42.5;
    double memory_mb = 125.3;
    double frame_latency_ms = 14.8;
    
    fprintf(f, "{\n");
    fprintf(f, "  \"timestamp\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    fprintf(f, "  \"cpu_percent\": %.1f,\n", cpu_percent);
    fprintf(f, "  \"memory_mb\": %.1f,\n", memory_mb);
    fprintf(f, "  \"frame_latency_ms\": %.1f\n", frame_latency_ms);
    fprintf(f, "}\n");
    
    fclose(f);
}

int main(void) {
    for (int i = 0; i < 10; i++) {
        log_metrics("perf_dashboard.jsonl");
        sleep(1);
    }
    return 0;
}
```

Output (`perf_dashboard.jsonl`):

```json
{
  "timestamp": "2026-01-15 14:30:22",
  "cpu_percent": 42.5,
  "memory_mb": 125.3,
  "frame_latency_ms": 14.8
}
```

Then parse and plot in Python/JS.

## 5) Automated Performance Testing (CI/CD Integration)

### Example: Commit hook that measures perf

```bash
#!/bin/bash
# .git/hooks/post-commit (run after each commit)

# Build
gcc -O2 -o myapp myapp.c

# Measure 5 times, average
TOTAL=0
for i in {1..5}; do
    TIME=$( { time ./myapp; } 2>&1 | grep real | awk '{print $2}' )
    TOTAL=$(echo "$TOTAL + $TIME" | bc)
done

AVG=$(echo "scale=3; $TOTAL / 5" | bc)

# Compare to baseline
BASELINE=1.234  # seconds, from perf_baseline.json
DIFF=$(echo "$AVG - $BASELINE" | bc)
PERCENT=$(echo "scale=1; ($DIFF / $BASELINE) * 100" | bc)

echo "Average runtime: ${AVG}s (baseline: ${BASELINE}s, change: ${PERCENT}%)"

if (( $(echo "$PERCENT > 10" | bc -l) )); then
    echo "WARNING: >10% performance regression!"
    exit 1
fi
```

## 6) Performance Budgets

A performance budget is a limit on how much a metric can change per commit.

**Example budget:**

```json
{
  "frame_latency_ms": {
    "budget": 0.5,
    "description": "Max 0.5ms increase per commit"
  },
  "memory_peak_mb": {
    "budget": 2.0,
    "description": "Max 2MB increase per commit"
  },
  "malloc_calls": {
    "budget": 50000,
    "description": "Max 50K additional allocations per commit"
  }
}
```

If a commit increases frame latency from 14.5ms to 15.2ms (+0.7ms), it violates the budget and should be reviewed.

## 7) Communicating Trends to Teams

### Daily standup slide

```
PERFORMANCE SNAPSHOT (Last 24 Hours)

Frame Latency (p99):
├─ Yesterday:  14.5 ± 1.2 ms ✓
├─ Today:      14.8 ± 1.4 ms ✓
└─ Trend:      ↗ +2.1% (within budget)

Memory (Peak):
├─ Yesterday:  87.3 MB
├─ Today:      89.1 MB
└─ Trend:      ↗ +2.1% (investigate commit abc123)

CPU Utilization:
├─ Yesterday:  52% avg
├─ Today:      48% avg
└─ Trend:      ↘ -7.7% (improvement from PR #456)
```

### Weekly performance report

```
WEEKLY PERFORMANCE REPORT (Jan 8–14, 2026)

Metric           | Mean     | Week Ago | Change   | Status
-----------------+----------+----------+----------+--------
Frame Latency p99| 14.8 ms  | 14.5 ms  | +2.1%    | ✓ OK
Memory Peak      | 89.1 MB  | 85.2 MB  | +4.6%    | ⚠ Watch
Cache Miss Rate  | 8.2%     | 7.9%     | +0.3%    | ✓ OK
Thermal Peak     | 62°C     | 58°C     | +6.9%    | ⚠ Investigate

Regressions:
- Commit abc123: +0.5ms frame latency (minor, in budget)
- PR #456: -7.7% CPU (improvement!)

Action Items:
1. Investigate memory growth (89.1 MB vs 85.2 MB)
2. Profile thermal increase (might be due to parallelization in PR #456)
```

## 8) Tools for Metrics & Dashboards

| Tool           | Best For                      | Setup Time |
|----------------|-------------------------------|------------|
| Prometheus     | Server metrics, time-series   | Moderate   |
| Grafana        | Visual dashboards             | Easy       |
| Influx DB      | High-volume metric storage    | Moderate   |
| Datadog        | Cross-platform monitoring    | Easy       |
| Custom JSON    | Simple app-level metrics      | Minimal    |
| perf stat      | CPU/cache counters            | Minimal    |

## 9) Real-World Example: Tracking a Streaming Engine

You're maintaining the mini-engine from Lesson 17. Track these metrics:

```json
{
  "name": "Mini Streaming Engine",
  "timestamp": "2026-01-15T14:22:33Z",
  "test_config": {
    "duration_sec": 60,
    "bitrate_kbps": 2000,
    "segments_count": 10
  },
  "metrics": {
    "throughput": {
      "mb_per_second": 0.25,
      "unit": "MB/s"
    },
    "buffer_utilization": {
      "mean_percent": 65,
      "peak_percent": 92,
      "min_percent": 12
    },
    "decode_latency": {
      "p50_ms": 8.2,
      "p99_ms": 15.3,
      "p99p9_ms": 22.1
    },
    "lock_contention": {
      "mutex_acquisitions": 1200000,
      "failed_acquisitions": 8432,
      "avg_hold_time_us": 0.4
    },
    "thread_switches": {
      "context_switches_per_sec": 750,
      "scheduler_events": 45000
    }
  }
}
```

Track this every commit. Over time, you'll see:
- Gradual improvements from optimizations
- Sudden regressions from code changes
- Seasonal patterns (e.g., thermal throttling on hot days)

## 10) Performance Regression Workflow

1. **Baseline established**: You have perf-baseline.json with known-good metrics
2. **New commit arrives**: Team submits code change
3. **Automated test runs**: CI runs performance benchmark
4. **Compare to baseline**: Check if metrics exceed budget
5. **Result**:
   - ✓ **Pass**: Commit accepted, dashboard updated
   - ⚠ **Warning**: 1–2σ above baseline, note in PR comments
   - ✗ **Fail**: 3σ+ above baseline, block merge, require investigation

**Example CI check:**

```bash
#!/bin/bash
# ci_perf_check.sh

NEW_LATENCY=$(./measure_latency.sh)
BASELINE_LATENCY=14.5
TOLERANCE=1.5  # 1.5ms budget

DIFF=$(echo "$NEW_LATENCY - $BASELINE_LATENCY" | bc)

if (( $(echo "$DIFF > $TOLERANCE" | bc -l) )); then
    echo "❌ PERF REGRESSION: $DIFF ms over budget ($TOLERANCE ms)"
    exit 1
else
    echo "✓ PERF OK: Within budget"
    exit 0
fi
```

## Summary

| Concept                | What You Learned                                        |
|------------------------|---------------------------------------------------------|
| Good metrics           | Measurable, actionable, relevant, normalized, traceable |
| Baselines              | Establish known-good reference points                   |
| Regression detection   | Statistical comparison (z-scores) against baseline     |
| Dashboards             | Collect & visualize metrics over time                   |
| CI/CD integration      | Automate perf testing in your build pipeline           |
| Performance budgets    | Limits on per-commit metric changes                     |
| Trend communication    | Share findings clearly with teams                       |

## Key Takeaways

- **Not all metrics matter**: Pick metrics tied to user experience or system health
- **Measure in the same way**: Consistency enables trend detection
- **Automate measurements**: Manual testing is error-prone
- **Set budgets**: Prevents death by a thousand cuts
- **Communicate clearly**: Non-technical stakeholders need simple summaries
- **Track over time**: Trends reveal patterns; single measurements lie

## Homework

1. Choose a simple program (e.g., Lesson 17 or a loop-heavy function)
2. Define 3–5 metrics (latency, memory, CPU, throughput, etc.)
3. Measure baseline: Run 10 times, record mean/stddev
4. Make a code change (e.g., add caching, optimize a loop)
5. Measure again, detect regression/improvement
6. Build a simple CSV dashboard and plot the trend
7. Write a one-paragraph summary of the performance change

**Starter code:**

```c
#include <stdio.h>
#include <time.h>

double measure_time(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Your workload
    for (long i = 0; i < 100000000; i++) {
        volatile int dummy = i * 2;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    return elapsed;
}

int main(void) {
    printf("timestamp,elapsed_s\n");
    for (int run = 0; run < 10; run++) {
        double t = measure_time();
        printf("%d,%.6f\n", run, t);
    }
    return 0;
}
```

Run this, save to CSV, then plot with Excel or gnuplot to visualize variance.
