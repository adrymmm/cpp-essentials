#pragma once
#include <cstddef>
#include <memory>

class buffer {
public:
    buffer() = default;
    explicit buffer(std::size_t size);

    buffer(const buffer& other);
    buffer& operator=(const buffer& other);

    buffer(buffer&& other) noexcept;
    buffer& operator=(buffer&& other) noexcept;

    void swap(buffer& other) noexcept;

    // i < size() required -- out of range is UB
    int& operator[](std::size_t i) noexcept;
    const int& operator[](std::size_t i) const noexcept;

    std::size_t size() const noexcept;

private:
    std::size_t size_ = 0;
    std::unique_ptr<int[]> data_;
};
