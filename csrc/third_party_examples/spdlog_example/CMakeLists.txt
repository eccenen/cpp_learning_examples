# CMakeLists.txt for spdlog_example

add_executable(spdlog_example spdlog_example.cpp)

# 链接公共依赖（包含 fmt 和 spdlog）
target_link_libraries(spdlog_example PRIVATE csrc::common)

# 设置输出目录
set_target_properties(spdlog_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/third_party_examples"
)

# 为不同配置设置输出目录
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config_upper)
    set_target_properties(spdlog_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/bin/third_party_examples"
    )
endforeach()
