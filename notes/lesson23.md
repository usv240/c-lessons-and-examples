## LESSON 23 — System Design Interview Framework

(How to structure a performance optimization answer in 45 minutes)

## Goals

By the end, you'll know:

- A repeatable framework for system design interviews
- How to talk about trade-offs clearly and concisely
- How to pivot when asked "what if" or "why"
- Communication templates that work under pressure
- How to avoid common pitfalls

## 1) The Interview Framework (STEP Framework)

Use this structure for every system design or optimization question. It takes ~40 minutes, leaving 5 for follow-ups.

### STEP 1: Understand the Problem (5 minutes)

**What to do:**
- Listen carefully
- Repeat back what you heard (shows you're listening)
- Ask clarifying questions

**Template:**
```
"So the problem is: [problem]. The constraint is [constraint].
Did I get that right? Should I assume [assumption]?"
```

**Example:**
```
Interviewer: "We have a streaming app that freezes on M1 Macs."

You: "So users are experiencing frame drops on M1 Macs during video playback.
I'm assuming:
1. It's during 1080p playback
2. Network is stable
3. The freeze lasts 1–2 seconds

Is that correct? Any other context?"
```

**Mistakes to avoid:**
- Don't jump to solutions immediately
- Don't assume constraints
- Don't nod silently—clarify out loud

### STEP 2: Define the Bottleneck (5 minutes)

**What to do:**
- Identify where time is actually spent
- Suggest measurement approach
- Pick the #1 bottleneck

**Template:**
```
"The bottleneck could be:
1. [CPU bound] — decoder taking 60% of time
2. [Memory] — allocations blocking
3. [I/O bound] — network latency

To find out, I'd measure:
- CPU profiles (perf/Instruments)
- Memory allocations
- Network throughput

Most likely: [reason why]"
```

**Example:**
```
"For frame freezes during playback:

Bottleneck could be:
1. CPU — decoder can't keep up (but M1 is fast)
2. Memory — too many allocations, GC pauses
3. Lock contention — threads waiting on mutexes
4. Thermal — P-cores throttled to E-core speed

I'd profile to confirm, but my guess is lock contention because:
- M1 has plenty of CPU
- Video playback is I/O heavy (network waiting)
- Multiple threads (network, decode, render) likely fighting over locks"
```

**Mistakes to avoid:**
- Don't guess without reasoning
- Don't list 10 possible causes (narrows it down)
- Don't skip the measurement step

### STEP 3: Propose a Solution (15 minutes)

**What to do:**
- Start simple
- Explain the fix in 2–3 sentences
- Show code or pseudocode
- Explain why it helps

**Template:**
```
"My solution:
1. [What] — [specific change]
2. [How it works] — [mechanism]
3. [Why it helps] — [addresses bottleneck]

Expected impact: [metric] improves by ~[percentage]
"
```

**Example (lock contention fix):**
```
"My solution: Replace a single global mutex with reader-writer locks.

How it works:
- Network thread (reader) acquires shared lock to read buffer state
- Decoder thread (reader) acquires shared lock to read packets
- Only the main scheduler (writer) needs exclusive lock
- Multiple readers can hold the lock simultaneously

Why it helps:
- Network and decode threads no longer block each other
- Main thread still protected (prevents race conditions)
- Expected: 50% reduction in context switches, smoother playback

Code:
pthread_rwlock_t buf_lock = PTHREAD_RWLOCK_INITIALIZER;

// Network thread
pthread_rwlock_rdlock(&buf_lock);    // Shared lock
packet *p = get_next_packet(&buffer);
pthread_rwlock_unlock(&buf_lock);

// Decoder thread
pthread_rwlock_rdlock(&buf_lock);    // Shared lock
decode_frame(p);
pthread_rwlock_unlock(&buf_lock);
"
```

**Mistakes to avoid:**
- Don't propose code without explaining first
- Don't claim huge improvements without justification ("100x faster" is unrealistic)
- Don't ignore trade-offs in the solution

### STEP 4: Discuss Trade-offs (10 minutes)

**What to do:**
- List 2–3 downsides of your solution
- Explain when it's worth it
- Suggest alternatives

**Template:**
```
"Trade-offs of my solution:

Pro:
- [benefit 1]
- [benefit 2]

Con:
- [downside 1] — [mitigation]
- [downside 2] — [when it matters]

Alternative:
- [other approach] — [why worse/better]

I choose my solution because: [clearest reasoning]
"
```

**Example (reader-writer lock trade-offs):**
```
"Trade-offs:

Pro:
- Multiple threads can read simultaneously
- Minimal code change

Con:
- Writer acquires lock longer (exclusive)
- If main thread writes often, we don't gain much

Mitigation:
- Profile to confirm main thread isn't the bottleneck
- If it is, move to lock-free data structure (atomic reads)

Alternative: Lock-free queue
- Better latency, but harder to implement
- Overkill if rwlock already fixes the problem

I choose rwlock because:
- Simple to implement (one change)
- High likelihood of helping (multiple reader threads)
- Low risk (well-tested primitive)
- Can upgrade to lock-free later if profiling shows it's still slow
"
```

**Mistakes to avoid:**
- Don't say your solution has no downsides
- Don't list downsides without mitigations
- Don't spend equal time on bad alternatives

### STEP 5: Validate & Pivot (5 minutes)

**What to do:**
- Summarize your solution
- Wait for follow-ups
- Be ready to pivot if interviewer asks "what if"

**Template:**
```
"Summary:
- Problem: [1 sentence]
- Root cause: [1 sentence]
- Solution: [1 sentence]
- Expected impact: [metric + % improvement]

Ready for questions!"
```

**Example pivots (be ready for these):**
```
Q: "What if lock contention isn't the real bottleneck?"
A: "Good point. I'd profile first to confirm. If it's actually
   CPU (decoder can't keep up), I'd move to a lock-free queue
   and optimize the decoder itself (better cache locality, SIMD)."

Q: "What about memory overhead of reader-writer locks?"
A: "Minimal—rwlock is just a few pointers. If memory is tight
   (embedded device), I'd use a simpler spinlock or even
   CAS-based atomic instead."

Q: "How would you test this?"
A: "Before & after profile with Instruments:
   - Measure context switches
   - Measure frame latency (p99)
   - Measure CPU utilization
   - Then A/B test on real devices"
```

**Mistakes to avoid:**
- Don't freeze when asked a follow-up
- Don't say "I don't know"—say "I'd investigate [how]"
- Don't stick to wrong answers if questioned

## 2) Communication Templates (Use These Exactly)

When talking about trade-offs, use these templates. They sound professional and show clear thinking.

### Template 1: Explaining a trade-off

```
"[Solution] is good because [benefit], but it has a downside:
[downside]. So I'd only use it if [condition]. In our case,
[condition is/isn't true], so [decision]."
```

**Example:**
```
"Inline assembly is good because it's fast, but it has a downside:
it's hard to maintain and compiler-specific. So I'd only use it if
profiling shows this function is actually a bottleneck. In our case,
the decoder is 45% of time, so it's worth it."
```

### Template 2: When you don't have all the info

```
"I don't know [detail], but I'd measure [how] to find out.
My best guess based on [reasoning] is [answer]."
```

**Example:**
```
"I don't know the exact L3 cache miss rate on M1, but I'd run
'perf stat' to find out. My best guess based on our access pattern
is ~5% miss rate, which is good."
```

### Template 3: Pivoting to a new idea

```
"You're right, [old assumption] might not be true. Let me reconsider.
If [new condition], then [new solution] would be better because
[reasoning]."
```

**Example:**
```
"You're right, lock contention might not be the issue. Let me
reconsider. If the network is actually the bottleneck (not threads),
then a larger buffer would be better because it reduces stalls when
network hiccups."
```

## 3) Common Pitfalls & Fixes

| Pitfall | What It Looks Like | Fix |
|---------|-------------------|-----|
| **No measurements** | "I think it's slow because..." | "I'd profile first: perf/Instruments" |
| **Vague trade-offs** | "The solution has trade-offs" | Name 2–3 specific trade-offs with mitigations |
| **Overcomplicating** | "I'd use a distributed lock-free queue with CRDT..." | Start simple (mutex), profile, upgrade if needed |
| **Ignoring follow-ups** | Talking past the interviewer's objection | Pause, acknowledge, reconsider |
| **No numbers** | "This is faster" | "I expect 30–50% latency improvement" |
| **Assuming too much** | Jumping to "it's definitely a cache miss" | Ask: "Should I assume..." |

## 4) Time Management

**45-minute interview breakdown:**

| Phase        | Time  | What | Key Phrase |
|--------------|-------|------|------------|
| Clarify      | 5 min | Repeat back the problem | "Did I get that right?" |
| Diagnose     | 5 min | Identify bottleneck | "The bottleneck is likely..." |
| Propose      | 15 min| Solution + code | "My solution is..." |
| Trade-offs   | 10 min| Pros, cons, alternatives | "Trade-offs are..." |
| Validate     | 5 min | Summary + ready for questions | "To summarize..." |
| **Spare**    | **5 min** | **Buffer for follow-ups** | |

**If you're running over:**
- Skip details ("I'd implement X, details omitted for time")
- Jump to summary ("In short: [solution], trade-off is [trade-off]")
- Ask if interviewer wants more depth

**If you finish early:**
- Discuss edge cases ("What if the network is offline?")
- Suggest monitoring ("I'd add metrics to track...")
- Offer alternatives ("Another approach: [solution]")

## 5) Practice Framework: 3 Sample Questions

### Sample Q1: "Our app feels sluggish on old iPhones. How would you optimize it?"

**Your answer (using STEP):**

**Understand (1 min):**
```
"So the app is slow on older iPhones (iPhone XS, not M1).
Should I assume it's a UI responsiveness issue (buttons lag)
rather than throughput (background tasks slow)?"
```

**Diagnose (1 min):**
```
"Likely bottleneck: CPU (older phones have weak CPU) or memory
(allocations trigger GC pauses). I'd profile with Instruments:
Time Profiler + Memory Allocations."
```

**Propose (5 min):**
```
"If it's CPU: Optimize hot loops (avoid expensive operations in
tight loops), use SIMD, reduce object creation.

If it's memory: Pool objects (reuse rather than new), reduce
allocations during frame render.

I'd start with memory because iPhones trigger GC under pressure."
```

**Trade-offs (2 min):**
```
"Object pooling is simple and effective, but trades memory for speed
(pool sits in RAM even when not in use). Worth it for UI because
responsiveness > peak memory."
```

**Validate (1 min):**
```
"Summary: Profile to find bottleneck (likely memory), implement
object pooling or better GC hints, measure improvement."
```

---

### Sample Q2: "Core Media drops frames during HLS streaming. How would you debug it?"

**Understand (1 min):**
```
"So users see stuttering during video playback. Should I assume:
- Network is stable (not buffering)?
- It's iOS/macOS (not web)?
- Happens on all devices or specific ones (e.g., M1)?
```

**Diagnose (1 min):**
```
"Could be: network jitter, decoder CPU, render thread blocking,
lock contention, or thermal throttle. I'd use Instruments System
Trace to see which thread is slow."
```

**Propose (5 min):**
```
"Steps:
1. Capture System Trace on device with frame drops
2. Look at Core Media threads (network, decode, render)
3. Find the slow one
4. If network: increase buffer, implement ABR
5. If decode: reduce resolution, use hardware decoder
6. If render: reduce render workload (disable animations)
7. If thermal: suggest lower bitrate"
```

**Trade-offs (2 min):**
```
"Each fix trades off differently:
- Larger buffer: more memory, higher latency
- Lower resolution: worse quality
- Reduced animations: worse UX but smoother playback

Pick the right trade-off for your users (e.g., Netflix users
prefer smooth playback over animation polish)."
```

**Validate (1 min):**
```
"Summary: Use System Trace to find bottleneck, implement targeted
fix based on what's slow, measure frame drop reduction."
```

---

### Sample Q3: "TLB misses cause 20% slowdown in your data processing app. How would you fix it?"

**Understand (1 min):**
```
"So perf shows TLB misses are 20% of CPU time. Should I assume:
- It's a read-heavy workload (not write)?
- Array size is hundreds of MB (memory-bound)?
```

**Diagnose (1 min):**
```
"TLB miss means accessing memory outside cached virtual→physical
translations. Large working set is the issue. Options: reduce
working set, improve locality, or use huge pages."
```

**Propose (5 min):**
```
"Fix 1: Improve cache locality (Lesson 14)—access data in
sequential order, not random.

Fix 2: Use huge pages (2MB or 1GB instead of 4KB)—fewer TLB
entries needed.

I'd try Fix 1 first (easier), then Fix 2 if needed."
```

**Trade-offs (2 min):**
```
"Locality: Changes code complexity slightly, improves cache too.
Huge pages: Requires OS support, less portable, can waste memory
if working set is small."
```

**Validate (1 min):**
```
"Summary: Improve memory access patterns first (easy win), then
try huge pages if needed. Expect 15–30% speedup."
```

---

## 6) What Interviewers Really Want to Hear

1. **Clear thinking** — You reason through problems step-by-step, not guessing
2. **Measurement mindset** — You profile before optimizing
3. **Trade-off awareness** — You know nothing is free; you choose wisely
4. **Communication** — You explain clearly and adjust when asked questions
5. **Humility** — You say "I don't know but I'd figure it out" vs. "I don't know"
6. **Pragmatism** — You choose simple solutions first, upgrade if needed

## Key Takeaways

- **Use STEP framework** for every system design question
- **Spend 5 min understanding**, not jumping to solutions
- **Use communication templates** to sound clear under pressure
- **Practice out loud**—verbalize your thinking
- **Be ready to pivot** when asked follow-ups
- **Leave time for questions** (don't talk for 44 minutes straight)

## Homework

1. Pick one of the sample questions above
2. Answer it out loud (pretend you're in an interview)
3. Time yourself (should take 10–15 min)
4. Record or ask a friend to listen
5. Fix:
   - Did you spend too long on one section?
   - Did you mention trade-offs?
   - Could you explain it in simpler terms?
6. Repeat with different questions until STEP feels natural

**Interview questions to practice:**
- "Our app crashes under memory pressure. How would you debug?"
- "Lock contention is slowing down our server. What would you do?"
- "Users on Android complain about battery drain. How would you optimize?"
- "Frame drops spike when the app starts. Why?"
- "malloc is showing up in 30% of samples. How would you fix it?"
