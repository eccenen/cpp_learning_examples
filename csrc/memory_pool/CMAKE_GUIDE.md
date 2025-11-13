# Memory Pool CMakeLists.txt 使用指南

## 概述

本项目的 CMakeLists.txt 已经重构为**自动发现源文件**的方式，不再需要手动为每个新增的源文件添加配置。

## 工作原理

### 1. 自动发现示例程序

CMakeLists.txt 会自动扫描以下位置的源文件：

- `example/base_*.cpp` - 基础示例程序
- `example/intermediate_*_example.cpp` - 中级示例程序  
- `example/advance_*_integration.cpp` - 高级示例程序

**命名规范：**
- 基础示例：`base_xxx.cpp` （例如：`base_smart_pointers.cpp`）
- 中级示例：`intermediate_xxx_example.cpp` （例如：`intermediate_fixed_block_pool_example.cpp`）
- 高级示例：`advance_xxx_integration.cpp` （例如：`advance_stl_integration.cpp`）

### 2. 自动发现基准测试

- `benchmarks/*.cpp` - 所有基准测试程序都会自动被构建

### 3. 自动发现单元测试

- `tests/*.cpp` - 所有单元测试程序（需要 doctest）

## 如何添加新的源文件

### 添加示例程序

只需按照命名规范创建新的 `.cpp` 文件，无需修改 CMakeLists.txt：

```bash
# 添加基础示例
touch example/base_new_feature.cpp

# 添加中级示例
touch example/intermediate_new_pool_example.cpp

# 添加高级示例
touch example/advance_new_integration.cpp
```

### 添加基准测试

```bash
touch benchmarks/new_benchmark.cpp
```

### 添加单元测试

```bash
touch tests/new_test.cpp
```

重新运行 `./compile.sh`，新文件会自动被发现并构建。

## 自动配置的内容

每个自动发现的可执行文件都会：

1. **自动包含路径**：
   - `${CMAKE_CURRENT_SOURCE_DIR}` - memory_pool 目录
   - `${CMAKE_SOURCE_DIR}/csrc` - 项目根源码目录

2. **自动链接库**：
   - `spdlog::spdlog` - 日志库

3. **自动添加到安装列表**

## 修改配置

如果需要修改所有可执行文件的公共配置，只需修改这两个变量：

```cmake
# 公共包含目录
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/csrc
)

# 公共链接库
set(COMMON_LINK_LIBS spdlog::spdlog)
```

## 特殊需求

如果某个特定的可执行文件需要特殊配置（例如额外的库），可以在自动发现循环后单独配置：

```cmake
# 在 foreach 循环后添加
if(TARGET special_executable)
    target_link_libraries(special_executable PRIVATE extra_library)
endif()
```

## 查看所有可执行文件

编译时会自动显示所有发现的可执行文件：

```
════════════════════════════════════════
  Memory Pool Module Configuration
════════════════════════════════════════
Executables:
  • advance_stl_integration
  • base_memory_leak_patterns
  • base_raw_pointer_lifecycle
  ...
════════════════════════════════════════
```

## 优势

1. **零维护**：添加新文件无需修改 CMakeLists.txt
2. **一致性**：所有可执行文件使用相同的编译配置
3. **可扩展**：易于修改全局配置
4. **清晰**：配置文件更简洁易读

## 注意事项

1. 源文件必须遵循命名规范才能被自动发现
2. 不符合规范的文件需要手动添加到 CMakeLists.txt
3. 如果需要排除某些文件，可以使用 `file(GLOB ... EXCLUDE ...)` 语法
