/**
 * @file
 * @brief Type Erasure（类型擦除）实现的模板方法示例。
 *
 * 说明：类型擦除通过将不同类型的具体实现包装为统一的运行时接口
 * （如 std::function 或自定义概念），在运行时提供多态性而无需继承。
 * 这对于需要将不同无公共基类的类型统一存储或传递的场景非常有用。
 *
 * 优点：
 * - 运行时多态：可以在容器中存储不同具体类型的对象并在运行时调
 *   用统一接口；
 * - 无需继承：具体类型无需实现共同基类，只需满足接口约定。
 *
 * 缺点：
 * - 性能开销：使用 std::function、间接调用及堆分配可能带来开销；
 * - 状态管理注意事项：捕获对象的方式（按值/按引用/使用 shared_ptr）
 *   会影响状态共享与生命周期，需谨慎设计。
 *
 * 建议：使用 shared_ptr 等方式确保被包装对象在所有回调中共享同一实
 * 例，以避免捕获独立副本导致的状态不一致或无限循环。
 */

#include "common.h"

// ==== 类型擦除包装 ====

// 1. 抽象基类
class ShapeConcept {
  public:
    virtual ~ShapeConcept()                             = default;
    virtual void                          draw() const  = 0;
    virtual double                        area() const  = 0;
    virtual std::unique_ptr<ShapeConcept> clone() const = 0;
};

// 2. 具体模型
template <typename T> class ShapeModel : public ShapeConcept {
  private:
    T shape_;

  public:
    ShapeModel(T shape) : shape_(std::move(shape)) {}

    void draw() const override {
        shape_.draw(); // 依赖静态多态
    }

    double area() const override {
        return shape_.area(); // 依赖静态多态
    }

    std::unique_ptr<ShapeConcept> clone() const override {
        return std::make_unique<ShapeModel>(shape_);
    }
};

// 3. 外部包装类
class Shape {
  private:
    std::unique_ptr<ShapeConcept> pimpl_;

  public:
    // 关键：模板构造函数
    template <typename T>
    Shape(T shape) : pimpl_(std::make_unique<ShapeModel<T>>(std::move(shape))) {}

    // 拷贝操作
    Shape(const Shape & other) : pimpl_(other.pimpl_->clone()) {}

    Shape & operator=(const Shape & other) {
        if (this != &other) {
            pimpl_ = other.pimpl_->clone();
        }
        return *this;
    }

    // 移动操作
    Shape(Shape &&)             = default;
    Shape & operator=(Shape &&) = default;

    // 统一接口
    void draw() const { pimpl_->draw(); }

    double area() const { return pimpl_->area(); }
};

// ==== 具体形状类型（彼此无关，没有共同基类） ====

class Circle {
  private:
    double radius_;

  public:
    Circle(double r) : radius_(r) {}

    void draw() const { fmt::print("Drawing Circle with radius {}\n", radius_); }

    double area() const { return 3.14159 * radius_ * radius_; }
};

class Rectangle {
  private:
    double width_, height_;

  public:
    Rectangle(double w, double h) : width_(w), height_(h) {}

    void draw() const { fmt::print("Drawing Rectangle {}x{}\n", width_, height_); }

    double area() const { return width_ * height_; }
};

// 甚至可以是来自第三方库的类，我们无法修改
struct LegacySquare {
    double side;

    LegacySquare(double s) : side(s) {}

    // 注意：方法名称不同！
    void render() const { fmt::print("Rendering Legacy Square with side {}\n", side); }

    double calculateArea() const { return side * side; }
};

// ==== 适配器模式：让LegacySquare适应我们的接口 ====

class SquareAdapter {
  private:
    LegacySquare square_;

  public:
    SquareAdapter(double side) : square_(side) {}

    SquareAdapter(LegacySquare sq) : square_(sq) {}

    void draw() const {
        square_.render(); // 适配不同的方法名
    }

    double area() const {
        return square_.calculateArea(); // 适配不同的方法名
    }
};

// ==== 使用示例 ====

int main() {
    std::vector<Shape> shapes;

    // 添加完全不同的类型到同一个容器中！
    shapes.emplace_back(Circle(5.0));
    shapes.emplace_back(Rectangle(4.0, 6.0));
    shapes.emplace_back(SquareAdapter(3.0)); // 使用适配器

    // 通过统一接口操作所有形状
    double total_area = 0;
    for (const auto & shape : shapes) {
        shape.draw();
        total_area += shape.area();
        fmt::print("Area: {}\n\n", shape.area());
    }

    fmt::print("Total area: {}\n", total_area);

    return 0;
}
