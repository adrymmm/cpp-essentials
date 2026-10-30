#include "spsc_queue.hpp"
#include <gtest/gtest.h>
#include <numeric>
#include <optional>
#include <thread>

TEST(Spsc, SingleThreadDrain) {
    SpscQueue<int, 4> q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    ASSERT_TRUE(q.push(3));

    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
}

TEST(Spsc, DrainAfterClose) {
    SpscQueue<int, 4> q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    q.close();

    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), std::nullopt);
    EXPECT_EQ(q.pop(), std::nullopt);
}

TEST(Spsc, PushAfterClose) {
    SpscQueue<int, 4> q;
    q.close();
    EXPECT_FALSE(q.push(1));
    EXPECT_EQ(q.pop(), std::nullopt);
}

TEST(Spsc, MThreadPopEmptyQueue) {
    SpscQueue<int, 4> q;
    std::optional<int> pop_res = -1; // sentinel
    {
        std::jthread t{[&q, &pop_res] {
            // 2nd thread pops empty queue
            pop_res = q.pop();
        }};
        // run close on main thread
        q.close();
    }
    // join is called here through jthread destructor
    EXPECT_EQ(pop_res, std::nullopt);
}

TEST(Spsc, MThreadPushFullQueue) {
    SpscQueue<int, 2> q;
    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    bool last_push = true;
    {
        std::jthread t{[&q, &last_push] {
            // 2nd thread pushes
            last_push = q.push(3);
        }};
        q.close();
    }
    // join is called here through jthread destructor
    EXPECT_FALSE(last_push);
}

// Fill a 4 elem queue up and then keep trying to push and pop to test interleaving
TEST(Spsc, MThreadStressTest) {
    SpscQueue<int, 4> q;
    std::vector<int> vect;
    bool status = true;
    {
        std::jthread prod{[&q, &status] {
            for (int i = 0; i < 100000; i++) {
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
    ASSERT_EQ(vect.size(), 100000u);

    // building sequential vector
    std::vector<int> seqv(100000);
    std::iota(seqv.begin(), seqv.end(), 0);
    ASSERT_EQ(vect, seqv);

}
