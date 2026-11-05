# cpp-essentials

Learning repo for modern C++ and concurrency. Work in progress.

## SPSC queues
- `SpscQueue`: mutex and condition variables.
- `SpscQueueAtomic`: same API, built on atomics with release/acquire ordering.

Each is tested with GoogleTest and built with ASan/UBSan and TSan.

## Known gap
No test reliably forces the drain race in `SpscQueueAtomic`, where the consumer could stop before reading the last item. The fix is backed by a happens-before argument, not a test: the last `tail_` store comes before the `closed_` release, so a consumer that acquires `closed_` and reloads `tail_` must see the final push.

## Next
Benchmark both queues on throughput and latency.
