#include "spsc_queue.hpp"
#include "spsc_queue_atomic.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <optional>
#include <string_view>
#include <thread>
#include <vector>

#if defined(__SANITIZE_ADDRESS__)
constexpr std::string_view sanitizers = "ASan + UBSan";
#elif defined(__SANITIZE_THREAD__)
constexpr std::string_view sanitizers = "TSan + UBSan";
#else
constexpr std::string_view sanitizers = "none";
#endif

#ifdef NDEBUG
constexpr std::string_view asserts = "NDEBUG";
#else
constexpr std::string_view asserts = "asserts on";
#endif

#ifdef __OPTIMIZE__
constexpr std::string_view opt = "optimised";
#else
constexpr std::string_view opt = "-O0";
#endif

// runs SPSC queue with n pushes/pops; returns nullopt if queue fails, or
// otherwise it returns the time elapsed to push/pop n in nanoseconds
template <typename Q>
[[nodiscard]] std::optional<std::chrono::nanoseconds> run_queue(int n) {
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
            while (!go.load(std::memory_order_acquire)) {
            }
            for (int i = 0; i < n; i++) {
                if (!q.push(i)) {
                    prod_ok = false;
                    break;
                }
            }
            q.close();
        }};

        std::jthread cons{[&q, &cons_ok, &go, &stop, n] {
            while (!go.load(std::memory_order_acquire)) {
            }
            int expected = 0;
            // pop until nullopt
            while (auto v = q.pop()) {
                // every pop is compared to the counter
                if (*v == expected)
                    ++expected;
                else
                    cons_ok = false;
            }
            // last item has to be total iterations (FIFO)
            if (expected != n)
                cons_ok = false;
            stop = std::chrono::steady_clock::now();
        }};

        // main thread starts timer HERE
        start = std::chrono::steady_clock::now();
        // release both threads
        go.store(true, std::memory_order_release);
    }

    if (prod_ok && cons_ok) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                    start);
    } else {
        return std::nullopt;
    }
}

int main() {
    constexpr int n_items = 10'000'000;
    constexpr int capacity = 1024;
    constexpr int runs = 11;
    std::vector<std::chrono::nanoseconds> mvec;
    std::vector<std::chrono::nanoseconds> avec;

    // warm-up runs; dont count in median benchmark
    if (!run_queue<SpscQueue<int, capacity>>(n_items)) {
        std::cerr << ("mutex: failed on warm-up\n");
        return 1;
    }

    if (!run_queue<SpscQueueAtomic<int, capacity>>(n_items)) {
        std::cerr << ("atomic: failed on warm-up\n");
        return 1;
    }

    // odd for median
    for (int i = 0; i < runs; ++i) {
        std::optional<std::chrono::nanoseconds> mutex_time =
            run_queue<SpscQueue<int, capacity>>(n_items);
        std::optional<std::chrono::nanoseconds> atomic_time =
            run_queue<SpscQueueAtomic<int, capacity>>(n_items);

        if (mutex_time) {
            mvec.push_back(*mutex_time);
        } else {
            std::cerr << std::format("mutex: failed on iteration {}\n", i);
            return 1;
        }

        if (atomic_time) {
            avec.push_back(*atomic_time);
        } else {
            std::cerr << std::format("atomic: failed on iteration {}\n", i);
            return 1;
        }
    }
    // sort then take median
    std::ranges::sort(mvec);
    std::ranges::sort(avec);

    auto ms = [](std::chrono::nanoseconds t) {
        return std::chrono::duration<double, std::milli>{t}.count();
    };
    auto row = [&](std::string_view name,
                   const std::vector<std::chrono::nanoseconds>& v) {
        const auto median = v[runs / 2];
        std::cout << std::format("{:<8}{:>12.1f}{:>10.1f}{:>10.1f}{:>11.1f}\n",
                                 name, ms(median), ms(v.front()), ms(v.back()),
                                 static_cast<double>(median.count()) / n_items);
    };

    std::cout << std::format("build: {}, {}, sanitizers: {}\n", opt, asserts,
                             sanitizers);
    std::cout << std::format(
        "n = {} items, capacity = {}, runs = {} (+1 warm-up)\n\n", n_items,
        capacity, runs);
    std::cout << std::format("{:<8}{:>12}{:>10}{:>10}{:>11}\n", "queue",
                             "median ms", "min ms", "max ms", "ns/item");
    row("mutex", mvec);
    row("atomic", avec);

    const double speedup = static_cast<double>(mvec[runs / 2].count()) /
                           static_cast<double>(avec[runs / 2].count());
    std::cout << std::format("speedup (median): {:.1f}x\n", speedup);
}
