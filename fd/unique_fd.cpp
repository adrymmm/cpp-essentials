#include "unique_fd.hpp"
#include <unistd.h>
#include <utility>

unique_fd::unique_fd(int fd) noexcept : fd_{fd} {}

unique_fd::~unique_fd() {
    // ignore on fail since fd is released anyway
    if (fd_ != -1) {
        ::close(fd_);
    }
}

unique_fd::unique_fd(unique_fd&& other) noexcept
    : fd_{std::exchange(other.fd_, -1)} {}

unique_fd& unique_fd::operator=(unique_fd&& other) noexcept {
    int tmp = std::exchange(other.fd_, -1);
    if (fd_ != -1)
        ::close(fd_);
    fd_ = tmp;
    return *this;
}

int unique_fd::get() const noexcept { return fd_; }
