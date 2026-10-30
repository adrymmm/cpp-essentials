#include "spsc_queue.hpp"
#include <gtest/gtest.h>
#include <optional>

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
