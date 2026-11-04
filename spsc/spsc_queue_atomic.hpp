#pragma once
#include <atomic>
#include <optional>
#include <concepts>
#include <array>
#include <thread>

template <std::semiregular T, std::size_t N> class SpscQueueAtomic {
public:
    std::optional<T> pop() {
        const auto h = head_.load(std::memory_order_relaxed);
        const auto next = (h + 1) % slots_;
        
        // spin on empty check
        while (h == tail_.load(std::memory_order_acquire))
            std::this_thread::yield();
        auto val = std::move(buf_[h]);
        head_.store(next, std::memory_order_release);
        return val;
    }

    [[nodiscard]] bool push(const T& elem) {
        const auto t = tail_.load(std::memory_order_relaxed);
        const auto next = (t + 1) % slots_;
        
        // full check
        while (next == head_.load(std::memory_order_acquire))
            std::this_thread::yield();
        buf_[t] = elem;
        tail_.store(next, std::memory_order_release);
        return true;
    }

private:
    // N + 1 guarantees full and empty check 
    static constexpr std::size_t slots_ = N + 1;

    std::array<T, slots_> buf_;      
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};
