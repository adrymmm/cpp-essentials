#include <gtest/gtest.h>
#include <vector>

struct counted_nothrow {
    inline static int moves = 0;
    inline static int copies = 0;

    counted_nothrow() = default;

    counted_nothrow(const counted_nothrow&) { ++copies; }

    counted_nothrow(counted_nothrow&&) noexcept { ++moves; }
};

struct counted_maythrow {
    inline static int moves = 0;
    inline static int copies = 0;

    counted_maythrow() = default;

    counted_maythrow(const counted_maythrow&) { ++copies; }

    counted_maythrow(counted_maythrow&&) { ++moves; }
};

TEST(Counting, NoexceptMoveUsedOnGrowth) {
    using T = counted_nothrow;
    T::moves = 0;
    T::copies = 0;

    std::vector<T> v;
    for (int i = 0; i < 100; ++i) {
        v.emplace_back();
    }

    EXPECT_EQ(T::copies, 0);
    EXPECT_GT(T::moves, 0);
}

TEST(Counting, MayThrowMoveCopiesOnGrowth) {
    using T = counted_maythrow;
    T::moves = 0;
    T::copies = 0;

    std::vector<T> v;
    for (int i = 0; i < 100; ++i) {
        v.emplace_back();
    }

    EXPECT_GT(T::copies, 0);
    EXPECT_EQ(T::moves, 0);
}
