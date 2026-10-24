#include "buffer.hpp"
#include <gtest/gtest.h>
#include <utility>

TEST(Buffer, ConstructAndIndex) {
    buffer b{3};
    b[0] = 1;
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b.size(), 3u);
}

TEST(Buffer, CopyCtorIsDeep) {
    buffer src{3};
    src[0] = 1;
    src[1] = 2;

    buffer dst{src};
    dst[0] = 7;

    EXPECT_EQ(src[0], 1);
    EXPECT_EQ(dst[1], 2);
    EXPECT_EQ(dst.size(), 3u);
}

TEST(Buffer, CopyAssignIsDeep) {
    buffer src{3};
    src[0] = 1;
    src[1] = 2;

    buffer dst{5};
    dst = src;
    dst[0] = 2;

    EXPECT_EQ(src[0], 1);
    EXPECT_EQ(dst[1], 2);
    EXPECT_EQ(dst.size(), 3u);
}

TEST(Buffer, MoveCtorEmptiesSource) {
    buffer src{5};
    src[0] = 1;
    buffer dst{std::move(src)};

    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(dst.size(), 5u);
    EXPECT_EQ(dst[0], 1);
}

TEST(Buffer, MoveAssignEmptiesSource) {
    buffer src{5};
    src[0] = 1;

    buffer dst{3};
    dst = std::move(src);

    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(dst.size(), 5u);
    EXPECT_EQ(dst[0], 1);
}

TEST(Buffer, SelfCopyAssignKeepsValues) {
    buffer b{5};
    b[0] = 1;
    buffer& alias = b;
    b = alias;

    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b.size(), 5u);
}

TEST(Buffer, SelfMoveAssignKeepsValues) {
    buffer b{5};
    b[0] = 1;
    buffer& alias = b;
    b = std::move(alias);

    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b.size(), 5u);
}
