#include "common.h"

// 经典类型擦除 + 现代C++特性实现的模板方法接口
class Drawable {
  private:
    struct Concept {
        virtual ~Concept()                             = default;
        virtual void                     draw() const  = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template <typename T> struct Model : Concept {
        T object;

        Model(T obj) : object(std::move(obj)) {}

        void draw() const override { object.draw(); }

        std::unique_ptr<Concept> clone() const override { return std::make_unique<Model>(object); }
    };

    std::unique_ptr<Concept> pimpl;

  public:
    // 构造函数
    template <typename T> Drawable(T obj) : pimpl(std::make_unique<Model<T>>(std::move(obj))) {}

    // 拷贝构造
    Drawable(const Drawable & other) : pimpl(other.pimpl->clone()) {}

    // 拷贝赋值
    Drawable & operator=(const Drawable & other) {
        if (this != &other) {
            pimpl = other.pimpl->clone();
        }
        return *this;
    }

    // 移动构造
    Drawable(Drawable &&) = default;

    // 移动赋值
    Drawable & operator=(Drawable &&) = default;

    void draw() const { pimpl->draw(); }
};

// 方法2：小型对象优化 + 类型擦除
template <size_t BufferSize = 64, size_t Alignment = 8> class TypeErasedFunction {
  private:
    struct Concept {
        virtual ~Concept()                                          = default;
        virtual void                     call()                     = 0;
        virtual std::unique_ptr<Concept> clone(void * buffer) const = 0;
        virtual void                     move(void * buffer)        = 0;
    };

    template <typename T> struct Model : Concept {
        T object;

        Model(T obj) : object(std::move(obj)) {}

        void call() override { object(); }

        std::unique_ptr<Concept> clone(void * buffer) const override {
            if constexpr (sizeof(Model) <= BufferSize && alignof(Model) <= Alignment) {
                return std::unique_ptr<Concept>(new (buffer) Model(object));
            } else {
                return std::make_unique<Model>(object);
            }
        }

        void move(void * buffer) override {
            if constexpr (sizeof(Model) <= BufferSize && alignof(Model) <= Alignment) {
                new (buffer) Model(std::move(object));
            }
        }
    };

    alignas(Alignment) char buffer[BufferSize];
    Concept * concept = nullptr;

    void cleanup() {
        if (concept) {
            concept->~Concept();
            concept = nullptr;
        }
    }

  public:
    template <typename T> TypeErasedFunction(T && func) {
        using ModelType = Model<std::decay_t<T>>;

        if constexpr (sizeof(ModelType) <= BufferSize && alignof(ModelType) <= Alignment) {
            concept = new (buffer) ModelType(std::forward<T>(func));
        } else {
            concept = new ModelType(std::forward<T>(func));
        }
    }

    ~TypeErasedFunction() { cleanup(); }

    TypeErasedFunction(const TypeErasedFunction & other) {
        if (other.concept) {
            concept = other.concept->clone(buffer).release();
        }
    }

    TypeErasedFunction(TypeErasedFunction && other) noexcept {
        if (other.concept) {
            other.concept->move(buffer);
            concept       = other.concept;
            other.concept = nullptr;
        }
    }

    void operator()() { concept->call(); }
};

// 方法3：现代模板方法模式 + 类型擦除

class Algorithm {
  private:
    struct Concept {
        virtual ~Concept()                             = default;
        virtual void                     step1()       = 0;
        virtual void                     step2()       = 0;
        virtual void                     step3()       = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template <typename T> struct Model : Concept {
        T object;

        Model(T obj) : object(std::move(obj)) {}

        void step1() override { object.step1(); }

        void step2() override { object.step2(); }

        void step3() override { object.step3(); }

        std::unique_ptr<Concept> clone() const override { return std::make_unique<Model>(object); }
    };

    std::unique_ptr<Concept> pimpl;

  public:
    template <typename T> Algorithm(T obj) : pimpl(std::make_unique<Model<T>>(std::move(obj))) {}

    Algorithm(const Algorithm & other) : pimpl(other.pimpl->clone()) {}

    Algorithm & operator=(const Algorithm & other) {
        if (this != &other) {
            pimpl = other.pimpl->clone();
        }
        return *this;
    }

    Algorithm(Algorithm &&)             = default;
    Algorithm & operator=(Algorithm &&) = default;

    // 模板方法 - 定义算法骨架
    void execute() {
        fmt::print("=== Starting Algorithm ===\n");
        pimpl->step1();
        pimpl->step2();
        pimpl->step3();
        fmt::print("=== Algorithm Complete ===\n\n");
    }
};

// 方法4：多操作类型擦除 + 值语义
class Value {
  private:
    struct Concept {
        virtual ~Concept()                                                   = default;
        virtual std::string              toString() const                    = 0;
        virtual std::unique_ptr<Concept> clone() const                       = 0;
        virtual bool                     equals(const Concept * other) const = 0;
        virtual size_t                   hash() const                        = 0;
    };

    template <typename T> struct Model : Concept {
        T value;

        Model(T val) : value(std::move(val)) {}

        std::string toString() const override {
            if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(value);
            } else {
                return std::string(value);
            }
        }

        std::unique_ptr<Concept> clone() const override { return std::make_unique<Model>(value); }

        bool equals(const Concept * other) const override {
            if (auto * model = dynamic_cast<const Model *>(other)) {
                return value == model->value;
            }
            return false;
        }

        size_t hash() const override { return std::hash<T>{}(value); }
    };

    std::unique_ptr<Concept> pimpl;

  public:
    template <typename T> Value(T value) : pimpl(std::make_unique<Model<T>>(std::move(value))) {}

    Value(const Value & other) : pimpl(other.pimpl->clone()) {}

    Value & operator=(const Value & other) {
        if (this != &other) {
            pimpl = other.pimpl->clone();
        }
        return *this;
    }

    Value(Value &&)             = default;
    Value & operator=(Value &&) = default;

    std::string toString() const { return pimpl->toString(); }

    bool operator==(const Value & other) const { return pimpl->equals(other.pimpl.get()); }

    bool operator!=(const Value & other) const { return !(*this == other); }

    size_t hash() const { return pimpl->hash(); }
};

namespace std {
template <> struct hash<Value> {
    size_t operator()(const Value & val) const { return val.hash(); }
};
} // namespace std

// 具体实现
class FastAlgorithm {
  public:
    void step1() { fmt::print("Fast: Step 1\n"); }

    void step2() { fmt::print("Fast: Step 2\n"); }

    void step3() { fmt::print("Fast: Step 3\n"); }
};

class RobustAlgorithm {
  private:
    std::string name;
  public:
    RobustAlgorithm(std::string n) : name(std::move(n)) {}

    void step1() { fmt::print("{}: Robust Step 1\n", name); }

    void step2() { fmt::print("{}: Robust Step 2\n", name); }

    void step3() { fmt::print("{}: Robust Step 3\n", name); }
};

// 测试代码
int main() {
    fmt::print("=== Better Type Erasure Demo ===\n\n");

    // 1. 算法类型擦除
    std::vector<Algorithm> algorithms;
    algorithms.emplace_back(FastAlgorithm{});
    algorithms.emplace_back(RobustAlgorithm{ "Processor1" });
    algorithms.emplace_back(RobustAlgorithm{ "Processor2" });

    for (auto & algo : algorithms) {
        algo.execute();
    }

    // 2. 值类型擦除
    std::vector<Value> values;
    values.emplace_back(42);
    values.emplace_back(3.14);
    values.emplace_back("Hello World");
    values.emplace_back(std::string("C++"));

    for (const auto & val : values) {
        fmt::print("Value: {}\n", val.toString());
    }

    // 3. 函数类型擦除
    TypeErasedFunction<> func1 = []() {
        fmt::print("Lambda function\n");
    };
    TypeErasedFunction<> func2 = [x = 10]() {
        fmt::print("Captured x: {}\n", x);
    };

    func1();
    func2();

    return 0;
}
