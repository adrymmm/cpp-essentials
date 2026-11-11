#pragma once
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <optional>
#include <thread>
#include <utility>

/* Atomic SpscQueue FIFO: one producer/ one consumer; waits spin
 * push: false if closed, pop: nullopt once closed and drained.
 * Caller needs to call close() after last push;
 * P controls padding with no padding as default*/
template <std::semiregular T, std::size_t N,
          std::size_t P = alignof(std::atomic<std::size_t>)>
class SpscQueueAtomic {
    static_assert(N > 0, "N: capacity must be positive");

public:
    // either no padding or one cache line (64 bytes)
    static_assert(P == alignof(std::atomic<std::size_t>) || P == 64 || P == 128,
                  "P: either natural alignment, 64, or 128 bytes");

    SpscQueueAtomic() {
        // each index starts at a multiple of P; for P = 64, its own cache line
        static_assert(offsetof(SpscQueueAtomic, tail_) % P == 0);
        static_assert(offsetof(SpscQueueAtomic, head_) % P == 0);
    }

    std::optional<T> pop() {
        const auto h = head_.load(std::memory_order_relaxed);
        const auto next = (h + 1) % slots_;

        // spin on empty check
        while (h == tail_.load(std::memory_order_acquire) &&
               !closed_.load(std::memory_order_acquire))
            std::this_thread::yield();

        // check if empty again in between closed and push
        if (h == tail_.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        auto val = std::move(buf_[h]);
        head_.store(next, std::memory_order_release);
        return val;
    }

    [[nodiscard]] bool push(const T& elem) {
        const auto t = tail_.load(std::memory_order_relaxed);
        const auto next = (t + 1) % slots_;

        // full check
        while (next == head_.load(std::memory_order_acquire) &&
               !closed_.load(std::memory_order_acquire))
            std::this_thread::yield();

        // closed wins over free space
        if (closed_.load(std::memory_order_acquire))
            return false;

        buf_[t] = elem;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    void close() {
        // sync with pop and push
        closed_.store(true, std::memory_order_release);
    }

private:
    // N + 1 guarantees full and empty check; one slot stays empty
    static constexpr std::size_t slots_ = N + 1;

    std::array<T, slots_> buf_;
    std::atomic<bool> closed_{false};
    alignas(P) std::atomic<std::size_t> head_{0};
    alignas(P) std::atomic<std::size_t> tail_{0};
};
