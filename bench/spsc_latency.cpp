#include "build_info.hpp"
#include "spsc_queue.hpp"
#include "spsc_queue_atomic.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <format>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

template <typename Q>
std::optional<std::vector<std::chrono::nanoseconds>> run_latency(int n) {
    std::vector<std::chrono::nanoseconds> samples(n);
    bool echo_ok = true;
    bool main_ok = true;

    Q q_A;
    Q q_B;

    {
        std::jthread echo{[&q_A, &q_B, &echo_ok] {
            while (auto v = q_A.pop()) {
                if (!q_B.push(*v)) {
                    echo_ok = false;
                    break;
                }
            }
            q_B.close();
        }};

        for (int i = 0; i < n; i++) {
            const auto t0 = std::chrono::steady_clock::now();

            if (!q_A.push(i)) {
                main_ok = false;
                break;
            }

            auto v = q_B.pop();
            const auto t1 = std::chrono::steady_clock::now();
            // check if v failed or index not aligned
            if (!v || *v != i) {
                main_ok = false;
                break;
            }

            samples[i] = t1 - t0;
        }
        q_A.close();
    }

    if (echo_ok && main_ok) {
        return samples;
    } else {
        return std::nullopt;
    }
}

int main() {
    constexpr int n_samples = 100'000;
    constexpr int n_warmup = 1'000;
    constexpr int capacity = 1024;
    using Mutex = SpscQueue<int, capacity>;
    using Atomic = SpscQueueAtomic<int, capacity>;

    // warm-up: samples discarded, check kept
    if (!run_latency<Mutex>(n_warmup)) {
        std::cerr << "mutex: failed on warm-up\n";
        return 1;
    }
    if (!run_latency<Atomic>(n_warmup)) {
        std::cerr << "atomic: failed on warm-up\n";
        return 1;
    }

    auto m = run_latency<Mutex>(n_samples);
    if (!m) {
        std::cerr << "mutex: failed\n";
        return 1;
    }
    auto a = run_latency<Atomic>(n_samples);
    if (!a) {
        std::cerr << "atomic: failed\n";
        return 1;
    }
    std::ranges::sort(*m);
    std::ranges::sort(*a);

    // percentile p of a sorted vector: index p * n, clamped to the last
    auto pct = [](const std::vector<std::chrono::nanoseconds>& v, double p) {
        const auto i = std::min(
            static_cast<std::size_t>(p * static_cast<double>(v.size())),
            v.size() - 1);
        return v[i].count();
    };

    auto row = [&](std::string_view name,
                   const std::vector<std::chrono::nanoseconds>& v) {
        std::cout << std::format("{:<8}{:>10}{:>10}{:>10}{:>12}\n", name,
                                 pct(v, 0.50), pct(v, 0.99), pct(v, 0.999),
                                 v.back().count());
    };

    std::cout << std::format("build: {}, {}, sanitizers: {}\n", opt, asserts,
                             sanitizers);
    std::cout << std::format(
        "round trip, n = {} (+{} warm-up), capacity = {}\n\n", n_samples,
        n_warmup, capacity);
    std::cout << std::format("{:<8}{:>10}{:>10}{:>10}{:>12}\n", "queue",
                             "p50 ns", "p99 ns", "p99.9 ns", "max ns");
    row("mutex", *m);
    row("atomic", *a);

    auto ratio = [&](double p) {
        return static_cast<double>(pct(*m, p)) /
               static_cast<double>(pct(*a, p));
    };
    std::cout << std::format("\nmutex / atomic: p50 {:.1f}x, p99 {:.1f}x\n",
                             ratio(0.50), ratio(0.99));
}
