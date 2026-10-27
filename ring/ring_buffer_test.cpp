#include "ring_buffer.hpp"
#include <gtest/gtest.h>

TEST(Ring, EmptyPop) {
    RingBuffer<std::string, 2> buf;
    EXPECT_EQ(buf.pop(), std::nullopt);
    EXPECT_EQ(buf.size(), 0u);
}

TEST(Ring, QueueFIFO) {
    RingBuffer<std::string, 4> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));
    ASSERT_TRUE(buf.push("C"));
    EXPECT_EQ(buf.pop(), "A");
    EXPECT_EQ(buf.pop(), "B");
    EXPECT_EQ(buf.pop(), "C");
}

TEST(Ring, PushOnFull) {
    RingBuffer<std::string, 2> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));
    EXPECT_FALSE(buf.push("C"));

    EXPECT_EQ(buf.size(), 2u);
    EXPECT_EQ(buf.pop(), "A");
    EXPECT_EQ(buf.pop(), "B");
}

TEST(Ring, WrapAround) {
    RingBuffer<std::string, 3> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));
    ASSERT_TRUE(buf.push("C"));
    EXPECT_EQ(buf.pop(), "A");
    // D wraps around
    ASSERT_TRUE(buf.push("D"));

    EXPECT_EQ(buf.pop(), "B");
    EXPECT_EQ(buf.pop(), "C");
    EXPECT_EQ(buf.pop(), "D");
}

/* std::string stroes strings up to 15 characters inside the object itself
 we want to test whether heap-allocated strings (>15) work with FIFO */
TEST(Ring, HeapAllocString) {
    RingBuffer<std::string, 2> buf;
    ASSERT_TRUE(buf.push("This is a longer sentence one"));
    ASSERT_TRUE(buf.push("This is a longer sentence two"));
    EXPECT_EQ(buf.pop(), "This is a longer sentence one");
    // wrap around
    ASSERT_TRUE(buf.push("This is a longer sentence three"));

    EXPECT_EQ(buf.pop(), "This is a longer sentence two");
    EXPECT_EQ(buf.pop(), "This is a longer sentence three");
}
