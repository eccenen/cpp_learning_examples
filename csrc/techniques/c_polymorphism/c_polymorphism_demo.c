/* c_polymorphism_demo.c - 演示如何在 C 中使用虚函数表实现多态
 *
 * 本示例展示如何在纯 C 语言中实现面向对象的概念，诸如继承和多态，
 * 通过以下方式实现：
 * 1. 结构体组合（用于模拟继承）
 * 2. 函数指针（模拟虚函数）
 * 3. 虚函数表（vtable 模式）
 */

#include "shape.h"

/* 多态函数 - 接受任意 Shape 并调用其虚函数 */
void process_shape(Shape* shape) {
    if (!shape) return;
    
    const char* type = Shape_get_type(shape);
    printf("\nProcessing %s:\n", type);
    
    /* 多态调用 - 根据实际类型表现不同 */
    Shape_draw(shape);
    
    /* 计算并显示面积 */
    double area = Shape_area(shape);
    printf("  Area: %.2f\n", area);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║  C Language Polymorphism Demo (VTable Pattern)   ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    /* 创建不同的 shape 实例（多态对象） */
    Shape* shapes[5];
    
    printf("=== Creating Shape Objects ===\n");
    shapes[0] = (Shape*)Circle_new(0.0, 0.0, 5.0);
    printf("Created Circle with radius 5.0\n");
    
    shapes[1] = (Shape*)Rectangle_new(10.0, 10.0, 4.0, 6.0);
    printf("Created Rectangle 4.0 x 6.0\n");
    
    shapes[2] = (Shape*)Triangle_new(0.0, 0.0, 4.0, 0.0, 2.0, 3.0);
    printf("Created Triangle\n");
    
    shapes[3] = (Shape*)Circle_new(5.0, 5.0, 3.0);
    printf("Created another Circle with radius 3.0\n");
    
    shapes[4] = (Shape*)Rectangle_new(0.0, 0.0, 10.0, 10.0);
    printf("Created another Rectangle 10.0 x 10.0\n");
    
    /* 演示多态 - 相同的函数可用于不同类型 */
    printf("\n=== Demonstrating Polymorphism ===");
    printf("\nCalling process_shape() on each object:\n");
    
    double total_area = 0.0;
    for (int i = 0; i < 5; i++) {
        process_shape(shapes[i]);
        total_area += Shape_area(shapes[i]);
    }
    
    printf("\n=== Summary ===\n");
    printf("Total area of all shapes: %.2f\n", total_area);
    
    /* 演示多态销毁（通过虚函数表调用析构） */
    printf("\n=== Cleaning Up (Polymorphic Destruction) ===\n");
    for (int i = 0; i < 5; i++) {
        Shape_destroy(shapes[i]);
    }
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║              Key Concepts Demonstrated:          ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║  1. Inheritance: Struct composition (base ptr)   ║\n");
    printf("║  2. Polymorphism: Function pointers in vtable    ║\n");
    printf("║  3. Encapsulation: Constructor/destructor funcs  ║\n");
    printf("║  4. Virtual functions: draw(), area(), etc.      ║\n");
    printf("║  5. Type safety: Cast base to derived safely     ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    
    printf("\n工作原理：\n");
    printf("  • 每个 '类'（Circle、Rectangle、Triangle）包含：\n");
    printf("    - 将 Shape 作为第一个成员以实现继承\n");
    printf("    - 独立的虚函数表（包含函数指针）\n");
    printf("    - 在构造时设置 vtable 指针\n");
    printf("  • Shape_xxx() 通过 vtable 分发调用\n");
    printf("  • 不同的 vtable 导致不同的行为（多态）\n");
    printf("  • 相同接口提供统一的 API（抽象）\n\n");
    
    return 0;
}
