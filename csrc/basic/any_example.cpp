// any_example.cpp
// Demonstrates std::any usage: storing values, type-checking, any_cast, exceptions,
// pointer cast, moving and storing custom types.
// Uses fmt for formatted output (via vcpkg-installed fmt).
// Also demonstrates spdlog for comparison with fmt::print.

#include "common.h"

struct Point {
    int x;
    int y;
};

// 使用 fmt::print 演示 std::any 的各种用法
void demo_with_fmt() {
    fmt::print("\n=== Part 1: std::any Demo with fmt::print ===\n\n");

    // 1) Storing values of different types
    std::any a = 42; // holds int
    fmt::print("1. Type Storage:\n");
    fmt::print("   a holds type: {}\n", a.type().name());

    a = std::string("hello"); // now holds std::string
    fmt::print("   a now holds type: {}\n", a.type().name());

    // 2) any_cast with value - throws bad_any_cast on mismatch
    fmt::print("\n2. any_cast with value:\n");
    try {
        auto s = std::any_cast<std::string>(a);
        fmt::print("   Successfully cast to string: '{}'\n", s);
    } catch (const std::bad_any_cast & e) {
        fmt::print("   bad_any_cast: {}\n", e.what());
    }

    // 3) any_cast with pointer - returns nullptr on mismatch
    fmt::print("\n3. any_cast with pointer:\n");
    if (auto p = std::any_cast<std::string>(&a)) {
        fmt::print("   Pointer cast works, value: '{}'\n", *p);
    } else {
        fmt::print("   Pointer cast failed\n");
    }

    // 4) Moving values into std::any
    fmt::print("\n4. Moving values:\n");
    std::any b = std::string("temporary");
    std::any c = std::move(b); // b becomes empty
    fmt::print("   After move, b.has_value() = {}\n", b.has_value());
    if (c.has_value()) {
        fmt::print("   c holds: '{}'\n", std::any_cast<std::string>(c));
    }

    // 5) Storing custom types
    fmt::print("\n5. Custom types:\n");
    std::any d = Point{ 3, 4 };
    try {
        auto pt = std::any_cast<Point>(d);
        fmt::print("   Point: ({}, {})\n", pt.x, pt.y);
    } catch (const std::bad_any_cast & e) {
        fmt::print("   Failed to cast Point: {}\n", e.what());
    }

    // 6) Type checking
    fmt::print("\n6. Type checking:\n");
    if (d.type() == typeid(Point)) {
        fmt::print("   d contains Point type: true\n");
    }

    // 7) Clearing an any
    fmt::print("\n7. Clearing:\n");
    d.reset();
    fmt::print("   After reset, d.has_value() = {}\n", d.has_value());

    fmt::print("\n--- End of fmt::print demo ---\n");
}

// 使用 spdlog 演示 std::any 的各种用法（展示结构化日志）
void demo_with_spdlog() {
    fmt::print("\n=== Part 2: std::any Demo with spdlog ===\n\n");

    // 配置 spdlog：设置日志级别和格式
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");

    spdlog::info("=== Starting std::any demonstration with spdlog ===");

    // 1) Storing values of different types
    std::any a = 42; // holds int
    spdlog::info("1. Type Storage:");
    spdlog::info("   a holds type: {}", a.type().name());

    a = std::string("hello"); // now holds std::string
    spdlog::info("   a now holds type: {}", a.type().name());
    spdlog::debug("   (debug level message - not shown by default)");

    // 2) any_cast with value - throws bad_any_cast on mismatch
    spdlog::info("2. any_cast with value:");
    try {
        auto s = std::any_cast<std::string>(a);
        spdlog::info("   Successfully cast to string: '{}'", s);
    } catch (const std::bad_any_cast & e) {
        spdlog::error("   bad_any_cast exception: {}", e.what());
    }

    // 3) any_cast with pointer - returns nullptr on mismatch
    spdlog::info("3. any_cast with pointer:");
    if (auto p = std::any_cast<std::string>(&a)) {
        spdlog::info("   Pointer cast successful, value: '{}'", *p);
    } else {
        spdlog::warn("   Pointer cast returned nullptr");
    }

    // 4) Moving values into std::any
    spdlog::info("4. Moving values:");
    std::any b = std::string("temporary");
    std::any c = std::move(b); // b becomes empty
    spdlog::info("   After move, b.has_value() = {}", b.has_value());
    if (c.has_value()) {
        spdlog::info("   c holds: '{}'", std::any_cast<std::string>(c));
    }

    // 5) Storing custom types
    spdlog::info("5. Custom types:");
    std::any d = Point{ 3, 4 };
    try {
        auto pt = std::any_cast<Point>(d);
        spdlog::info("   Point coordinates: ({}, {})", pt.x, pt.y);
    } catch (const std::bad_any_cast & e) {
        spdlog::error("   Failed to cast Point: {}", e.what());
    }

    // 6) Type checking
    spdlog::info("6. Type checking:");
    if (d.type() == typeid(Point)) {
        spdlog::info("   Type verification: d contains Point type");
    }

    // 7) Clearing an any
    spdlog::info("7. Clearing:");
    d.reset();
    spdlog::info("   After reset, d.has_value() = {}", d.has_value());

    spdlog::info("=== std::any demonstration completed ===");
    fmt::print("\n--- End of spdlog demo ---\n");
}

int main() {
    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║         std::any Usage Demo - fmt vs spdlog Comparison         ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");

    // 第一部分：使用 fmt::print 演示
    demo_with_fmt();

    // 第二部分：使用 spdlog 演示
    demo_with_spdlog();

    // 对比说明
    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║                    Comparison Summary                          ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
    fmt::print("\n");
    fmt::print("fmt::print:\n");
    fmt::print("  ✓ Simple, direct console output\n");
    fmt::print("  ✓ No configuration needed\n");
    fmt::print("  ✓ Python-style format strings\n");
    fmt::print("  ✗ No log levels\n");
    fmt::print("  ✗ No file output\n");
    fmt::print("  ✗ No runtime control\n");
    fmt::print("\n");
    fmt::print("spdlog:\n");
    fmt::print("  ✓ Structured logging with levels (trace/debug/info/warn/error/critical)\n");
    fmt::print("  ✓ Multiple output targets (console, files, network, etc.)\n");
    fmt::print("  ✓ Runtime control of log levels and formats\n");
    fmt::print("  ✓ Thread-safe, supports async logging\n");
    fmt::print("  ✓ Built-in log rotation and archiving\n");
    fmt::print("  ✓ Uses fmt internally for formatting\n");
    fmt::print("\n");
    fmt::print("Use Case Recommendations:\n");
    fmt::print("  • fmt::print  → Quick debugging, simple scripts, educational examples\n");
    fmt::print("  • spdlog      → Production code, services, applications needing logs\n");
    fmt::print("\n");

    return 0;
}
