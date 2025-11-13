# CMakeLists.txt for no_main_executable example
# 演示如何在 main.cpp 中不写 main 函数，但通过链接 main_provider.cpp 生成可执行文件

# 创建一个可执行文件，链接两个源文件：
# - main.cpp: 用户代码，不包含 main 函数
# - main_provider.cpp: 提供 main 函数入口
add_executable(techniques_no_main_executable
    main.cpp
    main_provider.cpp
)

# 链接公共依赖
target_link_libraries(techniques_no_main_executable PRIVATE csrc::common)

# 设置输出目录（与其他 techniques 示例保持一致）
set_target_properties(techniques_no_main_executable PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/techniques"
)

# 为不同配置设置输出目录
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config_upper)
    set_target_properties(techniques_no_main_executable PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/bin/techniques"
    )
endforeach()
