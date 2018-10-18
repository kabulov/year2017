#include <iostream>
#include <algorithm>
#include <random>
#include <limits>
#include <chrono>
#include <cassert>
#include <string>
#include <vector>
#include <iterator>
#include <initializer_list>

struct Generator {
    std::mt19937_64 generator;
    std::uniform_int_distribution<uint64_t> distribution;
    Generator() :
            generator(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
            distribution(0, std::numeric_limits<uint64_t>::max())
    {
    }
    uint64_t Toss(){
        return distribution(generator);
    }
};

template<typename ValueType>
struct TreeNode {
    ValueType key;
    uint64_t priority;
    TreeNode *left, *right;
    TreeNode *parent;

    TreeNode(ValueType new_key, uint64_t random_value) {
        key = new_key;
        priority = random_value;
        left = right = nullptr;
        parent = nullptr;
    }
};

template<typename ValueType>
class Set {
private:
    using TPtr = TreeNode<ValueType>*;

public:
    class iterator : public std::iterator<std::bidirectional_iterator_tag, ValueType> {
    public:
        iterator() : node_(nullptr), is_last(false) {}
        iterator(const iterator& other) : node_(other.node_), is_last(other.is_last) {}
        iterator(TPtr ptr) : node_(ptr), is_last(false) {}
        iterator(TPtr ptr, bool last) : node_(ptr), is_last(last) {}

        const ValueType& operator*() const {
            return node_->key;
        }

        const ValueType* operator->() const {
            return &(node_->key);
        }

        bool operator==(const iterator& other) {
            return is_last == other.is_last && node_ == other.node_;
        }

        bool operator!=(const iterator& other) {
            return !(operator==(other));
        }

        iterator& operator--() {
            if (is_last) {
                is_last = false;
            } else {
                node_ = Prev(node_);
            }
            return *this;
        }

        iterator operator--(int) {
            iterator other(node_, is_last);
            if (is_last) {
                is_last = false;
            } else {
                node_ = Prev(node_);
            }
            return other;
        }

        iterator& operator++() {
            TPtr new_node = Next(node_);
            if (new_node == nullptr) {
                is_last = true;
            } else {
                node_ = new_node;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator other(node_, is_last);
            TPtr new_node = Next(node_);
            if (new_node == nullptr) {
                is_last = true;
            } else {
                node_ = new_node;
            }
            return other;
        }
    private:
        TPtr node_;
        bool is_last;
    };

    Set() : root_(nullptr), generator_() {
    }

    ~Set() {
        clear(root_);
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) : root_(nullptr), generator_() {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    Set(std::initializer_list<ValueType> init_list) :
            Set(init_list.begin(), init_list.end())
    {
    }

    Set(const Set& other) : root_(nullptr), generator_() {
        my_deep_copy(other);
    }

    Set& operator=(const Set& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        my_deep_copy(other);
        return *this;
    }

    void insert(const ValueType& key) {
        Insert(root_, key);
    }

    void erase(const ValueType& key) {
        Erase(root_, key);
    }

    iterator find(const ValueType& key) const {
        TPtr found = Search(root_, key);
        if (found == nullptr) {
            return end();
        }
        return iterator(found);
    }

    iterator lower_bound(const ValueType& key) const {
        TPtr found = LowerBound(root_, key);
        if (found == nullptr) {
            return end();
        }
        return iterator(found);
    }

    bool empty() const {
        return root_ == nullptr;
    }

    size_t size() const {
        int sz = find_size(root_);
        return sz;
    }

    void clear() {
        clear(root_);
    }

    iterator begin() const {
        TPtr fst = First();
        if (fst == nullptr) {
            return end();
        }
        return iterator(fst);
    }

    iterator end() const {
        TPtr lst = Last();
        return iterator(lst, true);
    }

private:
    TPtr root_;
    Generator generator_;

    void Split(TPtr root, TPtr& left, TPtr& right, const ValueType& key) {
        if (root == nullptr) {
            left = right = nullptr;
            return;
        }
        if (root->key < key) {
            Split(root->right, root->right, right, key);
            left = root;
            if (right != nullptr) {
                right->parent = nullptr;
            }
            if (left->right != nullptr) {
                left->right->parent = left;
            }
        } else {
            Split(root->left, left, root->left, key);
            right = root;
            if (left != nullptr) {
                left->parent = nullptr;
            }
            if (right->left != nullptr) {
                right->left->parent = right;
            }
        }
    }

    void Merge(TPtr& root, TPtr left, TPtr right) {
        if (left == nullptr || right == nullptr) {
            root = right == nullptr ? left : right;
            return;
        }
        if (left->priority > right->priority) {
            Merge(left->right, left->right, right);
            root = left;
            if (root->right != nullptr) {
                root->right->parent = root;
            }
        } else {
            Merge(right->left, left, right->left);
            root = right;
            if (root->left != nullptr) {
                root->left->parent = root;
            }
        }
    }

    void Insert(TPtr& root, TPtr vertex, TPtr parent) {
        if (root == nullptr) {
            root = vertex;
            root->parent = parent;
            return;
        }
        if (root->priority > vertex->priority) {
            if (vertex->key < root->key) {
                Insert(root->left, vertex, root);
            } else {
                Insert(root->right, vertex, root);
            }
            return;
        }
        Split(root, vertex->left, vertex->right, vertex->key);
        root = vertex;
        root->parent = parent;
        if (root->left != nullptr) {
            root->left->parent = root;
        }
        if (root->right != nullptr) {
            root->right->parent = root;
        }
    }

    void Insert(TPtr& root, const ValueType& key) {
        if (Search(root, key) != nullptr) {
            return;
        }
        uint64_t priority = generator_.Toss();
        TPtr new_node = new TreeNode<ValueType>(key, priority);
        Insert(root, new_node, root == nullptr ? nullptr : root->parent);
    }

    void Erase(TPtr& root, const ValueType& key) {
        if (root == nullptr) {
            return;
        }
        if (!(key < root->key) && !(root->key < key)) {
            TPtr old_root = root;
            Merge(root, root->left, root->right);
            if (root != nullptr) {
                root->parent = old_root->parent;
            }
            delete old_root;
            old_root = nullptr;
        } else if (key < root->key) {
            Erase(root->left, key);
        } else {
            Erase(root->right, key);
        }
    }

    TPtr Search(TPtr root, const ValueType& key) const {
        if (root == nullptr) {
            return nullptr;
        }
        if (!(root->key < key) && !(key < root->key)) {
            return root;
        }
        if (key < root->key) {
            return Search(root->left, key);
        } else {
            return Search(root->right, key);
        }
    }

    void clear(TPtr& root) {
        if (root == nullptr) {
            return;
        }
        clear(root->left);
        clear(root->right);
        delete root;
        root = nullptr;
    }

    void my_deep_copy(const Set& other) {
        assert(root_ == nullptr);
        for (iterator it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
    }

    size_t find_size(TPtr root) const {
        if (root == nullptr) {
            return 0u;
        }
        size_t sz = 1;
        sz += find_size(root->left);
        sz += find_size(root->right);
        return sz;
    }

    TPtr LowerBound(TPtr root, const ValueType& key) const {
        if (root == nullptr) {
            return nullptr;
        }
        if (root->key < key) {
            return LowerBound(root->right, key);
        }
        TPtr found = LowerBound(root->left, key);
        if (found == nullptr) {
            return root;
        }
        return found;
    }

    TPtr AntiLowerBound(TPtr root, const ValueType& key) {
        if (root == nullptr) {
            return nullptr;
        }
        if (key < root->key) {
            return AntiLowerBound(root->left, key);
        }
        TPtr found = AntiLowerBound(root->right, key);
        if (found == nullptr) {
            return root;
        }
        return found;
    }

    static TPtr Next(TPtr node) {
        if (node->right != nullptr) {
            node = node->right;
            while (node->left != nullptr) {
                node = node->left;
            }
            return node;
        } else {
            while (node->parent != nullptr && !is_left_child(node->parent, node)) {
                node = node->parent;
            }
            if (node->parent == nullptr) {
                return nullptr;
            }
            return node->parent;
        }
    }

    static TPtr Prev(TPtr node) {
        if (node->left != nullptr) {
            node = node->left;
            while (node->right != nullptr) {
                node = node->right;
            }
            return node;
        } else {
            while (node->parent != nullptr && is_left_child(node->parent, node)) {
                node = node->parent;
            }
            return node->parent;
        }
    }

    static bool is_left_child(TPtr parent, TPtr child) {
        return parent->left == child;
    }

    TPtr First() const {
        if (root_ == nullptr) {
            return nullptr;
        }
        TPtr cur = root_;
        while (cur->left != nullptr) {
            cur = cur->left;
        }
        return cur;
    }

    TPtr Last() const {
        if (root_ == nullptr) {
            return nullptr;
        }
        TPtr cur = root_;
        while (cur->right != nullptr) {
            cur = cur->right;
        }
        return cur;
    }
};
