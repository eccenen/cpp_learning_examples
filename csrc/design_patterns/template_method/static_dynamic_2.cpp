/**
 * @brief 策略模式+模板方法模式示例：混合静态与动态多态
 */

#include "common.h"

// === 动态多态接口 ===
class DataProcessor {
  public:
    virtual ~DataProcessor() = default;

    // 动态接口 - 用于运行时配置和管理
    virtual std::string name() const                              = 0;
    virtual void        setParameters(const std::string & params) = 0;
    virtual std::string getConfig() const                         = 0;

    // 统一的处理接口
    virtual void processData(const std::vector<double> & input, std::vector<double> & output) = 0;
};

// === 静态多态基础模板 ===
template <typename AlgorithmImpl> class OptimizedProcessor : public DataProcessor {
  protected:
    AlgorithmImpl algorithm_; // 具体的算法实现

  public:
    // 动态接口实现 - 委托给具体算法
    std::string name() const override { return algorithm_.getName(); }

    void setParameters(const std::string & params) override { algorithm_.setParams(params); }

    std::string getConfig() const override { return algorithm_.getConfig(); }

    // 关键：使用静态多态进行高性能处理
    void processData(const std::vector<double> & input, std::vector<double> & output) override {
        // 编译期优化的处理流程
        algorithm_.preProcess(input);
        algorithm_.compute(input, output);
        algorithm_.postProcess(output);
    }

    // 额外的模板方法 - 只在编译时可用
    template <typename Transformer>
    auto processWithTransform(const std::vector<double> & input, Transformer transform) {
        return algorithm_.template processTransformed<Transformer>(input, transform);
    }
};

// === 具体算法实现（静态多态）===

// 快速傅里叶变换处理器
class FFTAlgorithm {
  private:
    int         fftSize_    = 1024;
    std::string windowType_ = "hamming";

  private:
    auto computeDirect(const std::vector<double> & input) {
        return input; // 简化的直接计算
    }

  public:
    std::string getName() const { return "FFTProcessor"; }

    void setParams(const std::string & params) {
        // 解析参数...
        spdlog::info("FFT setting params: {}", params);
    }

    std::string getConfig() const {
        return "FFT Size: " + std::to_string(fftSize_) + ", Window: " + windowType_;
    }

    // 高性能实现 - 无虚函数开销
    void preProcess(const std::vector<double> & input) {
        // 预处理：数据验证、窗口函数应用等
        spdlog::info("FFT preprocessing {} samples", input.size());
    }

    void compute(const std::vector<double> & input, std::vector<double> & output) {
        // 实际的FFT计算 - 这里用模拟代替
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = input[i] * 2.0; // 模拟计算
        }
        spdlog::info("FFT computation completed");
    }

    void postProcess(std::vector<double> & output) {
        // 后处理：归一化、格式调整等
        spdlog::info("FFT postprocessing");
    }

    // 模板方法 - 编译期特化
    template <typename Transformer>
    auto processTransformed(const std::vector<double> & input, Transformer transform) {
        std::vector<double> temp = input;
        for (auto & val : temp) {
            val = transform(val);
        }
        return computeDirect(temp); // 内联优化
    }
};

// 滤波器处理器
class FilterAlgorithm {
  private:
    double      cutoffFreq_ = 1000.0;
    std::string filterType_ = "lowpass";

  public:
    std::string getName() const { return "FilterProcessor"; }

    void setParams(const std::string & params) {
        spdlog::info("Filter setting params: {}", params);
    }

    std::string getConfig() const {
        return "Cutoff: " + std::to_string(cutoffFreq_) + ", Type: " + filterType_;
    }

    void preProcess(const std::vector<double> & input) {
        spdlog::info("Filter preprocessing (input size: {})", input.size());
    }

    void compute(const std::vector<double> & input, std::vector<double> & output) {
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = input[i] * 0.5; // 模拟滤波
        }
        spdlog::info("Filter computation completed");
    }

    void postProcess(std::vector<double> & output) {
        spdlog::info("Filter postprocessing (output size: {})", output.size());
    }

    template <typename Transformer>
    auto processTransformed(const std::vector<double> & input, Transformer transform) {
        // 滤波器特定的变换处理
        return input; // 简化实现
    }
};

// 归一化处理器（简单示例）
class NormalizeAlgorithm {
  private:
    double scale_ = 1.0;

  public:
    std::string getName() const { return "NormalizeProcessor"; }

    void setParams(const std::string & params) {
        spdlog::info("Normalize setting params: {}", params);
    }

    std::string getConfig() const { return "Normalize scale: " + std::to_string(scale_); }

    void preProcess([[maybe_unused]] const std::vector<double> & input) {
        // 预处理：可以添加数据验证等
    }

    void compute(const std::vector<double> & input, std::vector<double> & output) {
        double maxv = 1.0;
        for (double v : input) {
            maxv = std::max(maxv, std::abs(v));
        }
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = input[i] / maxv;
        }
        spdlog::info("Normalize computation completed");
    }

    void postProcess([[maybe_unused]] std::vector<double> & output) {
        // 后处理：可以添加输出格式化等
    }

    template <typename Transformer>
    auto processTransformed(const std::vector<double> & input, Transformer transform) {
        std::vector<double> tmp = input;
        for (auto & v : tmp) {
            v = transform(v);
        }
        return tmp;
    }
};

// === 使用示例 ===
void mixed_polymorphism_example() {
    // 创建不同类型的处理器 - 统一通过动态接口管理
    std::vector<std::unique_ptr<DataProcessor>> processors;

    processors.push_back(std::make_unique<OptimizedProcessor<FFTAlgorithm>>());
    processors.push_back(std::make_unique<OptimizedProcessor<FilterAlgorithm>>());

    // 统一配置和管理（动态多态的优势）
    for (auto & processor : processors) {
        processor->setParameters("high_performance_mode=true");
        spdlog::info("Processor: {}, Config: {}", processor->name(), processor->getConfig());
    }

    // 处理数据 - 内部使用静态多态的高性能实现
    std::vector<double> inputData = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<double> outputData;

    for (auto & processor : processors) {
        processor->processData(inputData, outputData);
        // 使用 fmt::join 格式化输出
        spdlog::info("Output: {}", fmt::join(outputData, " "));
    }

    // 使用 processWithTransform：编译期的变换（仅在具体模板类可见）
    spdlog::info("=== processWithTransform demo (compile-time transformer) ===");
    OptimizedProcessor<FFTAlgorithm> fftProc;
    // 使用 lambda 作为 Transformer
    auto transformed = fftProc.processWithTransform(inputData, [](double v) { return v + 1.0; });
    spdlog::info("Transformed size: {}", transformed.size());

    // 性能对比：静态多态调用 vs. 动态多态调度
    spdlog::info("=== Performance micro-benchmark ===");
    const int           iters = 2000;
    std::vector<double> out;

    // 动态分派计时
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; ++i) {
        processors[0]->processData(inputData, out);
    }
    auto t1     = std::chrono::high_resolution_clock::now();
    auto dyn_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    // 静态多态计时（直接使用模板类）
    OptimizedProcessor<FFTAlgorithm> localFFT;
    auto                             t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; ++i) {
        localFFT.processData(inputData, out);
    }
    auto t3      = std::chrono::high_resolution_clock::now();
    auto stat_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    spdlog::info("Dynamic dispatch time (ms): {}", dyn_ms);
    spdlog::info("Static template time (ms): {}", stat_ms);

    // 简单流水线示例：FFT -> Normalize -> Filter
    spdlog::info("=== Simple pipeline: FFT -> Normalize -> Filter ===");
    OptimizedProcessor<NormalizeAlgorithm> normProc;
    OptimizedProcessor<FilterAlgorithm>    filterProc;

    std::vector<double> step1, step2, step3;
    localFFT.processData(inputData, step1);
    normProc.processData(step1, step2);
    filterProc.processData(step2, step3);
    // 使用 fmt::join 格式化输出
    spdlog::info("Pipeline output: {}", fmt::join(step3, " "));

    // 运行时识别（演示）
    spdlog::info("=== Runtime identification demo ===");
    if (auto * raw = processors[1].get()) {
        // 通过动态多态接口访问运行时类型信息
        spdlog::info("Runtime type (via name()): {}", raw->name());
    }
}

int main() {
    spdlog::info("=== Mixed Polymorphism Demo ===");
    mixed_polymorphism_example();
    return 0;
}
