# 内存池模块 CMakeLists.txt

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# 初级示例
# ============================================================================

# 原始指针生命周期管理
add_executable(basic_raw_pointer_lifecycle
    beginner/raw_pointer_lifecycle.cpp
)

# 智能指针使用
add_executable(basic_smart_pointers
    beginner/smart_pointers.cpp
)

# ============================================================================
# 中级示例
# ============================================================================

# 固定块内存池
add_executable(intermediate_fixed_block_pool
    intermediate/fixed_block_pool_example.cpp
)
target_include_directories(intermediate_fixed_block_pool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 栈式分配器
add_executable(intermediate_stack_allocator
    intermediate/stack_allocator_example.cpp
)
target_include_directories(intermediate_stack_allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================================
# 高级示例
# ============================================================================

# STL分配器集成
add_executable(advanced_stl_integration
    advanced/stl_integration_example.cpp
)
target_include_directories(advanced_stl_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================================
# 性能基准测试
# ============================================================================

add_executable(memory_pool_benchmark
    benchmarks/performance_benchmark.cpp
)
target_include_directories(memory_pool_benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 公共头文件路径
set(MEMORY_POOL_COMMON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/common)

# ============================================================================
# 单元测试（需要doctest）
# ============================================================================

# 检查是否有doctest
find_package(doctest QUIET)

if(doctest_FOUND)
    add_executable(memory_pool_tests
        tests/memory_pool_tests.cpp
    )
    target_link_libraries(memory_pool_tests PRIVATE doctest::doctest)
    target_include_directories(memory_pool_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    # 添加测试
    enable_testing()
    add_test(NAME MemoryPoolTests COMMAND memory_pool_tests)
    
    message(STATUS "✓ Doctest found - unit tests enabled")
else()
    message(STATUS "⚠ Doctest not found - unit tests disabled")
    message(STATUS "  To enable: install doctest or use -Ddoctest_DIR=<path>")
endif()

# ============================================================================
# 安装目标
# ============================================================================

# 安装所有可执行文件
install(TARGETS
    basic_raw_pointer_lifecycle
    basic_smart_pointers
    intermediate_fixed_block_pool
    intermediate_stack_allocator
    advanced_stl_integration
    memory_pool_benchmark
    RUNTIME DESTINATION bin/memory_pool
)

# 安装头文件
install(DIRECTORY
    common/
    intermediate/
    advanced/
    DESTINATION include/memory_pool
    FILES_MATCHING PATTERN "*.h"
)

# ============================================================================
# 可选：Address Sanitizer支持
# ============================================================================

option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        message(STATUS "✓ Address Sanitizer enabled")
    else()
        message(WARNING "Address Sanitizer not supported on this compiler")
    endif()
endif()

# ============================================================================
# 输出信息
# ============================================================================

message(STATUS "")
message(STATUS "════════════════════════════════════════")
message(STATUS "  Memory Pool Module Configuration")
message(STATUS "════════════════════════════════════════")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "")
message(STATUS "Executables:")
message(STATUS "  • basic_raw_pointer_lifecycle")
message(STATUS "  • basic_smart_pointers")
message(STATUS "  • intermediate_fixed_block_pool")
message(STATUS "  • intermediate_stack_allocator")
message(STATUS "  • advanced_stl_integration")
message(STATUS "  • memory_pool_benchmark")
if(doctest_FOUND)
    message(STATUS "  • memory_pool_tests (with doctest)")
endif()
message(STATUS "════════════════════════════════════════")
message(STATUS "")
