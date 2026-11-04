#pragma once
#include <atomic>
#include <optional>
#include <concepts>
#include <array>
#include <utility>
#include <cstddef>
#include <thread>

/* Lock-free SpscQueue FIFO: one producer/ one consumer; waits spin
 * push: false if closed, pop: nullopt once closed and drained.
 * Caller needs to call close() after last push*/
template <std::semiregular T, std::size_t N> class SpscQueueAtomic {
    static_assert(N > 0, "capacity must be positive");
public:
    std::optional<T> pop() {
        const auto h = head_.load(std::memory_order_relaxed);
        const auto next = (h + 1) % slots_;
        
        // spin on empty check
        while (h == tail_.load(std::memory_order_acquire) && !closed_.load(std::memory_order_acquire))
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
        while (next == head_.load(std::memory_order_acquire) && !closed_.load(std::memory_order_acquire))
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
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};
