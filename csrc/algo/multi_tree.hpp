/**
 * 通用多叉树结构
 *
 * 用于构建和管理树形数据结构，支持任意类型的节点数据。
 * 特别适用于ONNX模型子图等场景。
 *
 * 功能特性：
 * 1. 模板化的树形结构，支持任意数据类型
 * 2. 使用unique_ptr实现高效的内存管理
 * 3. 支持范围for循环（层序遍历）
 * 4. 灵活的节点管理和查找功能
 * 5. 便捷的节点添加和删除接口
 * 6. 自动缓存管理和安全的内存释放
 */

#ifndef MULTI_TREE_HPP_
#define MULTI_TREE_HPP_

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace algo {

/**
 * 通用树节点结构
 * @tparam T 节点存储的数据类型
 */
template <typename T> class TreeNode {
  public:
    using node_ptr_t = std::unique_ptr<TreeNode<T>>;
    using data_ptr_t = std::unique_ptr<T>;

    // ========== 构造函数 ==========

    TreeNode()  = default;
    ~TreeNode() = default;

    explicit TreeNode(const std::string & node_name) : node_name_(node_name) {}

    TreeNode(const std::string & node_name, const std::unordered_set<std::string> & input_names) :
        node_name_(node_name),
        input_names_(input_names) {}

    TreeNode(const std::string & node_name, const std::unordered_set<std::string> & input_names,
             data_ptr_t data) :
        node_name_(node_name),
        input_names_(input_names),
        data_(std::move(data)) {}

    explicit TreeNode(data_ptr_t data) : data_(std::move(data)) {}

    // 禁用拷贝，仅允许移动
    TreeNode(const TreeNode &)                 = delete;
    TreeNode & operator=(const TreeNode &)     = delete;
    TreeNode(TreeNode &&) noexcept             = default;
    TreeNode & operator=(TreeNode &&) noexcept = default;

    // ========== Setters ==========

    void addInputName(const std::string & name) { input_names_.insert(name); }

    void setNodeName(const std::string & node_name) { node_name_ = node_name; }

    void setInputNames(const std::unordered_set<std::string> & input_names) {
        input_names_ = input_names;
    }

    void setData(data_ptr_t data) { data_ = std::move(data); }

    void setParent(TreeNode * parent) { parent_ = parent; }

    /**
     * 创建并添加新子节点
     */
    template <typename... Args> TreeNode * createChild(Args &&... args) {
        auto   child     = std::make_unique<TreeNode>(std::forward<Args>(args)...);
        auto * child_ptr = child.get();
        child_ptr->setParent(this);
        children_.push_back(std::move(child));
        return child_ptr;
    }

    /**
     * 添加已存在的子节点
     */
    TreeNode * addChild(node_ptr_t child) {
        if (!child) {
            throw std::invalid_argument("Child node pointer cannot be null");
        }
        auto * child_ptr = child.get();
        child_ptr->setParent(this);
        children_.push_back(std::move(child));
        return child_ptr;
    }

    /**
     * 移除指定的子节点
     */
    node_ptr_t removeChild(TreeNode * child) {
        auto it = std::find_if(children_.begin(), children_.end(),
                               [child](const node_ptr_t & node) { return node.get() == child; });

        if (it != children_.end()) {
            auto removed = std::move(*it);
            removed->setParent(nullptr);

            // 将被删除节点的子节点提升到当前节点下，保持树的连通性
            auto & grandchildren = removed->getChildren();
            for (auto & grandchild : grandchildren) {
                grandchild->setParent(this);
            }

            // 将孙子节点插入到被删除节点的位置
            children_.insert(children_.erase(it), std::make_move_iterator(grandchildren.begin()),
                             std::make_move_iterator(grandchildren.end()));

            // 清空被删除节点的子节点列表（所有权已转移）
            grandchildren.clear();

            return removed;
        }
        return nullptr;
    }

    /**
     * 根据索引移除子节点
     */
    node_ptr_t removeChildAt(size_t index) {
        if (index >= children_.size()) {
            throw std::out_of_range("Child index out of range");
        }
        auto removed = std::move(children_[index]);
        removed->setParent(nullptr);

        // 将被删除节点的子节点提升到当前节点下，保持树的连通性
        auto & grandchildren = removed->getChildren();
        for (auto & grandchild : grandchildren) {
            grandchild->setParent(this);
        }

        // 将孙子节点插入到被删除节点的位置
        auto insert_pos = children_.erase(children_.begin() + index);
        children_.insert(insert_pos, std::make_move_iterator(grandchildren.begin()),
                         std::make_move_iterator(grandchildren.end()));

        // 清空被删除节点的子节点列表（所有权已转移）
        grandchildren.clear();

        return removed;
    }

    /**
     * 清空所有子节点（后序删除，避免栈溢出）
     */
    void clearChildren() {
        // 后序遍历删除，确保子节点先被删除
        while (!children_.empty()) {
            children_.back()->clearChildren();
            children_.pop_back();
        }
    }

    // ========== Getters ==========

    const std::string & getNodeName() const { return node_name_; }

    const std::unordered_set<std::string> & getInputNames() const { return input_names_; }

    T * getData() { return data_.get(); }

    const T * getData() const { return data_.get(); }

    // 非const 版本，用于修改或移动子节点所有权
    std::vector<node_ptr_t> & getChildren() { return children_; }

    // const 版本，供只读访问使用
    const std::vector<node_ptr_t> & getChildren() const { return children_; }

    size_t getChildrenCount() const { return children_.size(); }

    TreeNode * getParent() { return parent_; }

    const TreeNode * getParent() const { return parent_; }

    TreeNode * getChildAt(size_t index) {
        if (index >= children_.size()) {
            throw std::out_of_range("Child index out of range");
        }
        return children_[index].get();
    }

    const TreeNode * getChildAt(size_t index) const {
        if (index >= children_.size()) {
            throw std::out_of_range("Child index out of range");
        }
        return children_[index].get();
    }

    bool isLeaf() const { return children_.empty(); }

    bool isRoot() const { return parent_ == nullptr; }

    bool hasData() const { return data_ != nullptr; }

    size_t getDepth() const {
        size_t           depth   = 0;
        const TreeNode * current = this;
        while (current->parent_ != nullptr) {
            ++depth;
            current = current->parent_;
        }
        return depth;
    }

    size_t getSubtreeSize() const {
        size_t count = 1;
        for (const auto & child : children_) {
            count += child->getSubtreeSize();
        }
        return count;
    }

    // ========== 树形打印 ==========

    /**
     * 打印以当前节点为根的子树
     * @param prefix 前缀字符串（用于缩进和连接线）
     * @param is_last 是否是父节点的最后一个子节点
     * @param show_details 是否显示详细信息（深度、子节点数等）
     */
    void printTree(const std::string & prefix = "", bool is_last = true,
                   bool show_details = false) const {
        // 打印当前节点的前缀和连接符
        std::cout << prefix;

        // 如果不是根节点，打印树形连接符
        if (parent_ != nullptr) {
            std::cout << (is_last ? "└── " : "├── ");
        }

        // 打印节点名称
        std::cout << node_name_;

        // 打印详细信息
        if (show_details) {
            std::cout << " [深度:" << getDepth() << ", 子节点:" << getChildrenCount();
            if (hasData()) {
                std::cout << ", 有数据";
            }
            if (!input_names_.empty()) {
                std::cout << ", 输入:" << input_names_.size();
            }
            std::cout << "]";
        }

        std::cout << std::endl;

        // 递归打印子节点
        for (size_t i = 0; i < children_.size(); ++i) {
            std::string new_prefix = prefix;
            // 如果当前节点不是根节点，需要添加缩进
            if (parent_ != nullptr) {
                new_prefix += (is_last ? "    " : "│   ");
            }
            bool is_last_child = (i == children_.size() - 1);
            children_[i]->printTree(new_prefix, is_last_child, show_details);
        }
    }

  private:
    std::string                     node_name_;
    std::unordered_set<std::string> input_names_;
    data_ptr_t                      data_{ nullptr };
    std::vector<node_ptr_t>         children_;
    TreeNode *                      parent_{ nullptr };
};

/**
 * 通用多叉树
 * @tparam T 节点存储的数据类型
 */
template <typename T> class MultiTree {
  public:
    using node_t     = TreeNode<T>;
    using node_ptr_t = std::unique_ptr<node_t>;
    using data_ptr_t = std::unique_ptr<T>;

    // ========== 层序遍历迭代器 ==========

    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = node_t *;
        using difference_type   = std::ptrdiff_t;
        using pointer           = node_t **;
        using reference         = node_t *&;

        Iterator() = default;

        explicit Iterator(node_t * root) {
            if (root) {
                queue_.push(root);
                advance();
            }
        }

        node_t * operator*() const { return current_; }

        node_t * operator->() const { return current_; }

        Iterator & operator++() {
            advance();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            advance();
            return tmp;
        }

        bool operator==(const Iterator & other) const { return current_ == other.current_; }

        bool operator!=(const Iterator & other) const { return !(*this == other); }

      private:
        void advance() {
            if (queue_.empty()) {
                current_ = nullptr;
                return;
            }

            current_ = queue_.front();
            queue_.pop();

            if (current_) {
                for (const auto & child : current_->getChildren()) {
                    queue_.push(child.get());
                }
            }
        }

        std::queue<node_t *> queue_;
        node_t *             current_{ nullptr };
    };

    class ConstIterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = const node_t *;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const node_t **;
        using reference         = const node_t *&;

        ConstIterator() = default;

        explicit ConstIterator(const node_t * root) {
            if (root) {
                queue_.push(root);
                advance();
            }
        }

        const node_t * operator*() const { return current_; }

        const node_t * operator->() const { return current_; }

        ConstIterator & operator++() {
            advance();
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            advance();
            return tmp;
        }

        bool operator==(const ConstIterator & other) const { return current_ == other.current_; }

        bool operator!=(const ConstIterator & other) const { return !(*this == other); }

      private:
        void advance() {
            if (queue_.empty()) {
                current_ = nullptr;
                return;
            }

            current_ = queue_.front();
            queue_.pop();

            if (current_) {
                for (const auto & child : current_->getChildren()) {
                    queue_.push(child.get());
                }
            }
        }

        std::queue<const node_t *> queue_;
        const node_t *             current_{ nullptr };
    };

    // ========== 构造函数 ==========

    MultiTree() = default;

    ~MultiTree() { clear(); }

    explicit MultiTree(const std::string & tree_name) : tree_name_(tree_name) {}

    // 禁用拷贝，仅允许移动
    MultiTree(const MultiTree &)                 = delete;
    MultiTree & operator=(const MultiTree &)     = delete;
    MultiTree(MultiTree &&) noexcept             = default;
    MultiTree & operator=(MultiTree &&) noexcept = default;

    // ========== 迭代器接口 ==========

    Iterator begin() { return Iterator(root_.get()); }

    Iterator end() { return Iterator(); }

    ConstIterator begin() const { return ConstIterator(root_.get()); }

    ConstIterator end() const { return ConstIterator(); }

    ConstIterator cbegin() const { return ConstIterator(root_.get()); }

    ConstIterator cend() const { return ConstIterator(); }

    // ========== 根节点操作 ==========

    void setRoot(node_ptr_t root) {
        root_ = std::move(root);
        invalidateCache();
    }

    template <typename... Args> node_t * createRoot(Args &&... args) {
        root_ = std::make_unique<node_t>(std::forward<Args>(args)...);
        invalidateCache();
        return root_.get();
    }

    node_t * getRoot() { return root_.get(); }

    const node_t * getRoot() const { return root_.get(); }

    node_ptr_t releaseRoot() {
        invalidateCache();
        return std::move(root_);
    }

    // ========== 树属性 ==========

    bool isEmpty() const { return root_ == nullptr; }

    size_t getNodeCount() const {
        if (isEmpty()) {
            return 0;
        }
        return root_->getSubtreeSize();
    }

    size_t getHeight() const {
        if (isEmpty()) {
            return 0;
        }
        return calculateHeight(root_.get());
    }

    /**
     * 安全清空树（后序删除，避免栈溢出，并释放所有内存）
     */
    void clear() {
        if (root_) {
            // 后序遍历删除，确保深层节点先被释放
            clearNodeRecursive(root_.get());
            root_.reset();
        }
        invalidateCache();
    }

    const std::string & getTreeName() const { return tree_name_; }

    void setTreeName(const std::string & name) { tree_name_ = name; }

    // ========== 节点查找（带缓存优化） ==========

    node_t * findNodeByName(const std::string & node_name) {
        if (isEmpty()) {
            return nullptr;
        }

        if (use_cache_) {
            buildCacheIfNeeded();
            auto it = name_cache_.find(node_name);
            return (it != name_cache_.end()) ? it->second : nullptr;
        }

        for (auto * node : *this) {
            if (node->getNodeName() == node_name) {
                return node;
            }
        }
        return nullptr;
    }

    const node_t * findNodeByName(const std::string & node_name) const {
        return const_cast<MultiTree *>(this)->findNodeByName(node_name);
    }

    node_t * findNodeByInputName(const std::string & input_name) {
        if (isEmpty()) {
            return nullptr;
        }

        for (auto * node : *this) {
            const auto & input_names = node->getInputNames();
            if (input_names.count(input_name) > 0) {
                return node;
            }
        }
        return nullptr;
    }

    node_t * findNodeByInputNames(const std::unordered_set<std::string> & input_names) {
        if (isEmpty() || input_names.empty()) {
            return nullptr;
        }

        for (auto * node : *this) {
            const auto & node_input_names = node->getInputNames();
            if (std::all_of(input_names.begin(), input_names.end(),
                            [&node_input_names](const std::string & name) {
                                return node_input_names.count(name) > 0;
                            })) {
                return node;
            }
        }
        return nullptr;
    }

    template <typename Predicate> node_t * findNodeIf(Predicate && pred) {
        for (auto * node : *this) {
            if (pred(node)) {
                return node;
            }
        }
        return nullptr;
    }

    template <typename Predicate> std::vector<node_t *> findAllNodesIf(Predicate && pred) {
        std::vector<node_t *> results;
        for (auto * node : *this) {
            if (pred(node)) {
                results.push_back(node);
            }
        }
        return results;
    }

    std::vector<node_t *> getPathToNode(node_t * target) {
        std::vector<node_t *> path;
        if (!target) {
            return path;
        }

        node_t * current = target;
        while (current) {
            path.push_back(current);
            current = current->getParent();
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    // ========== 便捷的节点添加接口 ==========

    template <typename... Args>
    node_t * createChildTo(const std::string & parent_name, Args &&... args) {
        auto * parent = findNodeByName(parent_name);
        if (!parent) {
            return nullptr;
        }
        invalidateCache();
        return parent->createChild(std::forward<Args>(args)...);
    }

    template <typename... Args> node_t * createChildTo(node_t * parent, Args &&... args) {
        if (!parent) {
            throw std::invalid_argument("Parent node cannot be null");
        }
        invalidateCache();
        return parent->createChild(std::forward<Args>(args)...);
    }

    /**
     * 链式添加子节点（简化构建）
     */
    template <typename... Args>
    MultiTree & addNode(const std::string & parent_name, Args &&... args) {
        createChildTo(parent_name, std::forward<Args>(args)...);
        return *this;
    }

    // ========== 遍历操作（仅层序遍历） ==========

    template <typename Visitor> void traverse(Visitor && visitor) {
        for (auto * node : *this) {
            visitor(node);
        }
    }

    template <typename Visitor> void traverse(Visitor && visitor) const {
        for (const auto * node : *this) {
            visitor(node);
        }
    }

    /**
     * 获取所有节点（层序）
     */
    std::vector<node_t *> getAllNodes() {
        std::vector<node_t *> nodes;
        for (auto * node : *this) {
            nodes.push_back(node);
        }
        return nodes;
    }

    /**
     * 分层遍历（按层返回节点）
     */
    std::vector<std::vector<node_t *>> getLevelOrder() {
        std::vector<std::vector<node_t *>> levels;
        if (isEmpty()) {
            return levels;
        }

        std::queue<node_t *> node_queue;
        node_queue.push(root_.get());

        while (!node_queue.empty()) {
            size_t                level_size = node_queue.size();
            std::vector<node_t *> current_level;

            for (size_t i = 0; i < level_size; ++i) {
                auto * node = node_queue.front();
                node_queue.pop();
                current_level.push_back(node);

                for (const auto & child : node->getChildren()) {
                    node_queue.push(child.get());
                }
            }
            levels.push_back(std::move(current_level));
        }
        return levels;
    }

    // ========== 树形打印 ==========

    /**
     * 打印整棵树的结构
     * @param show_details 是否显示详细信息（深度、子节点数等）
     */
    void printTree(bool show_details = false) const {
        if (isEmpty()) {
            std::cout << "(空树)" << std::endl;
            return;
        }

        if (!tree_name_.empty()) {
            std::cout << "树: " << tree_name_ << std::endl;
        }

        root_->printTree("", true, show_details);

        if (show_details) {
            std::cout << "\n统计信息:" << std::endl;
            std::cout << "  总节点数: " << getNodeCount() << std::endl;
            std::cout << "  树高度: " << getHeight() << std::endl;
        }
    }

    /**
     * 横向展开打印整棵树（根节点在顶部，子节点向下方两侧展开）
     * @param merge_duplicates 是否合并同名节点（默认true）
     */
    void printTreeHorizontal(bool merge_duplicates = true) const {
        if (isEmpty()) {
            std::cout << "(空树)" << std::endl;
            return;
        }

        if (!tree_name_.empty()) {
            std::cout << "树: " << tree_name_ << std::endl;
        }

        if (merge_duplicates) {
            printTreeHorizontalMerged();
        } else {
            printTreeHorizontalSimple();
        }
    }

  private:
    /**
     * 简单版本的横向打印（不合并同名节点）
     */
    void printTreeHorizontalSimple() const {
        // 计算树的宽度和高度
        size_t tree_height = getHeight();
        size_t tree_width = calculateTreeWidth(root_.get()) * 3; // 增加宽度以便更好地显示

        // 创建二维画布
        std::vector<std::vector<std::string>> canvas(tree_height * 2 - 1,
                                                     std::vector<std::string>(tree_width, " "));

        // 在画布上绘制树
        drawNodeHorizontal(canvas, root_.get(), 0, 0, tree_width);

        // 打印画布（移除尾部空格）
        for (const auto & row : canvas) {
            std::string line;
            for (const auto & cell : row) {
                line += cell;
            }
            while (!line.empty() && line.back() == ' ') {
                line.pop_back();
            }
            std::cout << line << std::endl;
        }
    }

    /**
     * 合并同名节点的横向打印
     */
    void printTreeHorizontalMerged() const {
        // 按层级收集节点，并去重同名节点
        std::map<size_t, std::map<std::string, std::vector<const node_t *>>> level_nodes;
        collectNodesByLevel(root_.get(), 0, level_nodes);

        // 创建简化的层级结构
        std::vector<std::vector<std::pair<std::string, std::vector<const node_t *>>>> levels;
        for (const auto & [level, nodes_map] : level_nodes) {
            std::vector<std::pair<std::string, std::vector<const node_t *>>> level_data;
            for (const auto & [name, node_list] : nodes_map) {
                level_data.push_back({ name, node_list });
            }
            levels.push_back(level_data);
        }

        // 计算总宽度
        size_t max_width = 0;
        for (const auto & level : levels) {
            size_t level_width = 0;
            for (const auto & node_info : level) {
                level_width += node_info.first.length() + 4;
            }
            max_width = std::max(max_width, level_width);
        }
        max_width = std::max(max_width, size_t(100));

        // 创建画布
        size_t canvas_height = levels.empty() ? 1 : levels.size() * 2 - 1;
        std::vector<std::vector<std::string>> canvas(canvas_height,
                                                     std::vector<std::string>(max_width, " "));

        // 绘制合并后的树
        drawMergedTree(canvas, levels, max_width);

        // 打印画布
        for (const auto & row : canvas) {
            std::string line;
            for (const auto & cell : row) {
                line += cell;
            }
            while (!line.empty() && line.back() == ' ') {
                line.pop_back();
            }
            std::cout << line << std::endl;
        }
    }

  public:
    // ========== 缓存控制 ==========

    void enableCache(bool enable = true) {
        use_cache_ = enable;
        if (!enable) {
            name_cache_.clear();
            cache_valid_ = false;
        }
    }

    void rebuildCache() {
        invalidateCache();
        buildCacheIfNeeded();
    }

  private:
    std::string tree_name_;
    node_ptr_t  root_{ nullptr };

    // 缓存相关
    bool                                      use_cache_{ true };
    bool                                      cache_valid_{ false };
    std::unordered_map<std::string, node_t *> name_cache_;

    size_t calculateHeight(const node_t * node) const {
        if (!node || node->isLeaf()) {
            return 1;
        }

        size_t max_height = 0;
        for (const auto & child : node->getChildren()) {
            max_height = std::max(max_height, calculateHeight(child.get()));
        }
        return max_height + 1;
    }

    /**
     * 递归清空节点（后序删除）
     */
    void clearNodeRecursive(node_t * node) {
        if (!node) {
            return;
        }

        // 先删除所有子节点
        for (const auto & child : node->getChildren()) {
            clearNodeRecursive(child.get());
        }

        // 清空子节点容器
        node->clearChildren();
    }

    void buildCacheIfNeeded() {
        if (cache_valid_) {
            return;
        }

        name_cache_.clear();
        if (!isEmpty()) {
            for (auto * node : *this) {
                const auto & name = node->getNodeName();
                if (!name.empty()) {
                    name_cache_[name] = node;
                }
            }
        }
        cache_valid_ = true;
    }

    void invalidateCache() { cache_valid_ = false; }

    /**
     * 递归收集每一层的节点，按名称去重
     */
    void collectNodesByLevel(
        const node_t * node, size_t level,
        std::map<size_t, std::map<std::string, std::vector<const node_t *>>> & level_nodes) const {
        if (!node) {
            return;
        }

        // 将节点添加到对应层级
        level_nodes[level][node->getNodeName()].push_back(node);

        // 递归处理子节点
        for (const auto & child : node->getChildren()) {
            collectNodesByLevel(child.get(), level + 1, level_nodes);
        }
    }

    /**
     * 绘制合并同名节点后的树
     */
    void drawMergedTree(
        std::vector<std::vector<std::string>> & canvas,
        const std::vector<std::vector<std::pair<std::string, std::vector<const node_t *>>>> &
               levels,
        size_t max_width) const {
        if (levels.empty()) {
            return;
        }

        // 为每层的节点分配位置
        std::vector<std::map<std::string, size_t>> level_positions(levels.size());

        for (size_t level_idx = 0; level_idx < levels.size(); ++level_idx) {
            const auto & level      = levels[level_idx];
            size_t       node_count = level.size();

            if (node_count == 0) {
                continue;
            }

            // 计算这一层所有节点名称的总长度
            size_t total_name_length = 0;
            for (const auto & [name, nodes] : level) {
                total_name_length += name.length();
            }

            // 计算可用于间距的空间
            size_t available_space =
                max_width > total_name_length ? max_width - total_name_length : max_width / 2;
            size_t spacing =
                node_count > 1 ? available_space / (node_count + 1) : available_space / 2;
            spacing = std::max(spacing, size_t(2)); // 最小间距

            // 分配位置
            size_t current_x = spacing;
            for (const auto & [name, nodes] : level) {
                size_t node_center               = current_x + name.length() / 2;
                level_positions[level_idx][name] = node_center;
                current_x += name.length() + spacing;
            }
        }

        // 绘制节点和连接线
        for (size_t level_idx = 0; level_idx < levels.size(); ++level_idx) {
            const auto & level = levels[level_idx];
            size_t       row   = level_idx * 2;

            // 绘制当前层的节点
            for (const auto & [name, nodes] : level) {
                size_t current_pos = level_positions[level_idx][name];
                size_t node_width  = name.length();
                size_t start_pos =
                    (current_pos > node_width / 2) ? current_pos - node_width / 2 : 0;

                // 绘制节点名称
                for (size_t i = 0; i < node_width && start_pos + i < canvas[0].size(); ++i) {
                    canvas[row][start_pos + i] = std::string(1, name[i]);
                }

                // 如果有下一层，绘制连接线
                if (level_idx + 1 < levels.size() && !nodes.empty() && row + 1 < canvas.size()) {
                    // 收集所有子节点的名称和位置
                    std::set<std::string> child_names;
                    std::vector<size_t>   child_positions;
                    std::set<std::string> processed_children;

                    for (const auto * node : nodes) {
                        for (const auto & child : node->getChildren()) {
                            std::string child_name = child->getNodeName();
                            if (processed_children.find(child_name) == processed_children.end()) {
                                child_names.insert(child_name);
                                child_positions.push_back(
                                    level_positions[level_idx + 1][child_name]);
                                processed_children.insert(child_name);
                            }
                        }
                    }

                    if (!child_positions.empty()) {
                        // 排序子节点位置
                        std::sort(child_positions.begin(), child_positions.end());

                        // 绘制连接线
                        if (child_positions.size() == 1) {
                            // 单个子节点：直线向下
                            size_t child_pos = child_positions[0];

                            // 从父节点到子节点画线
                            size_t from = std::min(current_pos, child_pos);
                            size_t to   = std::max(current_pos, child_pos);

                            if (from == to) {
                                // 垂直线
                                if (current_pos < canvas[0].size()) {
                                    canvas[row + 1][current_pos] = "│";
                                }
                            } else {
                                // 斜线连接
                                for (size_t i = from; i <= to && i < canvas[0].size(); ++i) {
                                    if (canvas[row + 1][i] == " ") {
                                        canvas[row + 1][i] = "─";
                                    }
                                }
                                if (from < canvas[0].size()) {
                                    canvas[row + 1][from] = (from == current_pos) ? "└" : "┌";
                                }
                                if (to < canvas[0].size()) {
                                    canvas[row + 1][to] = (to == current_pos) ? "┘" : "┐";
                                }
                            }
                        } else {
                            // 多个子节点
                            size_t leftmost  = child_positions.front();
                            size_t rightmost = child_positions.back();

                            // 绘制横线
                            for (size_t i = leftmost; i <= rightmost && i < canvas[0].size(); ++i) {
                                if (canvas[row + 1][i] == " ") {
                                    canvas[row + 1][i] = "─";
                                }
                            }

                            // 绘制分支点
                            for (size_t i = 0; i < child_positions.size(); ++i) {
                                size_t pos = child_positions[i];
                                if (pos >= canvas[0].size()) {
                                    continue;
                                }

                                if (i == 0) {
                                    canvas[row + 1][pos] = "┌";
                                } else if (i == child_positions.size() - 1) {
                                    canvas[row + 1][pos] = "┐";
                                } else {
                                    canvas[row + 1][pos] = "┬";
                                }
                            }

                            // 连接父节点到横线
                            if (current_pos >= leftmost && current_pos <= rightmost &&
                                current_pos < canvas[0].size()) {
                                canvas[row + 1][current_pos] = "┴";
                            } else if (current_pos < leftmost && current_pos < canvas[0].size()) {
                                // 父节点在左边
                                for (size_t i = current_pos; i < leftmost && i < canvas[0].size();
                                     ++i) {
                                    if (canvas[row + 1][i] == " ") {
                                        canvas[row + 1][i] = "─";
                                    }
                                }
                                if (current_pos < canvas[0].size()) {
                                    canvas[row + 1][current_pos] = "┘";
                                }
                                canvas[row + 1][leftmost] = "┌";
                            } else if (current_pos > rightmost && current_pos < canvas[0].size()) {
                                // 父节点在右边
                                for (size_t i = rightmost; i < current_pos && i < canvas[0].size();
                                     ++i) {
                                    if (canvas[row + 1][i] == " ") {
                                        canvas[row + 1][i] = "─";
                                    }
                                }
                                canvas[row + 1][rightmost] = "┐";
                                if (current_pos < canvas[0].size()) {
                                    canvas[row + 1][current_pos] = "└";
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * 计算以节点为根的子树在横向打印时需要的宽度
     */
    size_t calculateTreeWidth(const node_t * node) const {
        if (!node) {
            return 0;
        }

        // 节点名称的宽度
        size_t node_width = node->getNodeName().length();

        if (node->isLeaf()) {
            return node_width;
        }

        // 计算所有子节点的总宽度
        size_t children_width = 0;
        for (const auto & child : node->getChildren()) {
            children_width += calculateTreeWidth(child.get());
        }

        // 加上子节点之间的间距
        if (node->getChildrenCount() > 1) {
            children_width += (node->getChildrenCount() - 1) * 2;
        }

        return std::max(node_width, children_width);
    }

    /**
     * 在画布上绘制节点及其子树（横向展开版本）
     * @param canvas 二维字符画布
     * @param node 当前节点
     * @param row 当前行位置
     * @param left 左边界
     * @param right 右边界
     */
    void drawNodeHorizontal(std::vector<std::vector<std::string>> & canvas, const node_t * node,
                            size_t row, size_t left, size_t right) const {
        if (!node || row >= canvas.size() || left >= right) {
            return;
        }

        // 计算节点在当前区域的中心位置
        size_t      node_width = node->getNodeName().length();
        size_t      center     = (left + right) / 2;
        size_t      node_start = (center > node_width / 2) ? center - node_width / 2 : 0;
        std::string node_name  = node->getNodeName();

        // 在画布上绘制节点名称
        for (size_t i = 0; i < node_width && node_start + i < canvas[0].size(); ++i) {
            canvas[row][node_start + i] = std::string(1, node_name[i]);
        }

        // 如果有子节点，绘制连接线
        if (!node->isLeaf() && row + 2 < canvas.size()) {
            const auto & children    = node->getChildren();
            size_t       child_count = children.size();

            if (child_count == 0) {
                return;
            }

            // 计算每个子节点的位置
            std::vector<size_t> child_centers;
            size_t              available_width = right - left;
            size_t              spacing         = available_width / (child_count + 1);

            for (size_t i = 0; i < child_count; ++i) {
                size_t child_center = left + spacing * (i + 1);
                child_centers.push_back(child_center);
            }

            // 绘制从父节点到子节点的连接线
            if (child_count == 1) {
                // 单个子节点：直线向下
                if (center < canvas[0].size()) {
                    canvas[row + 1][center] = "│";
                }
            } else {
                // 多个子节点：绘制分叉结构
                size_t leftmost  = child_centers.front();
                size_t rightmost = child_centers.back();

                // 绘制横线
                for (size_t i = leftmost; i <= rightmost && i < canvas[0].size(); ++i) {
                    if (canvas[row + 1][i] == " ") {
                        canvas[row + 1][i] = "─";
                    }
                }

                // 绘制分支点
                for (size_t i = 0; i < child_count; ++i) {
                    size_t pos = child_centers[i];
                    if (pos >= canvas[0].size()) {
                        continue;
                    }

                    if (i == 0) {
                        canvas[row + 1][pos] = "┌";
                    } else if (i == child_count - 1) {
                        canvas[row + 1][pos] = "┐";
                    } else {
                        canvas[row + 1][pos] = "┬";
                    }
                }

                // 连接父节点到横线
                if (center >= leftmost && center <= rightmost && center < canvas[0].size()) {
                    canvas[row + 1][center] = "┴";
                } else if (center < leftmost && center < canvas[0].size()) {
                    // 父节点在左边，绘制斜线
                    for (size_t i = center; i <= leftmost && i < canvas[0].size(); ++i) {
                        if (canvas[row + 1][i] == " ") {
                            canvas[row + 1][i] = "─";
                        }
                    }
                    canvas[row + 1][center]   = "┘";
                    canvas[row + 1][leftmost] = "┌";
                } else if (center > rightmost && center < canvas[0].size()) {
                    // 父节点在右边
                    for (size_t i = rightmost; i <= center && i < canvas[0].size(); ++i) {
                        if (canvas[row + 1][i] == " ") {
                            canvas[row + 1][i] = "─";
                        }
                    }
                    canvas[row + 1][center]    = "└";
                    canvas[row + 1][rightmost] = "┐";
                }
            }

            // 递归绘制子节点
            for (size_t i = 0; i < child_count; ++i) {
                size_t child_left = (i == 0) ? left : (child_centers[i - 1] + child_centers[i]) / 2;
                size_t child_right =
                    (i == child_count - 1) ? right : (child_centers[i] + child_centers[i + 1]) / 2;

                drawNodeHorizontal(canvas, children[i].get(), row + 2, child_left, child_right);
            }
        }
    }
};

// ========== ONNX特化 ==========

#ifdef ONNX_INCLUDED
#    include <onnx/onnx_pb.h>

namespace onnx_utils {

class OnnxNodeData {
  public:
    OnnxNodeData() = default;

    explicit OnnxNodeData(onnx::NodeProto * node) : node_(node) {}

    onnx::NodeProto * getNode() const { return node_; }

    void setNode(onnx::NodeProto * node) { node_ = node; }

    bool isValid() const { return node_ != nullptr; }

    std::string getOpType() const { return node_ ? node_->op_type() : ""; }

    int getInputCount() const { return node_ ? node_->input_size() : 0; }

    int getOutputCount() const { return node_ ? node_->output_size() : 0; }

  private:
    onnx::NodeProto * node_{ nullptr };
};

using OnnxTree     = algo::MultiTree<OnnxNodeData>;
using OnnxTreeNode = algo::TreeNode<OnnxNodeData>;

/**
 * 便捷的ONNX子图构建器
 */
class OnnxSubgraphBuilder {
  public:
    explicit OnnxSubgraphBuilder(const std::string & name) : tree_(name) {}

    OnnxSubgraphBuilder & root(const std::string & node_name) {
        tree_.createRoot(node_name);
        return *this;
    }

    OnnxSubgraphBuilder & child(const std::string & parent_name, const std::string & child_name) {
        tree_.createChildTo(parent_name, child_name);
        return *this;
    }

    OnnxTree build() { return std::move(tree_); }

  private:
    OnnxTree tree_;
};

} // namespace onnx_utils
#endif

} // namespace algo

#endif // MULTI_TREE_HPP_
