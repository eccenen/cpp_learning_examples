// Copyright 2025 cpp-qa-lab
// Common header file for all csrc examples
//
// This header provides commonly used standard library and third-party includes
// to reduce repetitive includes across source files. Following Google C++20 style.

#ifndef CPP_QA_LAB_CSRC_COMMON_H_
#define CPP_QA_LAB_CSRC_COMMON_H_

// Third-party libraries
// fmt
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
// spdlog
#include <spdlog/spdlog.h>

// C++ Standard Library - Utilities
#include <any>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

// C++ Standard Library - Containers
#include <string>
#include <vector>

// C++ Standard Library - I/O (for legacy code only, prefer fmt)
#include <algorithm>

//
#include <iostream>

//
#include <memory>

// C Standard Library (commonly used)
#include <cstddef>
#include <cstdint>
#include <cstdio>

// Hint to include-what-you-use / clangd include-cleaner that this header
// is intentionally a central aggregator and should be kept when included.
// IWYU pragma: keep

// Provide a small, inline, maybe_unused symbol so static analysis sees a
// reference coming from this header. This is harmless and helps tools
// (clangd/include-cleaner) stop suggesting removal of the include.
[[maybe_unused]] inline constexpr bool kCppQaLabCsrcCommonHeaderIsUsed = true;

#endif // CPP_QA_LAB_CSRC_COMMON_H_
