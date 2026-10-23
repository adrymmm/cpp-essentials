#include "unique_fd.hpp"
#include <gtest/gtest.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <utility>
#include <type_traits>

// check unique_fd cant be copied at compile-time
static_assert(!std::is_copy_constructible_v<unique_fd>, "unique_fd should not be copyable");
static_assert(!std::is_copy_assignable_v<unique_fd>, "unique_fd should not be copy assignable");

TEST(UniqueFd, DestrClosesFd) {
    int fds[2];

    // check if pipe worked
    ASSERT_EQ(pipe(fds), 0);

    unique_fd first{fds[1]};
    {
        unique_fd second{fds[0]};
    }

    int status = fcntl(fds[0], F_GETFD);

    EXPECT_EQ(errno, EBADF);
    EXPECT_EQ(status, -1);
}

TEST(UniqueFd, MoveCtorTransfersFd) {
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    unique_fd trash{fds[1]};

    unique_fd a{fds[0]};
    unique_fd b{std::move(a)};
    
    EXPECT_EQ(b.get(), fds[0]);
    EXPECT_EQ(a.get(), -1);
}

TEST(UniqueFd, SelfMoveKeepsFd) {
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    unique_fd trash{fds[1]};

    unique_fd a{fds[0]};

    unique_fd& alias = a;
    alias = std::move(a);

    int status = fcntl(fds[0], F_GETFD);
    EXPECT_NE(status, -1);  // check fd is still open
    EXPECT_EQ(a.get(), fds[0]);
}
