#pragma once
#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>

/* T should be default-constructible (done through array); state is held through
 * head_ and size_ (head_ + size_) % N (tail), size_ == N (full) size_ == 0
 * (empty)*/
template <typename T, std::size_t N> class RingBuffer {
public:
    static_assert(N > 0, "buffer cannot be zero");

    class iterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        iterator(RingBuffer& rb, std::size_t i) : rb_{&rb}, i_{i} {}

        // dereferencing maps from index position to position in buffer (due to
        // wraparoudn)
        T& operator*() const {
            auto slot = (rb_->head_ + i_) % N;
            return rb_->buf_[slot];
        }

        iterator& operator++() {
            ++i_;
            return *this;
        }

        iterator operator++(int) {
            auto old{*this}; // save old value
            ++*this;         // advance iterator
            return old;
        }

        bool operator==(const iterator&) const = default;

    private:
        RingBuffer* rb_{};
        std::size_t i_ = 0;
    };

    iterator begin() { return iterator{*this, 0}; }

    iterator end() { return iterator{*this, size_}; }

    bool full() const noexcept { return (size_ == N); }
    bool empty() const noexcept { return (size_ == 0); }
    std::size_t size() const noexcept { return size_; }

    // returns nullopt when empty, otherwise front element moved out
    std::optional<T> pop() {
        if (empty()) {
            return std::nullopt;
        }

        std::size_t old_head = head_;
        head_ = (head_ + 1) % N; // advance head with wrap-around
        --size_;
        return std::move(buf_[old_head]);
    }

    // returns false when full and buffer is unchanged
    [[nodiscard]] bool push(const T& elem) {
        if (full())
            return false;
        std::size_t tail = (head_ + size_) % N;
        buf_[tail] = elem;
        ++size_;
        return true;
    }

private:
    std::array<T, N> buf_;
    std::size_t size_ = 0;
    std::size_t head_ = 0;
};
