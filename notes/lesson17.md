## LESSON 17 — Capstone Project: Mini Streaming Engine in C

(End-to-end integration of concurrency, buffering, and performance)

## Goal

Design and implement a simplified HTTP Live Streaming (HLS)-style media pipeline using real system concepts in compact C code.

You’ll simulate:

- A network thread fetching “segments”
- A decoder thread processing them
- A renderer thread consuming frames
- Synchronization, buffering, and adaptive bitrate logic

## 1) Architecture Overview

```text
+-----------------+     +-----------------+     +----------------+
|  Network Thread | ---> |  Decode Thread  | ---> | Render Thread |
|  (Producer)     |     |  (Transformer)  |     | (Consumer)    |
+-----------------+     +-----------------+     +----------------+
        |                        |                      |
   Fetch chunks             Parse & delay          Print "frames"
```

Shared buffers:

- Ring buffer for downloaded “segments”
- Protected by a mutex and condition variable
- Capacity-limited to force synchronization

## 2) Core Components

| Component         | Purpose                             |
|-------------------|-------------------------------------|
| network_thread()  | Simulates downloading segments      |
| decode_thread()   | Simulates frame decoding            |
| render_thread()   | Simulates playback/rendering        |
| buffer[]          | Shared between producer/consumer    |
| pthread_mutex_t   | Prevents race conditions            |
| pthread_cond_t    | Coordinates buffer availability     |

## 3) Sample Implementation

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define SEGMENTS 15

int buffer[BUFFER_SIZE];
int count = 0, in = 0, out = 0;
int done = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void* network_thread(void* arg) {
    for (int i = 1; i <= SEGMENTS; i++) {
        pthread_mutex_lock(&lock);
        while (count == BUFFER_SIZE)
            pthread_cond_wait(&not_full, &lock);

        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        printf("Downloaded segment %d\n", i);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);
        usleep(200000); // simulate download delay (0.2s)
    }

    pthread_mutex_lock(&lock);
    done = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&lock);

    return NULL;
}

void* decode_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        while (count == 0 && !done)
            pthread_cond_wait(&not_empty, &lock);

        if (count == 0 && done) {
            pthread_mutex_unlock(&lock);
            break;
        }

        int seg = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);

        printf("Decoding segment %d...\n", seg);
        usleep(150000); // simulate decoding delay
    }
    return NULL;
}

int main() {
    pthread_t net, dec;
    pthread_create(&net, NULL, network_thread, NULL);
    pthread_create(&dec, NULL, decode_thread, NULL);

    pthread_join(net, NULL);
    pthread_join(dec, NULL);

    printf("All segments processed successfully!\n");
    return 0;
}
```

## 4) Add Adaptive Bitrate (Bonus)

Simulate network speed variations by toggling bitrate occasionally:

```c
int bitrate = 2000; // in kbps
if (rand() % 5 == 0) {
    bitrate = (bitrate == 2000) ? 800 : 2000;
    printf("Switched bitrate to %d kbps\n", bitrate);
}
```

Add this inside your network loop before “downloading” segments.

## 5) Profiling Your Engine

| Tool                | Purpose                    | Example/Notes                |
|---------------------|----------------------------|------------------------------|
| time                | Measure runtime            | `time ./stream`              |
| htop                | Monitor CPU threads        | Concurrency test             |
| valgrind            | Check leaks                | Ensure clean exits           |
| perf                | Count context switches     | `perf stat ./stream`         |

## 6) Key Learning Outcomes

| Concept            | Where It Appears                          |
|--------------------|-------------------------------------------|
| Thread creation    | `pthread_create()`                        |
| Synchronization    | `pthread_mutex`, `pthread_cond`           |
| Producer-consumer  | Buffer between network and decode         |
| Performance control| Sleeps and adaptive bitrate               |
| OS scheduling      | Context switching in threads              |
| Profiling          | Runtime and system tools                  |

## 7) Possible Extensions

- Add a renderer thread to simulate playback delay
- Add a statistics thread that prints buffer size every second
- Use real network I/O with libcurl or sockets
- Integrate FFmpeg APIs to decode actual frames (advanced)

## 8) Conceptual Flow Summary

```text
Network thread → fills buffer
    ↓ (mutex, cond)
Decoder thread → consumes data
    ↓
Renderer thread → displays output
    ↓
Scheduler & profiler → measure timing, detect stalls
```

This is a microcosm of Apple’s Core Media / AVFoundation pipeline.

## Summary

| Layer      | Concept                                | What You Implemented       |
|------------|----------------------------------------|----------------------------|
| Language   | C (threads, I/O, buffers)              | Core logic                 |
| OS         | Processes, scheduling, synchronization | Thread control             |
| Hardware   | Cache and cores                        | Parallel execution         |
| Framework  | Media pipeline                         | Producer → Consumer        |
| Performance| Profiling                               | Efficient coordination     |