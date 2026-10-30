#pragma once
#include "ring_buffer.hpp"
#include <optional>
#include <concepts>
#include <mutex>
#include <cstddef>
#include <condition_variable>

/* multi-threading wrapper on ring_buffer; one producer/one consumer thread;
 * push waits for space, false means closed; pop waits for an item, nullopt means
 * closed and drained; two condition variables needed for extension to more consumer/producer pairs*/
template <std::semiregular T, std::size_t N> 
class SpscQueue {
public:
    std::optional<T> pop() {
        std::unique_lock lk(mut_);
        // wait for item or close
        not_empty_.wait(lk, [this]{ return !rb_.empty() || closed_; });
        std::optional<T> val = rb_.pop();

        //notify push that queue is not full
        lk.unlock();
        not_full_.notify_one();
        return val;
    }

    [[nodiscard]] bool push(const T& elem) {
        std::unique_lock lk(mut_);
        // wait for space or close
        not_full_.wait(lk, [this] { return !rb_.full() || closed_; });

        // check if close flag turned in between waiting and close() being called to return status
        if (closed_ == true) return false;

        bool status = rb_.push(elem);

        //notify pop that queue is not empty
        lk.unlock();
        not_empty_.notify_one();
        return status;
    }

    void close() {
        {   
            std::scoped_lock lk(mut_);
            closed_ = true;
        }
        // notify every thread waiting that queue is closed (_all is valid for more producer/consumer pairs)
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    std::mutex mut_;
    RingBuffer<T, N> rb_;
    bool closed_ = false;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
};
