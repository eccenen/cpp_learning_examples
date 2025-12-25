/**
 * MultiTree 多叉树使用示例
 *
 * 本示例展示了如何使用优化后的多叉树数据结构
 */

#include <iostream>
#include <string>

#include "multi_tree.hpp"

// 简单的节点数据类型
struct SimpleNodeData {
    int         value_;
    std::string description_;

    SimpleNodeData(int value, const std::string & desc) : value_(value), description_(desc) {}
};

void printSeparator(const std::string & title) {
    std::cout << "\n========== " << title << " ==========\n";
}

/**
 * 示例1: 基本使用 - 创建简单的树结构
 */
void example1_basic_usage() {
    printSeparator("示例1: 基本使用");

    // 创建一个简单类型的树（仅存储节点名称）
    algo::MultiTree<SimpleNodeData> tree("示例树");

    // 创建根节点
    auto * root = tree.createRoot("root");
    std::cout << "创建根节点: " << root->getNodeName() << std::endl;

    // 在根节点下添加子节点
    auto * child1 = root->createChild("child1");
    auto * child2 = root->createChild("child2");
    auto * child3 = root->createChild("child3");

    // 在子节点下继续添加节点
    child1->createChild("child1_1");
    child1->createChild("child1_2");
    child2->createChild("child2_1");

    std::cout << "树节点总数: " << tree.getNodeCount() << std::endl;
    std::cout << "树高度: " << tree.getHeight() << std::endl;
}

/**
 * 示例2: 使用范围for循环遍历（层序）
 */
void example2_range_for_loop() {
    printSeparator("示例2: 范围for循环遍历");

    algo::MultiTree<SimpleNodeData> tree("遍历示例");
    auto *                          root = tree.createRoot("A");
    auto *                          b    = root->createChild("B");
    auto *                          c    = root->createChild("C");
    b->createChild("D");
    b->createChild("E");
    c->createChild("F");

    std::cout << "层序遍历结果: ";
    for (auto * node : tree) {
        std::cout << node->getNodeName() << " ";
    }
    std::cout << std::endl;
}

/**
 * 示例3: 带输入名称的节点（适用于ONNX等场景）
 */
void example3_nodes_with_inputs() {
    printSeparator("示例3: 带输入名称的节点");

    algo::MultiTree<SimpleNodeData> tree("计算图");

    // 创建根节点
    auto * root = tree.createRoot("mul");
    // 添加输入名称
    root->addInputName("input_tensor");
    root->addInputName("weight");

    // 或者直接创建带输入的节点
    std::unordered_set<std::string> add_inputs{ "mul_output", "bias" };
    auto *                          add_node = root->createChild("add", add_inputs);

    std::unordered_set<std::string> relu_inputs{ "add_output" };
    auto *                          relu_node = add_node->createChild("relu", relu_inputs);

    std::cout << "根节点: " << root->getNodeName() << std::endl;
    std::cout << "根节点输入数量: " << root->getInputNames().size() << std::endl;

    // 根据输入名称查找节点
    auto * found = tree.findNodeByInputName("bias");
    if (found) {
        std::cout << "找到包含输入'bias'的节点: " << found->getNodeName() << std::endl;
    }
}

/**
 * 示例4: 通过树接口添加节点
 */
void example4_tree_interface() {
    printSeparator("示例4: 通过树接口添加节点");

    algo::MultiTree<SimpleNodeData> tree("子图");
    tree.createRoot("root");

    // 通过父节点名称添加子节点（更方便）
    tree.createChildTo("root", "layer1");
    tree.createChildTo("root", "layer2");
    tree.createChildTo("layer1", "op1");
    tree.createChildTo("layer1", "op2");

    // 链式调用
    tree.addNode("layer2", "op3").addNode("layer2", "op4").addNode("op3", "final");

    std::cout << "树结构 (层序): ";
    for (auto * node : tree) {
        std::cout << node->getNodeName() << " ";
    }
    std::cout << std::endl;
}

/**
 * 示例5: 节点查找
 */
void example5_node_search() {
    printSeparator("示例5: 节点查找");

    algo::MultiTree<SimpleNodeData> tree("查找示例");
    auto *                          root = tree.createRoot("root");
    root->createChild("child1");
    auto * child2 = root->createChild("child2");
    child2->createChild("grandchild");

    // 按名称查找
    auto * found1 = tree.findNodeByName("grandchild");
    if (found1) {
        std::cout << "找到节点: " << found1->getNodeName() << ", 深度: " << found1->getDepth()
                  << std::endl;
    }

    // 使用谓词查找
    auto * found2 =
        tree.findNodeIf([](const auto * node) { return node->isLeaf() && node->getDepth() > 1; });
    if (found2) {
        std::cout << "找到叶子节点: " << found2->getNodeName() << std::endl;
    }

    // 查找所有叶子节点
    auto leaves = tree.findAllNodesIf([](const auto * node) { return node->isLeaf(); });
    std::cout << "所有叶子节点: ";
    for (auto * leaf : leaves) {
        std::cout << leaf->getNodeName() << " ";
    }
    std::cout << std::endl;
}

/**
 * 示例6: 节点路径和父节点访问
 */
void example6_path_and_parent() {
    printSeparator("示例6: 节点路径");

    algo::MultiTree<SimpleNodeData> tree("路径示例");
    auto *                          root = tree.createRoot("A");
    auto *                          b    = root->createChild("B");
    auto *                          c    = b->createChild("C");
    auto *                          d    = c->createChild("D");

    // 获取从根到D的路径
    auto path = tree.getPathToNode(d);
    std::cout << "从根到D的路径: ";
    for (auto * node : path) {
        std::cout << node->getNodeName();
        if (node != path.back()) {
            std::cout << " -> ";
        }
    }
    std::cout << std::endl;

    // 通过父节点指针向上访问
    std::cout << "从D向上访问: ";
    auto * current = d;
    while (current) {
        std::cout << current->getNodeName();
        current = current->getParent();
        if (current) {
            std::cout << " -> ";
        }
    }
    std::cout << std::endl;
}

/**
 * 示例7: 分层遍历
 */
void example7_level_order() {
    printSeparator("示例7: 分层遍历");

    algo::MultiTree<SimpleNodeData> tree("层次结构");
    auto *                          root = tree.createRoot("L0");
    auto *                          l1_1 = root->createChild("L1-1");
    auto *                          l1_2 = root->createChild("L1-2");
    l1_1->createChild("L2-1");
    l1_1->createChild("L2-2");
    l1_2->createChild("L2-3");

    auto levels = tree.getLevelOrder();
    for (size_t i = 0; i < levels.size(); ++i) {
        std::cout << "第" << i << "层: ";
        for (auto * node : levels[i]) {
            std::cout << node->getNodeName() << " ";
        }
        std::cout << std::endl;
    }
}

/**
 * 示例8: 节点数据
 */
void example8_node_data() {
    printSeparator("示例8: 节点数据");

    algo::MultiTree<SimpleNodeData> tree("数据树");

    // 创建带数据的节点
    auto * root = tree.createRoot("root");
    root->setData(std::make_unique<SimpleNodeData>(100, "根节点数据"));

    auto * child = root->createChild("child");
    child->setData(std::make_unique<SimpleNodeData>(200, "子节点数据"));

    // 访问节点数据
    for (auto * node : tree) {
        std::cout << "节点: " << node->getNodeName();
        if (node->hasData()) {
            auto * data = node->getData();
            std::cout << ", 值: " << data->value_ << ", 描述: " << data->description_;
        }
        std::cout << std::endl;
    }
}

/**
 * 示例9: 节点删除和修改
 */
void example9_modify_tree() {
    printSeparator("示例9: 修改树结构");

    algo::MultiTree<SimpleNodeData> tree("可修改树");
    auto *                          root = tree.createRoot("root");
    root->createChild("keep1");
    auto * to_remove = root->createChild("remove_me");
    root->createChild("keep2");

    std::cout << "删除前: ";
    for (auto * node : tree) {
        std::cout << node->getNodeName() << " ";
    }
    std::cout << std::endl;

    // 删除节点
    auto removed = root->removeChild(to_remove);

    std::cout << "删除后: ";
    for (auto * node : tree) {
        std::cout << node->getNodeName() << " ";
    }
    std::cout << std::endl;
}

/**
 * 示例10: 使用traverse方法
 */
void example10_traverse() {
    printSeparator("示例10: traverse方法");

    algo::MultiTree<SimpleNodeData> tree("遍历方法示例");
    auto *                          root = tree.createRoot("A");
    root->createChild("B");
    root->createChild("C");

    // 使用traverse方法（lambda）
    std::cout << "使用traverse: ";
    tree.traverse([](auto * node) { std::cout << node->getNodeName() << " "; });
    std::cout << std::endl;

    // 统计节点
    int count = 0;
    tree.traverse([&count](auto * node) { ++count; });
    std::cout << "节点总数: " << count << std::endl;
}

/**
 * 示例11: 性能 - 缓存使用
 */
void example11_cache() {
    printSeparator("示例11: 缓存功能");

    algo::MultiTree<SimpleNodeData> tree("缓存示例");
    auto *                          root = tree.createRoot("root");
    for (int i = 0; i < 10; ++i) {
        root->createChild("child_" + std::to_string(i));
    }

    // 第一次查找会构建缓存
    auto * found1 = tree.findNodeByName("child_5");
    std::cout << "第一次查找: " << found1->getNodeName() << std::endl;

    // 后续查找使用缓存，O(1)时间
    auto * found2 = tree.findNodeByName("child_8");
    std::cout << "第二次查找（使用缓存）: " << found2->getNodeName() << std::endl;

    // 可以手动控制缓存
    tree.enableCache(false); // 禁用缓存
    tree.enableCache(true); // 启用缓存
    tree.rebuildCache(); // 重建缓存
}

/**
 * 示例12: 内存安全 - clear操作
 */
void example12_memory_safety() {
    printSeparator("示例12: 内存安全");

    algo::MultiTree<SimpleNodeData> tree("内存示例");
    auto *                          root = tree.createRoot("root");

    // 创建深层结构
    auto * current = root;
    for (int i = 0; i < 5; ++i) {
        current = current->createChild("level_" + std::to_string(i));
    }

    std::cout << "清除前节点数: " << tree.getNodeCount() << std::endl;

    // clear()会安全地释放所有内存（后序删除）
    tree.clear();

    std::cout << "清除后节点数: " << tree.getNodeCount() << std::endl;
    std::cout << "树是否为空: " << (tree.isEmpty() ? "是" : "否") << std::endl;
}

/**
 * 示例13: 树形打印 - 可视化树的结构
 */
void example13_print_tree() {
    printSeparator("示例13: 树形打印");

    algo::MultiTree<SimpleNodeData> tree("组织架构");
    auto *                          root = tree.createRoot("CEO");

    // 构建一个组织架构树
    auto * cto = root->createChild("CTO");
    auto * cfo = root->createChild("CFO");
    auto * coo = root->createChild("COO");

    // CTO 下的团队
    auto * backend_team  = cto->createChild("后端团队");
    auto * frontend_team = cto->createChild("前端团队");
    auto * ai_team       = cto->createChild("AI团队");

    backend_team->createChild("服务器开发");
    backend_team->createChild("数据库管理");
    frontend_team->createChild("Web开发");
    frontend_team->createChild("移动开发");
    ai_team->createChild("算法研发");
    ai_team->createChild("数据标注");

    // CFO 下的团队
    cfo->createChild("财务部");
    cfo->createChild("审计部");

    // COO 下的团队
    auto * ops = coo->createChild("运营部");
    ops->createChild("市场营销");
    ops->createChild("客户服务");

    std::cout << "\n基本树形打印:\n";
    tree.printTree();

    std::cout << "\n\n带详细信息的树形打印:\n";
    tree.printTree(true);

    // 演示单个节点的打印
    std::cout << "\n\n只打印CTO部门的子树:\n";
    cto->printTree("", true, false);
}

/**
 * 示例14: 横向展开树形打印
 */
void example14_print_tree_horizontal() {
    printSeparator("示例14: 横向展开树形打印");

    // 示例1: 简单的二叉树
    algo::MultiTree<SimpleNodeData> tree1("简单示例");
    auto *                          root1 = tree1.createRoot("root");
    auto *                          c1    = root1->createChild("child1");
    auto *                          c2    = root1->createChild("child2");
    c1->createChild("gc1");
    c1->createChild("gc2");
    c2->createChild("gc3");

    std::cout << "\n示例1 - 简单二叉树:\n";
    tree1.printTreeHorizontal();

    // 示例2: 不对称树
    algo::MultiTree<SimpleNodeData> tree2("不对称树");
    auto *                          root2 = tree2.createRoot("A");
    auto *                          b     = root2->createChild("B");
    auto *                          c     = root2->createChild("C");
    auto *                          d     = root2->createChild("D");

    b->createChild("E");
    auto * f = b->createChild("F");
    f->createChild("G");
    f->createChild("H");

    c->createChild("I");

    std::cout << "\n\n示例2 - 不对称树:\n";
    tree2.printTreeHorizontal();

    // 示例3: 多子节点
    algo::MultiTree<SimpleNodeData> tree3("多叉树");
    auto *                          root3 = tree3.createRoot("根");
    root3->createChild("子1");
    root3->createChild("子2");
    root3->createChild("子3");
    root3->createChild("子4");
    root3->createChild("子5");

    std::cout << "\n\n示例3 - 多个子节点:\n";
    tree3.printTreeHorizontal();

    // 示例4: 单节点树
    algo::MultiTree<SimpleNodeData> tree4("单节点");
    tree4.createRoot("单独节点");

    std::cout << "\n\n示例4 - 单节点树:\n";
    tree4.printTreeHorizontal();

    // 示例5: 空树
    algo::MultiTree<SimpleNodeData> tree5("空树");
    std::cout << "\n\n示例5 - 空树:\n";
    tree5.printTreeHorizontal();
}

void example15_merge_nodes() {
    printSeparator("示例15: 合并同名节点的横向打印");

    // 示例1: 演示节点合并效果
    algo::MultiTree<SimpleNodeData> tree("节点合并示例");
    auto *                          root = tree.createRoot("root_node");

    auto * child_0 = root->createChild("child_0");
    auto * child_1 = root->createChild("child_1");
    auto * child_2 = root->createChild("child_2");

    auto * child_0_0 = child_0->createChild("child_0_0");
    auto * child_0_1 = child_0->createChild("child_0_1");

    child_1->createChild("child_1_0");
    child_2->createChild("child_2_0");

    // 创建共享子节点（两个父节点指向同名节点）
    child_0_0->createChild("child_0_0_0");
    child_0_1->createChild("child_0_0_0"); // 同名子节点

    std::cout << "\n垂直打印 (传统方式):\n";
    tree.printTree();

    std::cout << "\n\n横向打印 - 未合并同名节点:\n";
    tree.printTreeHorizontal(false); // 不合并

    std::cout << "\n\n横向打印 - 合并同名节点:\n";
    tree.printTreeHorizontal(true); // 合并同名节点

    std::cout << "\n说明:\n";
    std::cout << "  - 'child_0_0_0' 是 'child_0_0' 和 'child_0_1' 的共同子节点\n";
    std::cout << "  - 合并模式下，同名节点只显示一次，多个父节点的连线会汇聚到它\n";
    std::cout << "  - 这在显示有循环引用或共享节点的图结构时非常有用\n";
}

int main() {
    std::cout << "=========================================\n";
    std::cout << "   MultiTree 多叉树使用示例\n";
    std::cout << "=========================================\n";

    try {
        example1_basic_usage();
        example2_range_for_loop();
        example3_nodes_with_inputs();
        example4_tree_interface();
        example5_node_search();
        example6_path_and_parent();
        example7_level_order();
        example8_node_data();
        example9_modify_tree();
        example10_traverse();
        example11_cache();
        example12_memory_safety();
        example13_print_tree();
        example14_print_tree_horizontal();
        example15_merge_nodes();

        std::cout << "\n所有示例执行完成！\n";

    } catch (const std::exception & e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
