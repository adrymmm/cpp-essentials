#include "spsc_queue.hpp"
#include <gtest/gtest.h>
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
