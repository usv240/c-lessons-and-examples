## LESSON 25 — Mock Interviews & Common Patterns

(Practice questions, model answers, pitfalls, communication patterns)

## Goals

By the end, you'll:

- Have 10 practice interview questions ready
- Know model answers that interviewers like
- Understand common pitfalls and how to avoid them
- Have scripts/templates for common situations

## 1) Practice Questions & Model Answers

Use these to practice the STEP framework from Lesson 23. Each answer is ~10 minutes.

---

### Q1: "Our app crashes when users run out of memory. How would you handle it?"

**Your answer:**

**Understand (1 min):**
```
"So the app crashes (likely OOM killer), not a graceful low-memory
warning. Should I assume:
- It's on iOS (most aggressive memory management)?
- Users can't see a warning before the crash?
- The app uses a lot of memory (images, video)?
```

**Diagnose (1 min):**
```
"Root cause: Memory allocation fails with no recovery. Three layers:
1. User-space: Check malloc return, handle NULL
2. System: Avoid triggering OOM killer
3. App: Reduce memory footprint when warned"
```

**Propose (5 min):**
```
"Fix:
1. Check malloc returns (C doesn't throw, just returns NULL)
2. Listen to iOS UIApplication.didReceiveMemoryWarning
3. Release caches when warned

Code:
// 1. Check malloc
void *buf = malloc(big_size);
if (!buf) {
    fprintf(stderr, "Out of memory!\n");
    return -1;  // Graceful error, not crash
}

// 2. Listen for memory warning (Objective-C)
[[NSNotificationCenter defaultCenter] addObserver:self
    selector:@selector(didReceiveMemoryWarning:)
    name:UIApplicationDidReceiveMemoryWarningNotification
    object:nil];

// 3. Handle it
- (void)didReceiveMemoryWarning {
    [self.imageCache removeAllObjects];  // Drop cache
    [self.videoBuffer clear];             // Release buffers
}
```
"

**Trade-offs (2 min):**
```
"Pro:
- App survives low-memory scenarios
- User experience degrades gracefully (lower quality, not crash)

Con:
- Checking every malloc adds code verbosity
- Releasing caches might cause lag if they're re-requested

Mitigation:
- Use try-catch style error handling for critical paths
- Cache intelligently (keep small items, drop large ones)"
```

**Validate (1 min):**
```
"Summary: Handle malloc failures, listen for memory warnings,
release non-critical caches when warned. App survives instead of crashing."
```

---

### Q2: "Thread A holds lock 1, Thread B holds lock 2, then each waits for the other. How would you find and fix this deadlock?"

**Your answer:**

**Understand (1 min):**
```
"So we have a deadlock (circular wait): A→B→A. Should I assume:
- It happens sometimes, not always (timing-dependent)?
- We have Instruments/perf available to debug?
- The locks are pthread_mutex_t (not lock-free)?
```

**Diagnose (1 min):**
```
"Deadlock is easy to detect: threads are stuck (not progressing),
waiting on locks. Could be circular lock (A→B→A) or same thread
acquiring same lock twice (recursive lock without RECURSIVE flag)."
```

**Propose (5 min):**
```
"Detection & fix:

1. Detect with deadlock detector:
   gcc -g -fno-omit-frame-pointer -pthread program.c
   
   // Enable lockdep (Linux)
   echo 1 > /proc/sys/kernel/lock_stat
   
   // Profile locks
   perf lock record ./program
   perf lock report  // Shows locks, wait times, contentions

2. Fix: Always acquire locks in the same order
   // BAD: Different orders
   Thread A: lock(L1), lock(L2)
   Thread B: lock(L2), lock(L1)  // Deadlock risk
   
   // GOOD: Consistent order
   Thread A: lock(L1), lock(L2)
   Thread B: lock(L1), lock(L2)  // Same order, no deadlock

3. Code fix:
   // Before: deadlock-prone
   void txn_a(void) {
       pthread_mutex_lock(&account1_lock);
       update_account1();
       pthread_mutex_lock(&account2_lock);  // Risk of deadlock
       update_account2();
       pthread_mutex_unlock(&account2_lock);
       pthread_mutex_unlock(&account1_lock);
   }
   
   void txn_b(void) {
       pthread_mutex_lock(&account2_lock);  // Different order!
       update_account2();
       pthread_mutex_lock(&account1_lock);  // Deadlock
       update_account1();
       pthread_mutex_unlock(&account1_lock);
       pthread_mutex_unlock(&account2_lock);
   }
   
   // After: always lock in order (account1 < account2)
   void txn_a(void) {
       pthread_mutex_lock(&account1_lock);
       pthread_mutex_lock(&account2_lock);
       update_account1();
       update_account2();
       pthread_mutex_unlock(&account2_lock);
       pthread_mutex_unlock(&account1_lock);
   }
   
   void txn_b(void) {
       pthread_mutex_lock(&account1_lock);  // Same order
       pthread_mutex_lock(&account2_lock);
       update_account1();
       update_account2();
       pthread_mutex_unlock(&account2_lock);
       pthread_mutex_unlock(&account1_lock);
   }
```

**Trade-offs (2 min):**
```
"Pro:
- Deadlock is prevented (not just detected)
- Simple rule: always lock in order

Con:
- Requires discipline (developers must follow lock order)
- If you miss one place, deadlock still possible

Alternative:
- Use lock-free structures (atomics)
- Use RCU (Linux) for readers
- Use try-lock with backoff (give up and retry)"
```

**Validate (1 min):**
```
"Summary: Establish global lock order, enforce it everywhere.
Use lockdep/perf to detect violations. Test with stress tools
(threadtest, chaos monkey)."
```

---

### Q3: "Our server handles 10K requests/sec on 8 cores, but only uses 2 cores. Why?"

**Your answer:**

**Understand (1 min):**
```
"So the server underutilizes cores (25% usage on 8 cores). Should I assume:
- It's a web server (HTTP, TCP)?
- Request latency is acceptable?
- You have perf/Instruments to profile?
```

**Diagnose (1 min):**
```
"Likely bottlenecks (in order):
1. Lock contention (all threads fighting one lock)
2. I/O blocking (threads wait on network/disk)
3. Scheduler imbalance (work not evenly distributed)
4. Cache locality (threads thrashing shared cache)

I'd profile with perf to see where time is spent."
```

**Propose (5 min):**
```
"Steps:
1. Profile:
   perf stat -e context-switches,cycles,stalled-cycles-frontend \\
             ./server
   
   If context-switches > 10K/sec: lock contention
   If stalled-cycles high: cache misses or I/O wait

2. Fix (most likely: lock contention)
   // Bad: single global lock
   pthread_mutex_t lock;
   shared_data_t global_state;
   
   void handle_request(request_t *req) {
       pthread_mutex_lock(&lock);
       process_request(req, &global_state);
       pthread_mutex_unlock(&lock);
   }
   
   // Good: lock-free or per-core state
   __thread shared_data_t local_state;  // Thread-local
   
   void handle_request(request_t *req) {
       process_request(req, &local_state);  // No lock!
   }
   
   // OR: reader-writer lock if mostly reads
   pthread_rwlock_t lock;
   
   void handle_request(request_t *req) {
       pthread_rwlock_rdlock(&lock);  // Read lock, parallel
       process_request(req, &global_state);
       pthread_rwlock_unlock(&lock);
   }

3. Expected impact: Scale from 2 cores to 6+ cores
   (each core can run independently)"
```

**Trade-offs (2 min):**
```
"Pro:
- Threads can run in parallel (no waiting)
- 3–4x throughput improvement typical

Con:
- Thread-local storage duplicates memory
- Reader-writer locks add complexity
- Need careful testing (data races easier with lock-free)

Alternative:
- Use lock-free queues (atomic CAS operations)
- Use async I/O (io_uring, libuv) instead of blocking threads"
```

**Validate (1 min):**
```
"Summary: Profile to find bottleneck (likely lock contention).
Redesign to reduce lock scope or use lock-free structures.
Expected 3–4x throughput improvement."
```

---

## 2) Common Pitfalls & Fixes

### Pitfall 1: Over-engineering the solution

**What it sounds like:**
```
"I would build a distributed lock-free queue with exponential backoff,
persistent metrics, and circuit breaker pattern..."
```

**Why it's bad:**
- Overkill, hard to maintain
- Shows you don't prioritize simple solutions
- Difficult to implement in 45 minutes

**Fix:**
```
"I'd start with a simple mutex. If profiling shows lock contention,
I'd upgrade to reader-writer lock. Only use lock-free if profiling
proves the lock is the bottleneck."
```

---

### Pitfall 2: No measurement mindset

**What it sounds like:**
```
"I think the problem is memory allocation, so I'd implement a pool."
```

**Why it's bad:**
- Guessing without data
- Might optimize wrong thing
- Interviewer will ask "but did you measure?"

**Fix:**
```
"I'd profile first with Instruments/perf to see where time is spent.
If malloc is >20% of CPU, I'd implement object pooling. Otherwise,
I'd look elsewhere."
```

---

### Pitfall 3: Ignoring trade-offs

**What it sounds like:**
```
"I'd use lock-free atomics for everything."
```

**Why it's bad:**
- No solution is free
- Shows incomplete thinking

**Fix:**
```
"Lock-free atomics are great for simple counters, but for complex
data structures they're error-prone. I'd use them judiciously after
profiling proves the lock is a bottleneck."
```

---

### Pitfall 4: Not listening to follow-ups

**What it sounds like:**
```
Interviewer: "What if the network is actually the bottleneck?"
You: "It's probably not because..."
```

**Why it's bad:**
- Interviewer is testing if you can pivot
- You're being defensive, not collaborative

**Fix:**
```
Interviewer: "What if the network is actually the bottleneck?"
You: "Good point, I hadn't considered that. If network is the issue,
then buffering is the answer, not optimization. Let me reconsider..."
```

---

### Pitfall 5: Vague time estimates

**What it sounds like:**
```
"This would improve performance significantly."
```

**Why it's bad:**
- No specificity
- Hard to evaluate

**Fix:**
```
"This would reduce frame latency from 25ms to 18ms (28% improvement),
which should eliminate the 60 fps stalls we're seeing."
```

---

## 3) Communication Templates (Use in Interviews)

### Template 1: "I don't know, but I'd figure it out"

```
"I don't know [specific detail], but I'd [how]. My best guess is [answer],
because [reasoning]."
```

**Example:**
```
"I don't know the exact L3 hit rate on M1, but I'd run 'perf stat'
to find out. My best guess is ~90% hit rate because the working set
is 200 MB and L3 is 8 MB per cluster, suggesting some misses."
```

---

### Template 2: "Let me reconsider"

```
"That's a good point. I was assuming [assumption], which might not be true.
If [condition], then [different solution] makes more sense because [reasoning]."
```

**Example:**
```
"That's a good point. I was assuming the network is stable, which might
not be true. If the network is jittery, then larger buffers make sense
to absorb variance, not optimization."
```

---

### Template 3: "I'd measure"

```
"I'd measure [metric] to find out. Current hypothesis is [H1],
but [H2] is also possible."
```

**Example:**
```
"I'd measure context switch rate with 'perf stat' to find out. Current
hypothesis is lock contention (would show high context switches), but
it could also be I/O blocking (would show high blocked time)."
```

---

### Template 4: Explaining trade-offs

```
"[Solution] is good because [benefit], but the trade-off is [downside].
So I'd use it if [condition]. In our case, [we should/shouldn't] because [why]."
```

**Example:**
```
"Lock-free atomics are good because they're fast, but the trade-off is
they're hard to debug and only work for simple operations. So I'd use
them if profiling shows the lock is a bottleneck. In our case, the lock
is only 8% of CPU, so a simpler solution (reader-writer lock) is better."
```

---

## 4) Time Management Checklist

Use this during the interview to stay on track:

```
[ ] 0–5 min: Clarified the problem. Repeated back. Asked 2–3 questions.
[ ] 5–10 min: Identified likely bottleneck. Explained how I'd measure.
[ ] 10–25 min: Proposed solution. Showed code or pseudocode.
[ ] 25–35 min: Discussed trade-offs (pros, cons, alternatives).
[ ] 35–40 min: Summarized. Ready for questions.
[ ] 40–45 min: Answered follow-ups or discussed edge cases.
```

**If you're behind:**
- Skip code details ("Implementation omitted for time")
- Jump to summary ("In short, the fix is [X]")

**If you're ahead:**
- Discuss edge cases ("What if X fails?")
- Suggest monitoring ("I'd add metrics to track...")
- Offer alternatives ("Another approach would be...")

---

## 5) Stress Test Scenarios (Be ready for these)

### Scenario 1: "That won't actually work because..."

**Interviewer:** "Your lock-free solution still needs synchronization for
the data structure itself. How does that help?"

**Your response (using template):**
```
"You're right, I oversimplified. The lock-free atomic only works for
simple values. For complex structures, I'd still need synchronization.
Let me reconsider: maybe lock-free isn't the answer here. RCU or
read-write locks might be better because..."
```

---

### Scenario 2: "What if the problem is different?"

**Interviewer:** "What if it's actually a cache miss problem, not a lock?"

**Your response:**
```
"Good point. If it's cache misses, then the fix is different:
improve memory locality instead of removing locks. How would I tell?
Run 'perf stat' and look for L3 cache miss rate. If > 10%, cache
is the issue. If lock contention is high (context switches > 1000/sec),
then locks are the issue."
```

---

### Scenario 3: "How confident are you?"

**Interviewer:** "You sound uncertain. Are you really confident in this solution?"

**Your response:**
```
"I'm confident in the approach (profile → identify bottleneck → fix →
measure), but not in the specific solution until I profile. The
bottleneck could be CPU, memory, I/O, or thermal. My top guess is
lock contention, but I wouldn't commit to a fix without data."
```

---

## 6) Apple Interview Specifics

### What Apple cares about

1. **Measurement mindset** — Always profile before guessing
2. **Trade-off awareness** — Know the cost of every decision
3. **Systems thinking** — Understand how components interact
4. **Communication** — Explain clearly; adjust when questioned
5. **Pragmatism** — Simple solutions first, complexity only when needed

### Red flags Apple will notice

- Jumping to solution without understanding the problem
- Claiming huge improvements without justification
- Not mentioning measurement/profiling
- Overcomplicating (distributed systems, exotic structures)
- Not adjusting when questioned

---

## 7) Final Checklist: Before Your Interview

```
[ ] I've practiced STEP framework on 5+ questions
[ ] I can explain GCD, QoS, Unified Memory clearly
[ ] I know what "profile first" means and can name tools
[ ] I can discuss trade-offs without sounding uncertain
[ ] I can pivot when asked follow-ups (not defensive)
[ ] I can say "I don't know, but I'd figure it out" with confidence
[ ] I can name Apple-specific patterns (not just generic patterns)
[ ] I've practiced talking out loud (sounds natural, not scripted)
[ ] I have 3–5 "hard problem I solved" stories ready
[ ] I know my weaknesses and can talk about them honestly
```

---

## 8) "Tell Me About a Hard Problem You Solved"

This is a critical interview question. Prepare 2–3 stories.

### Story structure (STAR)

- **Situation**: Context (what was the problem?)
- **Task**: Your role (what were you responsible for?)
- **Action**: What you did (steps, tools, code)
- **Result**: Outcome (metrics, impact)

### Example story: "Optimize a slow app"

**Situation:**
```
"I was working on a video streaming app. Users reported the app
felt sluggish when playing 4K video on iPhones."
```

**Task:**
```
"I was the engineer responsible for the media pipeline, so I owned
fixing the issue."
```

**Action:**
```
"I profiled with Instruments (Time Profiler, Memory Allocations).
Found that malloc was 30% of CPU time. Implemented object pooling
to reuse buffers. Also noticed network thread blocked by decode
thread—switched to reader-writer lock."
```

**Result:**
```
"App felt smooth (60 fps maintained). Frame drops went from 12/min
to 2/min. Memory allocations dropped from 500K/sec to 50K/sec.
Users reported app 'feels much faster' in feedback."
```

**Tips:**
- Use metrics ("30% of CPU", "500K/sec")
- Show you profiled before acting
- Explain the fix clearly
- Quantify the impact

---

## Homework

1. **Pick 3 practice questions** from Section 1 and answer out loud (10 min each)
2. **Record yourself** (phone camera)
3. **Review**: Did you spend too long explaining? Did you mention trade-offs?
4. **Practice again** with different questions until it feels natural
5. **Prepare your "hard problem" story** using STAR format

**Practice questions to add:**
- "Our database is slow. How would you find the bottleneck?"
- "Thermal throttling kills our GPU compute. What would you do?"
- "We're losing 30% throughput from thread contention. How to fix?"
- "Cache misses are high (20%). How would you optimize?"
- "Memory leaks cause app to crash after 1 hour. How to find and fix?"

**Tip:** Practice with friends. Have them interrupt you with follow-ups:
- "But what if X?"
- "How confident are you?"
- "Can you simplify that?"
- "Why not use Y instead?"

This trains you to stay calm and pivot when questioned.
