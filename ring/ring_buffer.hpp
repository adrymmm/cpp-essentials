#pragma once
#include <cstddef>
#include <array>
#include <optional>
#include <utility>

template<typename T, std::size_t N>
class RingBuffer {
    public:
        static_assert(N > 0, "buffer cannot be zero");
        
        bool full() const noexcept { return (size_ == N); }
        bool empty() const noexcept { return (size_ == 0); }
        std::size_t size() const noexcept { return size_; }
        
        // caller needs to check for nullopt
        std::optional<T> pop() {
            if (empty()) { return std::nullopt; }

            std::size_t old_head = head_;
            head_ = (head_ + 1) % N;  // advance head with wrap-around
            --size_;
            return std::move(buf_[old_head]);
        }
        
        // caller has to handle fail (full buffer)
        [[nodiscard]] bool push(const T& elem) {
            if (full()) return false;
            std::size_t tail = (head_ + size_) % N;
            buf_[tail] = elem;
            ++size_;
            return true;
        }

    // TODO Iterators and concept on T

    private:
        std::array<T, N> buf_;
        std::size_t size_ = 0;
        std::size_t head_ = 0;
};
