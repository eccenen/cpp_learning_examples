# spdlog 示例与对比

## 概述

本目录展示了 spdlog（高性能 C++ 日志库）的各种使用场景，并在 `basic/any_example.cpp` 中与 fmt::print 进行对比。

## spdlog vs fmt::print

### fmt::print
- **用途**：简单的控制台格式化输出
- **特点**：
  - 直接输出到 stdout/stderr
  - 无日志级别概念
  - 轻量、简洁、适合调试和简单输出
  - 使用 Python 风格的格式化字符串

### spdlog
- **用途**：结构化日志系统
- **特点**：
  - 多级别日志（trace/debug/info/warn/error/critical）
  - 支持多种输出目标（控制台、文件、滚动文件、每日文件等）
  - 异步日志（高性能场景）
  - 线程安全
  - 可运行时控制日志级别和格式
  - 内部使用 fmt 进行格式化

## 示例文件

### 1. `spdlog_example.cpp` - 综合示例

演示 spdlog 的各种特性：

1. **基本日志级别**
   ```cpp
   spdlog::trace("Detailed trace");
   spdlog::debug("Debug info");
   spdlog::info("General info");
   spdlog::warn("Warning");
   spdlog::error("Error occurred");
   spdlog::critical("Critical failure");
   ```

2. **格式化日志**（类似 fmt::print）
   ```cpp
   spdlog::info("User: {}, Age: {}, Score: {:.2f}", name, age, score);
   ```

3. **文件日志**
   - 基本文件日志：`basic_file_sink`
   - 滚动日志：`rotating_file_sink`（文件大小超限自动轮转）
   - 每日日志：`daily_file_sink`（每天创建新文件）

4. **异步日志**
   - 高性能场景下使用
   - 后台线程池处理日志写入
   - 不阻塞主线程

5. **多 Sink**
   - 同时输出到控制台和文件
   - 不同 sink 可设置不同的日志级别

6. **Logger 管理**
   - 创建多个命名 logger
   - 按名字获取和管理 logger

### 2. `basic/any_example.cpp` - 对比示例

在原有的 std::any 示例中，保留了 fmt::print，同时添加了 spdlog 调用，方便对比两者的使用方式和输出效果。

**运行输出示例：**
```
=== std::any Demo (with fmt::print and spdlog comparison) ===

[18:17:13] [info] === Starting std::any demonstration ===
a holds type: i                                      <-- fmt::print
[18:17:13] [info] a holds type: i                   <-- spdlog
```

## 构建和运行

```bash
# 构建
cd /path/to/cpp-qa-lab
./compile.sh

# 运行综合示例
./build/bin/spdlog_example/spdlog_example

# 运行对比示例
./build/bin/basic/basic_any_example

# 查看生成的日志文件
ls -lh logs/
cat logs/basic.log
cat logs/async.log
```

## 生成的日志文件

运行 `spdlog_example` 后会在 `logs/` 目录生成以下文件：

- `basic.log` - 基本文件日志
- `rotating.log` - 滚动日志文件
- `daily_YYYY-MM-DD.log` - 每日日志文件
- `async.log` - 异步日志文件（1000条消息）
- `multi_sink.log` - 多 sink 日志文件

## 使用建议

### 何时使用 fmt::print
- 简单的调试输出
- 临时查看变量值
- 小型示例和脚本
- 不需要日志级别控制的场景

### 何时使用 spdlog
- 生产环境的应用程序
- 需要日志分级（区分 debug/info/error）
- 需要日志持久化（写入文件）
- 需要日志轮转和归档
- 高并发/高性能场景（使用异步日志）
- 需要运行时控制日志行为

## 配置示例

### 设置全局日志级别
```cpp
spdlog::set_level(spdlog::level::debug);  // 显示 debug 及以上级别
```

### 自定义日志格式
```cpp
// 添加时间戳、日志级别、线程ID
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
```

### 创建文件 logger
```cpp
auto file_logger = spdlog::basic_logger_mt("my_logger", "logs/app.log");
file_logger->info("Application started");
```

### 创建异步 logger
```cpp
spdlog::init_thread_pool(8192, 1);
auto async_logger = spdlog::basic_logger_mt<spdlog::async_factory>(
    "async_logger", "logs/async.log");
```

## 与项目的集成

spdlog 已集成到 `csrc::common` 中，所有示例程序都可以直接使用：

```cpp
#include "common.h"
#include "spdlog/spdlog.h"  // 如果需要更多 spdlog 功能

int main() {
    spdlog::info("Hello from spdlog!");
    // ... your code
}
```

## 性能对比

| 特性 | fmt::print | spdlog (同步) | spdlog (异步) |
|------|-----------|--------------|---------------|
| 控制台输出 | ✓ | ✓ | ✓ |
| 文件输出 | ✗ | ✓ | ✓ |
| 日志级别 | ✗ | ✓ | ✓ |
| 线程安全 | ✗ | ✓ | ✓ |
| 性能 | 快 | 中等 | 极快 |
| 格式化 | ✓ (fmt) | ✓ (fmt) | ✓ (fmt) |
| 日志轮转 | ✗ | ✓ | ✓ |

## 参考资料

- [spdlog GitHub](https://github.com/gabime/spdlog)
- [spdlog 文档](https://github.com/gabime/spdlog/wiki)
- [fmt 格式化语法](https://fmt.dev/latest/syntax.html)
