#pragma once
#include <string_view>

#if defined(__SANITIZE_ADDRESS__)
inline constexpr std::string_view sanitizers = "ASan + UBSan";
#elif defined(__SANITIZE_THREAD__)
inline constexpr std::string_view sanitizers = "TSan + UBSan";
#else
inline constexpr std::string_view sanitizers = "none";
#endif

#ifdef NDEBUG
inline constexpr std::string_view asserts = "NDEBUG";
#else
inline constexpr std::string_view asserts = "asserts on";
#endif

#ifdef __OPTIMIZE__
inline constexpr std::string_view opt = "optimised";
#else
inline constexpr std::string_view opt = "-O0";
#endif
