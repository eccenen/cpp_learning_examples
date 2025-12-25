# MultiTree 快速使用指南

## 📁 文件说明

- **multi_tree.hpp** - 优化后的多叉树头文件（核心实现）
- **multi_tree_example.cpp** - 12个完整的使用示例
- **README.md** - 详细的API文档和使用说明
- **CMakeLists.txt** - 编译配置

## 🚀 5分钟快速上手

### 1. 最简单的例子

```cpp
#include "multi_tree.hpp"
#include <iostream>

int main() {
    // 创建树
    algo::MultiTree<int> tree("my_tree");
    
    // 创建根节点
    auto* root = tree.createRoot("root");
    
    // 添加子节点
    root->createChild("child1");
    root->createChild("child2");
    
    // 遍历（自动层序）
    for (auto* node : tree) {
        std::cout << node->getNodeName() << " ";
    }
    // 输出：root child1 child2
    
    return 0;
}
```

### 2. 编译运行

```bash
# 编译示例程序
cd /path/to/cpp-qa-lab/build
cmake ..
make multi_tree_example

# 运行
./bin/algo/multi_tree_example
```

### 3. 在你的项目中使用

```cpp
// 只需包含头文件即可
#include "csrc/algo/multi_tree.hpp"

// 定义你的数据类型
struct MyData {
    int id;
    std::string info;
};

// 创建树
algo::MultiTree<MyData> tree;
auto* root = tree.createRoot("root");

// 添加带数据的节点
auto* child = root->createChild("child");
child->setData(std::make_unique<MyData>(MyData{42, "hello"}));

// 访问数据
if (child->hasData()) {
    auto* data = child->getData();
    std::cout << data->id << ": " << data->info << std::endl;
}
```

## 🎯 核心功能速查

### 创建节点
```cpp
auto* root = tree.createRoot("root");           // 创建根
auto* child = root->createChild("child");       // 直接在节点上添加
auto* node = tree.createChildTo("root", "n");   // 通过树接口添加
```

### 遍历
```cpp
for (auto* node : tree) { /* ... */ }           // 范围for（最常用）
tree.traverse([](auto* n) { /* ... */ });       // lambda遍历
auto levels = tree.getLevelOrder();             // 分层访问
```

### 查找
```cpp
auto* n1 = tree.findNodeByName("target");       // 按名称
auto* n2 = tree.findNodeIf([](auto* n) {       // 自定义条件
    return n->isLeaf();
});
```

### 信息
```cpp
tree.getNodeCount();     // 节点总数
tree.getHeight();        // 树高度
node->getDepth();        // 节点深度
node->isLeaf();          // 是否叶子
node->getParent();       // 父节点
```

## 💡 12个示例说明

运行 `multi_tree_example` 可以看到：

1. ✅ 基本使用 - 创建、添加、遍历
2. ✅ 范围for循环 - 最自然的遍历方式
3. ✅ 带输入名称的节点 - 适用于ONNX场景
4. ✅ 树接口添加 - 更方便的构建方式
5. ✅ 节点查找 - 多种查找方法
6. ✅ 节点路径 - 获取从根到节点的路径
7. ✅ 分层遍历 - 按层访问节点
8. ✅ 节点数据 - 存储和访问自定义数据
9. ✅ 修改树结构 - 删除节点
10. ✅ traverse方法 - lambda遍历
11. ✅ 缓存功能 - 加速查找
12. ✅ 内存安全 - 安全释放

## 🔧 主要优化点

对比原始RoPENode实现，新版本的改进：

| 特性 | 原版本 | 新版本 |
|------|--------|--------|
| 命名 | RoPENode（特定） | MultiTree（通用） |
| 内存管理 | shared_ptr | unique_ptr（更高效） |
| 遍历方式 | 回调函数 | 迭代器 + 回调 |
| 添加节点 | 需要手动操作 | 多种便捷接口 |
| 查找性能 | O(n) | O(1)（缓存） |
| 内存释放 | 简单reset | 后序安全删除 |
| 父节点访问 | 无 | 支持向上遍历 |

## 📝 使用建议

1. **优先使用范围for循环** - 最简洁直观
2. **启用缓存** - 频繁查找时性能更好（默认开启）
3. **使用 `createChildTo`** - 通过名称添加更方便
4. **谨慎使用原始指针** - 节点所有权属于树，不要手动delete

## ⚠️ 注意事项

- 树使用移动语义，不支持拷贝
- 修改树结构会使迭代器失效
- `clear()` 会安全释放所有内存
- 节点指针在树销毁后失效

## 📚 更多信息

详细API文档请查看 `README.md`

## 🎓 适用场景

- ONNX模型子图构建 ⭐⭐⭐⭐⭐
- AST（抽象语法树） ⭐⭐⭐⭐⭐
- 文件系统树 ⭐⭐⭐⭐
- 组织架构 ⭐⭐⭐⭐
- 决策树 ⭐⭐⭐⭐

---

**开始使用**: 直接运行示例程序或查看 `multi_tree_example.cpp` 中的代码！
