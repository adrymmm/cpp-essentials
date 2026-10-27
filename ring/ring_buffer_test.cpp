#include "ring_buffer.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

// check if iterator meets forwrad iter concept
static_assert(std::forward_iterator<RingBuffer<int, 3>::iterator>);

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

TEST(Ring, RangeForFullBuffer) {
    RingBuffer<std::string, 2> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));
    ASSERT_EQ(buf.pop(), "A");
    // wrap-around
    ASSERT_TRUE(buf.push("C"));

    std::vector<std::string> vec{};

    for (const auto& x : buf) {
        vec.push_back(x);
    }

    EXPECT_EQ(vec, (std::vector<std::string>{"B", "C"}));
}

TEST(Ring, FindAndCountOnBuf) {
    RingBuffer<std::string, 4> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));
    ASSERT_TRUE(buf.push("C"));
    ASSERT_TRUE(buf.push("B"));

    auto it = std::ranges::find(buf, "C");
    auto absent = std::ranges::find(buf, "D");

    EXPECT_EQ(absent, buf.end()); // absent not found, returns end()
    ASSERT_NE(it, buf.end());     // it found
    EXPECT_EQ(*it, "C");

    auto count = std::ranges::count(buf, "B");
    ASSERT_EQ(count, 2);
}

TEST(Ring, Postfix) {
    RingBuffer<std::string, 2> buf;
    ASSERT_TRUE(buf.push("A"));
    ASSERT_TRUE(buf.push("B"));

    auto it = buf.begin();
    auto old = it++;

    EXPECT_EQ(*old, "A");
    EXPECT_EQ(*it, "B");
}
