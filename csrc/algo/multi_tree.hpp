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
#include <memory>
#include <queue>
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
            children_.erase(it);
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
        children_.erase(children_.begin() + index);
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
