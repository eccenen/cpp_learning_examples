# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
add_compile_options(-Wall -Wextra -O2 -g)

# 查找pthread库
find_package(Threads REQUIRED)

# 添加头文件目录（当前目录）
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 设置示例可执行文件输出目录到 build/bin/linux_program
set(PTHREAD_EXAMPLES_RUNTIME_DIR ${CMAKE_BINARY_DIR}/bin/linux_program)
set(PTHREAD_EXAMPLES_LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib/linux_program)

# 基础线程操作示例
add_executable(basic_thread_ops basic_thread_ops.cpp)
target_link_libraries(basic_thread_ops PRIVATE Threads::Threads)
set_target_properties(basic_thread_ops PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_RUNTIME_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}")

# 同步机制示例
add_executable(sync_mechanisms sync_mechanisms.cpp sync_primitives.cpp)
target_link_libraries(sync_mechanisms PRIVATE Threads::Threads)
set_target_properties(sync_mechanisms PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_RUNTIME_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}")

# 高级特性示例
add_executable(advanced_features advanced_features.cpp)
target_link_libraries(advanced_features PRIVATE Threads::Threads)
set_target_properties(advanced_features PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_RUNTIME_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}")

# 线程池实现
add_executable(thread_pool_demo thread_pool_demo.cpp thread_pool.cpp)
target_link_libraries(thread_pool_demo PRIVATE Threads::Threads)
set_target_properties(thread_pool_demo PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_RUNTIME_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}")

# 纯pthread使用示例（不使用C++标准库thread）
add_executable(pthread_comprehensive_example pthread_comprehensive_example.cpp)
target_link_libraries(pthread_comprehensive_example PRIVATE Threads::Threads)
set_target_properties(pthread_comprehensive_example PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_RUNTIME_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${PTHREAD_EXAMPLES_LIBRARY_DIR}")