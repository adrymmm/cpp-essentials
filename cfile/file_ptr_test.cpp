#include "file_ptr.hpp"
#include "unique_fd.hpp"
#include <cerrno>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

static_assert(sizeof(file_ptr) == sizeof(std::FILE*));
static_assert(sizeof(std::unique_ptr<std::FILE, void (*)(std::FILE*)>) ==
              2 * sizeof(std::FILE*));

// instead of closing it counts how many times it called
struct file_closer_count {
    static inline int count = 0;

    void operator()(std::FILE*) const { ++count; }
};

TEST(CFile, ClosesOnScopeExit) {
    int fds[2];

    ASSERT_EQ(pipe(fds), 0);

    unique_fd w{fds[1]};
    {
        file_ptr p{fdopen(fds[0], "r")};
        ASSERT_NE(p, nullptr);
    }
    int status = fcntl(fds[0], F_GETFD);
    int err = errno;

    EXPECT_EQ(status, -1);
    EXPECT_EQ(err, EBADF);
}

TEST(CFile, NullNeverReachesDeleter) {
    file_closer_count::count = 0;
    { std::unique_ptr<std::FILE, file_closer_count> p{nullptr}; }
    EXPECT_EQ(file_closer_count::count, 0);
}
