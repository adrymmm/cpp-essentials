    # cpp-essentials

    Learning repo for modern C++ and concurrency.

    ## SPSC queues
    - `SpscQueue`: mutex and condition variables.
    - `SpscQueueAtomic`: same API, built on atomics with release/acquire ordering.

    Each is tested with GoogleTest and built with ASan/UBSan and TSan.

    ## Results

    PC, 4 cores × HT, pinned to two physical cores (`taskset -c 0,2`). Release (-O3), no sanitizers. Capacity 1024. Ranges over 3 invocations.

    **Throughput**: 10M ints, median of 11 runs.

    | queue | ns/item |
    |---|---|
    | mutex | 166–187 |
    | atomic | 10.7–12.9 |
    | atomic, padded 64 B | 10.1–10.4 |
    | atomic, padded 128 B | 10.6–11.8 |

    **Latency**: ping-pong round trip, 100k samples, one item in flight.

    | queue | p50 µs | p99 µs | p99.9 µs |
    |---|---|---|---|
    | mutex | 8.5–8.8 | 24–26 | 58–137 |
    | atomic | 1.1 | 3.2–13.6 | 15.6–16.3 |

    - Atomic is ~14–15× faster in throughput and ~7.5× faster at p50 latency. The mutex queue sleeps and wakes on every item, so that cost is in every sample, not just the tail.
    - Padding is machine-dependent: 64 B padding is up to 1.27× faster here, but was 0.71× (slower) on my laptop. 128 B is consistently slower than 64 B; cause not found.
    - Throughput ≠ latency: a full queue's push-to-pop time measures queue depth, so latency is measured separately.

    ## Known gap
    No test reliably forces the drain race in `SpscQueueAtomic`, where the consumer could stop before reading the last item. The fix is backed by a happens-before argument, not a test: the last `tail_` store comes before the `closed_` release, so a consumer that acquires `closed_` and reloads `tail_` must see the final push.