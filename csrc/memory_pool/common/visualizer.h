// csrc/memory_pool/common/visualizer.h
#ifndef CPP_QA_LAB_MEMORY_POOL_VISUALIZER_H_
#define CPP_QA_LAB_MEMORY_POOL_VISUALIZER_H_

#include "memory_pool_common.h"

namespace memory_pool {

class Memoryvisualizer {
  public:
    // 可视化内存块
    static void visualizeMemoryBlock(void * ptr, size_t size, const std::string & label) {
        spdlog::info("\n┌─ {} ─────────────────────", label);
        spdlog::info("│ 地址: {}", ptr);
        spdlog::info("│ 大小: {} bytes", size);
        spdlog::info("│ 对齐: {}", isAligned(ptr, 8) ? "8字节对齐 ✓" : "未对齐 ✗");
        spdlog::info("└─────────────────────────────");
    }

    // 可视化内存池布局
    static void visualizePoolLayout(void * base, size_t block_size, size_t block_count) {
        spdlog::info("\n╔══════════════════════════════════════╗");
        spdlog::info("║       内存池布局可视化               ║");
        spdlog::info("╠══════════════════════════════════════╣");
        spdlog::info("║ 基址:     {}         ║", base);
        spdlog::info("║ 块大小:   {} bytes              ║", block_size);
        spdlog::info("║ 块数量:   {}                    ║", block_count);
        spdlog::info("║ 总大小:   {} bytes            ║", block_size * block_count);
        spdlog::info("╚══════════════════════════════════════╝");

        // ASCII 艺术图
        spdlog::info("\n内存布局:");
        for (size_t i = 0; i < std::min(block_count, size_t(10)); ++i) {
            void * block_addr = static_cast<char *>(base) + i * block_size;
            spdlog::info("[Block {}] @ {} ({} bytes)", i, block_addr, block_size);
        }
        if (block_count > 10) {
            spdlog::info("... ({} 个块)", block_count - 10);
        }
    }

    // 可视化空闲列表
    template <typename NodeType>
    static void visualizeFreeList(NodeType * head, const std::string & name) {
        spdlog::info("\n╔══════════════════════════════════════╗");
        spdlog::info("║  {} 空闲列表", name);
        spdlog::info("╠══════════════════════════════════════╣");

        int        count   = 0;
        NodeType * current = head;
        while (current && count < 10) {
            spdlog::info("║ [{}] @ {} → next: {}", count, static_cast<void *>(current),
                         static_cast<void *>(current->next));
            current = current->next;
            count++;
        }

        if (current) {
            spdlog::info("║ ... (更多节点)");
        }
        spdlog::info("╚══════════════════════════════════════╝");
    }
};

} // namespace memory_pool

#endif // CPP_QA_LAB_MEMORY_POOL_VISUALIZER_H_
