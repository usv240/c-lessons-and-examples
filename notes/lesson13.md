## LESSON 13 — Advanced Concurrency & Synchronization

(The logic that keeps multi-threaded systems fast and safe)

## Goals

By the end you’ll understand:

- What concurrency really means (and how it differs from parallelism)
- How threads share memory safely
- Core synchronization primitives: mutex, semaphore, condition variable, barrier, atomic operations
- Deadlocks and how to avoid them
- How Apple and Linux implement concurrency (pthread ↔ GCD)

## 1) Concurrency vs Parallelism

| Term        | Meaning                                      | Analogy                      |
|-------------|----------------------------------------------|------------------------------|
| Concurrency | Multiple tasks in progress (may interleave)  | One cook juggling 3 dishes   |
| Parallelism | Multiple tasks at the same time              | 3 cooks working together     |

Threads provide concurrency; multiple CPU cores provide true parallelism.

## 2) Shared Memory Problem

When two threads update the same variable:

```c
int counter = 0;

void* add(void* arg){
    for(int i=0;i<100000;i++)
        counter++;
}
```

Result may be less than 200,000 — a race condition. Reason: `counter++` breaks down into read → add → write (3 CPU ops); interleaving breaks atomicity.

## 3) Mutex (Mutual Exclusion)

Guarantees that only one thread can access a region at once.

```c
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* add(void* arg){
    for(int i=0;i<100000;i++){
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}
```

- `pthread_mutex_lock` blocks if another thread holds the lock.
- Rule: Always unlock in every control path. Consider `pthread_cleanup_push`/`pthread_cleanup_pop` or C++ RAII wrappers to avoid stalls.

## 4) Semaphores (Counting Locks)

Allow up to N threads to enter a critical region.

```c
#include <semaphore.h>

sem_t sem;
sem_init(&sem, 0, 3);   // allow 3 threads
sem_wait(&sem);         // enter
/* work */
sem_post(&sem);         // exit
```

Common uses:

- Connection pools
- Producer/Consumer buffers
- Rate limiting

## 5) Condition Variables (Signaling Events)

Threads can wait for a condition to become true.

```c
#include <pthread.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

void* producer(void* arg){
    pthread_mutex_lock(&mtx);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    return NULL;
}

void* consumer(void* arg){
    pthread_mutex_lock(&mtx);
    while(!ready)
        pthread_cond_wait(&cond, &mtx);
    printf("Go!\n");
    pthread_mutex_unlock(&mtx);
    return NULL;
}
```

Used in thread communication (pipelines, queues, render loops).

## 6) Atomic Operations

Hardware-level single-instruction updates:

```c
#include <stdatomic.h>

atomic_int counter = 0;
atomic_fetch_add(&counter, 1);
```

- Lock-free and fast on multi-core CPUs.
- Great for counters, flags, reference counts.

## 7) Deadlocks and How to Avoid Them

Deadlocks happen when threads wait on each other forever.

Example:

```text
T1: lock A → wait B
T2: lock B → wait A
```

Avoidance rules:

- Always acquire locks in the same global order.
- Use timeout locks (e.g., `pthread_mutex_timedlock`).
- Keep critical sections small.
- Prefer try-locks (`pthread_mutex_trylock`) or higher-level abstractions.

## 8) Barriers

Synchronize all threads to a common checkpoint. Each waits until every thread has reached the barrier.

```c
#include <pthread.h>

pthread_barrier_t b;
pthread_barrier_init(&b, NULL, 4);
// ...
pthread_barrier_wait(&b); // all 4 must arrive
```

## 9) Thread-Safe Design Patterns

| Pattern            | Use                                      |
|--------------------|------------------------------------------|
| Producer–Consumer  | Queue with condition variables and mutex |
| Reader–Writer Lock | Many readers, one writer (`pthread_rwlock_t`) |
| Thread Pool        | Reuse worker threads for tasks           |
| Async Queues       | Non-blocking message passing             |

## 10) Apple Concurrency (GCD — Grand Central Dispatch)

Instead of raw pthread code:

```objective-c
#include <dispatch/dispatch.h>

dispatch_queue_t q = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
dispatch_async(q, ^{
    printf("Running on GCD thread!\n");
});
```

- Queues manage tasks for you and automatically balance CPU cores.
- Used widely in macOS/iOS (Core Media, AVFoundation).

## 11) Performance Tips

- Prefer fine-grained locks over global ones.
- Use lock-free structures where possible.
- Watch for false sharing (cache lines shared by different threads).
- Profile with Instruments (Thread Sanitizer) or `perf lock`.

## 12) Summary

| Concept           | Purpose                            |
|-------------------|------------------------------------|
| Mutex             | One thread at a time               |
| Semaphore         | Allow N threads at a time          |
| Condition Variable| Wait/signal between threads        |
| Atomic Ops        | Lock-free safe updates             |
| Barrier           | Sync threads at a checkpoint       |
| Deadlock          | Mutual wait; avoid via ordering    |
| GCD (Apple)       | Modern dispatch-queue model        |