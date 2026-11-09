#include "spsc_queue_atomic.hpp"
#include "spsc_queue.hpp"
#include <iostream>
#include <format>
#include <atomic>
#include <chrono>
#include <thread>
#include <optional>


// runs SPSC queue with n pushes/pops; returns nullopt if queue fails, or otherwise
// it returns the time elapsed to push/pop n in nanoseconds
template <typename Q> 
std::optional<std::chrono::nanoseconds> run_queue(int n) {
    // sync flag
    std::atomic<bool> go{false};
    std::chrono::steady_clock::time_point stop;
    std::chrono::steady_clock::time_point start;
    bool prod_ok = true;
    bool cons_ok = true;

    Q q;
    {
        std::jthread prod{[&q, &prod_ok, &go, n] {
            // spin on go
            while(!go.load(std::memory_order_acquire)) {};   
            for (int i = 0; i < n; i++) {
                if (!q.push(i)) {
                    prod_ok = false;
                    break;  
                }
            }
            q.close();
        }};

        std::jthread cons{[&q, &cons_ok, &go, &stop, n] {
            while(!go.load(std::memory_order_acquire)) {};   
            int expected = 0;
            // pop until nullopt
            while (auto v = q.pop()) {
                // every pop is compared to the counter
                if (*v == expected) ++expected;
                else cons_ok = false;
            }
            // last item has to be total iterations (FIFO)
            if (expected != n) cons_ok = false;
            stop = std::chrono::steady_clock::now();
        }};

        // main thread starts timer HERE
        start = std::chrono::steady_clock::now();
        // release both threads
        go.store(true, std::memory_order_release);
    }

    if (prod_ok && cons_ok) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    } 
    else {
        return std::nullopt;
    }
} 

int main() {
    const int TIMES = 10'000'000;
    const int SIZE = 1024;
    bool failed = false;
    
    std::optional<std::chrono::nanoseconds> mutex_time = run_queue<SpscQueue<int, SIZE>>(TIMES);
    std::optional<std::chrono::nanoseconds> atomic_time = run_queue<SpscQueueAtomic<int, SIZE>>(TIMES);
    
    if (mutex_time) {
        std::cout << std::format("mutex: {}\n", *mutex_time);
    }
    else {
        std::cerr << ("mutex: failed\n");
        failed = true;
    }

    if (atomic_time) {
        std::cout << std::format("atomic: {}\n", *atomic_time);
    }
    else {
        std::cerr << ("atomic: failed\n");
        failed = true;
    }
    return failed ? 1 : 0;
}
