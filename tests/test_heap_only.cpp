
#include "common.hpp"
#include "heap_only.h"

TEST_CASE("create returns non-null and value is preserved") {
    auto p = HeapOnly::create(7);
    REQUIRE(p != nullptr);
    CHECK(p->getValue() == 7);
}

// Note: compile-time checks such as preventing stack allocation or copy can be
// demonstrated by attempting to compile the following lines; they are left as
// comments because they should not compile:
// HeapOnly s(1); // error: constructor is private
// HeapOnly copy = *p; // error: copy ctor deleted
