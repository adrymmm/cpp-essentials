#include "spsc_queue.hpp"
#include "spsc_queue_atomic.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <cstddef>
#include <numeric>
#include <optional>
#include <thread>

// defining test suite so both mutex and atomic queues can run on the same test bodies
template <typename C>
class SpscQueueTest : public ::testing::Test {};

using SpscQueues = ::testing::Types<SpscQueue<int, 4>, SpscQueueAtomic<int, 4>>;
TYPED_TEST_SUITE(SpscQueueTest, SpscQueues);

TYPED_TEST(SpscQueueTest, SingleThreadDrain) {
    TypeParam q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    ASSERT_TRUE(q.push(3));

    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
}

TYPED_TEST(SpscQueueTest, DrainAfterClose) {
    TypeParam q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    q.close();

    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), std::nullopt);
    EXPECT_EQ(q.pop(), std::nullopt);
}

TYPED_TEST(SpscQueueTest, PushAfterClose) {
    TypeParam q;
    q.close();
    EXPECT_FALSE(q.push(1));
    EXPECT_EQ(q.pop(), std::nullopt);
}

TYPED_TEST(SpscQueueTest, MThreadPopEmptyQueue) {
    TypeParam q;
    std::optional<int> pop_res = -1; // sentinel
    {
        std::jthread t{[&q, &pop_res] {
            // 2nd thread pops empty queue
            pop_res = q.pop();
        }};
        // give popper time to block so close must wake it
        std::this_thread::sleep_for(std::chrono::milliseconds{50});
        // run close on main thread
        q.close();
    }
    // join is called here through jthread destructor
    EXPECT_EQ(pop_res, std::nullopt);
}

TYPED_TEST(SpscQueueTest, MThreadPushFullQueue) {
    TypeParam q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    ASSERT_TRUE(q.push(3));
    ASSERT_TRUE(q.push(4));
    bool last_push = true;
    {
        std::jthread t{[&q, &last_push] {
            // 2nd thread pushes
            last_push = q.push(5);
        }};
        // give pusher time to block so close must wake it
        std::this_thread::sleep_for(std::chrono::milliseconds{50});
        q.close();
    }
    // join is called here through jthread destructor
    EXPECT_FALSE(last_push);
}

// Fill a 4 elem queue up and then keep trying to push and pop to test interleaving
TYPED_TEST(SpscQueueTest, MThreadStressTest) {
    TypeParam q;
    std::vector<int> vect;
    constexpr int n = 100000;
    bool status = true;
    {
        std::jthread prod{[&q, &status] {
            for (int i = 0; i < n; i++) {
                status = q.push(i) && status;
            }
            q.close();
        }};
    
    std::jthread cons{[&q, &vect] {
        // while v is not optnull
        while (auto v = q.pop()) {
            vect.push_back(*v);
                }
            }};
    }
    // main thread continues here
    EXPECT_TRUE(status);
    ASSERT_EQ(vect.size(), static_cast<std::size_t>(n));

    // building sequential vector
    std::vector<int> seqv(n);
    std::iota(seqv.begin(), seqv.end(), 0);
    ASSERT_EQ(vect, seqv);

}

TYPED_TEST(SpscQueueTest, BoundedStress) {
    TypeParam q;
    std::vector<int> vect;
    bool status = true;
    constexpr int n = 100000;
    {
        std::jthread prod{[&q, &status] {
            for (int i = 0; i < n; ++i) {
                status = q.push(i) && status;
            }
        }};
    
        std::jthread cons{[&q, &vect] {
            for (int i = 0; i < n; ++i) {
                auto v = q.pop();
                if (v)
                    vect.push_back(*v);
                }
            }};
    }

    // main thread continues here
    EXPECT_TRUE(status);
    ASSERT_EQ(vect.size(), static_cast<std::size_t>(n));

    // building sequential vector
    std::vector<int> seqv(n);
    std::iota(seqv.begin(), seqv.end(), 0);
    ASSERT_EQ(vect, seqv);
}
