# MultiTree 示例编译配置

cmake_minimum_required(VERSION 3.10)

# 添加示例可执行文件
add_executable(multi_tree_example multi_tree_example.cpp)

# 设置C++标准
target_compile_features(multi_tree_example PRIVATE cxx_std_17)

# 包含头文件目录
target_include_directories(multi_tree_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置输出目录
set_target_properties(multi_tree_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/algo"
)

# 安装目标（可选）
install(TARGETS multi_tree_example
    RUNTIME DESTINATION bin
)
