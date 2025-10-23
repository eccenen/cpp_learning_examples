// any_example.cpp
// Demonstrates std::any usage: storing values, type-checking, any_cast, exceptions,
// pointer cast, moving and storing custom types.
// Uses fmt for formatted output (via vcpkg-installed fmt).

#include <fmt/core.h>

#include <any>
#include <string>
#include <typeinfo>

struct Point {
    int x;
    int y;
};

int main() {
    // 1) Storing values of different types
    std::any a = 42; // holds int
    fmt::print("a holds type: {}\n", a.type().name());

    a = std::string("hello"); // now holds std::string
    fmt::print("a now holds type: {}\n", a.type().name());

    // 2) any_cast with value - throws bad_any_cast on mismatch
    try {
        auto s = std::any_cast<std::string>(a);
        fmt::print("string value: {}\n", s);
    } catch (const std::bad_any_cast & e) {
        fmt::print("bad_any_cast: {}\n", e.what());
    }

    // 3) any_cast with pointer - returns nullptr on mismatch
    if (auto p = std::any_cast<std::string>(&a)) {
        fmt::print("pointer any_cast works, value: {}\n", *p);
    } else {
        fmt::print("pointer any_cast failed\n");
    }

    // 4) Moving values into std::any
    std::any b = std::string("temporary");
    std::any c = std::move(b); // b becomes empty
    fmt::print("after move, b has value? {}\n", (!b.has_value() ? "no" : "yes"));
    if (c.has_value()) {
        fmt::print("c holds: {}\n", std::any_cast<std::string>(c));
    }

    // 5) Storing custom types
    std::any d = Point{ 3, 4 };
    try {
        auto pt = std::any_cast<Point>(d);
        fmt::print("Point: ({}, {})\n", pt.x, pt.y);
    } catch (const std::bad_any_cast & e) {
        fmt::print("bad_any_cast when casting Point: {}\n", e.what());
    }

    // 6) Type checking
    if (d.type() == typeid(Point)) {
        fmt::print("d contains Point type\n");
    }

    // 7) Clearing an any
    d.reset();
    fmt::print("d.has_value() after reset: {}\n", (d.has_value() ? "yes" : "no"));

    return 0;
}
