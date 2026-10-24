#include "buffer.hpp"
#include <algorithm>
#include <utility>

buffer::buffer(std::size_t size) : size_{size}, data_{new int[size]{}} {}

buffer::~buffer() {
    // safe to delete nullptr -- no guard
    delete[] data_;
}

buffer::buffer(const buffer& other)
    : size_{other.size()}, data_{new int[other.size()]} {
    std::copy(other.data_, other.data_ + other.size(), data_);
}

buffer& buffer::operator=(const buffer& other) {
    int* temp = new int[other.size()];
    std::copy(other.data_, other.data_ + other.size(), temp);
    delete[] data_;
    data_ = temp;
    size_ = other.size();
    return *this;
}

buffer::buffer(buffer&& other) noexcept
    : size_{std::exchange(other.size_, 0)},
      data_{std::exchange(other.data_, nullptr)} {}

buffer& buffer::operator=(buffer&& other) noexcept {
    int* new_data = std::exchange(other.data_, nullptr);
    std::size_t new_size = std::exchange(other.size_, 0);
    delete[] data_;
    data_ = new_data;
    size_ = new_size;
    return *this;
}

int& buffer::operator[](std::size_t i) noexcept { return data_[i]; }

const int& buffer::operator[](std::size_t i) const noexcept { return data_[i]; }

std::size_t buffer::size() const noexcept { return size_; }
