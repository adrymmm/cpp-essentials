#pragma once

class unique_fd {
    public:
        unique_fd() = default;
        // explicit to guard against silent ownership
        explicit unique_fd(int fd) noexcept;
        ~unique_fd();

        // copy deleted
        unique_fd(const unique_fd& other) = delete;
        unique_fd& operator=(const unique_fd& other) = delete;
        
        // move is noexcept so containers move instead of copy when they grow
        unique_fd(unique_fd&& other) noexcept;

        /* self-move is safe if: store other's fd_ in tmp ->
         * leave other at -1 -> close own fd -> store taken fd*/
        unique_fd& operator=(unique_fd&& other) noexcept;
        
        // caller msut not close result
        int get() const noexcept;

    private:
        // open()'s error code -- owns nothing 
        int fd_ = -1;
};
