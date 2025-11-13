#include "../common/memory_pool_common.h"
#include "../common/visualizer.h"

using namespace memory_pool;

// ============================================================================
// 内存对齐演示
// ============================================================================

// 未对齐的结构
struct UnalignedStruct {
    char   c; // 1 byte
    int    i; // 4 bytes
    char   c2; // 1 byte
    double d; // 8 bytes
};

// 对齐的结构
struct AlignedStruct {
    double d; // 8 bytes
    int    i; // 4 bytes
    char   c; // 1 byte
    char   c2; // 1 byte
    // padding: 2 bytes
};

// 显式对齐
struct alignas(64) CacheLineAligned {
    int data[4];
};

void DemoStructAlignment() {
    spdlog::info("\n=== 1. 结构体对齐演示 ===\n");

    spdlog::info("UnalignedStruct:");
    spdlog::info("  - sizeof: {} bytes", sizeof(UnalignedStruct));
    spdlog::info("  - alignof: {} bytes", alignof(UnalignedStruct));

    spdlog::info("\nAlignedStruct:");
    spdlog::info("  - sizeof: {} bytes", sizeof(AlignedStruct));
    spdlog::info("  - alignof: {} bytes", alignof(AlignedStruct));

    spdlog::info("\nCacheLineAligned:");
    spdlog::info("  - sizeof: {} bytes", sizeof(CacheLineAligned));
    spdlog::info("  - alignof: {} bytes", alignof(CacheLineAligned));

    // 字段偏移
    spdlog::info("\nUnalignedStruct 字段偏移:");
    spdlog::info("  - c:  offset {}", offsetof(UnalignedStruct, c));
    spdlog::info("  - i:  offset {}", offsetof(UnalignedStruct, i));
    spdlog::info("  - c2: offset {}", offsetof(UnalignedStruct, c2));
    spdlog::info("  - d:  offset {}", offsetof(UnalignedStruct, d));
}

void DemoAlignedAllocation() {
    spdlog::info("\n=== 2. 对齐内存分配 ===\n");

    constexpr size_t size      = 256;
    constexpr size_t alignment = 64;

    // C++17 aligned_alloc (需要 size 是 alignment 的倍数)
    void * ptr1 = std::aligned_alloc(alignment, size);
    spdlog::info("aligned_alloc(64, 256):");
    spdlog::info("  - 地址: {}", ptr1);
    spdlog::info("  - 对齐检查: {}", isAligned(ptr1, alignment) ? "✓" : "✗");
    std::free(ptr1);

    // C11 aligned_alloc
    void * ptr2 = std::aligned_alloc(32, 128);
    spdlog::info("\naligned_alloc(32, 128):");
    spdlog::info("  - 地址: {}", ptr2);
    spdlog::info("  - 对齐检查: {}", isAligned(ptr2, 32) ? "✓" : "✗");
    std::free(ptr2);
}

void DemoAlignmentPerformance() {
    spdlog::info("\n=== 3. 对齐对性能的影响 ===\n");

    constexpr size_t iterations = 10000000;
    constexpr size_t array_size = 1024;

    // 未对齐的数组
    char *   unaligned_base = new char[array_size + 64];
    double * unaligned_arr  = reinterpret_cast<double *>(unaligned_base + 1); // 故意不对齐

    {
        Timer  timer;
        double sum = 0.0;
        for (size_t iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < array_size / sizeof(double); ++i) {
                unaligned_arr[i] = i * 1.5;
                sum += unaligned_arr[i];
            }
        }
        spdlog::info("未对齐访问: {:.2f} ms (sum={})", timer.elapsedMs(), sum);
    }

    // 对齐的数组
    double * aligned_arr = new (std::align_val_t{ 64 }) double[array_size / sizeof(double)];

    {
        Timer  timer;
        double sum = 0.0;
        for (size_t iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < array_size / sizeof(double); ++i) {
                aligned_arr[i] = i * 1.5;
                sum += aligned_arr[i];
            }
        }
        spdlog::info("对齐访问:   {:.2f} ms (sum={})", timer.elapsedUs(), sum);
    }

    delete[] unaligned_base;
    ::operator delete[](aligned_arr, std::align_val_t{ 64 });
}

void DemoCustomAlignedAllocator() {
    spdlog::info("\n=== 4. 自定义对齐分配器 ===\n");

    class AlignedAllocator {
      public:
        static void * allocate(size_t size, size_t alignment) {
            size       = alignUp(size, alignment);
            void * ptr = std::aligned_alloc(alignment, size);
            spdlog::info("分配 {} bytes (对齐 {}) @ {}", size, alignment, ptr);
            return ptr;
        }

        static void deallocate(void * ptr) {
            spdlog::info("释放 @ {}", ptr);
            std::free(ptr);
        }
    };

    // 分配不同对齐要求的内存
    void * ptr16 = AlignedAllocator::allocate(100, 16);
    void * ptr32 = AlignedAllocator::allocate(100, 32);
    void * ptr64 = AlignedAllocator::allocate(100, 64);

    spdlog::info("\n对齐检查:");
    spdlog::info("  16-byte: {}", isAligned(ptr16, 16) ? "✓" : "✗");
    spdlog::info("  32-byte: {}", isAligned(ptr32, 32) ? "✓" : "✗");
    spdlog::info("  64-byte: {}", isAligned(ptr64, 64) ? "✓" : "✗");

    AlignedAllocator::deallocate(ptr16);
    AlignedAllocator::deallocate(ptr32);
    AlignedAllocator::deallocate(ptr64);
}

void DemoSIMDAlignment() {
    spdlog::info("\n=== 5. SIMD 对齐要求 ===\n");

    // SSE 需要 16 字节对齐
    // AVX 需要 32 字节对齐
    // AVX-512 需要 64 字节对齐

    constexpr size_t vec_size = 16;

    alignas(16) float sse_data[vec_size];
    alignas(32) float avx_data[vec_size];
    alignas(64) float avx512_data[vec_size];

    spdlog::info("SIMD 数据对齐:");
    spdlog::info("  SSE   (16-byte): {} - {}", static_cast<void *>(sse_data),
                 isAligned(sse_data, 16) ? "✓" : "✗");
    spdlog::info("  AVX   (32-byte): {} - {}", static_cast<void *>(avx_data),
                 isAligned(avx_data, 32) ? "✓" : "✗");
    spdlog::info("  AVX512(64-byte): {} - {}", static_cast<void *>(avx512_data),
                 isAligned(avx512_data, 64) ? "✓" : "✗");
}

int main() {
    spdlog::set_pattern("[%^%l%$] %v");

    spdlog::info("╔════════════════════════════════════════════════════════╗");
    spdlog::info("║                内存对齐演示                        ║");
    spdlog::info("╚════════════════════════════════════════════════════════╝");

    DemoStructAlignment();
    DemoAlignedAllocation();
    DemoAlignmentPerformance();
    DemoCustomAlignedAllocator();
}
