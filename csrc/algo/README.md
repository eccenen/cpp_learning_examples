# MultiTree - 通用多叉树数据结构

一个高效、易用的C++多叉树模板库，支持任意类型的节点数据。

## 特性

✅ **模板化设计** - 支持任意数据类型  
✅ **高效内存管理** - 使用 `unique_ptr` 实现零开销抽象  
✅ **范围for循环** - 支持标准C++迭代器，可直接使用 `for (auto* node : tree)`  
✅ **智能缓存** - 节点查找自动缓存，O(1)时间复杂度  
✅ **安全释放** - 后序遍历删除，避免栈溢出  
✅ **丰富API** - 查找、遍历、修改等完整功能  

## 快速开始

### 基本使用

```cpp
#include "multi_tree.hpp"

// 创建树
algo::MultiTree<MyData> tree("我的树");

// 创建根节点
auto* root = tree.createRoot("root");

// 添加子节点
auto* child1 = root->createChild("child1");
auto* child2 = root->createChild("child2");

// 使用范围for循环遍历（层序）
for (auto* node : tree) {
    std::cout << node->getNodeName() << std::endl;
}
```

### 通过树接口添加节点

```cpp
algo::MultiTree<MyData> tree("子图");
tree.createRoot("root");

// 通过父节点名称添加
tree.createChildTo("root", "layer1");
tree.createChildTo("layer1", "op1");

// 链式调用
tree.addNode("root", "layer2")
    .addNode("layer2", "op2");
```

### 节点查找

```cpp
// 按名称查找
auto* node = tree.findNodeByName("layer1");

// 按输入名称查找
auto* found = tree.findNodeByInputName("input_tensor");

// 谓词查找
auto* leaf = tree.findNodeIf([](auto* n) {
    return n->isLeaf();
});

// 查找所有匹配节点
auto leaves = tree.findAllNodesIf([](auto* n) {
    return n->isLeaf();
});
```

### 带输入名称的节点

```cpp
// 创建节点时指定输入
std::unordered_set<std::string> inputs{"x", "weight"};
auto* conv = root->createChild("conv", inputs);

// 或者后续添加
conv->addInputName("bias");
```

### 节点路径

```cpp
// 获取从根到目标节点的路径
auto path = tree.getPathToNode(target_node);

// 通过父指针向上遍历
auto* current = node;
while (current) {
    // 处理节点
    current = current->getParent();
}
```

### 分层遍历

```cpp
auto levels = tree.getLevelOrder();
for (size_t i = 0; i < levels.size(); ++i) {
    std::cout << "第" << i << "层: ";
    for (auto* node : levels[i]) {
        std::cout << node->getNodeName() << " ";
    }
    std::cout << std::endl;
}
```

## 编译和运行

### 编译示例

```bash
cd /path/to/cpp-qa-lab
mkdir -p build && cd build
cmake ..
make multi_tree_example
```

### 运行示例

```bash
./build/bin/algo/multi_tree_example
```

## API 文档

### TreeNode 类

**构造函数：**
- `TreeNode(const std::string& node_name)`
- `TreeNode(const std::string& node_name, const std::unordered_set<std::string>& input_names)`
- `TreeNode(const std::string& node_name, const std::unordered_set<std::string>& input_names, data_ptr_t data)`

**主要方法：**
- `createChild(Args&&... args)` - 创建并添加子节点
- `addChild(node_ptr_t child)` - 添加已存在的子节点
- `removeChild(TreeNode* child)` - 移除指定子节点
- `getNodeName()` - 获取节点名称
- `getInputNames()` - 获取输入名称集合
- `getChildren()` - 获取子节点列表
- `getParent()` - 获取父节点
- `getDepth()` - 获取节点深度
- `isLeaf()` - 判断是否为叶子节点
- `isRoot()` - 判断是否为根节点

### MultiTree 类

**构造函数：**
- `MultiTree()`
- `MultiTree(const std::string& tree_name)`

**根节点操作：**
- `createRoot(Args&&... args)` - 创建根节点
- `getRoot()` - 获取根节点
- `setRoot(node_ptr_t root)` - 设置根节点

**查找方法：**
- `findNodeByName(const std::string& name)` - 按名称查找
- `findNodeByInputName(const std::string& input_name)` - 按输入名称查找
- `findNodeByInputNames(const std::unordered_set<std::string>& input_names)` - 按输入名称集合查找
- `findNodeIf(Predicate&& pred)` - 谓词查找
- `findAllNodesIf(Predicate&& pred)` - 查找所有匹配节点

**添加节点：**
- `createChildTo(const std::string& parent_name, Args&&... args)` - 向指定父节点添加子节点
- `addNode(const std::string& parent_name, Args&&... args)` - 链式添加节点

**遍历方法：**
- `begin() / end()` - 迭代器（范围for循环）
- `traverse(Visitor&& visitor)` - 层序遍历
- `getAllNodes()` - 获取所有节点
- `getLevelOrder()` - 分层遍历

**其他方法：**
- `isEmpty()` - 判断树是否为空
- `getNodeCount()` - 获取节点总数
- `getHeight()` - 获取树高度
- `clear()` - 清空树
- `enableCache(bool)` - 启用/禁用缓存
- `rebuildCache()` - 重建缓存

## 性能特点

- ✅ 节点查找：首次 O(n)，缓存后 O(1)
- ✅ 节点添加：O(1)
- ✅ 遍历：O(n)
- ✅ 内存管理：零开销（使用 `unique_ptr`）
- ✅ 迭代器：惰性求值，不预先构建列表

## 命名规范

本项目遵循以下命名规范：

- **函数名**：小驼峰（camelCase）- `createChild`, `findNodeByName`
- **类名**：大驼峰（PascalCase）- `MultiTree`, `TreeNode`
- **变量名**：下划线（snake_case）- `node_name_`, `input_names_`

## 注意事项

1. **移动语义**：树和节点使用 `unique_ptr`，仅支持移动，不支持拷贝
2. **缓存失效**：修改树结构（添加/删除节点）会自动使缓存失效
3. **内存安全**：`clear()` 使用后序删除，避免深层递归导致的栈溢出
4. **迭代器有效性**：修改树结构会使迭代器失效，需重新获取

## 应用场景

- ✅ ONNX模型子图构建
- ✅ AST（抽象语法树）
- ✅ 文件系统目录树
- ✅ 组织架构图
- ✅ 决策树
- ✅ 游戏场景图

## 许可证

本项目是 cpp-qa-lab 的一部分，遵循项目的整体许可证。
