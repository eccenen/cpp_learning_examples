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

# 查找spdlog包
find_package(spdlog CONFIG REQUIRED)

# 公共头文件路径
set(MEMORY_POOL_COMMON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/common)

# 公共包含目录
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/csrc
)

# 公共链接库
set(COMMON_LINK_LIBS spdlog::spdlog)

# ============================================================================
# Core 库（把中级实现整理到静态库中，供示例和测试共用）
# ============================================================================
# 收集中级实现源文件（这些文件包含 FixedSizePool/FixedBlockPool/StackAllocator 等实现）
# 明确列出 core 实现源文件，便于控制和避免误包含示例驱动文件
set(CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/example/intermediate_fixed_size_pool.cpp"
    # 如需添加更多实现文件，请在此处追加路径，例如：
    # "${CMAKE_CURRENT_SOURCE_DIR}/example/intermediate_fixed_block_pool.cpp"
    # "${CMAKE_CURRENT_SOURCE_DIR}/example/advance_thread_safe_pool.cpp"
)

add_library(memory_pool_core STATIC ${CORE_SOURCES})
target_include_directories(memory_pool_core PUBLIC ${COMMON_INCLUDE_DIRS})
target_link_libraries(memory_pool_core PUBLIC ${COMMON_LINK_LIBS})
# 导出库名以便后续可执行文件链接
list(APPEND MEMORY_POOL_LIBS memory_pool_core)
if(NOT DEFINED MEMORY_POOL_RUNTIME_DIR)
    set(MEMORY_POOL_RUNTIME_DIR "${CMAKE_BINARY_DIR}/bin/memory_pool")
endif()

# 设置 core 库输出到同一目录下（静态库放 archive）
set_target_properties(memory_pool_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ============================================================================
# 辅助函数：为单个源文件创建可执行文件
# ============================================================================
function(add_memory_pool_executable source_file)
    # 获取不带扩展名的文件名
    get_filename_component(exec_name ${source_file} NAME_WE)
    
    # 创建可执行文件
    add_executable(${exec_name} ${source_file})
    
    # 设置包含目录
    target_include_directories(${exec_name} PRIVATE ${COMMON_INCLUDE_DIRS})
    
    # 链接库（公共库 + core 库）
    target_link_libraries(${exec_name} PRIVATE ${COMMON_LINK_LIBS})
    if(TARGET memory_pool_core)
        target_link_libraries(${exec_name} PRIVATE memory_pool_core)
    endif()
    # 将可执行文件统一输出到 build/bin/memory_pool
    set_target_properties(${exec_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${MEMORY_POOL_RUNTIME_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MEMORY_POOL_RUNTIME_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MEMORY_POOL_RUNTIME_DIR}"
    )
    
    # 添加到可执行文件列表
    list(APPEND MEMORY_POOL_EXECUTABLES ${exec_name})
    set(MEMORY_POOL_EXECUTABLES ${MEMORY_POOL_EXECUTABLES} PARENT_SCOPE)
endfunction()

# ============================================================================
# 自动发现并构建示例程序
# ============================================================================

# 收集example目录下所有的.cpp文件（排除头文件对应的实现）
file(GLOB EXAMPLE_SOURCES 
    "example/base_*.cpp"
    "example/intermediate_*_example.cpp"
    "example/advance_*_integration.cpp"
)

# 为每个示例源文件创建可执行文件
foreach(source_file ${EXAMPLE_SOURCES})
    add_memory_pool_executable(${source_file})
endforeach()

# ============================================================================
# 构建基准测试程序
# ============================================================================

file(GLOB BENCHMARK_SOURCES "benchmarks/*.cpp")
foreach(source_file ${BENCHMARK_SOURCES})
    add_memory_pool_executable(${source_file})
endforeach()

# ============================================================================
# 单元测试（需要doctest）
# ============================================================================

# 检查是否有doctest
find_package(doctest QUIET)

if(doctest_FOUND)
    file(GLOB TEST_SOURCES "tests/*.cpp")
    foreach(source_file ${TEST_SOURCES})
        get_filename_component(test_name ${source_file} NAME_WE)
        add_executable(${test_name} ${source_file})
        target_include_directories(${test_name} PRIVATE ${COMMON_INCLUDE_DIRS})
        target_link_libraries(${test_name} PRIVATE ${COMMON_LINK_LIBS} doctest::doctest)
        if(TARGET memory_pool_core)
            target_link_libraries(${test_name} PRIVATE memory_pool_core)
        endif()
        
        # 添加到测试列表
        list(APPEND MEMORY_POOL_EXECUTABLES ${test_name})
        
        # 添加测试
        enable_testing()
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
    
    message(STATUS "✓ Doctest found - unit tests enabled")
else()
    message(STATUS "⚠ Doctest not found - unit tests disabled")
    message(STATUS "  To enable: install doctest or use -Ddoctest_DIR=<path>")
endif()

# ============================================================================
# 安装目标
# ============================================================================

# 安装所有可执行文件（如果有的话）
if(MEMORY_POOL_EXECUTABLES)
    install(TARGETS ${MEMORY_POOL_EXECUTABLES}
        RUNTIME DESTINATION bin/memory_pool
    )
endif()

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
foreach(exec ${MEMORY_POOL_EXECUTABLES})
    message(STATUS "  • ${exec}")
endforeach()
message(STATUS "════════════════════════════════════════")
message(STATUS "")
