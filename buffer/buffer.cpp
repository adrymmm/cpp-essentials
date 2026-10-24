#include "buffer.hpp"
#include <algorithm>
#include <memory>
#include <utility>

buffer::buffer(std::size_t size)
    : size_{size}, data_{std::make_unique<int[]>(size)} {}

void buffer::swap(buffer& other) noexcept {
    std::swap(size_, other.size_);
    std::swap(data_, other.data_);
}

buffer::buffer(const buffer& other)
    : size_{other.size()},
      data_{std::make_unique_for_overwrite<int[]>(other.size())} {
    std::copy(other.data_.get(), other.data_.get() + other.size(), data_.get());
}

buffer& buffer::operator=(const buffer& other) {
    // idiomatic copy-and-swap
    buffer copy = other;
    copy.swap(*this);
    return *this;
}

buffer::buffer(buffer&& other) noexcept
    : size_{std::exchange(other.size_, 0)}, data_{std::move(other.data_)} {}

buffer& buffer::operator=(buffer&& other) noexcept {
    data_ = std::move(other.data_);
    size_ = std::exchange(other.size_, 0);
    return *this;
}

int& buffer::operator[](std::size_t i) noexcept { return data_[i]; }

const int& buffer::operator[](std::size_t i) const noexcept { return data_[i]; }

std::size_t buffer::size() const noexcept { return size_; }
