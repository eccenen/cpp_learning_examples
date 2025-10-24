// spdlog_example.cpp
// 演示 spdlog 的各种使用场景：
// - 基本日志级别 (trace/debug/info/warn/error/critical)
// - 文件日志 (basic_file, rotating_file, daily_file)
// - 异步日志 (async logger)
// - 自定义格式
// - 多 sink (同时输出到控制台和文件)

#include "common.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

void demo_basic_logging() {
    fmt::print("\n=== 1. Basic Logging (Different Levels) ===\n");

    // 默认日志级别是 info，trace 和 debug 不会显示
    spdlog::trace("This is a TRACE message (won't show by default)");
    spdlog::debug("This is a DEBUG message (won't show by default)");
    spdlog::info("This is an INFO message");
    spdlog::warn("This is a WARN message");
    spdlog::error("This is an ERROR message");
    spdlog::critical("This is a CRITICAL message");

    // 修改全局日志级别
    fmt::print("\n--- Changing log level to trace ---\n");
    spdlog::set_level(spdlog::level::trace);
    spdlog::trace("Now TRACE is visible!");
    spdlog::debug("Now DEBUG is visible!");

    // 恢复默认级别
    spdlog::set_level(spdlog::level::info);
}

void demo_formatted_logging() {
    fmt::print("\n=== 2. Formatted Logging (Like fmt::print) ===\n");

    std::string name  = "Alice";
    int         age   = 30;
    double      score = 95.5;

    spdlog::info("User: {}, Age: {}, Score: {:.2f}", name, age, score);
    spdlog::warn("Processing {} items, found {} errors", 100, 5);

    // 自定义格式（添加时间戳、线程ID等）
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
    spdlog::info("Custom format with timestamp and thread ID");

    // 恢复默认格式
    spdlog::set_pattern("%+");
}

void demo_file_logging() {
    fmt::print("\n=== 3. File Logging ===\n");

    try {
        // 基本文件日志
        auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/basic.log");
        file_logger->info("This message goes to basic.log");
        file_logger->warn("File logging is simple!");

        // 滚动日志（超过 1MB 则创建新文件，保留最多 3 个文件）
        auto rotating_logger =
            spdlog::rotating_logger_mt("rotating_logger", "logs/rotating.log", 1024 * 1024, 3);
        rotating_logger->info("This goes to rotating log file");

        // 每日日志（每天生成新文件）
        auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.log", 2, 30);
        daily_logger->info("This goes to daily log file (created at 02:30)");

        fmt::print("Log files created in logs/ directory\n");
        fmt::print("  - basic.log: Simple file logging\n");
        fmt::print("  - rotating.log: Rotating file (max 1MB x 3 files)\n");
        fmt::print("  - daily.log: Daily log file\n");

    } catch (const spdlog::spdlog_ex & ex) {
        fmt::print("Log initialization failed: {}\n", ex.what());
    }
}

void demo_async_logging() {
    fmt::print("\n=== 4. Async Logging (High Performance) ===\n");

    try {
        // 初始化异步日志（线程池大小 8192，线程数 1）
        spdlog::init_thread_pool(8192, 1);

        auto async_file =
            spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async.log");

        // 模拟高频日志写入
        fmt::print("Writing 1000 async log messages...\n");
        for (int i = 0; i < 1000; ++i) {
            async_file->info("Async message #{}", i);
        }

        // 确保所有异步消息被写入
        async_file->flush();
        fmt::print("Async logging completed. Check logs/async.log\n");

    } catch (const spdlog::spdlog_ex & ex) {
        fmt::print("Async log initialization failed: {}\n", ex.what());
    }
}

void demo_multi_sink() {
    fmt::print("\n=== 5. Multi-Sink (Console + File) ===\n");

    try {
        // 创建多个 sink：控制台 + 文件
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multi_sink.log", true);
        file_sink->set_level(spdlog::level::trace);

        std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
        auto                          multi_logger =
            std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());

        // 注册为默认 logger
        spdlog::register_logger(multi_logger);
        spdlog::set_default_logger(multi_logger);

        // 这些日志会同时输出到控制台和文件
        spdlog::trace("TRACE: only in file (console level is info)");
        spdlog::debug("DEBUG: only in file (console level is info)");
        spdlog::info("INFO: visible in both console and file");
        spdlog::warn("WARN: visible in both console and file");

        fmt::print("Multi-sink logging: console + logs/multi_sink.log\n");

    } catch (const spdlog::spdlog_ex & ex) {
        fmt::print("Multi-sink initialization failed: {}\n", ex.what());
    }
}

void demo_logger_management() {
    fmt::print("\n=== 6. Logger Management ===\n");

    // 创建多个命名 logger
    auto network_logger  = spdlog::stdout_color_mt("network");
    auto database_logger = spdlog::stdout_color_mt("database");

    network_logger->info("Network connection established");
    database_logger->info("Database query executed");

    // 通过名字获取 logger
    auto net = spdlog::get("network");
    if (net) {
        net->warn("Network latency detected");
    }

    // 删除特定 logger
    spdlog::drop("network");
    fmt::print("Dropped 'network' logger\n");

    // 删除所有 logger
    spdlog::drop_all();
    fmt::print("All loggers dropped\n");
}

int main() {
    fmt::print("=== spdlog Comprehensive Demo ===\n");
    fmt::print("This demo shows various spdlog features\n");

    // 创建 logs 目录（如果不存在）
    system("mkdir -p logs");

    demo_basic_logging();
    demo_formatted_logging();
    demo_file_logging();
    demo_async_logging();
    demo_multi_sink();
    demo_logger_management();

    fmt::print("\n=== Demo Complete ===\n");
    fmt::print("Check the logs/ directory for generated log files\n");

    return 0;
}
