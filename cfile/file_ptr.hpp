#pragma once
#include <cstdio>
#include <memory>

struct file_closer {
    void operator()(std::FILE* f) const { std::fclose(f); }
};

using file_ptr = std::unique_ptr<std::FILE, file_closer>;
